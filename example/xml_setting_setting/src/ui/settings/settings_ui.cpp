#include "settings_ui.h"

#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/debug/logger.h>

#include "app/globals.h"

namespace setter {

SettingsUi::SettingsUi(Globals& g)
	: ds::ui::Sprite(g.mEngine)
	, mParams(nullptr)
	, mTest("Heyo")
	, mDrawParams(true)
	, mGlobals(g)
{

	mParams = ci::params::InterfaceGl::create(mEngine.getWindow(), "App parameters", ci::Vec2i(600, 250));

	setTransparent(false);
// 
// 	
// 	mGlobals.getSettingsLayout().forEachFloatKey([this](const std::string& key){
// 
// 		const std::function<void(float)> &setterFn = [this, key](float value){
// 			ds::cfg::Settings& settings = const_cast<ds::cfg::Settings&>(mGlobals.getSettingsLayout());
// 			ds::cfg::Settings::Editor editor(settings);
// 			editor.setFloat(key, value); };
// 
// 		const std::function<float()> &getterFn = [this, key](){
// 			return mGlobals.getSettingsLayout().getFloat(key);
// 		};
// 
// 		mParams->addParam(key, setterFn, getterFn);
// 	});
// 	

	registerSettings(&mGlobals.getSettingsLayout());

}

void SettingsUi::registerSettings(const ds::cfg::Settings* settings){


	settings->forEachFloatKey([this, settings](const std::string& key){
		const std::function<void(float)> &setterFn = [this, settings, key](float value){
			ds::cfg::Settings::Editor editor(*const_cast<ds::cfg::Settings*>(settings));
			editor.setFloat(key, value); };

		const std::function<float()> &getterFn = [this, settings, key](){
			return settings->getFloat(key);
		};

		mParams->addParam(key, setterFn, getterFn);
	});

	settings->forEachColorAKey([this, settings](const std::string& key){

		const std::function<void(ci::ColorA)> &setterFn = [this, settings, key](ci::ColorA value){
			ds::cfg::Settings::Editor editor(*const_cast<ds::cfg::Settings*>(settings));
			editor.setColorA(key, value); };

		const std::function<ci::ColorA()> &getterFn = [this, settings, key](){
			return settings->getColorA(key);
		};

		mParams->addParam(key, setterFn, getterFn);
	});

	settings->forEachIntKey([this, settings](const std::string& key){

		const std::function<void(ci::ColorA)> &setterFn = [this, settings, key](ci::ColorA value){
			ds::cfg::Settings::Editor editor(*const_cast<ds::cfg::Settings*>(settings));
			editor.setColorA(key, value); };

		const std::function<ci::ColorA()> &getterFn = [this, settings, key](){
			return settings->getColorA(key);
		};

		mParams->addParam(key, setterFn, getterFn);
	});
}



void SettingsUi::drawLocalClient(){
	if(mDrawParams){
		mParams->show(true);
		mParams->draw();
	} else {
		mParams->show(false);
	}
}

SettingsUi::~SettingsUi(){
	if(mParams){
		mParams->clear(); 
	}
}

void SettingsUi::toggleVisibility(){
	mDrawParams = !mDrawParams;
}


} // namespace setter