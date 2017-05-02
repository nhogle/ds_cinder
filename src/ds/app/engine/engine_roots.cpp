#include "stdafx.h"

#include "ds/app/engine/engine_roots.h"

#include "ds/app/engine/engine.h"
#include "ds/app/auto_draw.h"

namespace ds {

/**
 * \class ds::EngineRoot
 */
ds::ui::Sprite* EngineRoot::make(ui::SpriteEngine& e, const ds::sprite_id_t id, const bool perspective)	{
	return new ds::ui::Sprite(e, id, perspective);
}

EngineRoot::EngineRoot(const RootList::Root& r, const sprite_id_t id)
		: mRootBuilder(r)
		, mSpriteId(id) {
}

EngineRoot::~EngineRoot() {
}

const RootList::Root& EngineRoot::getBuilder() const {
	return mRootBuilder;
}

/**
 * \class ds::OrthRoot
 */
OrthRoot::OrthRoot(Engine& e, const RootList::Root& r, const sprite_id_t id)
		: inherited(r, id)
		, mEngine(e)
		, mCameraDirty(false)
		, mSprite(EngineRoot::make(e, id, false))
		, mSrcRect(0.0f, 0.0f, -1.0f, -1.0f)
		, mDstRect(0.0f, 0.0f, -1.0f, -1.0f)
		, mNearPlane(-1.0f)
		, mFarPlane(1.0f)
{
}

void OrthRoot::setup(const Settings& s) {
	mSrcRect = s.mSrcRect;
	mDstRect = s.mDstRect;

	// Src rect and dst rect is the new way of managing screen size, offset and position.
	if (mSrcRect.x2 > mSrcRect.x1 && mSrcRect.y2 > mSrcRect.y1
			&& mDstRect.x2 > mDstRect.x1 && mDstRect.y2 > mDstRect.y1) {
		mSprite->setSize(mSrcRect.getWidth(), mSrcRect.getHeight());
	} 
}

void OrthRoot::postAppSetup() {
	mSprite->postAppSetup();
}

void OrthRoot::slaveTo(EngineRoot*) {
}

ds::ui::Sprite* OrthRoot::getSprite() {
	return mSprite.get();
}

void OrthRoot::clearChildren() {
	mSprite->clearChildren();
}

void OrthRoot::updateClient(const ds::UpdateParams& p) {
	mSprite->updateClient(p);
}

void OrthRoot::updateServer(const ds::UpdateParams& p) {
	mSprite->updateServer(p);
}

void OrthRoot::drawClient(const DrawParams& p, AutoDrawService* auto_draw) {
	if(mCameraDirty) {
		setCinderCamera();
	}
	setGlCamera();

	ci::gl::ScopedDepth depthScope( false );

	ci::mat4 m = ci::gl::getModelMatrix();
	mSprite->drawClient(m, p);

	if (auto_draw) auto_draw->drawClient(m, p);
}

void OrthRoot::drawServer(const DrawParams& p) {
	if(mCameraDirty) {
		setCinderCamera();
	}
	setGlCamera();

	ci::gl::ScopedDepth depthScope( false );

	ci::mat4 m = ci::gl::getModelMatrix();
	mSprite->drawServer(m, p);
}

ui::Sprite* OrthRoot::getHit(const ci::vec3& point) {
	return mSprite->getHit(point);
}

void OrthRoot::setCinderCamera() {
	mCameraDirty = false;

	if(getBuilder().mDrawScaled) {
		mCamera.setOrtho(mSrcRect.x1, mSrcRect.x2, mSrcRect.y2, mSrcRect.y1, mNearPlane, mFarPlane);
	}
	else {
		mCamera.setOrtho(0.0f, mDstRect.getWidth(), mDstRect.getHeight(), 0.0f, mNearPlane, mFarPlane);
	}
}

void OrthRoot::markCameraDirty() {
	mSrcRect = mEngine.getSrcRect();
	mCameraDirty = true;
}

void OrthRoot::setGlCamera() {
	ci::gl::setMatrices(mCamera);
}

/**
 * \class ds::PerspRoot
 */
PerspRoot::PerspRoot(Engine& e, const RootList::Root& r, const sprite_id_t id, const PerspCameraParams& p)
		: inherited(r, id)
		, mEngine(e)
		, mCameraDirty(false)
		, mSprite(EngineRoot::make(e, id, true))
		, mMaster(nullptr)
		, mPicking(mCamera)
		, mCameraParams(p)
{
}

void PerspRoot::setup(const Settings& s) {
	mSprite->setSize(s.mWorldSize.x, s.mWorldSize.y);
	mSprite->setDrawSorted(true);
}

void PerspRoot::postAppSetup() {
	mSprite->postAppSetup();
}

void PerspRoot::slaveTo(EngineRoot* r) {
	mMaster = dynamic_cast<PerspRoot*>(r);
	if (!mMaster) return;
}

ds::ui::Sprite* PerspRoot::getSprite() {
	return mSprite.get();
}

void PerspRoot::clearChildren() {
	mSprite->clearChildren();
}

void PerspRoot::updateClient(const ds::UpdateParams& p) {
	mSprite->updateClient(p);
}

void PerspRoot::updateServer(const ds::UpdateParams& p) {
	mSprite->updateServer(p);
}

void PerspRoot::drawClient(const DrawParams& p, AutoDrawService* auto_draw) {
	drawFunc([this, &p](){mSprite->drawClient(ci::gl::getModelMatrix(), p);});

	if (auto_draw) auto_draw->drawClient(ci::gl::getModelMatrix(), p);
}

void PerspRoot::drawServer(const DrawParams& p) {
	drawFunc([this, &p](){mSprite->drawServer(ci::gl::getModelMatrix(), p);});
}

ui::Sprite* PerspRoot::getHit(const ci::vec3& point) {
	ui::Sprite*		s = nullptr;

	// Note: Again, we are using the actual window size here, not
	// the Engine mDstRect size.
	auto screenSize = glm::vec2( ci::app::getWindowSize() );

	// Sigh... convert world coordinate back to screen coordinate
	const ci::vec2 srcOffset = mEngine.getSrcRect().getUpperLeft();
	const ci::vec2 screenScale = screenSize / mEngine.getSrcRect().getSize();
	const ci::vec2 screenPoint = (ci::vec2(point) - srcOffset) * screenScale;

	// We need to flip the y screen coordinate because OpenGL
	// Defines 0, 0 to be the lower left corner of the viewport
	const auto viewportPoint = glm::vec3(screenPoint.x, screenSize.y-screenPoint.y, 0.0f);

	// Compute the pick Ray.  We need to unproject the 
	// viewport point into camera-space coordinates
	const auto viewMat = mCamera.getViewMatrix();
	const auto projMat = mCamera.getProjectionMatrix();
	const auto viewport = glm::vec4(0.0f, 0.0f, screenSize);
	glm::vec3 worldPosNear = glm::unProject(viewportPoint, viewMat, projMat, viewport);
	glm::vec3 rayDirection = glm::normalize(worldPosNear - mCamera.getEyePoint());

	ci::Ray pickRay(mCamera.getEyePoint(), rayDirection);
	auto camDirection = glm::normalize( mCamera.getViewDirection() );

	drawFunc([this, pickRay, camDirection, &s](){s = mPicking.pickAt(pickRay, camDirection, *(mSprite.get())); });
	return s;
}

PerspCameraParams PerspRoot::getCamera() const {
	if (mMaster) return mMaster->getCamera();

	return mCameraParams;
}

const ci::CameraPersp& PerspRoot::getCameraRef() const {
	if (mMaster) return mMaster->getCameraRef();

	return mCamera;
}

void PerspRoot::setCameraRef(const ci::CameraPersp& cam){
	mCamera = cam;
}

void PerspRoot::setCamera(const PerspCameraParams& p) {
	if (mMaster) {
#ifdef _DEBUG
		throw std::runtime_error("PerspRoot::setCamera() illegal: root is a slave");
#endif
		return;
	}
	if (p == getCamera()) return;

	mCameraParams = p;

	mCameraDirty = true;
}

void PerspRoot::markCameraDirty() {
	mCameraDirty = true;
}

void PerspRoot::setCinderCamera() {
	mCameraDirty = false;

	if (mMaster) {
		if (mMaster->mCameraDirty) mMaster->setCinderCamera();
	}
	else {
		// The perspective camera is configured to render to a viewport that has the same
		// aspect ratio as the world size.  If the actual SrcRect being displayed is different
		// from the World rectangle, we need to adjust the fov and lens-shift proportionally.
		// This allows us to pan the SrcRect anywhere around the world space while still drawing 
		// perspective content exactly the same (just panned).
		const float ww = mEngine.getWorldWidth();
		const float wh = mEngine.getWorldHeight();
		const auto srcRect = mEngine.getSrcRect();
		const float sw = srcRect.getWidth();
		const float sh = srcRect.getHeight();

		const float tanHalfFov = ci::math<float>::tan(ci::toRadians(mCameraParams.mFov / 2.0f));
		const float adjustedFov = 2.0f * ci::toDegrees(ci::math<float>::atan2(sh * tanHalfFov, wh));
		const float adjustedLensShiftH =  (1.0 - ww / sw * (mCameraParams.mLensShiftH + 1.0f) + (2.0f * srcRect.x1 / sw));
        const float adjustedLensShiftV = -(1.0 - wh / sh * (mCameraParams.mLensShiftV + 1.0f) + (2.0f * srcRect.y1 / sh));

		mCamera.setEyePoint(mCameraParams.mPosition);
		mCamera.lookAt(mCameraParams.mTarget);
		mCamera.setPerspective(adjustedFov, ci::app::getWindowAspectRatio(), mCameraParams.mNearPlane, mCameraParams.mFarPlane);
		mCamera.setLensShiftHorizontal(adjustedLensShiftH);
		mCamera.setLensShiftVertical(adjustedLensShiftV);
	}
}

void PerspRoot::setGlCamera() {
	if (mMaster) {
		ci::gl::setMatrices(mMaster->mCamera);
	} else {
		ci::gl::setMatrices(mCamera);
	}
}

void PerspRoot::drawFunc(const std::function<void(void)>& fn) {
	if (mCameraDirty) {
		setCinderCamera();
	}
	setGlCamera();

	// enable the depth buffer (after all, we are doing 3D)
	ci::gl::ScopedDepth depthScope( true );
	glClear(GL_DEPTH_BUFFER_BIT);

	fn();
}

/**
 * \class ds::PerspRoot::OldPick
 */
PerspRoot::OldPick::OldPick(ci::Camera& c)
		: mCamera(c) {
}

ds::ui::Sprite* PerspRoot::OldPick::pickAt(const ci::Ray& pickRay, const ci::vec3& camDirection, ds::ui::Sprite& root) {
	ds::CameraPick pick(pickRay, camDirection);
	return root.getPerspectiveHit(pick);
}

} // namespace ds
