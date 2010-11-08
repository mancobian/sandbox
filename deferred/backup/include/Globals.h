/*
 *  Globals.h
 *  Samples
 *
 *  Created by Arris Ray on 6/25/09.
 *  Copyright 2009 The Secret Design Collective. All rights reserved.
 *
 */
 
#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>
#include <Ogre.h>

///////////////////////////////////////////////////////////////////////////////
// MACROS
///////////////////////////////////////////////////////////////////////////////

#define LOG(msg) Ogre::LogManager::getSingletonPtr()->logMessage(msg);

///////////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////////

#define SHOW_CONFIG 0
#define UNIT_SPHERE_SCALING_FACTOR (1.0f / 100.0f) * 2.0f  // Scaling factor to re-scale a prefab sphere to a unit sphere (with diameter = 1)
#define UNIT_CUBE_SCALING_FACTOR (1.0f / 100.0f)  // Scaling factor to re-scale a prefab sphere to a unit sphere (with diameter = 1)
#define LIMIT 500.0f
#define LIGHT_SCALE 250.0f
#define ENTITY_SCALE 10.0f

namespace tsdc {

namespace global {

template <typename T>
class IFactory
{
	public:
		virtual ~IFactory() {}
		
	public:
		virtual const char* Type() const = 0;
		virtual T* Create() const = 0;
		virtual void Destroy(T *value) const = 0;
}; // class Factory

}; // namespace global

namespace utils {
namespace io {

std::string ParseFileExtension(const std::string &filename);
bool HasFileExtension(const std::string &filename);

}; // namespace io
}; // namespace utils

}; // namespace tsdc

#endif // GLOBALS_H
