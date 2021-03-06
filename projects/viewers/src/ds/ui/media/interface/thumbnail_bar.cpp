#include "stdafx.h"

#include "thumbnail_bar.h"

#include <ds/app/app.h>
#include <ds/app/engine/engine.h>
#include <ds/app/environment.h>
#include <ds/ui/sprite/sprite_engine.h>
#include <ds/ui/sprite/image.h>
#include <ds/debug/logger.h>
#include <ds/util/string_util.h>
#include <ds/ui/util/ui_utils.h>
#include <ds/ui/scroll/scroll_area.h>
#include <ds/ui/scroll/scroll_bar.h>
#include <ds/data/color_list.h>

namespace {
class Init {
public:
	Init() {
		ds::App::AddStartup([](ds::Engine& e) {
			e.registerSpriteImporter("thumbnail_bar", [](ds::ui::SpriteEngine& enginey)->ds::ui::Sprite*{
				return new ds::ui::ThumbnailBar(enginey);
			});

		});
	}
};

Init INIT;
}

namespace ds {
namespace ui {

ThumbnailBar::ThumbnailBar(ds::ui::SpriteEngine& se)
	: ds::ui::Sprite(se)
	, mFileList(nullptr)
	, mPadding(5.0f)
	, mHighlightColor(ci::Color::white())
	, mHighlightItemIndex(0)
{

	setColor(ci::Color::black());
	setTransparent(false);

	mFileList = new ds::ui::ScrollList(mEngine, false);
	addChildPtr(mFileList);

	mFileList->setItemTappedCallback([this](ds::ui::Sprite* bs, const ci::vec3& cent){
		Image* rpi = dynamic_cast<Image*>(bs);
		if(rpi && mClickedCallback){
			// look up the resource from the image sprite in the image map
			auto findy = mImageMap.find(rpi);
			if(findy != mImageMap.end()){
				mClickedCallback(findy->second);
			}
		}
	});

	mFileList->setCreateItemCallback([this]()->ds::ui::Sprite* {
		return new ds::ui::Image(mEngine);
	});

	mFileList->setDataCallback([this](ds::ui::Sprite* bs, int dbId){
		Image* rpi = dynamic_cast<Image*>(bs);
		if(rpi){
			rpi->enable(true);
			rpi->enableMultiTouch(ds::ui::MULTITOUCH_INFO_ONLY);
			mImageMap[rpi] = mInfoMap[dbId];
			rpi->setImageResource(mInfoMap[dbId], ds::ui::Image::IMG_CACHE_F);
			setImageSize(rpi);

			if(dbId == mHighlightItemIndex){
				rpi->setColor(mHighlightColor);
			} else {
				rpi->setColor(ci::Color::white());
			}
		}
	});

	mFileList->setAnimateOnCallback([this](ds::ui::Sprite* bs, const float delay){
		Image* rpi = dynamic_cast<Image*>(bs);
		if(rpi){
		//	rpi->animateOn(delay);
		}
	});

	mFileList->setStateChangeCallback([this](ds::ui::Sprite* bs, const bool highlighted){
		Image* rpi = dynamic_cast<Image*>(bs);
		if(rpi){
			if(highlighted){
				rpi->setOpacity(0.75f);
			} else {
				rpi->setOpacity(1.0f);
			}
		}
	});

	/*
	auto scrollBar = dynamic_cast<ds::ui::ScrollBar*>(spriteMap["scroll_bar"]);
	if(scrollBar){
		scrollBar->linkScrollList(mFileList);
		scrollBar->getBackgroundSprite()->setOpacity(0.05f);
	}
	*/
}

void ThumbnailBar::onSizeChanged(){
	layout();
}

void ThumbnailBar::layout(){
	if(mFileList){

		mItemSize = 100.0f;
		const float w = getWidth();
		const float h = getHeight();
		float mSourceAspect = 1.0f;

		if(mSourceResource.getHeight() < 1.0f){
			// warning?
		} else {
			mSourceAspect = mSourceResource.getWidth() / mSourceResource.getHeight();
			mItemSize = mSourceAspect * (h - mPadding * 2.0f);
		}

		mFileList->setLayoutParams(0.0f, 0.0f, mItemSize + mPadding);
		mFileList->setSize(w - mPadding * 2.0f, h - mPadding * 2.0f); 
		mFileList->setPosition(mPadding, mPadding);
		mFileList->forEachLoadedSprite([this](ds::ui::Sprite* bs){
			setImageSize(dynamic_cast<Image*>(bs));
		});
		mFileList->layout();

		updateHighlight();
	}
}

void ThumbnailBar::setHighlightColor(const ci::Color& highlightColor){
	mHighlightColor = highlightColor;
	updateHighlight();
}

void ThumbnailBar::setHighlightedItem(const int itemIndex){
	mHighlightItemIndex = itemIndex;
	updateHighlight();
}

void ThumbnailBar::setImageSize(ds::ui::Image* img){
	if(!img) return;

	float imageAsp = img->getWidth() / img->getHeight();
	if(imageAsp > mSourceAspect){
		img->setScale(mItemSize / img->getWidth());
	} else {
		img->setScale((getHeight() - mPadding * 2.0f) / img->getHeight());
	}

}

void ThumbnailBar::updateHighlight(){
	if(mFileList){
		mFileList->forEachLoadedSprite([this](ds::ui::Sprite* bs){
			bs->setColor(ci::Color::white());
		}); 

		auto findy = mInfoMap.find(mHighlightItemIndex);
		if(findy != mInfoMap.end()){
			for (auto it : mImageMap){
				if(it.second == findy->second){
					it.first->setColor(mHighlightColor);
					break;
				}
			}
		}
	}
}

void ThumbnailBar::setData(ds::Resource& parentResource){
	if(!mFileList) return;

	mSourceResource = parentResource;
	
	mInfoMap.clear();
	mImageMap.clear();

	std::vector<int> productIds;
	int mediaId = 0;
	for(auto it : mSourceResource.getChildrenResources()){
		int thisId = mediaId++;
		productIds.push_back(thisId);
		mInfoMap[thisId] = it;
	}

	mFileList->setContent(productIds);

	layout();
}

} // namespace ui
} // namespace ds
