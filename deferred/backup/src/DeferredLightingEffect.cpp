/*
 *  DeferredLightingEffect.cpp
 *  Samples
 *
 *  Created by Arris Ray on 6/18/09.
 *  Copyright 2009 The Secret Design Collective. All rights reserved.
 *
 */

#include "DeferredLightingEffect.h"
#include "Globals.h"
#include "Light.h"

using namespace tsdc;
using namespace tsdc::global;

///////////////////////////////////////////////////////////////////////////////
// STATIC MEMBER INITIALIZATION
///////////////////////////////////////////////////////////////////////////////

template<> tsdc::DeferredLightingEffectFactory* Ogre::Singleton<tsdc::DeferredLightingEffectFactory>::ms_Singleton = 0;
const unsigned char DeferredLightingEffect::RENDER_QUEUE_DEBUG = 91;
const std::string DeferredLightingEffect::GBUFFER_MATERIAL_NAME = "tsdc/material/gbuffer";
const std::string DeferredLightingEffect::GBUFFER_SCHEME_NAME = "tsdc/scheme/gbuffer";
const std::string DeferredLightingEffect::GBUFFER_TECHNIQUE_NAME = "tsdc/technique/gbuffer/base";
const std::string DeferredLightingEffect::GBUFFER_PASS_NAME = "tsdc/pass/gbuffer";
const std::string DeferredLightingEffect::BACKFACE_TECHNIQUE_NAME = "tsdc/technique/backface";
const std::string DeferredLightingEffect::FRONTFACE_TECHNIQUE_NAME = "tsdc/technique/frontface";
const std::string DeferredLightingEffect::TEST_COMPOSITOR_NAME = "tsdc/compositor/test";
const std::string DeferredLightingEffect::LIGHTING_RENDER_TARGET_NAME = "rt_light";
const std::string DeferredLightingEffect::LIGHTING_PASS_NAME = "tsdc/pass/lighting";

///////////////////////////////////////////////////////////////////////////////
// CLASS IMPLEMENTATION: DeferredLightingEffect
///////////////////////////////////////////////////////////////////////////////

DeferredLightingEffect::DeferredLightingEffect() :
	_viewport(NULL)
	, _camera(NULL)
	, _scene_manager(NULL)
	, _scene_root_node(NULL)
	, _light_root_node(NULL)
{
}

DeferredLightingEffect::~DeferredLightingEffect()
{
	this->Destroy();
}

void DeferredLightingEffect::Create(Ogre::Viewport *viewport)
{	
	// Store scene render helper objects
	this->_viewport = viewport;
	this->_camera = this->_viewport->getCamera();
	this->_scene_manager = this->_viewport->getCamera()->getSceneManager();

	// Get a reference to the g-buffer material & technique
	this->_gbuffer_material = Ogre::MaterialManager::getSingleton().getByName(GBUFFER_MATERIAL_NAME);
	for (unsigned short i=0; i<this->_gbuffer_material->getNumTechniques(); ++i)
	{
		Ogre::Technique *technique = this->_gbuffer_material->getTechnique(i);
		if (technique->getSchemeName().compare(GBUFFER_SCHEME_NAME) == 0)
		{
			this->_gbuffer_technique = technique;
			break;
		}
	}
	assert (this->_gbuffer_technique != NULL);

	this->_compositor_instance = Ogre::CompositorManager::getSingleton().addCompositor(
		this->_viewport
		, TEST_COMPOSITOR_NAME);
	this->Show(); // NOTE: This appears to be required to add the compositor listener below!

	// Attach listeners
	this->AttachListeners();
	
	// Hide effect
	//this->Hide();
}

void DeferredLightingEffect::Destroy()
{
	// Detach listeners
	this->DetachListeners();
	
	// Release pointers
	this->_gbuffer_material.setNull();
	this->_point_light_material.setNull();
	this->_earlyz_material.setNull();
}

void DeferredLightingEffect::AddEntities(OSMScene::EntityList &entities)
{
	OSMScene::EntityList::iterator iter = entities.begin(),
		end = entities.end();
	while (iter != end)
	{
		this->AddEntity(*iter);
		++iter;
	}
}

void DeferredLightingEffect::AddEntity(Ogre::Entity *entity)
{
	// Get a reference to the g-buffer pass
	Ogre::Pass *gbuffer_pass = this->_gbuffer_technique->getPass(0);
	if (!gbuffer_pass->hasVertexProgram() || !gbuffer_pass->hasFragmentProgram())
	{
		Ogre::LogManager::getSingleton().logMessage("[ERROR] G-buffer pass is missing a vertex and/or fragment shader");
		assert (false);
		return;
	}

	for (unsigned short i=0; i<entity->getNumSubEntities(); ++i)
	{
		Ogre::SubEntity *subentity = entity->getSubEntity(i);
		Ogre::MaterialPtr material = subentity->getMaterial();
		for (unsigned short j=0; j<material->getNumTechniques(); ++j)
		{
			bool has_gbuffer_pass = false;
			bool has_lighting_pass = false;
			Ogre::Technique *technique = material->getTechnique(j);
			technique->setSchemeName(DeferredLightingEffect::GBUFFER_SCHEME_NAME);

			// Determine if this technique contains a lighting or gbuffer pass
			for (unsigned short k=0; k<technique->getNumPasses(); ++k)
			{
				Ogre::Pass *pass = technique->getPass(k);
				if ( pass->getName().compare(DeferredLightingEffect::LIGHTING_PASS_NAME) == 0 )
					has_lighting_pass = true;
				if ( pass->getName().compare(DeferredLightingEffect::GBUFFER_PASS_NAME) == 0 )
					has_gbuffer_pass = true;
			}
			if (has_lighting_pass || has_gbuffer_pass)
				continue;

			// Add g-buffer pass to each technique without a lighting or gbuffer pass
			Ogre::Pass *gbuffer_pass_copy = technique->createPass();
			*gbuffer_pass_copy = *gbuffer_pass;
			gbuffer_pass_copy->setName( DeferredLightingEffect::GBUFFER_PASS_NAME );
			gbuffer_pass_copy->setVertexProgram( gbuffer_pass->getVertexProgramName() );
			gbuffer_pass_copy->setFragmentProgram( gbuffer_pass->getFragmentProgramName() );
		}
		material->compile();
	}

	std::stringstream msg;
	msg << "[GBUFFER] Adding g-buffer scheme to entity (subentities="
		<< entity->getNumSubEntities() << "): " << entity->getName() << std::endl;
	for (unsigned short i=0; i<entity->getNumSubEntities(); ++i)
	{
		Ogre::SubEntity *subentity = entity->getSubEntity(i);
		Ogre::MaterialPtr material = subentity->getMaterial();
		msg << "[GBUFFER]\t=>Sub-entity material (techniques=" << material->getNumTechniques()
			<< "): " << material->getName() << std::endl;
		for (unsigned short j=0; j<material->getNumTechniques(); ++j)
		{
			Ogre::Technique *technique = material->getTechnique(j);
			msg << "[GBUFFER]\t\t=>Technique (passes=" << technique->getNumPasses() << "): "
				<< technique->getName() << std::endl;
			msg << "[GBUFFER]\t\t=>Scheme: " << technique->getSchemeName() << std::endl;
			for (unsigned short k=0; k<technique->getNumPasses(); ++k)
			{
				Ogre::Pass *pass = technique->getPass(k);
				msg << "[GBUFFER]\t\t\t=>Pass: " << pass->getName() << std::endl;
			}
		}
	}
	Ogre::LogManager::getSingleton().logMessage(msg.str());
}

void DeferredLightingEffect::AttachListeners()
{
	// Attach listeners
	Ogre::MaterialManager::getSingleton().addListener(this);
	this->_compositor_instance->getRenderTarget(DeferredLightingEffect::LIGHTING_RENDER_TARGET_NAME)->addListener(this);
	this->_compositor_instance->addListener(this);
	this->_scene_manager->addRenderQueueListener(this);	
	this->_scene_manager->getRenderQueue()->setRenderableListener(this);
}

void DeferredLightingEffect::DetachListeners()
{
	// If the effect is not enabled, listeners have already been detached!
	if (!this->IsEnabled())
		return;
		
	// Detach listeners
	Ogre::MaterialManager::getSingleton().removeListener(this);
	this->_scene_manager->removeRenderQueueListener(this);	
	this->_scene_manager->getRenderQueue()->setRenderableListener(NULL);
	this->_compositor_instance->removeListener(this);
	this->_compositor_instance->getRenderTarget(DeferredLightingEffect::LIGHTING_RENDER_TARGET_NAME)->removeListener(this);
}

void DeferredLightingEffect::Show()
{
	//Ogre::CompositorManager::getSingleton().setCompositorEnabled(this->_viewport, TEST_COMPOSITOR_NAME, true);
	this->_compositor_instance->setEnabled(true);
	this->AttachListeners();
}

void DeferredLightingEffect::Hide()
{
	//Ogre::CompositorManager::getSingleton().setCompositorEnabled(this->_viewport, TEST_COMPOSITOR_NAME, false);
	this->DetachListeners();
	this->_compositor_instance->setEnabled(false);
}

void DeferredLightingEffect::AddLight(Light *light)
{
	assert (light != NULL);
	Ogre::Entity *entity = light->BoundingVolume();
	assert (entity != NULL);
	this->_light_entities.push_back(entity);
}

Ogre::Technique* DeferredLightingEffect::handleSchemeNotFound (unsigned short schemeIndex, 
	const Ogre::String &schemeName, 
	Ogre::Material *originalMaterial, 
	unsigned short lodIndex, 
	const Ogre::Renderable *rend)
{
	return NULL;
#if 0
	if (schemeName.compare(GBUFFER_SCHEME_NAME) == 0)
	{
		std::stringstream msg;
		msg << "[GBUFFER] Adding g-buffer scheme to material: " << originalMaterial->getName() << std::endl;
		msg << "[GBUFFER]\t=>Num techniques: " << originalMaterial->getNumTechniques() << std::endl;
		for (unsigned short i=0; i<originalMaterial->getNumTechniques(); ++i)
		{
			Ogre::Technique *t = originalMaterial->getTechnique(i);
			msg << "[GBUFFER]\t=>Num passes in technique <" << t->getName() << ">: " << t->getNumPasses() << std::endl;
			for (unsigned short j=0; j<t->getNumPasses(); ++j)
			{
				Ogre::Pass *p = t->getPass(j);
				msg << "[GBUFFER]\t\t=>Pass: " << p->getName() << std::endl;
			}
		}
		Ogre::LogManager::getSingleton().logMessage(msg.str());

		// Get a reference to the g-buffer pass
		Ogre::Pass *gbuffer_pass = this->_gbuffer_technique->getPass(0);
		if (!gbuffer_pass->hasVertexProgram() || !gbuffer_pass->hasFragmentProgram())
		{
			Ogre::LogManager::getSingleton().logMessage("[ERROR] G-buffer pass is missing a vertex and/or fragment shader");
			assert (false);
			return NULL;
		}

		// Error check: Light geometry should not be stored into the g-buffer
		if (Ogre::any_cast<const tsdc::Light*>(&rend->getUserAny()) != NULL)
		{
			// Ogre::LogManager::getSingleton().logMessage("No light associated with renderable with material: " + rend->getMaterial()->getName());
			return NULL;
		}

		// Get a reference to the original material's active technique
		Ogre::Technique *intech = originalMaterial->getTechnique(0);
		assert (intech != NULL);
		Ogre::Technique *newtech = originalMaterial->createTechnique();
		*newtech = *intech; 
		assert (newtech != NULL);
		newtech->setName(intech->getName() + "_copy");

		// Print info for every pass in technique
		Ogre::LogManager::getSingleton().logMessage("[DEBUG] Technique: " + newtech->getName());
		for (unsigned short i=0; i<intech->getNumPasses(); ++i)
		{
			Ogre::Pass *pass = newtech->getPass(i);
			pass->setName(intech->getPass(i)->getName() + "_copy");
			Ogre::LogManager::getSingleton().logMessage("[DEBUG]\t=> Pass: " + pass->getName());
		}

		// Add the g-buffer pass to the new technique
		Ogre::Pass *newpass = newtech->createPass();
		assert (newpass != NULL);
		*newpass = *gbuffer_pass;
		assert (newpass != NULL);
		newpass->setName(newtech->getName() + "_" + gbuffer_pass->getName() + "_copy");
		newtech->setName(intech->getName() + "_copy");

		// Configure the new technique's g-buffer pass
		newpass->setLightingEnabled(false);
		newpass->setCullingMode(Ogre::CULL_CLOCKWISE); // see also: CULL_CLOCKWISE, CULL_ANTICLOCKWISE, CULL_NONE
		newpass->setManualCullingMode(Ogre::MANUAL_CULL_BACK); // see also: MANUAL_CULL_FRONT, MANUAL_CULL_BACK, MANUAL_CULL_NONE
		newpass->setDepthFunction(Ogre::CMPF_LESS_EQUAL);
		newpass->setDepthCheckEnabled(true);
		newpass->setDepthWriteEnabled(true);

		// Copy g-buffer shaders into the original material's active technique
		newpass->setVertexProgram( gbuffer_pass->getVertexProgramName() );
		newpass->setFragmentProgram( gbuffer_pass->getFragmentProgramName() );

		// Add g-buffer scheme to the original material
		newtech->setSchemeName(schemeName);
		originalMaterial->compile();
		
		Ogre::LogManager::getSingleton().logMessage("=> Original material: " + originalMaterial->getName());
		return newtech;
	}
	return NULL;
#endif
}

void DeferredLightingEffect::UpdateShaderParams() const
{
	EntityList::const_iterator iter = this->_light_entities.begin(),
		end = this->_light_entities.end();
	while (iter != end)
	{
		const Ogre::Entity *entity = *iter;
		for (unsigned i=0; i<entity->getNumSubEntities(); ++i)
		{
			this->UpdateLightRenderable(entity->getSubEntity(i));
		}
		++iter;
	}
}

void DeferredLightingEffect::UpdateLightRenderable(Ogre::Renderable *renderable) const
{	
	// Error check: Is this renderable light geometry? 
	if (Ogre::any_cast<const tsdc::Light*>(&renderable->getUserAny()) == NULL)
	{
		// Ogre::LogManager::getSingleton().logMessage("No light associated with renderable with material: " + renderable->getMaterial()->getName());
		return;
	}
	const tsdc::Light *light = Ogre::any_cast<const tsdc::Light*>(renderable->getUserAny());

	// Get compositor render target surfaces
	assert (this->_compositor_instance);
	Ogre::String mrt0 = this->_compositor_instance->getTextureInstanceName("mrt0", 0);
	Ogre::String mrt1 = this->_compositor_instance->getTextureInstanceName("mrt0", 1);
	Ogre::String mrt2 = this->_compositor_instance->getTextureInstanceName("mrt0", 2);
	Ogre::String mrt3 = this->_compositor_instance->getTextureInstanceName("mrt0", 3);

	// Get point light pass
	const Ogre::MaterialPtr mat = renderable->getMaterial();
	for (unsigned short i=0; i<mat->getNumTechniques(); ++i)
	{
		Ogre::Technique *technique = mat->getTechnique(i);
		Ogre::Pass *pass = technique->getPass(DeferredLightingEffect::LIGHTING_PASS_NAME);
		pass->getTextureUnitState(0)->setTextureName(mrt0);
		pass->getTextureUnitState(1)->setTextureName(mrt1);
		pass->getTextureUnitState(2)->setTextureName(mrt2);
		pass->getTextureUnitState(3)->setTextureName(mrt3);
	}

	// Update light shader params
	this->UpdateCommonLightParams(renderable);
	switch (light->OgreLight()->getType())
	{
		case Ogre::Light::LT_SPOTLIGHT:
		{
			this->UpdateSpotLightParams(renderable);
			break;
		}
		case Ogre::Light::LT_POINT:
		case Ogre::Light::LT_DIRECTIONAL:
		default:
		{
			this->UpdatePointLightParams(renderable);
			break;
		}
	}

#if 0
	// Print debug
	Ogre::LogManager::getSingleton().logMessage("Light " + light->getName() + " (lpos/cpos): " + 
		Ogre::StringConverter::toString(light->getParentSceneNode()->_getDerivedPosition()) + " / " + 
		Ogre::StringConverter::toString(this->_camera->getDerivedPosition()) );
		Ogre::LogManager::getSingleton().logMessage("\t=>Light attenuation range: " + Ogre::StringConverter::toString(light->getAttenuationRange()));
	Ogre::LogManager::getSingleton().logMessage("\t=> Bounding box center: " + Ogre::StringConverter::toString(light->getParentSceneNode()->_getWorldAABB().getCenter()));
	Ogre::LogManager::getSingleton().logMessage("mrt0:0 => " + mrt0);
	Ogre::LogManager::getSingleton().logMessage("mrt0:1 => " + mrt1);
	Ogre::LogManager::getSingleton().logMessage("mrt0:2 => " + mrt2);
	Ogre::LogManager::getSingleton().logMessage("Camera far clip distance: " + Ogre::StringConverter::toString(clip_distances.y));
	Ogre::LogManager::getSingleton().logMessage("Rendereable queued: " + Ogre::StringConverter::toString(groupID));
	Ogre::LogManager::getSingleton().logMessage("\t=> Pos: " +
		Ogre::StringConverter::toString(light->getParentSceneNode()->getPosition()));
	Ogre::LogManager::getSingleton().logMessage("\t=> Derived Pos: " +
		Ogre::StringConverter::toString(light->getParentSceneNode()->_getDerivedPosition()));
	Ogre::LogManager::getSingleton().logMessage("\t=> View Pos: " +
		Ogre::StringConverter::toString(light_position));
#endif
}

void DeferredLightingEffect::UpdateCommonLightParams(Ogre::Renderable *rend) const
{
	// Error check: Is this renderable light geometry?
	if (Ogre::any_cast<const tsdc::Light*>(&rend->getUserAny()) == NULL)
	{
		// Ogre::LogManager::getSingleton().logMessage("No light associated with renderable with material: " + rend->getMaterial()->getName());
		return;
	}
	const tsdc::Light *light = Ogre::any_cast<const tsdc::Light*>(rend->getUserAny());

	// Prepare params
	Ogre::Vector4 light_world_position(light->OgreLight()->getParentSceneNode()->_getDerivedPosition());
	Ogre::Vector4 light_view_position(this->_camera->getViewMatrix(true) * light_world_position);
	Ogre::Vector4 attenuation = Ogre::Vector4(
		light->OgreLight()->getAttenuationRange()
		, light->OgreLight()->getAttenuationConstant()
		, light->OgreLight()->getAttenuationLinear()
		, light->OgreLight()->getAttenuationQuadric());

	// Assign params
	rend->setCustomParameter(tsdc::Light::PARAM_LIGHT_POSITION, light_view_position);
	rend->setCustomParameter(tsdc::Light::PARAM_LIGHT_ATTENUATION, attenuation);
}

void DeferredLightingEffect::UpdatePointLightParams(Ogre::Renderable *rend) const
{
#if 0
	// Error check: Is this renderable light geometry?
	if (Ogre::any_cast<const tsdc::Light*>(&rend->getUserAny()) == NULL)
	{
		Ogre::LogManager::getSingleton().logMessage("No light associated with renderable with material: " + rend->getMaterial()->getName());
		return;
	}
	const tsdc::Light *light = Ogre::any_cast<const tsdc::Light*>(rend->getUserAny());
#endif
}

void DeferredLightingEffect::UpdateSpotLightParams(Ogre::Renderable *rend) const
{
	// Error check: Is this renderable light geometry?
	if (Ogre::any_cast<const tsdc::Light*>(&rend->getUserAny()) == NULL)
	{
		// Ogre::LogManager::getSingleton().logMessage("No light associated with renderable with material: " + rend->getMaterial()->getName());
		return;
	}
	const tsdc::Light *light = Ogre::any_cast<const tsdc::Light*>(rend->getUserAny());

	// Prepare params
	Ogre::Matrix4 view_matrix = this->_camera->getViewMatrix(true);
	Ogre::Matrix3 camrot;
	view_matrix.extract3x3Matrix(camrot);

	Ogre::Quaternion lightnodeori = light->OgreLight()->getParentSceneNode()->_getDerivedOrientation();
	Ogre::Matrix3 lightnoderot;
	lightnodeori.ToRotationMatrix(lightnoderot);

	//Ogre::Vector3 lightdir = Ogre::Vector3::NEGATIVE_UNIT_Y; // HACK: Manually specify the light direction...
	//Ogre::Vector3 lightori = lightdir;
	//Ogre::Vector4 param_spotlight_orientation( Ogre::Vector3(camrot * lightori).normalisedCopy() );
	Ogre::Vector3 lightdir = light->OgreLight()->getDirection(); // HACK: Manually specify the light direction...
	Ogre::Vector3 lightori = lightnoderot * lightdir * Ogre::Vector3::UNIT_SCALE;

	Ogre::Vector4 param_spotlight_orientation;
	if (light->OgreLight()->getName().compare("_tsdc_cam_light") == 0)
	{
		param_spotlight_orientation = Ogre::Vector4( light->OgreLight()->getDirection() );

#if 0
	LOG ("Camera light node <" + light->OgreLight()->getParentSceneNode()->getName() + "> orientation: " + Ogre::StringConverter::toString(param_spotlight_orientation) );
	LOG ("Camera light <" + light->OgreLight()->getName() + "> direction: " + Ogre::StringConverter::toString(light->OgreLight()->getDirection()) );
	LOG ("Camera light bounding volume parent node <" + light->BoundingVolume()->getParentSceneNode()->getName() + "> orientation: " + Ogre::StringConverter::toString(light->BoundingVolume()->getParentSceneNode()->getOrientation()) );
#endif
	}
	else
	{
		//Ogre::Vector4 param_spotlight_orientation( Ogre::Quaternion::IDENTITY * light->OgreLight()->getDirection() );
		param_spotlight_orientation = Ogre::Vector4( camrot * lightori );
	}
	Ogre::Vector4 param_spotlight_range(
		Ogre::Math::Cos(light->OgreLight()->getSpotlightInnerAngle())
		, Ogre::Math::Cos(light->OgreLight()->getSpotlightOuterAngle())
		, light->OgreLight()->getSpotlightFalloff()
		, light->SpotlightAngleOffset());

#if 0
	if (light->OgreLight()->getName().compare("parkinglot_Fspot102") == 0)
	{
		LOG ("---");
		LOG ("Light <" + light->OgreLight()->getName() + "> spotlight cos(inner-angle): " + Ogre::StringConverter::toString(param_spotlight_range.x) );
		LOG ("Light <" + light->OgreLight()->getName() + "> spotlight cos(outer-angle): " + Ogre::StringConverter::toString(param_spotlight_range.y) );
		LOG ("Light <" + light->OgreLight()->getName() + "> spotlight falloff: " + Ogre::StringConverter::toString(param_spotlight_range.z) );
		LOG ("Light <" + light->OgreLight()->getName() + "> spotlight angle offset: " + Ogre::StringConverter::toString(param_spotlight_range.w) );
	}
#endif

	assert (light->BoundingVolume()->getParentSceneNode()->_getDerivedPosition() == light->OgreLight()->getDerivedPosition());

	// Assign params
	rend->setCustomParameter(tsdc::Light::PARAM_SPOTLIGHT_ORIENTATION, param_spotlight_orientation);
	rend->setCustomParameter(tsdc::Light::PARAM_SPOTLIGHT_RANGE, param_spotlight_range);
}


bool DeferredLightingEffect::renderableQueued (Ogre::Renderable *rend
	, Ogre::uint8 groupID
	, Ogre::ushort priority
	, Ogre::Technique **ppTech
	, Ogre::RenderQueue *pQueue)
{
	// Error check: Is this renderable light geometry? 
	if (Ogre::any_cast<const tsdc::Light*>(&rend->getUserAny()) == NULL)
	{
		//Ogre::LogManager::getSingleton().logMessage("No light associated with renderable with material: " + rend->getMaterial()->getName());
		return true;
	}
	const tsdc::Light *light = Ogre::any_cast<const tsdc::Light*>(rend->getUserAny());
	Ogre::Material *material = (*ppTech)->getParent();

	// Update renderable texture inputs
	this->UpdateLightRenderable(rend);
#if 0
	if (light->OgreLight()->getName().compare("_tsdc_camlight") == 0)
	{
		*ppTech = material->getTechnique(DeferredLightingEffect::FRONTFACE_TECHNIQUE_NAME);
		//*ppTech = material->getTechnique(DeferredLightingEffect::BACKFACE_TECHNIQUE_NAME);
	}
	else
	{
		// Is camera inside light bounding mesh?
		if (light->OgreLight()->getParentSceneNode()->_getWorldAABB().intersects(this->_camera->getDerivedPosition()))
		{
			// Yes! Enable front face culling!
			*ppTech = material->getTechnique(DeferredLightingEffect::BACKFACE_TECHNIQUE_NAME);
			//(*ppTech)->setDepthBias(20.0f, 1.0f);
		}
		else
		{
			// No! Restore back face culling!
			*ppTech = material->getTechnique(DeferredLightingEffect::FRONTFACE_TECHNIQUE_NAME);
			//(*ppTech)->setDepthBias(0.0f, 1.0f);
		}
	}
#endif
	return true;
}


void DeferredLightingEffect::preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
{
}

void DeferredLightingEffect::postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt)
{
}

void DeferredLightingEffect::notifyMaterialSetup (Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
}

void DeferredLightingEffect::notifyMaterialRender (Ogre::uint32 pass_id, Ogre::MaterialPtr &mat)
{
}

bool DeferredLightingEffect::objectRendering(const Ogre::MovableObject *movable, const Ogre::Camera *camera)
{
	return true;
}

void DeferredLightingEffect::renderQueueStarted(Ogre::uint8 id, const Ogre::String &invocation, bool &skipThisQueue)
{
}

void DeferredLightingEffect::renderQueueEnded(Ogre::uint8 id, const Ogre::String &invocation, bool &repeatThisQueue)
{
}
