#include "stdafx.h"

#include "content_model.h" 

#include <ds/util/string_util.h>
#include <ds/util/color_util.h>


namespace ds {
namespace model {
namespace {
const int							EMPTY_INT = 0;
const std::string					EMPTY_STRING;
const ds::Resource					EMPTY_RESOURCE;
const std::vector<ContentModelRef>		EMPTY_DATAMODELREF_VECTOR;
const ContentModelRef										EMPTY_DATAMODEL;
const ContentProperty										EMPTY_PROPERTY;
const std::map<std::string, ContentProperty>				EMPTY_PROPERTY_MAP;

}

ContentProperty::ContentProperty()
	: mName("")
	, mValue("")
	, mResource(EMPTY_RESOURCE)
{
}

ContentProperty::ContentProperty(const std::string& name, const std::string& value) {
	setValue(value);
	setName(name);
}
const std::string& ContentProperty::getName() const {
	return mName;
}

void ContentProperty::setName(const std::string& name) {
	mName = name;
}

const std::string& ContentProperty::getValue() const {
	return mValue;
}

void ContentProperty::setValue(const std::string& value) {
	mValue = value;
}

void ContentProperty::setValue(const std::wstring& value) {
	mValue = ds::utf8_from_wstr(value);
}

void ContentProperty::setValue(const int& value) {
	mValue = ds::value_to_string<int>(value);
}

void ContentProperty::setValue(const double& value) {
	mValue = ds::value_to_string<double>(value);
}

void ContentProperty::setValue(const float& value) {
	mValue = ds::value_to_string<float>(value);
}

void ContentProperty::setValue(const ci::Color& value) {
	mValue = ds::unparseColor(value);
}

void ContentProperty::setValue(const ci::ColorA& value) {
	mValue = ds::unparseColor(value);
}

void ContentProperty::setValue(const ci::vec2& value) {
	mValue = ds::unparseVector(value);
}

void ContentProperty::setValue(const ci::vec3& value) {
	mValue = ds::unparseVector(value);
}

void ContentProperty::setValue(const ci::Rectf& value) {
	mValue = ds::unparseRect(value);
}

ds::Resource ContentProperty::getResource() const {
	return mResource;
}

void ContentProperty::setResource(const ds::Resource& resource) {
	mResource = resource;
}

bool ContentProperty::getBool() const {
	return ds::parseBoolean(mValue);
}

int ContentProperty::getInt() const {
	return ds::string_to_int(mValue);
}

float ContentProperty::getFloat() const {
	return ds::string_to_float(mValue);
}

double ContentProperty::getDouble() const {
	return ds::string_to_double(mValue);
}

const ci::Color ContentProperty::getColor(ds::ui::SpriteEngine& eng) const {
	return ds::parseColor(mValue, eng);
}

const ci::ColorA ContentProperty::getColorA(ds::ui::SpriteEngine& eng) const {
	return ds::parseColor(mValue, eng);
}

const std::string& ContentProperty::getString() const {
	return getValue();
}

const std::wstring ContentProperty::getWString() const {
	return ds::wstr_from_utf8(getString());
}

const ci::vec2 ContentProperty::getVec2() const {
	return ci::vec2(ds::parseVector(mValue));
}

const ci::vec3 ContentProperty::getVec3() const {
	return ds::parseVector(mValue);
}

const ci::Rectf ContentProperty::getRect() const {
	return ds::parseRect(mValue);
}



class ContentModelRef::Data {
public:
	Data()
		: mName(EMPTY_STRING)
		, mLabel(EMPTY_STRING)
		, mId(EMPTY_INT)
	{}

	std::string mName;
	std::string mLabel;
	int mId;
	std::map<std::string, ContentProperty> mProperties;
	std::vector<ContentModelRef> mChildren;

};

ContentModelRef::ContentModelRef() {}


ContentModelRef::ContentModelRef(const std::string& name, const int id, const std::string& label) {
	setName(name);
	setId(id);
	setLabel(label);
}

const int& ContentModelRef::getId() const {
	if(!mData) return EMPTY_INT;
	return mData->mId;
}

void ContentModelRef::setId(const int& id) {
	createData();
	mData->mId = id;
}

const std::string& ContentModelRef::getName() const {
	if(!mData) return EMPTY_STRING;
	return mData->mName;
}

void ContentModelRef::setName(const std::string& name) {
	createData();
	mData->mName = name;
}

const std::string& ContentModelRef::getLabel() const {
	if(!mData) return EMPTY_STRING;
	return mData->mLabel;
}

void ContentModelRef::setLabel(const std::string& name) {
	createData();
	mData->mLabel = name;
}

const bool ContentModelRef::empty() const {
	if(!mData) return true;
	if(mData->mId == EMPTY_INT
	   && mData->mName == EMPTY_STRING
	   && mData->mChildren.empty()
	   && mData->mProperties.empty()
	   ) {
		return true;
	}

	return false;
}


const std::map<std::string, ContentProperty>& ContentModelRef::getProperties() {
	if(!mData) return EMPTY_PROPERTY_MAP;
	return mData->mProperties;
}

void ContentModelRef::setProperties(const std::map<std::string, ContentProperty>& newProperties) {
	createData();
	mData->mProperties = newProperties;
}

const ds::model::ContentProperty ContentModelRef::getProperty(const std::string& propertyName) {
	if(!mData) return EMPTY_PROPERTY;
	auto findy = mData->mProperties.find(propertyName);
	if(findy != mData->mProperties.end()) {
		return findy->second;
	}

	return EMPTY_PROPERTY;
}

const std::string ContentModelRef::getPropertyValue(const std::string& propertyName) {
	return getProperty(propertyName).getValue();
}

bool ContentModelRef::getPropertyBool(const std::string& propertyName) {
	return getProperty(propertyName).getBool();
}

int ContentModelRef::getPropertyInt(const std::string& propertyName) {
	return getProperty(propertyName).getInt();
}

float ContentModelRef::getPropertyFloat(const std::string& propertyName) {
	return getProperty(propertyName).getFloat();
}

double ContentModelRef::getPropertyDouble(const std::string& propertyName) {
	return getProperty(propertyName).getDouble();
}

const ci::Color ContentModelRef::getPropertyColor(ds::ui::SpriteEngine& eng, const std::string& propertyName) {
	return getProperty(propertyName).getColor(eng);
}

const ci::ColorA ContentModelRef::getPropertyColorA(ds::ui::SpriteEngine& eng, const std::string& propertyName) {
	return getProperty(propertyName).getColorA(eng);
}

const std::string ContentModelRef::getPropertyString(const std::string& propertyName) {
	return getProperty(propertyName).getString();
}

const std::wstring ContentModelRef::getPropertyWString(const std::string& propertyName) {
	return getProperty(propertyName).getWString();
}

const ci::vec2 ContentModelRef::getPropertyVec2(const std::string& propertyName) {
	return getProperty(propertyName).getVec2();
}

const ci::vec3 ContentModelRef::getPropertyVec3(const std::string& propertyName) {
	return getProperty(propertyName).getVec3();
}

const ci::Rectf ContentModelRef::getPropertyRect(const std::string& propertyName) {
	return getProperty(propertyName).getRect();
}

void ContentModelRef::setProperty(const std::string& propertyName, ContentProperty datamodel) {
	createData();

	mData->mProperties[propertyName] = datamodel;
}

void ContentModelRef::setProperty(const std::string& propertyName, const std::string& propertyValue) {
	createData();
	mData->mProperties[propertyName] = ContentProperty(propertyName, propertyValue);
}

void ContentModelRef::setProperty(const std::string& propertyName, const std::wstring& value) {
	ContentProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void ContentModelRef::setProperty(const std::string& propertyName, const int& value) {
	ContentProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void ContentModelRef::setProperty(const std::string& propertyName, const double& value) {
	ContentProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void ContentModelRef::setProperty(const std::string& propertyName, const float& value) {
	ContentProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void ContentModelRef::setProperty(const std::string& propertyName, const ci::Color& value) {
	ContentProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void ContentModelRef::setProperty(const std::string& propertyName, const ci::ColorA& value) {
	ContentProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void ContentModelRef::setProperty(const std::string& propertyName, const ci::vec2& value) {
	ContentProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void ContentModelRef::setProperty(const std::string& propertyName, const ci::vec3& value) {
	ContentProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

void ContentModelRef::setProperty(const std::string& propertyName, const ci::Rectf& value) {
	ContentProperty dp;
	dp.setName(propertyName);
	dp.setValue(value);
	setProperty(propertyName, dp);
}

const std::vector<ContentModelRef>& ContentModelRef::getChildren() const {
	if(!mData) return EMPTY_DATAMODELREF_VECTOR;
	return  mData->mChildren;
}

ContentModelRef ContentModelRef::getChild(const size_t index) {
	createData();

	if(index < mData->mChildren.size()) {
		return mData->mChildren[index];
	} else if(!mData->mChildren.empty()) {
		return mData->mChildren.back();
	}

	return EMPTY_DATAMODEL;
}

ContentModelRef ContentModelRef::getChildById(const int id) {
	createData();

	for(auto it : mData->mChildren) {
		if(it.getId() == id) return it;
	}

	return EMPTY_DATAMODEL;
}

ContentModelRef ContentModelRef::getChildByName(const std::string& childName) {
	createData();

	if(childName.find(".") != std::string::npos) {
		auto childrens = ds::split(childName, ".", true);
		if(childrens.empty()) {
			DS_LOG_WARNING("DataModelRef::getChild() Cannot find a child with the name \".\"");
		} else {
			ContentModelRef curChild = getChildByName(childrens.front());
			for(int i = 1; i < childrens.size(); i++) {
				curChild = curChild.getChildByName(childrens[i]);
			}
			return curChild;
		}
	}

	for(auto it : mData->mChildren) {
		if(it.getName() == childName) return it;
	}

	return EMPTY_DATAMODEL;
}

void ContentModelRef::addChild(ContentModelRef datamodel, const size_t index) {
	createData();

	if(index < mData->mChildren.size()) {
		mData->mChildren.insert(mData->mChildren.begin() + index, datamodel);
	} else {
		mData->mChildren.emplace_back(datamodel);
	}
}

bool ContentModelRef::hasChild(const std::string& name) {
	if(!mData || mData->mChildren.empty()) return false;

	for(auto it : mData->mChildren) {
		if(it.getName() == name) return true;
	}

	return false;
}

bool ContentModelRef::hasChildren() {
	if(!mData) return false;
	return !mData->mChildren.empty();
}

void ContentModelRef::setChildren(std::vector<ds::model::ContentModelRef> children) {
	createData();
	mData->mChildren = children;
}

void ContentModelRef::printTree(const bool verbose, const std::string& indent) {
	if(empty() || !mData) {
		DS_LOG_INFO(indent << "Empty DataModel.");
	} else {
		DS_LOG_INFO(indent << "DataModel id:" << mData->mId << " name:" << mData->mName);
		if(verbose) {

			for(auto it : mData->mProperties) {
				if(!it.second.getResource().empty()) {
					DS_LOG_INFO(indent << "          resource:" << it.second.getResource().getAbsoluteFilePath());
				} else {
					DS_LOG_INFO(indent << "          prop:" << it.first << " value:" << it.second.getValue());
				}
			}
		}

		if(!mData->mChildren.empty()) {

			for(auto it : mData->mChildren) {
				//	DS_LOG_INFO(indent << "          child:" << it.getName());
				it.printTree(verbose, indent + "  ");

			}
		}
	}
}

//----------- PRIVATE --------------//
void ContentModelRef::createData() {
	if(!mData) mData.reset(new Data());
}

} // namespace model
} // namespace ds
