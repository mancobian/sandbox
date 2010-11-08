#include "OgreApplication.h"
#include "Light.h"
#include "Geometry.h"
#include "Effect.h"
#include "DeferredLightingEffect.h"

using namespace std;
using namespace Ogre;
using namespace tsdc;
using namespace tsdc::global;

const char* OgreApplication::SCENE_RESOURCE_GROUP = "Scene";

OgreApplication::OgreApplication() :
	_is_shutdown(false)
	, _num_frames(0)
	, _num_lights_remaining(0)
	, _move_speed(5.0f)
	, _elapsed_time(0.0f)
	, _application_title("OgreApplication v1.0")
	, _rotate_speed(0.1f)
{	
	// Create custom managers
	new LightManager();
	new EffectManager();
	new GeometryManager();
}

OgreApplication::~OgreApplication()
{	
	// Destroy custom manager(s)
	delete GeometryManager::getSingletonPtr();
	delete LightManager::getSingletonPtr();
	delete EffectManager::getSingletonPtr();
	
	// Cleanup the application environment
	this->Shutdown();

	// Enable key-repeat
	system("xset r on");
}

void OgreApplication::Start()
{
	this->Init();
	
	// Start!
	this->_log->logMessage("=== START ===");
	this->Run();
	this->_log->logMessage("=== END ===");
}

void OgreApplication::Shutdown()
{	
	// Destroy oFusion scene loader
	delete this->_ofusion_scene_loader;
	this->_ofusion_scene_loader = NULL;
	
	// Destroy input
	this->_input_manager->destroyInputObject(this->_keyboard);
	//this->_input_manager->destroyInputObject(this->_mouse);
	OIS::InputManager::destroyInputSystem(this->_input_manager);

	// Destroy Ogre root object
	this->_log->logMessage("Application shutdown successfully!");
	delete this->_timer;
	delete this->_root;
}

void OgreApplication::CreateObjects(
    const size_t NUM_X, 
	const size_t NUM_Z, // X/Z segments
    const std::string &mesh, 
	const std::string &material)
{
    Ogre::SceneNode *rootNode = this->_scene_manager->getRootSceneNode();

    for (size_t x = 0; x < NUM_X; ++x) {
        for (size_t z = 0; z < NUM_Z; ++z) {
            Ogre::Vector3 p(
                Ogre::Math::RangeRandom(-LIMIT, LIMIT),
                Ogre::Math::RangeRandom(0, LIMIT),
                Ogre::Math::RangeRandom(-LIMIT, LIMIT));

            Ogre::Entity *ent = this->_scene_manager->createEntity(
                mesh + Ogre::StringConverter::toString(x) + "_" +
                Ogre::StringConverter::toString(z), mesh);

            Ogre::SceneNode *node = rootNode->createChildSceneNode();
            node->attachObject(ent);
            node->setPosition(p);

            // scale from Ogre examples centimeters to meters
			float scale = UNIT_SPHERE_SCALING_FACTOR * ENTITY_SCALE;
            node->setScale(Ogre::Vector3(scale, scale, scale));
        }
    }
}

void OgreApplication::CreateLights(const size_t NUM_X, const size_t NUM_Z)
{	
#if 0
	// Create scene lights
	Ogre::SceneNode *rootNode = this->_scene_manager->getRootSceneNode();
	for (size_t x = 0; x < NUM_X; ++x) 
	{
        for (size_t z = 0; z < NUM_Z; ++z) 
		{
			// Store new light entity
			//this->_light_entities.push_back(ent);
		}
	}
#endif
}

void OgreApplication::CreateScene()
{
	// create a simply "plane" mesh programmatically
    Ogre::MeshPtr planeMesh = Ogre::MeshManager::getSingleton().createPlane(
        "plane.mesh", // simulate actual ".mesh", "roflcopter cleared for landing, over"
        OgreApplication::SCENE_RESOURCE_GROUP, // in the SCENE_RESOURCE_GROUP group
        Ogre::Plane(Ogre::Vector3::UNIT_Y, 0), // plane facing up
        100, 100, // world coordinates.  we want it nice and big
        1, 1, // number of segments...  might want to tesselate it if you wish
        true, // yes, we need normals for proper lighting
        1, // one texture coordinate set
        1, 1, // UV tiling (texture coordinates)
        Ogre::Vector3::NEGATIVE_UNIT_Z); // up vector.  if it's facing up, then it's up is -Z
    // make sure it loaded
    planeMesh->load();

    //Ogre::SceneNode *rootNode = this->_scene_manager->getRootSceneNode();

    // make an entity for this plane
    Ogre::Entity *ent = this->_scene_manager->createEntity("planeEnt", "plane.mesh");
    ent->setMaterialName("metal");
    // make a node for it, scale to 10 by 10 meters
    //Ogre::SceneNode *node = rootNode->createChildSceneNode();

    // create some random objects
    const size_t NUM_X = 5, 
		NUM_Z = 5;

    this->CreateObjects(NUM_X, NUM_Z, "knot.mesh");
    this->CreateObjects(NUM_X, NUM_Z, "ogrehead.mesh");
	this->CreateLights(10, 10);

    // random position I found to be pretty
    this->_camera->setPosition(Ogre::Vector3(110.0, 140.0, 230.0));
	this->_camera->lookAt(-15.0f, 25.0f, 0.0f);
	this->_camera->setFarClipDistance(1000.0f);
#if 0
	// Get a reference to the debug material & technique
	Ogre::MaterialPtr debug_material = Ogre::MaterialManager::getSingleton().getByName(OgreApplication::DEBUG_MATERIAL_NAME);
	for (unsigned short i=0; i<debug_material->getNumTechniques(); ++i)
	{
		Ogre::Technique *technique = debug_material->getTechnique(i);
		if (technique->getSchemeName().compare(OgreApplication::DEBUG_SCHEME_NAME) == 0)
		{
			this->_debug_technique = technique;
			break;
		}
	}
	assert (this->_debug_technique != NULL);
	this->_debug_material = debug_material.get();
	this->_debug_material->load();
#endif
}

void OgreApplication::Init()
{
	// Init Ogre environment
	this->InitOgre();
	
	// Init display
	this->InitDisplay();

	// Initialize input
	this->InitInput();

	// Initialize scene resources
	this->InitSceneResources();

	// Initialize effects
	this->InitEffects();
	
	// Create the scene
	if (!this->_scene_name.empty())
		this->LoadScene(this->_scene_name);
	else
		this->CreateScene();

	// Reset scene vars
	this->_is_shutdown = false;
}

void OgreApplication::InitOgre()
{
	// Create log manager
	new Ogre::LogManager();	
	this->_log = Ogre::LogManager::getSingleton().createLog("config/ogre.log", true, true, false);
	this->_log->setDebugOutputEnabled(true);
	
	// Create Ogre root object
	this->_root = new Ogre::Root("config/plugins.cfg", "config/ogre.cfg");

	// Create scene manager
	this->_scene_manager = this->_root->createSceneManager(Ogre::ST_GENERIC, "MainScene");

	// Setup app configuration and initialize app
#if 1
	if (!this->_root->restoreConfig())
	{
		if (!this->_root->showConfigDialog())
		{
			assert (false && "Unable to load or create Ogre config!");
			exit(1);
		}
	}
#else
	this->_root->showConfigDialog();
#endif

	// Create timer
	this->_timer = new Ogre::Timer();
	this->_timer->reset();
	
	// Configure geometry manager
	GeometryManager::getSingletonPtr()->SceneManager(this->_scene_manager);

	// Init global resource system
	this->InitResourceSystem();
}

void OgreApplication::InitResourceSystem()
{
	ResourceGroupManager::getSingletonPtr()->createResourceGroup(OgreApplication::SCENE_RESOURCE_GROUP);
}

void OgreApplication::InitDisplay()
{
	// Create the main display window
	this->_render_window = this->_root->initialise(true, this->_application_title.c_str());
	assert (this->_render_window->isAutoUpdated());

	// Create camera and node
	this->_cam_node_yaw = this->_scene_manager->getRootSceneNode()->createChildSceneNode("_tsdc_cam_node_yaw");
	this->_cam_node_pitch = this->_cam_node_yaw->createChildSceneNode("_tsdc_cam_node_pitch");
	this->_camera = this->_scene_manager->createCamera("Camera");
	this->_cam_node_pitch->attachObject(this->_camera);

	// Configure main camera
	this->_cam_node_yaw->setPosition(Vector3(460.0f, 22.0f, -360.0f));
	//this->_cam_node_yaw->lookAt(Vector3(-624.0f, 300.0f, 1227.0f), Ogre::Node::TS_WORLD);
	this->_camera->setNearClipDistance(1.0f);
	this->_camera->setFarClipDistance(10000.0f);
    this->_camera->setFOVy(Ogre::Degree(55.0f));

	// Create main viewport
	this->_viewport = this->_render_window->addViewport(this->_camera);
	this->_viewport->setBackgroundColour(Ogre::ColourValue(0.85, 0.85, 0.0, 0.0));

	// Configure camera aspect ratio
	this->_camera->setAspectRatio(Real(this->_viewport->getActualWidth()) / Real(this->_viewport->getActualHeight()));
	this->_viewport->setCamera(this->_camera);

	// Active main render window
	this->_render_window->setActive(true);
}

void OgreApplication::InitInput()
{
	// Customize window attributes
	unsigned long hWnd = 0;
    OIS::ParamList paramList;
    this->_render_window->getCustomAttribute("WINDOW", &hWnd);
	paramList.insert(OIS::ParamList::value_type("WINDOW", Ogre::StringConverter::toString(hWnd)));

	// Create input objects
	this->_input_manager = OIS::InputManager::createInputSystem(paramList);
    this->_keyboard = static_cast<OIS::Keyboard*>(this->_input_manager->createInputObject(OIS::OISKeyboard, true));
	//this->_mouse = static_cast<OIS::Mouse*>(this->_input_manager->createInputObject(OIS::OISMouse, true));
    
	// Configure input objects
	//this->_mouse->getMouseState().height = this->_render_window->getHeight();
	//this->_mouse->getMouseState().width = this->_render_window->getWidth();
	//this->_mouse->setEventCallback(this);
	this->_keyboard->setEventCallback(this);
}

void OgreApplication::InitEffects()
{
	// Create effect factory(s)
	new DeferredLightingEffectFactory();
	
	// Configure effect manager
	EffectManager::getSingletonPtr()->RegisterEffect(DeferredLightingEffectFactory::getSingletonPtr());
	
	// Create deferred lighting effect
	this->_deferred_lighting_effect = dynamic_cast<DeferredLightingEffect*>(EffectManager::getSingletonPtr()->CreateEffect(
		DeferredLightingEffectFactory::getSingletonPtr()->Type()) );
	this->_deferred_lighting_effect->Create(this->_viewport);
	this->_deferred_lighting_effect->Show();
	
	// Configure light manager
	LightManager::getSingletonPtr()->SceneManager(this->_scene_manager);
	this->InitLights();
}

void OgreApplication::InitLights()
{
	// Set light root node
	LightManager::getSingletonPtr()->LightRootNode( this->_light_root_node );
#if 1
	// Create deferred lighting light wrapper
	OSMScene::LightList &lights = this->_ofusion_scene_loader->getLightList();
	OSMScene::LightList::iterator iter = lights.begin(),
		end = lights.end();
	while (iter != end)
	{
		// Local vars
		const float SCALE_SCALAR = 5000.0f;
		Light::CtorParams params;
		Ogre::Radian inner_angle(Ogre::Degree(5.0f));
		Ogre::Radian outer_angle(Ogre::Degree(45.0f));
		Ogre::Light *ogre_light = *iter;

		// HACK: Re-configure selected spotlight params
		ogre_light->setDirection(Ogre::Vector3::NEGATIVE_UNIT_Y);
		ogre_light->setSpotlightInnerAngle(inner_angle);
		ogre_light->setSpotlightOuterAngle(outer_angle);
		ogre_light->setAttenuation(SCALE_SCALAR,
			ogre_light->getAttenuationConstant(),
			ogre_light->getAttenuationLinear(),
			ogre_light->getAttenuationQuadric());

		// Create deferred light wrapper
		Light *light = LightManager::getSingletonPtr()->CreateLight(ogre_light, &params);
		light->BoundingVolume()->setListener(this->_deferred_lighting_effect);
		this->_deferred_lighting_effect->AddLight(light);
		++iter;
	}
#endif
	// TODO: Add controls to enable/disable at runtime
	this->InitCameraLight();
}

void OgreApplication::InitCameraLight()
{
	// Local vars
	const float SCALE_SCALAR = 5000.0f;
	Ogre::Radian inner_angle(Ogre::Degree(5.0f));
	Ogre::Radian outer_angle(Ogre::Degree(25.0f));
	Light::CtorParams params;

	// Manually create Ogre light
	Ogre::Light *ogre_light = this->_scene_manager->createLight("_tsdc_cam_light");
	ogre_light->setType(Ogre::Light::LT_SPOTLIGHT);
	ogre_light->setSpotlightInnerAngle(inner_angle);
	ogre_light->setSpotlightOuterAngle(outer_angle);
	ogre_light->setDirection(Ogre::Vector3::NEGATIVE_UNIT_Z);
	ogre_light->setAttenuation(SCALE_SCALAR,
		ogre_light->getAttenuationConstant(),
		ogre_light->getAttenuationLinear(),
		ogre_light->getAttenuationQuadric());

	// Attach cam light bounding volume to cam node
	this->_cam_light_node = this->_cam_node_pitch->createChildSceneNode("_tsdc_cam_light_node");
	this->_cam_light_node->attachObject(ogre_light);

	// Create TSDC light
	Light *light = LightManager::getSingletonPtr()->CreateLight(ogre_light, &params);
	light->BoundingVolume()->setListener(this->_deferred_lighting_effect);
	this->_deferred_lighting_effect->AddLight(light);

	// Update camlight pos/ori
	Ogre::Quaternion rot = Ogre::Vector3::NEGATIVE_UNIT_Y.getRotationTo(Ogre::Vector3::NEGATIVE_UNIT_Z);
	light->BoundingVolume()->getParentSceneNode()->rotate(rot);
	this->_cam_light_node->translate(Ogre::Vector3::NEGATIVE_UNIT_Z * 50.0f, Ogre::Node::TS_LOCAL);
}

void OgreApplication::InitSceneResources()
{
	// Configure scene properties
	this->_scene_manager->setAmbientLight(Ogre::ColourValue(0.7, 0.7, 0.7));

	// Initialize scene resources from configuration file
	Ogre::String secName, typeName, archName;
	Ogre::ConfigFile cf;

#if 0
	Ogre::String mResourcePath;
	mResourcePath = bundlePath() + "/Contents/Resources/";
	cf.load(mResourcePath + "deferred_lighting_resources.cfg");
#else
	cf.load("config/resources.cfg");
#endif

	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;

			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
        }
    }
	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	
	// Create scene loader
	this->_ofusion_scene_loader = new OSMScene(this->_scene_manager, this->_render_window);

	// Create overlays
	this->_debug_overlay = OverlayManager::getSingleton().getByName("Core/DebugOverlay");
	this->_debug_overlay->show();
}

void OgreApplication::LoadScene(const std::string &file)
{
#if 0
	// Create the scene root node
	this->_scene_root_node = this->_scene_manager->getRootSceneNode()->createChildSceneNode();
	
	// Create the lights root node
	this->_light_root_node = this->_scene_manager->getRootSceneNode()->createChildSceneNode();
#endif
	// Load the specified scene
	this->_ofusion_scene_loader->initialise(file.c_str());
	this->_ofusion_scene_loader->createScene();

	// Add entities to deferred lighting effect
	this->_deferred_lighting_effect->AddEntities( this->_ofusion_scene_loader->getEntityList() );
}

void OgreApplication::UnloadScene()
{

}

void OgreApplication::UpdateStats(const double elapsed_time)
{
	static Ogre::String currFps = "Current FPS: "; 
	static Ogre::String avgFps = "Average FPS: "; 
	static Ogre::String bestFps = "Best FPS: "; 
	static Ogre::String worstFps = "Worst FPS: "; 
	static Ogre::String tris = "Triangle Count: "; 
	static Ogre::String batches = "Batch Count: "; 

	Ogre::OverlayElement* guiAvg = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/AverageFps"); 
	Ogre::OverlayElement* guiCurr = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/CurrFps"); 
	//Ogre::OverlayElement* guiBest = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/BestFps");
	//Ogre::OverlayElement* guiWorst = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/WorstFps");

	//size_t current_fps = (size_t)( 1000.0f / elapsed_time );
	//size_t average_fps = (size_t)( (this->_num_frames * 1000.0f) / this->_elapsed_time );
	const Ogre::RenderTarget* render_target = this->_render_window->getViewport(0)->getTarget();
	
	const Ogre::RenderTarget::FrameStats& stats = this->_render_window->getStatistics(); 
	guiAvg->setCaption(avgFps + Ogre::StringConverter::toString(stats.avgFPS)); 
	guiCurr->setCaption(currFps + Ogre::StringConverter::toString(stats.lastFPS)); 
	
	Ogre::OverlayElement* guiTris = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/NumTris"); 
	size_t triangle_count = render_target->getTriangleCount();
	guiTris->setCaption(tris + Ogre::StringConverter::toString(triangle_count)); 

	Ogre::OverlayElement* guiBatches = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/NumBatches"); 
	size_t batch_count = render_target->getBatchCount();
    guiBatches->setCaption(batches + Ogre::StringConverter::toString(batch_count)); 

	Ogre::OverlayElement* guiDbg = Ogre::OverlayManager::getSingleton().getOverlayElement("Core/DebugText"); 

	std::stringstream ss;
	ss << "Current pos: x: " << this->_cam_node_yaw->_getDerivedPosition().x
		<< ", y: " << this->_cam_node_yaw->_getDerivedPosition().y
		<< ", z: " << this->_cam_node_yaw->_getDerivedPosition().z
		<< "\nCurrent orientation: " << Ogre::StringConverter::toString(this->_cam_node_yaw->_getDerivedOrientation());
	guiDbg->setCaption(ss.str());
	
#if 0
	this->_log->logMessage(ss.str());
	this->_log->logMessage(guiAvg->getCaption());
	this->_log->logMessage(guiCurr->getCaption());
	this->_log->logMessage(guiBest->getCaption());
	this->_log->logMessage(guiWorst->getCaption());
	this->_log->logMessage(guiTris->getCaption());
	this->_log->logMessage(guiBatches->getCaption());
#endif
}

void OgreApplication::CaptureInput()
{
	this->_keyboard->capture();
	//this->_mouse->capture();
}

void OgreApplication::MoveCamera()
{
	if (this->_keyboard->isKeyDown(OIS::KC_A))
	{
		this->_translation_vector.x = -this->_move_scale;
	}

	if (this->_keyboard->isKeyDown(OIS::KC_D))
	{
		this->_translation_vector.x = this->_move_scale;
	}

	if (this->_keyboard->isKeyDown(OIS::KC_W))
	{
		this->_translation_vector.z = -this->_move_scale;
	}

	if (this->_keyboard->isKeyDown(OIS::KC_S))
	{
		this->_translation_vector.z = this->_move_scale;
	}
	
	if (this->_keyboard->isKeyDown(OIS::KC_E))
	{
		this->_translation_vector.y = this->_move_scale;
	}
	
	if (this->_keyboard->isKeyDown(OIS::KC_C))
	{
		this->_translation_vector.y -= this->_move_scale;
	}

	// TODO: TEMP

	if (this->_keyboard->isKeyDown(OIS::KC_LEFT))
	{
		this->_cam_node_yaw->rotate(
			Ogre::Quaternion(Ogre::Degree(this->_rotate_scale), Ogre::Vector3::UNIT_Y) );
	}

	if (this->_keyboard->isKeyDown(OIS::KC_RIGHT))
	{
		this->_cam_node_yaw->rotate(
			Ogre::Quaternion(Ogre::Degree(-this->_rotate_scale), Ogre::Vector3::UNIT_Y) );
	}

	if (this->_keyboard->isKeyDown(OIS::KC_UP))
	{
		this->_cam_node_pitch->rotate(
			Ogre::Quaternion(Ogre::Degree(this->_rotate_scale), Ogre::Vector3::UNIT_X) );
	}

	if(this->_keyboard->isKeyDown(OIS::KC_DOWN))
	{
		this->_cam_node_pitch->rotate(
			Ogre::Quaternion(Ogre::Degree(-this->_rotate_scale), Ogre::Vector3::UNIT_X) );
	}

	if (this->_keyboard->isKeyDown(OIS::KC_LSHIFT)) 
	{
		this->_cam_node_yaw->translate(this->_translation_vector, Ogre::Node::TS_LOCAL);
	}
	else
	{
		this->_cam_node_yaw->translate(this->_translation_vector / 10.0f, Ogre::Node::TS_LOCAL);
	}

	this->_translation_vector = Ogre::Vector3::ZERO;
}

bool OgreApplication::keyPressed(const OIS::KeyEvent &keyEventRef)
{
	if (this->_keyboard->isKeyDown(OIS::KC_ESCAPE))
	{
		this->_is_shutdown = true;
		return true;
	}
	if (this->_keyboard->isKeyDown(OIS::KC_L))
	{
		this->_cam_light_node->flipVisibility();
	}
	if (this->_keyboard->isKeyDown(OIS::KC_1))
	{
		if (this->_deferred_lighting_effect->IsEnabled())
			this->_deferred_lighting_effect->Hide();
		else
			this->_deferred_lighting_effect->Show();
	}
	if (this->_keyboard->isKeyDown(OIS::KC_M))
	{
		static int mode = 0;		
		if (mode == 2)
		{
			this->_camera->setPolygonMode(PM_SOLID);
			mode = 0;
		}
		else if (mode == 0)
		{
			 this->_camera->setPolygonMode(PM_WIREFRAME);
			 mode = 1;
		}
		else if (mode == 1)
		{
			this->_camera->setPolygonMode(PM_POINTS);
			mode = 2;
		}
	}
	if (this->_keyboard->isKeyDown(OIS::KC_O))
	{
		if (this->_debug_overlay)
		{
			if (!this->_debug_overlay->isVisible())
				this->_debug_overlay->show();
			else
				this->_debug_overlay->hide();
		}
	}
	if (this->_keyboard->isKeyDown(OIS::KC_EQUALS))
	{
		static const std::string PREFIX = "deferred_lighting_";
		static const std::string SUFFIX = ".png";
		Ogre::Root::getSingleton().getAutoCreatedWindow()->writeContentsToTimestampedFile(PREFIX, SUFFIX);
	}
	return true;
}

bool OgreApplication::keyReleased(const OIS::KeyEvent &keyEventRef)
{
	return true;
}

bool OgreApplication::mouseMoved(const OIS::MouseEvent &mouse_event)
{
	this->_camera->yaw(Degree(mouse_event.state.X.rel * -0.1));
	this->_camera->pitch(Degree(mouse_event.state.Y.rel * -0.1));
	return true;
}

bool OgreApplication::mousePressed(const OIS::MouseEvent &mouse_event, OIS::MouseButtonID id)
{
	return true;
}

bool OgreApplication::mouseReleased(const OIS::MouseEvent &mouse_event, OIS::MouseButtonID id)
{
	return true;
}

void OgreApplication::Run()
{
	this->_log->logMessage("Start main loop...");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	
	std::stringstream ss;
	unsigned long elapsed = 0;
	unsigned long last = 0;
	//static const double FPS = 60.0f;
	//static const double FPMS = (FPS / 1000.0f);
	//static const double FRAMELEN_MS = 1.0f / FPMS;
#if 0
	ss << "Frames per ms: " << FPMS << " frames";
	this->_log->logMessage(ss.str());			
	ss.str("");
	
	ss << "Frame length (ms): " << FRAMELEN_MS << " ms";
	this->_log->logMessage(ss.str());			
	ss.str("");
#endif
	// Store frame start time
	last = this->_timer->getMilliseconds();
	
	this->_render_window->resetStatistics();

	this->_render_window->setActive(true);

	while(!this->_is_shutdown) 
	{
		if (this->_render_window->isClosed()) 
		{
			this->_is_shutdown = true;
			break;
		}

//#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		// Flush pending messages in the system message queue
		Ogre::WindowEventUtilities::messagePump();
//#endif

		if (this->_render_window->isActive())
		{	
#if 0
			// Calculate elapsed frame time
			elapsed = this->_timer->getMilliseconds() - last;
			
			// Throttle frame rate to be constant
			if (elapsed < FRAMELEN_MS)
			{
				useconds_t length = (FRAMELEN_MS - elapsed);
				length *= 1000.0f;
				usleep(length);
			}
			
			// Update frame stats
			this->_elapsed_time += elapsed;
			++this->_num_frames;
#endif			
			// Calculate elapsed frame time
			elapsed = this->_timer->getMilliseconds() - last;
			last = this->_timer->getMilliseconds();

			// Reset frame variables
			this->_move_scale = this->_move_speed * elapsed;
			this->_rotate_scale = this->_rotate_speed * elapsed;
			this->_translation_vector = Vector3::ZERO;

			// Handle per frame logic
			this->CaptureInput();
			this->MoveCamera();
			this->UpdateStats((double)elapsed);
			
			// Render frame
			this->_root->renderOneFrame();
		}
	}
	this->_log->logMessage("Main loop is finished!");
	this->_log->logMessage("Begin shutdown...");
}
