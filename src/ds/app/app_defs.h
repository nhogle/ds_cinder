#pragma once
#ifndef DS_APP_APPDEFS_H_
#define DS_APP_APPDEFS_H_

#include <vector>
#include "ds/params/camera_params.h"

namespace ds {

typedef int                 sprite_id_t;
// System-defined illegal sprite
static const sprite_id_t    EMPTY_SPRITE_ID = 0;

// Blob termination for network traffic
static const char           TERMINATOR_CHAR = 0;

/**
 * \class ds::RootList
 * \brief Used during the app constructor to supply a list
 * of roots and any associated params. Example use:
 * AppSubclass() : App(ds::RootList().ortho().persp().ortho())
 * or
 * AppSubclass() : App(ds::RootList().ortho().persp().ortho())
 */
class RootList {
public:
	// Not explicit on purpose. Provide backwards compatibilty
	// for the old way of specifying roots.
	RootList(const std::vector<int>* roots = nullptr);

	bool							empty() const;

	// ADD ROOT TYPES.
	RootList&						ortho();
	RootList&						persp();

	// SET ROOT PARAMETERS. All of these apply to the currently active root.
	// Use OpenGL SELECT for picking.
	RootList&						pickSelect();
	
	RootList&						perspFov(const float);
	RootList&						perspPosition(const ci::Vec3f&);
	RootList&						perspTarget(const ci::Vec3f&);
	RootList&						perspNear(const float);
	RootList&						perspFar(const float);


private:
	friend class Engine;
	class Root {
	public:
		Root();

		enum Type					{ kOrtho, kPerspective };
		Type						mType;
		enum Pick					{ kDefault, kSelect };
		Pick						mPick;
		PerspCameraParams			mPersp;
	};
	std::vector<Root>				mRoots;
};

} // namespace ds

#endif // DS_APP_APPDEFS_H_