#ifndef OGREAPPLICATION_H
#define OGREAPPLICATION_H

#include <time.h>
#include <iostream>
#include <map>
#include <Ogre.h>
#include <OIS.h>
#include <OgreOSMScene.h>
#include "Globals.h"
#include "IApplication.h"

namespace tsdc {

class LightManager;
class EffectManager;
class DeferredLightingEffect;

class OgreApplication : 
	public tsdc::BaseApplication
	, public OIS::KeyListener
	//, public OIS::MouseListener
#if 0
	, public Ogre::MovableObject::Listener
#endif
{		
	public:
		OgreApplication();
		virtual ~OgreApplication();

	public:
		virtual void Start();
		virtual void Shutdown();
		virtual void LoadScene(const std::string &file);
		virtual void UnloadScene();	
		virtual void CaptureInput();
		
	public:
        inline Ogre::SceneManager* SceneManager() { return this->_scene_manager; }
		inline Ogre::Camera* Camera() { return this->_camera; }
		inline void SceneName(const std::string &value) { this->_scene_name = value; }
		inline std::string SceneName() const { return this->_scene_name; }

	public:
		virtual bool keyPressed(const OIS::KeyEvent &key_event_ref);
		virtual bool keyReleased(const OIS::KeyEvent &key_event_ref);
		virtual bool mouseMoved(const OIS::MouseEvent &mouse_event);
		virtual bool mousePressed(const OIS::MouseEvent &mouse_event, OIS::MouseButtonID id); 
		virtual bool mouseReleased(const OIS::MouseEvent &mouse_event, OIS::MouseButtonID id);

	protected:
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
		Ogre::String bundlePath();
#endif
		void Init();
		void InitOgre();
		void InitResourceSystem();
		void InitDisplay();
		void InitInput();
		void InitEffects();
		void InitLights();
		void InitCameraLight();
		void InitSceneResources();
		void InitShaderResources();
		void CreateObjects(
			const size_t NUM_X, 
			const size_t NUM_Z, // X/Z segments
			const std::string &mesh, 
			const std::string &material = "");
		void CreateLights(const size_t NUM_X, const size_t NUM_Z);
		void CreateScene();
		void MoveCamera();
		void UpdateStats(const double elapsed_time);
		virtual void Run();

	protected:
		static const char* SCENE_RESOURCE_GROUP;

	protected:
		bool _is_shutdown;
		size_t _num_frames,
			_num_lights_remaining;
		float _move_scale,
			_move_speed,
			_elapsed_time;
		std::string _application_title,
			_scene_name;
		OSMScene *_ofusion_scene_loader;
		Ogre::Degree _rotate_speed,
			_rotate_scale;
		Ogre::Vector3 _translation_vector;
		Ogre::Root* _root;
		Ogre::SceneManager* _scene_manager;
		Ogre::RenderWindow* _render_window;
		Ogre::Camera* _camera;
		Ogre::Viewport* _viewport;
		Ogre::Log* _log;
		Ogre::Timer* _timer;
		Ogre::CompositorInstance *_deferred_lighting_compositor;
		Ogre::Overlay* _debug_overlay;
		Ogre::Overlay* _info_overlay;
		Ogre::LightList _lights;
		Ogre::Texture *_light_colour_sampler;
		OIS::InputManager* _input_manager;
		OIS::Keyboard* _keyboard;
		//OIS::Mouse* _mouse;
		LightManager *_light_manager;
		EffectManager *_effect_manager;
		DeferredLightingEffect *_deferred_lighting_effect;
		Ogre::SceneNode *_scene_root_node,
			*_light_root_node,
			*_cam_node_pitch,
			*_cam_node_yaw,
			*_cam_light_node;
		
#if 0		
	public: // Ogre::MovableObject::Listener
		virtual bool objectRendering (const Ogre::MovableObject*, const Ogre::Camera*);
		virtual const Ogre::LightList* objectQueryLights (const Ogre::MovableObject*);
#endif
		
}; // class OgreApplication

}; // namespace tsdc

#endif // OGREAPPLICATION_H
