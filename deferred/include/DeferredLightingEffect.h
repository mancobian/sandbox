/*
 *  DeferredLightingEffect.h
 *  Samples
 *
 *  Created by Arris Ray on 6/18/09.
 *  Copyright 2009 The Secret Design Collective. All rights reserved.
 *
 */
 
#ifndef DEFERREDLIGHTINGEFFECT_H
#define DEFERREDLIGHTINGEFFECT_H

#include <list>
#include <OGRE/Ogre.h>
#include <OgreOSMScene.h>
#include "Effect.h"
#include "Globals.h"

namespace tsdc {

class Light;

///////////////////////////////////////////////////////////////////////////////
// CLASS DECLARATION: DeferredLightingEffect
///////////////////////////////////////////////////////////////////////////////

class DeferredLightingEffect : 
	public Effect
	// , public Ogre::MaterialManager::Listener
	, public Ogre::RenderTargetListener
	, public Ogre::CompositorInstance::Listener
	, public Ogre::MovableObject::Listener
	, public Ogre::RenderQueueListener
	, public Ogre::RenderQueue::RenderableListener
{		
	public:
		typedef std::map<Ogre::MovableObject*, Ogre::Light*> RenderableToLightMap;
		typedef std::list<Ogre::Entity*> EntityList;
		
	public:
		static const unsigned char RENDER_QUEUE_DEBUG;
		static const std::string EFFECT_COMPOSITOR;
		static const std::string GBUFFER_STAGE_MATERIAL;
		static const std::string GBUFFER_STAGE_SCHEME;
		static const std::string GBUFFER_STAGE_TECHNIQUE;
		static const std::string GBUFFER_STAGE_PASS;
		static const std::string LIGHTS_STAGE_MATERIAL;
		static const std::string LIGHTS_STAGE_PASS;
		static const std::string OUTPUT_STAGE_MATERIAL;
		static const std::string OUTPUT_STAGE_PASS;
		static const std::string FRONTFACE_TECHNIQUE_NAME;
		static const std::string BACKFACE_TECHNIQUE_NAME;

    public:
        DeferredLightingEffect();
        virtual ~DeferredLightingEffect();
		
	public:
		inline bool IsEnabled() const { return (this->_compositor_instance) ? this->_compositor_instance->getEnabled() : false; }
		inline void SceneRootNode(Ogre::SceneNode *value) { this->_scene_root_node = value; }
		inline void LightRootNode(Ogre::SceneNode *value) { this->_light_root_node = value; }
		inline Ogre::MaterialPtr PointLightMaterial() const { return this->_point_light_material; }
		
	public:
		virtual void Create(Ogre::Viewport *viewport);
		virtual void Destroy();
		virtual void Show();
		virtual void Hide();
		void AddLight(Light *light);
		void AddEntity(Ogre::Entity *entity);
		void AddEntities(OSMScene::EntityList &entities);
		
	public:
		virtual bool objectRendering(const Ogre::MovableObject *, const Ogre::Camera *);
		
	public: // Ogre::RenderQueue::RenderableListener
		virtual bool renderableQueued (Ogre::Renderable *rend
			, Ogre::uint8 groupID
			, Ogre::ushort priority
			, Ogre::Technique **ppTech
			, Ogre::RenderQueue *pQueue);
		
	public: // Ogre::RenderQueueListener
		virtual void renderQueueStarted(Ogre::uint8 id, const Ogre::String &invocation, bool &skipThisQueue);
		virtual void renderQueueEnded(Ogre::uint8 id, const Ogre::String &invocation, bool &repeatThisQueue);
		
	public: // Ogre::RenderTargetListener
		void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
		void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
		
#if 0
	public: // Ogre::MaterialManager::Listener
		virtual Ogre::Technique* handleSchemeNotFound (unsigned short schemeIndex, 
			const Ogre::String &schemeName, 
			Ogre::Material *originalMaterial, 
			unsigned short lodIndex, 
			const Ogre::Renderable *rend);
#endif
			
	public: // Ogre::CompositorInstance::Listener
		virtual void notifyMaterialSetup (Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
		virtual void notifyMaterialRender (Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
		
	protected:
		void AttachListeners();
		void DetachListeners();
		void UpdateLightRenderable(Ogre::Renderable *renderable) const;
		void UpdatePointLightParams(Ogre::Renderable *rend) const;
		void UpdateCommonLightParams(Ogre::Renderable *rend) const;
		void UpdateSpotLightParams(Ogre::Renderable *rend) const;
		void assignOutputStageTextureInputs(Ogre::uint32 pass_id, Ogre::MaterialPtr &mat);
		
	protected:
		enum EffectPassIds
		{
			EARLY_Z = 1,
			SCENE = 2,
			POINT_LIGHT = 3
		};
		
	protected:
		static const std::string LIGHTING_RENDER_TARGET_NAME;
			
	protected:
		Ogre::Viewport *_viewport;
		Ogre::Camera *_camera;
		Ogre::SceneManager *_scene_manager;
		Ogre::MaterialPtr _gbuffer_material,
			_point_light_material,
			_earlyz_material;
		Ogre::Technique *_gbuffer_technique,
			*_earlyz_technique;
		Ogre::CompositorInstance *_compositor_instance;
		EntityList _light_entities,
			_effect_entities;
		RenderableToLightMap _renderable_to_light_map;
		Ogre::SceneNode *_scene_root_node,
			*_light_root_node;
}; // class DeferredLightingEffect

///////////////////////////////////////////////////////////////////////////////
// CLASS DECLARATION: DeferredLightingEffectFactory
///////////////////////////////////////////////////////////////////////////////

class DeferredLightingEffectFactory : 
	public tsdc::global::IFactory<Effect>
	, public Ogre::Singleton<DeferredLightingEffectFactory>
{
	public:
		virtual ~DeferredLightingEffectFactory() {}
		
	public:
		const char* Type() const { return "DEFERRED_LIGHTING_EFFECT"; }
		virtual Effect* Create() const { return new DeferredLightingEffect(); }
		virtual void Destroy(Effect *value) const { delete value; }
}; // class DeferredLightingEffectFactory

}; // namespace tsdc

#endif // DEFERREDLIGHTINGEFFECT_H
