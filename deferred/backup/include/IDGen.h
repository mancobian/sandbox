/*
 *  IDGen.h
 *  Samples
 *
 *  Created by Arris Ray on 8/27/09.
 *  Copyright 2009 The Secret Design Collective. All rights reserved.
 *
 */
 #ifndef IDGEN_H
 #define IDGEN_H
 
#include <string>
#include <uuid.h>

namespace tsdc {

class IDGen
{
	public:
		IDGen();
		virtual ~IDGen();
		
	public:
		virtual std::string GenerateID(const std::string &prefix = ""
			, const std::string &suffix = "") const;

	protected:
		uuid_t *_uuid;
}; // class IDGen

}; // namespace tsdc

#endif // IDGEN_H
