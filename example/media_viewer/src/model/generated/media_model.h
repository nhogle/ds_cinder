#pragma once
#ifndef DS_MODEL_AUTOGENERATED_Media
#define DS_MODEL_AUTOGENERATED_Media

#include <ds/data/resource.h>
#include <memory>
#include <vector>
#include <cinder/Vector.h>

#include "media_model.h"


namespace ds {
namespace model{




/**
* \class ds::model::MediaRef
*			Auto-generated data model for Media
*			Don't manually edit this file. Instead drop the source yaml file onto the yaml_importer in the utility folder of ds_cinder. 
*/
class MediaRef {
public:

	MediaRef();

	const unsigned int& getId() const;
	const std::wstring& getTitle() const;
	const std::wstring& getBody() const;
	const ds::Resource& getPrimaryResource() const;
	const int& getParentId() const;
	const std::vector<MediaRef>& getMediaRef() const;


	MediaRef& setId(const unsigned int& Id);
	MediaRef& setTitle(const std::wstring& Title);
	MediaRef& setBody(const std::wstring& Body);
	MediaRef& setPrimaryResource(const ds::Resource& PrimaryResource);
	MediaRef& setParentId(const int& ParentId);
	MediaRef& setMediaRef(const std::vector<MediaRef>& MediaRef);



private:
	class Data;
	std::shared_ptr<Data>	mData;
};

} // namespace model
} // namespace ds

#endif