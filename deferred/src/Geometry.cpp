/*
 *  Geometry.cpp
 *  Samples
 *
 *  Created by Arris Ray on 9/1/09.
 *  Copyright 2009 The Secret Design Collective. All rights reserved.
 *
 */

#include "Geometry.h"

using namespace tsdc;

template<> tsdc::GeometryManager* Ogre::Singleton<tsdc::GeometryManager>::ms_Singleton = 0;
const std::string tsdc::GeometryManager::UNIT_CUBE = "_unit_cube.mesh";
const std::string tsdc::GeometryManager::UNIT_PYRAMID = "_unit_pyramid.mesh";

GeometryManager::GeometryManager(Ogre::SceneManager *scene_manager) :
	_scene_manager(scene_manager)
{
}

GeometryManager::~GeometryManager()
{
}

Ogre::Entity* GeometryManager::CreateCube(const std::string &name)
{
	return this->CreateGeometryFromFile(name, GeometryManager::UNIT_CUBE);
}

Ogre::Entity* GeometryManager::CreatePyramid(const std::string &name)
{
	return this->CreateGeometryFromFile(name, GeometryManager::UNIT_PYRAMID);
}

Ogre::Entity* GeometryManager::CreateGeometryFromFile(const std::string &name, const std::string &file)
{
	// Error check: Require a scene manager
	assert (this->_scene_manager != NULL);
	
	// Create the unit cube
	// result.first: ResourcePtr
	// result.second: bool, indicator specifying whether the resource was newly created
	Ogre::MeshManager::ResourceCreateOrRetrieveResult result = Ogre::MeshManager::getSingletonPtr()->createOrRetrieve(file, 
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	if (result.first.isNull())
		return NULL;
		
	// Create new entity
	return this->_scene_manager->createEntity(name, file);
}

