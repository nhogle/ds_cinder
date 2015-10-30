#include "media_viewer.h"


#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>
#include <ds/ui/util/ui_utils.h>

#include <ds/data/resource.h>

#include "ds/ui/media/player/video_player.h"
#include "ds/ui/media/player/pdf_player.h"
#include "ds/ui/media/player/web_player.h"

#include "ds/ui/media/media_interface.h"


namespace ds {
namespace ui {

MediaViewer::MediaViewer(ds::ui::SpriteEngine& eng, const bool embedInterface)
	: BasePanel(eng)
	, mInitialized(false)
	, mVideoPlayer(nullptr)
	, mPDFPlayer(nullptr)
	, mWebPlayer(nullptr)
	, mThumbnailImage(nullptr)
	, mPrimaryImage(nullptr)
	, mInterface(nullptr)
	, mEmbedInterface(embedInterface)
	, mDefaultBoundWidth(mEngine.getWorldWidth())
	, mDefaultBoundHeight(mEngine.getWorldHeight())
{
}

MediaViewer::MediaViewer(ds::ui::SpriteEngine& eng, const std::string& mediaPath, const bool embedInterface)
	: BasePanel(eng)
	, mInitialized(false)
	, mResource(mediaPath, ds::Resource::parseTypeFromFilename(mediaPath))
	, mVideoPlayer(nullptr)
	, mPDFPlayer(nullptr)
	, mWebPlayer(nullptr)
	, mThumbnailImage(nullptr)
	, mPrimaryImage(nullptr)
	, mInterface(nullptr)
	, mEmbedInterface(embedInterface)
	, mDefaultBoundWidth(mEngine.getWorldWidth())
	, mDefaultBoundHeight(mEngine.getWorldHeight())
{
}

MediaViewer::MediaViewer(ds::ui::SpriteEngine& eng, const ds::Resource& resource, const bool embedInterface)
	: BasePanel(eng)
	, mInitialized(false)
	, mResource(resource)
	, mVideoPlayer(nullptr)
	, mPDFPlayer(nullptr)
	, mWebPlayer(nullptr)
	, mThumbnailImage(nullptr)
	, mPrimaryImage(nullptr)
	, mInterface(nullptr)
	, mEmbedInterface(embedInterface)
	, mDefaultBoundWidth(mEngine.getWorldWidth())
	, mDefaultBoundHeight(mEngine.getWorldHeight())
{
}

void MediaViewer::initializeIfNeeded(){
	if(!mInitialized){
		initialize();
	}
}

void MediaViewer::loadMedia(const std::string& mediaPath, const bool initializeImmediately) {
	if(mInitialized){
		uninitialize();
	}

	mResource = ds::Resource(mediaPath, ds::Resource::parseTypeFromFilename(mediaPath));

	if(initializeImmediately){
		initializeIfNeeded();
	}
}

void MediaViewer::loadMedia(const ds::Resource& reccy, const bool initializeImmediately) {
	if(mInitialized){
		uninitialize();
	}

	mResource = reccy;

	if(initializeImmediately){
		initializeIfNeeded();
	}
}


void MediaViewer::setDefaultBounds(const float defaultWidth, const float defaultHeight){
	mDefaultBoundWidth = defaultWidth;
	mDefaultBoundHeight = defaultHeight;
}

void MediaViewer::initialize(){

	// do this first to avoid recursion problems
	mInitialized = true;

	if(mResource.getThumbnailId() > 0 || !mResource.getThumbnailFilePath().empty()){
		mThumbnailImage = new ds::ui::Image(mEngine);
		addChildPtr(mThumbnailImage);
		if(mResource.getThumbnailId() > 0){
			mThumbnailImage->setImageResource(mResource.getThumbnailId());
		} else {
			mThumbnailImage->setImageFile(mResource.getThumbnailFilePath());
		}
		mThumbnailImage->setOpacity(0.0f);
		mThumbnailImage->setStatusCallback([this](ds::ui::Image::Status status){
			if(status.mCode == status.STATUS_LOADED && mThumbnailImage){
				mThumbnailImage->tweenOpacity(1.0f, mAnimDuration);
			}
		});
	}

	const int mediaType = mResource.getType();
	if(mediaType == ds::Resource::ERROR_TYPE || mediaType == ds::Resource::FONT_TYPE){
		DS_LOG_WARNING("Whoopsies - tried to open a media player on an invalid file type. " << mResource.getAbsoluteFilePath());
		return;
	}

	float contentWidth = 1.0f;
	float contentHeight = 1.0f;
	mContentAspectRatio = 1.0f;

	if(mediaType == ds::Resource::IMAGE_TYPE){
		mPrimaryImage = new ds::ui::Image(mEngine);
		addChildPtr(mPrimaryImage);
		mPrimaryImage->setImageFile(mResource.getAbsoluteFilePath());
		mPrimaryImage->setOpacity(0.0f);
		mPrimaryImage->setStatusCallback([this](ds::ui::Image::Status status){
			if(status.mCode == status.STATUS_LOADED && mPrimaryImage){
				mPrimaryImage->tweenOpacity(1.0f, mAnimDuration);
			}
		});

		mContentAspectRatio = mPrimaryImage->getWidth() / mPrimaryImage->getHeight();
		contentWidth = mPrimaryImage->getWidth();
		contentHeight = mPrimaryImage->getHeight();
	} else if(mediaType == ds::Resource::VIDEO_TYPE){
		mVideoPlayer = new VideoPlayer(mEngine, mEmbedInterface);
		addChildPtr(mVideoPlayer);
		mVideoPlayer->setMedia(mResource.getAbsoluteFilePath());

		mContentAspectRatio = mVideoPlayer->getWidth() / mVideoPlayer->getHeight();
		contentWidth = mVideoPlayer->getWidth();
		contentHeight = mVideoPlayer->getHeight();

		if(!mEmbedInterface){
			mInterface = mVideoPlayer->getExternalInterface();
		}

	} else if(mediaType == ds::Resource::PDF_TYPE){
		mPDFPlayer = new PDFPlayer(mEngine, mEmbedInterface);
		addChildPtr(mPDFPlayer);
		mPDFPlayer->setMedia(mResource.getAbsoluteFilePath());

		mContentAspectRatio = mPDFPlayer->getWidth() / mPDFPlayer->getHeight();
		contentWidth = mPDFPlayer->getWidth();
		contentHeight = mPDFPlayer->getHeight();

		if(!mEmbedInterface){
			mInterface = mPDFPlayer->getExternalInterface();
		}

	} else if(mediaType == ds::Resource::WEB_TYPE){
		mWebPlayer = new WebPlayer(mEngine, mEmbedInterface);
		addChildPtr(mWebPlayer);
		mWebPlayer->setMedia(mResource.getAbsoluteFilePath());

		mContentAspectRatio = mWebPlayer->getWidth() / mWebPlayer->getHeight();
		contentWidth = mWebPlayer->getWidth();
		contentHeight = mWebPlayer->getHeight();

		if(!mEmbedInterface){
			mInterface = mWebPlayer->getExternalInterface();
		}

		setTapCallback([this](ds::ui::Sprite* bs, const ci::Vec3f& pos){
			if(mWebPlayer){
				mWebPlayer->sendClick(pos);
			}
		});

	} else {
		DS_LOG_WARNING("Whoopsies - tried to open a media player on an invalid file type. " << mResource.getAbsoluteFilePath() << " " << ds::utf8_from_wstr(mResource.getTypeName()));
	}

	// calculate a default size that maximizes size
	const float engineWidth = mDefaultBoundWidth;
	const float engineHeight = mDefaultBoundHeight;
	const float engineAspect = engineWidth / engineHeight;

	// calculate a width to make the player fit maximally
	float scaleFactor = 1.0f;
	float idealWidth = engineWidth;
	float idealHeight = engineHeight;
	if(mContentAspectRatio < engineAspect){
		scaleFactor = engineHeight / contentHeight;
		idealWidth = contentWidth * scaleFactor;
	} else if(mContentAspectRatio > engineAspect){
		scaleFactor = engineWidth / contentWidth;
		idealHeight = contentHeight * scaleFactor;
	}

	mDefaultSize = ci::Vec2f(idealWidth, idealHeight);
	// setting size is necessary to get size limits to work
	setSize(idealWidth, idealHeight);
	setSizeLimits();
	setViewerSize(mDefaultSize.x, mDefaultSize.y);
}

void MediaViewer::uninitialize() {
	if(mThumbnailImage){
		mThumbnailImage->release();
	}
	if(mVideoPlayer){
		mVideoPlayer->release();
	}
	if(mPDFPlayer){
		mPDFPlayer->release();
	}
	if(mPrimaryImage){
		mPrimaryImage->release();
	}
	if(mWebPlayer){
		mWebPlayer->release();
	}

	mThumbnailImage = nullptr;
	mVideoPlayer = nullptr;
	mPDFPlayer = nullptr;
	mPrimaryImage = nullptr;
	mWebPlayer = nullptr;
}


void MediaViewer::onLayout(){
	initializeIfNeeded();

	if(mVideoPlayer){
		mVideoPlayer->setSize(getWidth(), getHeight());
	}

	if(mPDFPlayer){
		mPDFPlayer->setSize(getWidth(), getHeight());
	}

	if(mThumbnailImage){
		fitInside(mThumbnailImage, ci::Rectf(0.0f, 0.0f, getWidth(), getHeight()), true);
	}

	if(mPrimaryImage){
		fitInside(mPrimaryImage, ci::Rectf(0.0f, 0.0f, getWidth(), getHeight()), true);
	}

	if(mWebPlayer){
		mWebPlayer->setSize(getWidth(), getHeight());
	}
}

void MediaViewer::enter(){
	initializeIfNeeded();

	if(mVideoPlayer){
		mVideoPlayer->play();
	}
}

void MediaViewer::exit(){
	if(mVideoPlayer){
		mVideoPlayer->pause();
	}
}

void MediaViewer::userInputReceived(){
	BasePanel::userInputReceived();
	if(mVideoPlayer){
		mVideoPlayer->showInterface();
	}
	if(mPDFPlayer){
		mPDFPlayer->showInterface();
	}
	if(mWebPlayer){
		mWebPlayer->showInterface();
	}
}

void MediaViewer::stopContent(){
	if(mVideoPlayer){
		mVideoPlayer->stop();
	}
}

} // namespace ui
} // namespace ds
