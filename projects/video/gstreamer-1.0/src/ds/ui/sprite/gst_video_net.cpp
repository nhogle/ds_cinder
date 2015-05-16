#include "gst_video_net.h"
#include "gst_video.h" //ugh.

#include <ds/data/data_buffer.h>
#include <ds/app/blob_registry.h>
#include <ds/ui/sprite/sprite.h>
#include <ds/app/engine/engine.h>
#include <ds/app/blob_reader.h>
#include <ds/app/app.h>

namespace {
static struct Initializer {
    Initializer() {
        ds::App::AddStartup([](ds::Engine& engine) {
            ds::ui::GstVideoNet::installSprite(engine);
        });
    }
} INIT;
}

namespace ds {
namespace ui {

char GstVideoNet::mBlobType = 0;

GstVideoNet::GstVideoNet(GstVideo& video)
    : mParamsDirty(ds::ui::INTERNAL_A_DIRTY)
    , mParamsAtt(99)
    , mVideoSprite(video)
{}

void GstVideoNet::installAsServer(BlobRegistry& registry)
{
    mBlobType = registry.add([](ds::BlobReader& r)
    {
        Sprite::handleBlobFromClient(r);
    });
}

void GstVideoNet::installAsClient(BlobRegistry& registry)
{
    mBlobType = registry.add([](ds::BlobReader& r)
    {
        Sprite::handleBlobFromServer<GstVideo>(r);
    });
}

void GstVideoNet::installSprite(Engine& engine)
{
    engine.installSprite(installAsServer, installAsClient);
}

}
} //!ds:ui