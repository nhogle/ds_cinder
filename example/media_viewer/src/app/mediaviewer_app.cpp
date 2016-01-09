#include "mediaviewer_app.h"

#include <Poco/String.h>
#include <Poco/File.h>
#include <Poco/Path.h>

#include <cinder/Rand.h>

#include <ds/app/environment.h>
#include <ds/debug/logger.h>
#include <ds/app/engine/engine.h>
#include <ds/util/string_util.h>
#include <ds/ui/media/media_viewer.h>

#include <cinder/Clipboard.h>

#include "app/app_defs.h"
#include "app/globals.h"
#include "events/app_events.h"
#include "ui/viewers/viewer_controller.h"


namespace mv {

MediaViewer::MediaViewer()
	: inherited(ds::RootList()
								.ortho() // sample ortho view
								.pickColor()

								.persp() // sample perp view
								.perspFov(60.0f)
								.perspPosition(ci::Vec3f(0.0, 0.0f, 10.0f))
								.perspTarget(ci::Vec3f(0.0f, 0.0f, 0.0f))
								.perspNear(0.0002f)
								.perspFar(20.0f)

								.ortho() ) // ortho view on top
	, mGlobals(mEngine , mAllData )
	, mQueryHandler(mEngine, mAllData)
	, mIdling( false )
	, mTouchDebug(mEngine)
	, mTouchMenu(nullptr)
	, mStreamer(nullptr)
{


	/*fonts in use */
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/NotoSans-Bold.ttf"), "noto-bold");
	mEngine.editFonts().install(ds::Environment::getAppFile("data/fonts/NotoSansCJKsc-Thin.otf"), "noto-thin");

	enableCommonKeystrokes(true);
}

void MediaViewer::setupServer(){


	/* Settings */
	mEngine.loadSettings(SETTINGS_LAYOUT, "layout.xml");
	mEngine.loadTextCfg("text.xml");
	const int numRoots = mEngine.getRootCount();
	int numPlacemats = 0;
	for(int i = 0; i < numRoots - 1; i++){
		// don't clear the last root, which is the debug draw
		if(mEngine.getRootBuilder(i).mDebugDraw) continue;

		ds::ui::Sprite& rooty = mEngine.getRootSprite(i);
		if(rooty.getPerspective()){
			const float clippFar = 10000.0f;
			const float fov = 60.0f;
			ds::PerspCameraParams p = mEngine.getPerspectiveCamera(i);
			p.mTarget = ci::Vec3f(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, 0.0f);
			p.mFarPlane = clippFar;
			p.mFov = fov;
			p.mPosition = ci::Vec3f(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, mEngine.getWorldWidth() / 2.0f);
			mEngine.setPerspectiveCamera(i, p);
		} else {
			mEngine.setOrthoViewPlanes(i, -10000.0f, 10000.0f);
		}

		rooty.clearChildren();
	}

	mStreamer = nullptr;
	mGlobals.initialize();

	ds::ui::Sprite &rootSprite = mEngine.getRootSprite();
	rootSprite.setTransparent(false);
	rootSprite.setColor(ci::Color(0.1f, 0.1f, 0.1f));
	
	// add sprites
	rootSprite.addChildPtr(new ViewerController(mGlobals));

	mTouchMenu = new ds::ui::TouchMenu(mEngine);
	rootSprite.addChildPtr(mTouchMenu);

	ds::ui::TouchMenu::TouchMenuConfig tmc;
	tmc.mBackgroundImage = "%APP%/data/images/menu/TouchMenuBlur.png";
	tmc.mItemTitleTextConfig = "touch:menu";
	tmc.mClusterRadius = 250.0f;
	tmc.mBackgroundOpacity = 0.7f;
	mTouchMenu->setMenuConfig(tmc);

	std::vector<ds::ui::TouchMenu::MenuItemModel> menuItemModels;
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Exit", "%APP%/data/images/menu/exit_app_normal.png", "%APP%/data/images/menu/exit_app_glow.png", [this](ci::Vec3f){ std::exit(0); }));
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Close All", "%APP%/data/images/menu/close_normal.png", "%APP%/data/images/menu/close_glow.png", [this](ci::Vec3f){ mEngine.getNotifier().notify(RequestCloseAllEvent()); }));
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Search", "%APP%/data/images/menu/search_normal.png", "%APP%/data/images/menu/search_glow.png", [this](ci::Vec3f pos){ /*mEngine.getNotifier().notify(RequestPresenterModeEvent(pos));*/ }));
	menuItemModels.push_back(ds::ui::TouchMenu::MenuItemModel(L"Layout", "%APP%/data/images/menu/pinboard_normal.png", "%APP%/data/images/menu/pinboard_glow.png", [this](ci::Vec3f){ mEngine.getNotifier().notify(RequestLayoutEvent()); }));


	mTouchMenu->setMenuItemModels(menuItemModels);

}

void MediaViewer::update() {
	inherited::update();

	if( mEngine.isIdling() && !mIdling ){
		//Start idling
		mIdling = true;
		mEngine.getNotifier().notify( IdleStartedEvent() );
	} else if ( !mEngine.isIdling() && mIdling ){
		//Stop idling
		mIdling = false;
		mEngine.getNotifier().notify( IdleEndedEvent() );
	}

}

void MediaViewer::keyDown(ci::app::KeyEvent event){
	using ci::app::KeyEvent;
	inherited::keyDown(event);
	if(event.getChar() == KeyEvent::KEY_r){ // R = reload all configs and start over without quitting app
		setupServer();

	} else if(event.getCode() == KeyEvent::KEY_v && event.isControlDown()){
		auto fileNameOrig = ci::Clipboard::getString();
		Poco::File filey = Poco::File(fileNameOrig);
		std::string extensionay = Poco::Path(filey.path()).getExtension();
		std::transform(extensionay.begin(), extensionay.end(), extensionay.begin(), ::tolower);
		if(extensionay.find("png") != std::string::npos
		   || extensionay.find("jpg") != std::string::npos
		   || extensionay.find("jpeg") != std::string::npos
		   || extensionay.find("gif") != std::string::npos
		   ){

			Poco::Path pathy = filey.path();
			std::string fileName = pathy.getFileName();
			fileName = fileName.substr(0, fileName.length() - 4);

			ds::model::MediaRef newMedia = ds::model::MediaRef();
			newMedia.setPrimaryResource(ds::Resource(fileNameOrig, ds::Resource::IMAGE_TYPE));
			newMedia.setTitle(ds::wstr_from_utf8(fileName));
			newMedia.setBody(ds::wstr_from_utf8(fileNameOrig));
			mEngine.getNotifier().notify(RequestMediaOpenEvent(newMedia, ci::Vec3f(mEngine.getWorldWidth() / 2.0f, mEngine.getWorldHeight() / 2.0f, 0.0f), 600.0f));
		}


	// Perspective camera movement
	} else if(event.getCode() == KeyEvent::KEY_d){
		moveCamera(ci::Vec3f(1.0f, 0.0f, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_a){
		moveCamera(ci::Vec3f(-1.0f, 0.0f, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_w){
		moveCamera(ci::Vec3f(0.0f, -1.0f, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_s){
		moveCamera(ci::Vec3f(0.0f, 1.0f, 0.0f));
	} else if(event.getCode() == KeyEvent::KEY_RIGHTBRACKET){
		moveCamera(ci::Vec3f(0.0f, 0.0f, 1.0f));
	} else if(event.getCode() == KeyEvent::KEY_LEFTBRACKET){
		moveCamera(ci::Vec3f(0.0f, 0.0f, -1.0f));
	} else if(event.getCode() == KeyEvent::KEY_EQUALS){
		ds::PerspCameraParams p = mEngine.getPerspectiveCamera(1);
		p.mFarPlane += 1.0f;
		std::cout << "Clip Far camera: " << p.mFarPlane << std::endl;
		mEngine.setPerspectiveCamera(1, p);
	} else if(event.getCode() == KeyEvent::KEY_MINUS){
		ds::PerspCameraParams p = mEngine.getPerspectiveCamera(1);
		p.mFarPlane -= 1.0f;
		std::cout << "Clip Far camera: " << p.mFarPlane << std::endl;
		mEngine.setPerspectiveCamera(1, p);

	// Shows all enabled sprites with a label for class type
	} else if(event.getCode() == KeyEvent::KEY_f){

		const int numRoots = mEngine.getRootCount();
		int numPlacemats = 0;
		for(int i = 0; i < numRoots - 1; i++){
			mEngine.getRootSprite(i).forEachChild([this](ds::ui::Sprite& sprite){
				if(sprite.isEnabled()){
					sprite.setTransparent(false);
					sprite.setColor(ci::Color(ci::randFloat(), ci::randFloat(), ci::randFloat()));
					sprite.setOpacity(0.95f);

					ds::ui::Text* labelly = mGlobals.getText("media_viewer:title").create(mEngine, &sprite);
					labelly->setText(typeid(sprite).name());
					labelly->enable(false);
					labelly->setColor(ci::Color::black());
				} else {

					ds::ui::Text* texty = dynamic_cast<ds::ui::Text*>(&sprite);
					if(!texty || (texty && texty->getColor() != ci::Color::black())) sprite.setTransparent(true);
				}
			}, true);
		}
	} else if(event.getCode() == KeyEvent::KEY_p){
		if(mStreamer){
			mStreamer->stop();
			mStreamer->release();
			mStreamer = nullptr;
		}
		mStreamer = new ds::ui::GstVideo(mEngine);
		mStreamer->startStream(mGlobals.getSettingsLayout().getText("streaming:pipeline", 0, ""),
							  mGlobals.getSettingsLayout().getInt("streaming:width", 0, 640), 
							  mGlobals.getSettingsLayout().getInt("streaming:height", 0, 480));
		mEngine.getRootSprite().addChildPtr(mStreamer);
		mStreamer->enable(true);
		mStreamer->enableMultiTouch(ds::ui::MULTITOUCH_CAN_POSITION | ds::ui::MULTITOUCH_CAN_SCALE);
		
	}
}

void MediaViewer::moveCamera(const ci::Vec3f& deltaMove){
	ds::PerspCameraParams p = mEngine.getPerspectiveCamera(1);
	p.mPosition += deltaMove;
	std::cout << "Moving camera: " << p.mPosition.x << " " << p.mPosition.y << " " << p.mPosition.z << std::endl;
	mEngine.setPerspectiveCamera(1, p);
}

void MediaViewer::mouseDown(ci::app::MouseEvent e) {
	mTouchDebug.mouseDown(e);
}

void MediaViewer::mouseDrag(ci::app::MouseEvent e) {
	mTouchDebug.mouseDrag(e);
}

void MediaViewer::mouseUp(ci::app::MouseEvent e) {
	mTouchDebug.mouseUp(e);
}

void MediaViewer::onTouchesBegan(ds::ui::TouchEvent e) {
	touchEventToTouchInfo(e, ds::ui::TouchInfo::Added);
}

void MediaViewer::onTouchesMoved(ds::ui::TouchEvent e){
	touchEventToTouchInfo(e, ds::ui::TouchInfo::Moved);
}

void MediaViewer::onTouchesEnded(ds::ui::TouchEvent e){
	touchEventToTouchInfo(e, ds::ui::TouchInfo::Removed);
}

void MediaViewer::touchEventToTouchInfo(ds::ui::TouchEvent& te, ds::ui::TouchInfo::Phase phasey){
	if(!mTouchMenu) return;

	static const int MOUSE_RESERVED_IDS = 2; // from touch_manager.cpp. Gross, I know. Whatever.
	for(std::vector<ci::app::TouchEvent::Touch>::const_iterator touchIt = te.getTouches().begin(); touchIt != te.getTouches().end(); ++touchIt) {
		ci::Vec2f touchyPointy = touchIt->getPos();

		if(mGlobals.getSettingsLayout().getBool("cluster:touch:translate_points", 0, false)){
			mEngine.translateTouchPoint(touchyPointy);
		}

		if(mEngine.shouldDiscardTouch(touchyPointy))
			continue;

		ds::ui::TouchInfo touchInfo;
		touchInfo.mCurrentGlobalPoint = ci::Vec3f(touchyPointy, 0.0f);
		touchInfo.mFingerId = touchIt->getId() + MOUSE_RESERVED_IDS;
		touchInfo.mPhase = phasey;
		mTouchMenu->handleTouchInfo(mTouchMenu, touchInfo);
	}
}

void MediaViewer::fileDrop(ci::app::FileDropEvent event){
	std::vector<std::string> paths;
	ci::Vec3f locationy = ci::Vec3f((float)event.getX(), (float)event.getY(), 0.0f);
	float incrementy = 50.0f;
	float startWidth = mEngine.getWorldWidth() / 4.0f;
	for(auto it = event.getFiles().begin(); it < event.getFiles().end(); ++it){
		//paths.push_back((*it).string());

// 		ds::ui::MediaViewer* mv = new ds::ui::MediaViewer(mEngine, (*it).string(), true);
// 		mv->initializeIfNeeded();
// 		mEngine.getRootSprite().addChildPtr(mv);

		Poco::File filey = Poco::File((*it).string());
		Poco::Path pathy = filey.path();
		std::string fileName = pathy.getFileName();
		fileName = fileName.substr(0, fileName.length() - 4);

		ds::model::MediaRef newMedia = ds::model::MediaRef();
		newMedia.setPrimaryResource(ds::Resource((*it).string(), ds::Resource::parseTypeFromFilename((*it).string())));
		newMedia.setTitle(ds::wstr_from_utf8(fileName));
		newMedia.setBody((*it).wstring());
		mEngine.getNotifier().notify(RequestMediaOpenEvent(newMedia, ci::Vec3f(locationy.x - startWidth/2.0f, locationy.y - startWidth/2.0f, 0.0f), startWidth));
		locationy.x += incrementy;
		locationy.y += incrementy;
	}

	event.setHandled();
}

} // namespace mv

// This line tells Cinder to actually create the application
CINDER_APP_BASIC(mv::MediaViewer, ci::app::RendererGl(ci::app::RendererGl::AA_MSAA_4))