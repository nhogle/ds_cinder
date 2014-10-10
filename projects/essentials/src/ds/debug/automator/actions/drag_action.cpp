#include "drag_action.h"
#include <ds/ui/sprite/sprite_engine.h>
#include <cinder/Rand.h>
#include <cinder/app/TouchEvent.h>

namespace ds {
namespace debug {

/**
 * \class ds::DragActionFactory
 */
float DragActionFactory::getLimit() const {
	return ci::randFloat(0.1f, 1.0f);
}

int DragActionFactory::getNumberOfFingers() const {
	return ci::randInt(1, 5);
}

BaseAction* DragActionFactory::build(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame) const {
	return new DragAction(freeList, engine, frame);
}

/**
 * \class ds::DragAction
 */
DragAction::DragAction(std::vector<int> &freeList, ds::ui::SpriteEngine& engine, const ci::Rectf& frame)
	: BaseAction(freeList, engine, frame)
	, mMagnitude(0.0f)
	, mUpdateTimeTotal(0.0f)
	, mUpdateTime(0.016f)
{
}

DragAction::~DragAction(){
}

bool DragAction::update(float dt){
	BaseAction::update(dt);

	if(mTotal >= mLimit){
		std::vector<ci::app::TouchEvent::Touch> touches;
		for(int i = 0; i < mNumberOfFingers; ++i){
			touches.push_back(ci::app::TouchEvent::Touch(mTouchPos[i], mPreviousTouch[i], mInUseList[i], dt, nullptr));
		}
		mEngine.injectTouchesEnded(ci::app::TouchEvent(mEngine.getWindow(), touches));
		return true;

	} else {
		mUpdateTimeTotal += dt;

		if(mUpdateTimeTotal >= mUpdateTime){
			mUpdateTimeTotal = 0.0f;
			std::vector<ci::app::TouchEvent::Touch> touches;
			for(int i = 0; i < mNumberOfFingers; ++i){
				mPreviousTouch[i] = mTouchPos[i];
				mTouchPos[i] += mDirection * mMagnitude * mUpdateTime;
				touches.push_back(ci::app::TouchEvent::Touch(mTouchPos[i], mTouchPos[i], mInUseList[i], dt, nullptr));
			}
			mEngine.injectTouchesMoved(ci::app::TouchEvent(mEngine.getWindow(), touches));
		}

	}

	return false;
}

void DragAction::setup(float limit, int numberOfFingers){

	BaseAction::setup(limit, numberOfFingers);

	mTouchPos.reserve(mInUseList.size());
	float radius = 20.0f;
	ci::Vec2f touchPos = ci::Vec2f(mFrame.getX1() + ci::randFloat(0.0f, mFrame.getWidth()), mFrame.getY1() + ci::randFloat(0.0f, mFrame.getHeight()) );

	float step = (2.0f*(float)M_PI) / mNumberOfFingers;
	float angle = (float)M_PI;

	std::vector<ci::app::TouchEvent::Touch> touches;
	for(auto it = mInUseList.begin(), it2 = mInUseList.end(); it != it2; ++it){

		ci::Vec2f nTouchPos = touchPos + ci::Vec2f(cos(angle)*radius - sin(angle)*radius, sin(angle)*radius + cos(angle)*radius);
		mTouchPos.push_back(nTouchPos);
		mPreviousTouch.push_back(nTouchPos);
		angle += step;
		touches.push_back(ci::app::TouchEvent::Touch(nTouchPos, nTouchPos, *it, 0.0, nullptr));
	}

	mEngine.injectTouchesBegin(ci::app::TouchEvent(mEngine.getWindow(), touches));

	mMagnitude = ci::randFloat (10.0f, 1500.0f);
	mDirection = ci::Vec2f(ci::randFloat(-1.0f, 1.0f), ci::randFloat(-1.0f, 1.0f)).normalized();
}

} // namespace debug
} // namespace ds