#pragma once
#ifndef DS_DATA_COLORLIST_H_
#define DS_DATA_COLORLIST_H_

#include <string>
#include <unordered_map>
#include "cinder/Color.h"

namespace ds {

	/**
	* \class ds::FontList
	* \brief A list of colors. Used to create a central color pallet usable by layout xmls. 
	*/
	class ColorList
	{
	public:
		ColorList();

		void                clear();
		bool				empty() const;

		// Short name can be supplied by the app and used to refer to colors from now on.
		// Often it might be something in a settings file.
		void                install(const ci::ColorA& color, const std::string& shortName = "");
		// Answer > 0 for a valid ID. Name can either be the file or short name, which should never conflict.
		int 						    getIdFromName(const std::string&) const;
		const ci::ColorA&  getColorFromId(const int id) const;
		// Clients give either a shortname and I give them a color
		const ci::ColorA&  getColorFromName(const std::string&) const;

	private:
		class Entry {
		public:
			Entry()           { }
			Entry(const ci::ColorA& color, const std::string& name) : mColor(color), mShortName(name) { }
			ci::ColorA		  mColor;
			std::string       mShortName;
		};
		std::unordered_map<int, Entry>
			mData;
	};

} // namespace ds

#endif // DS_DATA_COLORLIST_H_