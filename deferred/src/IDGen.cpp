/*
 *  IDGen.cpp
 *  Samples
 *
 *  Created by Arris Ray on 8/27/09.
 *  Copyright 2009 The Secret Design Collective. All rights reserved.
 *
 */

#include "IDGen.h"
#include <sstream>

using namespace tsdc;

///////////////////////////////////////////////////////////////////////////////
// IDGEN IMPLEMENTATION
///////////////////////////////////////////////////////////////////////////////

IDGen::IDGen()
{
	uuid_create(&this->_uuid);
}

IDGen::~IDGen()
{
	uuid_destroy(this->_uuid);
}

std::string IDGen::GenerateID(const std::string &prefix, const std::string &suffix) const
{
	// Create a UUID
	char *struuid = NULL;
	uuid_make(this->_uuid, UUID_MAKE_V1);
	uuid_export(this->_uuid, UUID_FMT_STR, &struuid, NULL);
	
	// Create a unique name based on the auto-generated UUID
	std::stringstream id;
	id << prefix << struuid << suffix;
	return id.str();
}
