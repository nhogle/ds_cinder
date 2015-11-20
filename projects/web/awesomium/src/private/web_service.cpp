#include "private/web_service.h"

#include <ds/app/engine/engine.h>
#include <Awesomium/WebCore.h>
#include <Awesomium/WebConfig.h>

namespace ds {
namespace web {

/**
 * \class ds::web::Service
 */
Service::Service(ds::Engine& e)
		: ds::AutoUpdate(e, AutoUpdateType::SERVER | AutoUpdateType::CLIENT)
		, mWebCorePtr(nullptr)
		, mWebSessionPtr(nullptr) {
}

Service::~Service() {
	if (mWebSessionPtr) {
		mWebSessionPtr->Release();
	}
	if (mWebCorePtr) {
		Awesomium::WebCore::Shutdown();
	}
}

void Service::start() {
	Awesomium::WebConfig cnf;
	cnf.log_level = Awesomium::kLogLevel_Verbose;
	cnf.remote_debugging_port = 6980;
	Awesomium::WebStringArray addOptionsArray;

	// see this gist for all the available options: https://gist.github.com/khrona/2999122
	std::string useGlDesktop = "--use-gl=desktop";
	addOptionsArray.Push(Awesomium::WebString::CreateFromUTF8(useGlDesktop.c_str(), useGlDesktop.length()));

	std::string disableFullscreen = "--disable-fullscreen=true";
	addOptionsArray.Push(Awesomium::WebString::CreateFromUTF8(disableFullscreen.c_str(), disableFullscreen.length()));

	cnf.additional_options = addOptionsArray;

	// initialize the Awesomium web engine
	mWebCorePtr = Awesomium::WebCore::Initialize(cnf);
	if (mWebCorePtr) {
		Awesomium::WebPreferences		prefs;
		prefs.allow_scripts_to_open_windows = false;
		prefs.enable_app_cache = false;
		prefs.enable_gpu_acceleration = true;
		prefs.enable_web_gl = true;
		
		mWebSessionPtr = mWebCorePtr->CreateWebSession(Awesomium::WebString(), prefs);
	}
}

Awesomium::WebCore* Service::getWebCore() const {
	return mWebCorePtr;
}

Awesomium::WebSession* Service::getWebSession() const {
	return mWebSessionPtr;
}

void Service::update(const ds::UpdateParams&) {
	if (mWebCorePtr) {
		mWebCorePtr->Update();
	}
}

} // namespace web
} // namespace ds