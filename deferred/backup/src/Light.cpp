/*
 *  Light.cpp
 *  Samples
 *
 *  Created by Arris Ray on 8/24/09.
 *  Copyright 2009 The Secret Design Collective. All rights reserved.
 *
 */

#include "Light.h"
#include <cstdio>
#include <sstream>
#include "Globals.h"
#include "Geometry.h"

using namespace std;
using namespace tsdc;
using namespace tsdc::global;

///////////////////////////////////////////////////////////////////////////////
// STATIC MEMBER INITIALIZATION
///////////////////////////////////////////////////////////////////////////////

template<> tsdc::LightManager* Ogre::Singleton<tsdc::LightManager>::ms_Singleton = 0;
const unsigned char tsdc::LightManager::RENDER_QUEUE_LIGHTS = 20;
const std::string tsdc::LightManager::POINT_LIGHT_MESH = "_unit_cube.mesh";
const std::string tsdc::LightManager::SPOT_LIGHT_MESH = "_unit_pyramid.mesh";
const std::string tsdc::LightManager::POINT_LIGHT_MATERIAL = "tsdc/material/point_light";
const std::string tsdc::LightManager::SPOT_LIGHT_MATERIAL = "tsdc/material/spot_light";

///////////////////////////////////////////////////////////////////////////////
// CLASS IMPLEMENTATION: Light
///////////////////////////////////////////////////////////////////////////////

#if 0
Light::Light(const Light::CtorParams *params) :
	_light(NULL)
	, _light_material(NULL)
	, _node(NULL)
	, _bounding_volume(NULL)
{
	this->Create();
}

Light::Light(Ogre::Vector3 position,
	Ogre::Quaternion orientation,
	Ogre::Vector3 scale) :
	_light(NULL)
	, _node(NULL)
	, _bounding_volume(NULL)
{
	this->Create(position, orientation, scale);
}
#endif

Light::Light(Ogre::Light *light, const Light::CtorParams *params) :
	_is_dirty(false)
	, _spotlight_angle_offset(0.0f)
	, _light(light)
	, _node(light->getParentSceneNode())
	, _bounding_volume(NULL)
{
	this->Create(light, params);
}

Light::~Light()
{
	// Local vars
	LightManager *light_manager = LightManager::getSingletonPtr();
	Ogre::SceneManager *scene_manager = light_manager->SceneManager();
	
	// Destroy light bounding volume mesh
	if (this->_bounding_volume)
	{
		scene_manager->destroyEntity(this->_bounding_volume);
		this->_bounding_volume = NULL;
	}
}

void Light::RenderQueueGroup(const unsigned char group_id)
{
	assert (this->_bounding_volume != NULL);
	this->_bounding_volume->setRenderQueueGroup(group_id);
}

unsigned char Light::RenderQueueGroup() const
{
	assert (this->_bounding_volume != NULL);
	return this->_bounding_volume->getRenderQueueGroup();
}

void Light::Create(Ogre::Light *light, const Light::CtorParams *params)
{
	// Error checking: Require an Ogre light and associated scene node
	assert (light != NULL);
	assert (light->getParentSceneNode() != NULL);
	
	// Create light bounding volume
	this->_bounding_volume = this->CreateBoundingVolume(this->_light);

	// Configure light and its bounding mesh to the scene
	this->_node = this->_light->getParentSceneNode()->createChildSceneNode();
	this->_node->attachObject(this->_bounding_volume);

	// Configure light properties
	switch (this->_light->getType())
	{
		case Ogre::Light::LT_SPOTLIGHT:
		{
#if 0
			LOG ("Light <" + this->_light->getName() + "> has an inner angle of " + Ogre::StringConverter::toString(this->_light->getSpotlightInnerAngle().valueRadians()) + " radians.");
			LOG ("Light <" + this->_light->getName() + "> has an outer angle of " + Ogre::StringConverter::toString(this->_light->getSpotlightOuterAngle().valueRadians()) + " radians.");
#endif

			// DEPRECATED: Calculate the spotlight angle offset
			// NOTE: This is used to guarantee a smooth dropoff to 0.0 at the
			// outer edge of the light's bounding volume when calculating
			// light intensity at a given point inside the spotlight cone's radius
			this->_spotlight_angle_offset = 0.0f; //Ogre::Math::HALF_PI - this->_light->getSpotlightOuterAngle().valueRadians();

			// Configure light bounding volume scale
			Ogre::Real yscale = this->_light->getAttenuationRange();
			Ogre::Real xzscale = yscale * Ogre::Math::Tan(this->_light->getSpotlightOuterAngle().valueRadians());
			xzscale *= 2.0f; // NOTE: xzscale is calculated as the radius of the bounding volume's base in the line above!
#if 0
			LOG ("Light <" + this->_light->getName() + "> tan(light outer angle): " + Ogre::StringConverter::toString(Ogre::Math::Tan(this->_light->getSpotlightOuterAngle())));
			fprintf (stdout, "SPOTLIGHT: %s\n", this->_light->getName().c_str());
			fprintf (stdout, "\tRange: %f\n", this->_light->getAttenuationRange());
			fprintf (stdout, "\tInner angle: %f\n", this->_light->getSpotlightInnerAngle().valueDegrees());
			fprintf (stdout, "\tOuter angle: %f\n", this->_light->getSpotlightOuterAngle().valueDegrees());
			fprintf (stdout, "\tFalloff (inner->outer angle): %f\n", this->_light->getSpotlightFalloff());
			fprintf (stdout, "\ty-scale: %f\n", yscale);
			fprintf (stdout, "\txz-scale: %f\n", xzscale);
			fprintf (stdout, "\ttan() of outer angle: %f\n", Ogre::Math::Tan(this->_light->getSpotlightInnerAngle().valueDegrees()));
#endif
			this->_node->setScale(xzscale, yscale, xzscale);
			break;
		}
		case Ogre::Light::LT_POINT:
		case Ogre::Light::LT_DIRECTIONAL:
		default:
		{
			// Scale bounding volume
			Ogre::Vector3 scale = (Ogre::Vector3::UNIT_SCALE * this->_light->getAttenuationRange());
			this->_node->setScale(scale);
			break;
		}
	}
}

#if 0
void Light::Create(Ogre::Vector3 position, 
	Ogre::Quaternion orientation,
	Ogre::Vector3 scale,
	const Light::CtorParams *params)
{	
	// Error checking: No pre-existing light should exist!
	assert (this->_light == NULL);
	
	// Local vars
	LightManager *light_manager = LightManager::getSingletonPtr();
	Ogre::SceneManager *scene_manager = light_manager->SceneManager();
	Ogre::Real constant = 1.0f,
		linear = 0.7f,
		quadratic = 1.8f;
		
	// Create and configure light
	// NOTE: Assume uniform scaling!
	std::string light_id = this->GenerateID("tsdc_light_");
	this->_light = scene_manager->createLight(light_id);
	this->_light->setType(Ogre::Light::LT_POINT);
	this->_light->setDiffuseColour(1.0f, 1.0f, 1.0f);
	this->_light->setSpecularColour(1.0f, 1.0f, 1.0f);
	this->_light->setAttenuation(scale.x, constant, linear, quadratic);
	
	// Create light bounding volume
	this->_bounding_volume = this->CreateBoundingVolume(this->_light);
	
	// Create light parent scene node
	this->_node = scene_manager->getRootSceneNode()->createChildSceneNode();
	this->_node->setPosition(position);
	this->_node->setOrientation(orientation);
	scale *= UNIT_CUBE_SCALING_FACTOR;
		
	// Attach light and its bounding mesh to the scene
	this->_node->attachObject(this->_light);
	this->_node->attachObject(this->_bounding_volume);
	this->_node->setScale(scale);
	this->_node->setDebugDisplayEnabled(true);
}
#endif

Ogre::Entity* Light::CreateBoundingVolume(Ogre::Light *light) const
{
	// Local vars
	LightManager *light_manager = LightManager::getSingletonPtr();
	
	// Create light volume bounding mesh
	Ogre::String material_name;
	Ogre::Entity *bounding_volume = NULL;
	std::string entity_id = this->GenerateID("tsdc_entity_");
	switch (light->getType())
	{
		case Ogre::Light::LT_SPOTLIGHT:
		{
			// Configure light bounding volume mesh
			material_name = light_manager->LightMaterial(Ogre::Light::LT_SPOTLIGHT)->getName();
			bounding_volume = LightManager::getSingletonPtr()->SceneManager()->createEntity(entity_id, LightManager::SPOT_LIGHT_MESH);
			bounding_volume->setMaterialName(material_name);
			bounding_volume->setRenderQueueGroup(LightManager::RENDER_QUEUE_LIGHTS);
			break;
		}
		case Ogre::Light::LT_POINT:
		case Ogre::Light::LT_DIRECTIONAL:
		default:
		{
			// Configure light bounding volume mesh
			Ogre::String material_name = light_manager->LightMaterial(Ogre::Light::LT_POINT)->getName();
			bounding_volume = LightManager::getSingletonPtr()->SceneManager()->createEntity(entity_id, LightManager::POINT_LIGHT_MESH);
			bounding_volume->setMaterialName(material_name);
			bounding_volume->setRenderQueueGroup(LightManager::RENDER_QUEUE_LIGHTS);
			break;
		}
	}
#if 0
	std::string msg = "Created light bounding volume mesh with ID ('";
	msg.append(entity_id);
	msg.append("') for light ID ('");
	msg.append(light->getName());
	msg.append("') with material ID ('");
	msg.append(light_manager->LightMaterial(Ogre::Light::LT_POINT)->getName());
	msg.append("')!");
	Ogre::LogManager::getSingletonPtr()->logMessage(msg.c_str());
	LOG ("Light <" + this->_light->getName() + "> assigned material <" + material_name + ">");
#endif

	// Assign the light to its bounding mesh
	std::stringstream msg;
	msg << "[OGRE] Assigning light <" << this->_light->getName();
	msg << "> to bounding volume subentities <" << bounding_volume->getName() << ">" << endl;
	Ogre::LogManager::getSingleton().logMessage(msg.str());
	for (unsigned i=0; i<bounding_volume->getNumSubEntities(); ++i)
	{
		Ogre::SubEntity *subentity = bounding_volume->getSubEntity(i);
		subentity->setUserAny(Ogre::Any(const_cast<const tsdc::Light*>(this)));

		msg << "[OGRE]\t=> Subentity material: " << subentity->getMaterialName() << endl;
		Ogre::LogManager::getSingleton().logMessage(msg.str());
		msg.str("");
	}
	return bounding_volume;
}

void Light::LightMaterial(Ogre::MaterialPtr material)
{
	// Assign light bounding volume mesh material
	//Ogre::LogManager::getSingleton().logMessage("Light::LightMaterial(): " + material->getName());
	assert (this->_bounding_volume != NULL);
	this->_bounding_volume->setMaterialName(material->getName());

	// Store a reference to the assigned light material
	this->_light_material.setNull();
	this->_light_material = material;
}

Ogre::MaterialPtr Light::LightMaterial()
{
	// Get light bounding volume mesh material
	return this->_light_material;
}

///////////////////////////////////////////////////////////////////////////////
// CLASS IMPLEMENTATION: LightManager
///////////////////////////////////////////////////////////////////////////////

LightManager::LightManager() :
	_scene_manager(NULL)
	, _light_root_node(NULL)
	, _point_light_material(NULL)
	, _spot_light_material(NULL)
{
}

LightManager::~LightManager()
{
	// Destroy all lights
	LightList::iterator iter = this->_lights.begin(),
		end = this->_lights.end();
	while (iter != end)
	{
		Light *light = *iter;
		delete light;
		++iter;
	}
	this->_lights.clear();
	
	// Release smart pointer reference(s)
	this->_point_light_material.setNull();
	this->_spot_light_material.setNull();
}

Light* LightManager::CreateLight(Ogre::Light *light, const Light::CtorParams *params)
{
	// Load light materials, if necessary
	if (this->_point_light_material.isNull() && this->_spot_light_material.isNull())
		this->InitLightMaterials();

	// Create new light
	Light *new_light = new Light(light, params);
	this->_lights.push_back(new_light);
	// Ogre::LogManager::getSingleton().logMessage(light->getName());
	// Ogre::LogManager::getSingleton().logMessage(this->_point_light_material->getName());
	return new_light;
}

void LightManager::DestroyLight(Light *light)
{
	this->_lights.remove(light);
	delete light;
}

void LightManager::InitLightMaterials()
{
	// Error check: Require Ogre's MaterialManager
	assert (Ogre::MaterialManager::getSingletonPtr() != NULL);
	assert (this->_point_light_material.isNull() && "Point light material exists!");
	assert (this->_spot_light_material.isNull() && "Spot light material exists!");

	// Store references to light materials
	this->_point_light_material = Ogre::MaterialManager::getSingletonPtr()->getByName(LightManager::POINT_LIGHT_MATERIAL);
	this->_spot_light_material = Ogre::MaterialManager::getSingletonPtr()->getByName(LightManager::SPOT_LIGHT_MATERIAL);
	LOG ("Initialized light materials!");

	// Error check: Require that the light materials exist
	assert (!this->_point_light_material.isNull() && "Point light material does not exist!");
	assert (!this->_spot_light_material.isNull() && "Spot light material does not exist!");
}

Ogre::MaterialPtr LightManager::LightMaterial(const Ogre::Light::LightTypes type) const
{
	switch (type)
	{
		case Ogre::Light::LT_SPOTLIGHT:
		{
			return this->_spot_light_material;
		}
		case Ogre::Light::LT_POINT:
		case Ogre::Light::LT_DIRECTIONAL:
		default:
		{
			return this->_point_light_material;
		}
	}
	return Ogre::MaterialPtr(NULL);
}

void LightManager::LightMaterial(Ogre::MaterialPtr material, const Ogre::Light::LightTypes type)
{
	switch (type)
	{
		case Ogre::Light::LT_SPOTLIGHT:
		{
			this->_spot_light_material = material;
			break;
		}
		case Ogre::Light::LT_POINT:
		case Ogre::Light::LT_DIRECTIONAL:
		default:
		{
			this->_point_light_material = material;
			break;
		}
	}

	Ogre::LogManager::getSingleton().logMessage("LightManager::LightMaterial(): " + material->getName());
	this->UpdateLights(type);
}

void LightManager::UpdateLights(const Ogre::Light::LightTypes type)
{
	LightList::iterator iter = this->_lights.begin(),
		end = this->_lights.end();
	while (iter != end)
	{
		Light *light = *iter;
		//Ogre::LogManager::getSingleton().logMessage("LightManager::LightMaterial(): " + this->_light_material->getName());
		switch (type)
		{
			case Ogre::Light::LT_SPOTLIGHT:
			{
				light->LightMaterial(this->_spot_light_material);
				break;
			}
			case Ogre::Light::LT_POINT:
			case Ogre::Light::LT_DIRECTIONAL:
			default:
			{
				light->LightMaterial(this->_point_light_material);
				break;
			}
		}
		++iter;
	}
}
