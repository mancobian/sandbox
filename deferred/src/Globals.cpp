/*
 *  Globals.cpp
 *  Samples
 *
 *  Created by Arris Ray on 6/25/09.
 *  Copyright 2009 The Secret Design Collective. All rights reserved.
 *
 */

#include "Globals.h"

std::string tsdc::utils::io::ParseFileExtension(const std::string &filename)
{
	size_t dot_index = filename.find_last_of(".");
	if (dot_index == std::string::npos) return "";
	return filename.substr(dot_index + 1);
}

bool tsdc::utils::io::HasFileExtension(const std::string &filename)
{
	if (!tsdc::utils::io::ParseFileExtension(filename).empty())
		return true;
	return false;
}