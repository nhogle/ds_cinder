#include "stdafx.h"
#include "smart_scroll_list.h"

#include <ds/ui/layout/smart_layout.h>

namespace ds {
namespace ui {

SmartScrollList::SmartScrollList(ds::ui::SpriteEngine& eng, const std::string& itemLayoutFile)
	: ScrollList(eng)
	, mContentItemTapped(nullptr)
{

	if(!itemLayoutFile.empty()) {
		setItemLayoutFile(itemLayoutFile);
	}

	setItemTappedCallback([this](ds::ui::Sprite* bs, const ci::vec3& cent) {
		SmartLayout* rpi = dynamic_cast<SmartLayout*>(bs);
		if(rpi) {
			if(mContentItemTapped) {
				mContentItemTapped(rpi, rpi->getContentModel());
			}
		}
	});

	setDataCallback([this](ds::ui::Sprite* bs, int dbId) {
		SmartLayout* rpi = dynamic_cast<SmartLayout*>(bs);
		if(rpi) {
			rpi->setContentModel(mContentMap[dbId]);

			if(mContentItemUpdated) {
				mContentItemUpdated(rpi);
			}
		}
	});

	setAnimateOnCallback([this](ds::ui::Sprite* bs, const float delay) {
		SmartLayout* rpi = dynamic_cast<SmartLayout*>(bs);
		if(rpi) {
			/// todo?
		}
	});

	setStateChangeCallback([this](ds::ui::Sprite* bs, const bool highlighted) {
		SmartLayout* rpi = dynamic_cast<SmartLayout*>(bs);
		if(rpi) {
			auto highlightSprite = rpi->getSprite("highlight");
			if(highlightSprite) {
				if(highlighted) {
					highlightSprite->show();
				} else {
					highlightSprite->hide();
				}
			}
		}
	});
}

void SmartScrollList::setContentList(ds::model::ContentModelRef parentModel) {
	setContentList(parentModel.getChildren());
}

void SmartScrollList::setContentList(std::vector<ds::model::ContentModelRef> theContents) {
	mContentMap.clear();
	int itemId = 1;
	std::vector<int> productIds;
	for(auto it : theContents) {
		productIds.emplace_back(itemId);
		mContentMap[itemId] = it;
		itemId++;
	}

	setContent(productIds);
}

void SmartScrollList::setItemLayoutFile(const std::string& itemLayout) {
	if(itemLayout.empty()) {
		DS_LOG_WARNING("Can't set a blank layout for sub items in SmartScrollList");
		return;
	}


	/// grab the height from the item, then get rid of it
	ds::ui::SmartLayout* tempItem = new ds::ui::SmartLayout(mEngine, itemLayout);
	tempItem->runLayout();
	float itemHeight = tempItem->getHeight();
	tempItem->release();

	setLayoutParams(0.0f, 0.0f, itemHeight);

	setCreateItemCallback([this, itemLayout]()->ds::ui::Sprite* {
		return new SmartLayout(mEngine, itemLayout);
	});

}

}
}