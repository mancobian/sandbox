/*
 *  BaseEffect.h
 *  Samples
 *
 *  Created by Arris Ray on 6/23/09.
 *  Copyright 2009 The Secret Design Collective. All rights reserved.
 *
 */
 
#ifndef EFFECT_H
#define EFFECT_H
 
#include <list>
#include <map>
#include <Ogre.h>
#include "IDGen.h"
#include "Globals.h"

namespace tsdc {

class Effect : public IDGen
{		
	public:
		virtual ~Effect();
		
	public:
		virtual bool IsEnabled() const = 0;
		virtual void Create(Ogre::Viewport *viewport) = 0;
		virtual void Destroy() = 0;
		virtual void Show() = 0;
		virtual void Hide() = 0;
		
	public:
		const tsdc::global::IFactory<Effect>* Factory() const { return this->_factory; }
		void Factory(const tsdc::global::IFactory<Effect>* value) { this->_factory = value; }
		
	protected:
		const tsdc::global::IFactory<Effect>* _factory;
}; // class Effect

class EffectManager : public Ogre::Singleton<EffectManager>
{		
	public:
		typedef std::list<Effect*> EffectList;
		typedef std::map<const char*, const tsdc::global::IFactory<Effect>* > EffectFactoryMap;
		typedef tsdc::global::IFactory<Effect> EffectFactory;
		
	public:
		EffectManager();
		virtual ~EffectManager();
		
	public:
		void RegisterEffect(const tsdc::global::IFactory<Effect> *factory);
		Effect* CreateEffect(const char *type);
		void DestroyEffect(Effect *effect);
		
	protected:
		EffectList _effects;
		EffectFactoryMap _factories;
}; // class EffectManager

}; // namespace tsdc

#endif // EFFECT_H
