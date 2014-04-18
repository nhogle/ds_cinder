#pragma once
#ifndef DS_APP_ENGINE_ENGINESERVER_H_
#define DS_APP_ENGINE_ENGINESERVER_H_

#include "ds/app/blob_reader.h"
#include "ds/app/engine/engine.h"
#include "ds/app/engine/engine_io.h"
#include "ds/network/udp_connection.h"
#include "ds/thread/gl_thread.h"
#include "ds/thread/work_manager.h"
#include "ds/ui/service/load_image_service.h"
#include "ds/ui/service/render_text_service.h"

namespace ds {

/**
 * \class ds::EngineServer
 * The Server engine contains all app-side behaviour, but no rendering.
 */
class EngineServer : public Engine {
  public:
    EngineServer(ds::App&, const ds::cfg::Settings&, ds::EngineData&, const ds::RootList&);
    ~EngineServer();

	virtual ds::WorkManager&		getWorkManager()		{ return mWorkManager; }
	virtual ui::LoadImageService&	getLoadImageService()	{ return mLoadImageService; }
	virtual ui::RenderTextService&	getRenderTextService()	{ return mRenderTextService; }

    virtual void                  installSprite(const std::function<void(ds::BlobRegistry&)>& asServer,
                                                const std::function<void(ds::BlobRegistry&)>& asClient);

    virtual void				          setup(ds::App&);
    virtual void                  setupTuio(ds::App&);
    virtual void	                update();
    virtual void                  draw();

    virtual void                  stopServices();
    virtual int                   getMode() const { return SERVER_MODE; }

private:
	void                          receiveHeader(ds::DataBuffer&);
	void                          receiveCommand(ds::DataBuffer&);

	typedef Engine inherited;
	WorkManager					mWorkManager;
	GlNoThread					mLoadImageThread;
	ui::LoadImageService		mLoadImageService;
	GlNoThread					mRenderTextThread;
	ui::RenderTextService		mRenderTextService;

//    ds::ZmqConnection             mConnection;
    ds::UdpConnection             mSendConnection;
    ds::UdpConnection             mReceiveConnection;
    EngineSender                  mSender;
    EngineReceiver                mReceiver;
    ds::BlobReader                mBlobReader;

    // STATES

    class State {
    public:
      State();
      virtual void                begin(EngineServer&);
      virtual void                update(EngineServer&) = 0;

    protected:
      void                        addHeader(ds::DataBuffer&, const int frame);
    };

    class RunningState : public State {
    public:
      RunningState();
      virtual void                begin(EngineServer&);
      virtual void                update(EngineServer&);

    private:
      int                         mFrame;
    };

    class SendWorldState : public State {
    public:
      SendWorldState();
      virtual void                update(EngineServer&);
    };

    State*                      mState;
    RunningState                mRunningState;
    SendWorldState              mSendWorldState;

    void                        setState(State&);
};

} // namespace ds

#endif // DS_APP_ENGINE_ENGINESERVER_H_