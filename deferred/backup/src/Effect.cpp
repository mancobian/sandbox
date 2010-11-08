/*
 *  BaseEffect.cpp
 *  Samples
 *
 *  Created by Arris Ray on 6/23/09.
 *  Copyright 2009 The Secret Design Collective. All rights reserved.
 *
 */

#include "Effect.h"

using namespace tsdc;
using namespace tsdc::global;

///////////////////////////////////////////////////////////////////////////////
// STATIC MEMBER INITIALIZATION
///////////////////////////////////////////////////////////////////////////////

template<> tsdc::EffectManager* Ogre::Singleton<tsdc::EffectManager>::ms_Singleton = 0;

///////////////////////////////////////////////////////////////////////////////
// CLASS IMPLEMENTATION: EffectManager
///////////////////////////////////////////////////////////////////////////////

EffectManager::EffectManager()
{
}

EffectManager::~EffectManager()
{
	// Destroy all managed effects
	EffectList::iterator iter = this->_effects.begin(),
		end = this->_effects.end();
	while (iter != end)
	{
		Effect *effect = *iter;
		std::string msg = "Destroying effect of type: ";
		msg.append(effect->Factory()->Type());
		Ogre::LogManager::getSingleton().logMessage(msg.c_str());
		effect->Factory()->Destroy(effect);
		++iter;
	}
	this->_effects.clear();
	Ogre::LogManager::getSingleton().logMessage("Destroyed all effects!");
}

void EffectManager::RegisterEffect(const IFactory<Effect> *factory)
{
	this->_factories.insert( std::make_pair(factory->Type(), factory) );
}

Effect* EffectManager::CreateEffect(const char *type)
{
	// Get the appropriate effect factory
	EffectFactoryMap::iterator iter = this->_factories.find(type);
	if (iter == this->_factories.end())
		return NULL;
	
	// Create and store new effect
	const EffectFactory *factory = iter->second;
	Effect *effect = factory->Create();
	effect->Factory(factory);
	this->_effects.push_back(effect);
	return effect;
}

void EffectManager::DestroyEffect(Effect *effect)
{
	// Remove and destroy effect
	this->_effects.remove(effect);
	effect->Factory()->Destroy(effect);
}

///////////////////////////////////////////////////////////////////////////////
// CLASS IMPLEMENTATION: Effect
///////////////////////////////////////////////////////////////////////////////

Effect::~Effect()
{
}
