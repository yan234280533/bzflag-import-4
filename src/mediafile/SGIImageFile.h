/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef BZF_SGI_IMAGE_FILE_H
#define BZF_SGI_IMAGE_FILE_H

#include "ImageFile.h"

class SGIImageFile : public ImageFile {
public:
	SGIImageFile(std::istream*);
	virtual ~SGIImageFile();

	static std::string	getExtension();

	// ImageFile overrides
	virtual bool		read(void* buffer);

protected:
	bool				readVerbatim(void* buffer);
	bool				readRLE(void* buffer);

private:
	bool				isVerbatim;
};

#endif
// ex: shiftwidth=4 tabstop=4
