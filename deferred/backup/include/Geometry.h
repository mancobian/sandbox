/*
 *  Geometry.h
 *  Samples
 *
 *  Created by Arris Ray on 9/1/09.
 *  Copyright 2009 The Secret Design Collective. All rights reserved.
 *
 */
 
#include <Ogre.h>

namespace tsdc {

class GeometryManager : public Ogre::Singleton<GeometryManager>
{
	public:
		static const std::string UNIT_CUBE;
		static const std::string UNIT_PYRAMID;
		
	public:
		GeometryManager(Ogre::SceneManager *scene_manager = NULL);
		virtual ~GeometryManager();
		
	public:
		inline Ogre::SceneManager* SceneManager() const { return this->_scene_manager; }
		inline void SceneManager(Ogre::SceneManager *value) { this->_scene_manager = value; }
		
	public:
		Ogre::Entity* CreateCube(const std::string &name);
		Ogre::Entity* CreatePyramid(const std::string &name);
		
	protected:
		Ogre::SceneManager *_scene_manager;
		
	protected:
		Ogre::Entity* CreateGeometryFromFile(const std::string &name, const std::string &file);
};

}; // namespace tsdc