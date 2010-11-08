/*
 *  Light.h
 *  Samples
 *
 *  Created by Arris Ray on 8/24/09.
 *  Copyright 2009 The Secret Design Collective. All rights reserved.
 *
 */
 
#include <Ogre.h>
#include "IDGen.h"

namespace tsdc {

class Light : public IDGen
{
	public:
		struct CtorParams
		{
			Ogre::Quaternion InitialSpotlightOrientation;
		};

	public:
		enum CommonLightShaderParameters
		{
			PARAM_LIGHT_POSITION = 0
			, PARAM_LIGHT_ATTENUATION
			, PARAM_LIGHT_MAX
		};

		enum SpotLightShaderParameters
		{
			PARAM_SPOTLIGHT_ORIENTATION = Light::PARAM_LIGHT_MAX
			, PARAM_SPOTLIGHT_RANGE
		};

		enum PointLightShaderParameters
		{
			PARAM_POINTLIGHT_RANGE = Light::PARAM_LIGHT_MAX
		};

	public:
		virtual ~Light();
		
	public:
		inline bool IsDirty() const { return this->_is_dirty; }
		inline void IsDirty(const bool value) { this->_is_dirty = value; }
		inline Ogre::Real SpotlightAngleOffset() const { return this->_spotlight_angle_offset; }
		inline Ogre::Light* OgreLight() const { return this->_light; }
		inline Ogre::Entity* BoundingVolume() const { return this->_bounding_volume; }
		inline Ogre::SceneNode* SceneNode() const { return this->_node; }
		
	public:
		void RenderQueueGroup(const unsigned char group_id);
		unsigned char RenderQueueGroup() const;
		void LightMaterial(Ogre::MaterialPtr material);
		Ogre::MaterialPtr LightMaterial();
		
	protected:
#if 0
		Light(const Light::CtorParams *params = NULL);
		Light(Ogre::Vector3 position,
			Ogre::Quaternion orientation,
			Ogre::Vector3 scale);
#endif
		Light(Ogre::Light *light, const Light::CtorParams *params = NULL);
		
	protected:
		void Create(Ogre::Light *light, const Light::CtorParams *params = NULL);
#if 0
		void Create(Ogre::Vector3 position = Ogre::Vector3::ZERO,
			Ogre::Quaternion orientation = Ogre::Quaternion::IDENTITY,
			Ogre::Vector3 scale = Ogre::Vector3::ZERO,
			const Light::CtorParams *params = NULL);
#endif
		Ogre::Entity* CreateBoundingVolume(Ogre::Light *light) const;
		
	protected:
		bool _is_dirty;
		Ogre::Real _spotlight_angle_offset; // TODO: Consider renaming this variable.
		Ogre::Light *_light;
		Ogre::MaterialPtr _light_material;
		Ogre::SceneNode *_node;
		Ogre::Entity *_bounding_volume;
		
	private:
		friend class LightManager;
}; // class Light

class LightManager : public Ogre::Singleton<LightManager>
{
	public:
		typedef std::list<Light*> LightList;
		
	public:
		static const unsigned char RENDER_QUEUE_LIGHTS;
		static const std::string POINT_LIGHT_MESH;
		static const std::string SPOT_LIGHT_MESH;
		static const std::string POINT_LIGHT_MATERIAL;
		static const std::string SPOT_LIGHT_MATERIAL;
		
	public:
		LightManager();
		~LightManager();
		
	public:
		inline Ogre::SceneManager* SceneManager() const { return this->_scene_manager; }
		inline void SceneManager(Ogre::SceneManager *value) { this->_scene_manager = value; }
		inline Ogre::SceneNode* LightRootNode() const { return this->_light_root_node; }
		inline void LightRootNode(Ogre::SceneNode *value) { this->_light_root_node = value; }
		
	public:
		Light* CreateLight(Ogre::Light *light, const Light::CtorParams *params = NULL);
		void DestroyLight(Light *light);
		Ogre::MaterialPtr LightMaterial(const Ogre::Light::LightTypes type) const;
		void LightMaterial(Ogre::MaterialPtr value, const Ogre::Light::LightTypes type);
		
	protected:
		void InitLightMaterials();
		void UpdateLights(const Ogre::Light::LightTypes type);
		
	protected:
		LightList _lights;
		Ogre::SceneManager *_scene_manager;
		Ogre::SceneNode *_light_root_node;
		Ogre::MaterialPtr _point_light_material,
			_spot_light_material;
};

}; // namespace tsdc
