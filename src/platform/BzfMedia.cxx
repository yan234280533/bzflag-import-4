/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "BzfMedia.h"
#include "TimeKeeper.h"
#include "wave.h"
#include <string.h>

BzfMedia::BzfMedia() : mediaDir("data") { }
BzfMedia::~BzfMedia() { }

double			BzfMedia::stopwatch(bool start)
{
  static TimeKeeper prev;
  if (start) {
    prev = TimeKeeper::getCurrent();
    return 0.0;
  }
  else {
    return (double)(TimeKeeper::getCurrent() - prev);
  }
}

std::string		BzfMedia::getMediaDirectory() const
{
  return mediaDir;
}

void			BzfMedia::setMediaDirectory(const std::string& _dir)
{
  mediaDir = _dir;
}

unsigned char*		BzfMedia::readImage(const std::string& filename,
				int& width, int& height, int& depth) const
{
  // try mediaDir/filename
  std::string name = makePath(mediaDir, filename);
  unsigned char* image = doReadImage(name.c_str(), width, height, depth);
  if (image) return image;

  // try filename as is
  image = doReadImage(filename.c_str(), width, height, depth);
  if (image) return image;

#if defined(INSTALL_DATA_DIR)
  // try standard-mediaDir/filename
  name = makePath(INSTALL_DATA_DIR, filename);
  image = doReadImage(name.c_str(), width, height, depth);
  if (image) return image;
#endif

  // try mediaDir/filename with replaced extension
  name = replaceExtension(makePath(mediaDir, filename), getImageExtension());
  image = doReadImage(name.c_str(), width, height, depth);
  if (image) return image;

  // try filename with replaced extension
  name = replaceExtension(filename, getImageExtension());
  image = doReadImage(name.c_str(), width, height, depth);
  if (image) return image;

#if defined(INSTALL_DATA_DIR)
  // try standard-mediaDir/filename with replaced extension
  name = makePath(INSTALL_DATA_DIR, filename);
  name = replaceExtension(name, getImageExtension());
  image = doReadImage(name.c_str(), width, height, depth);
  if (image) return image;
#endif

  return NULL;
}

float*			BzfMedia::readSound(const std::string& filename,
				int& numFrames, int& rate) const
{
  // try mediaDir/filename
  std::string name = makePath(mediaDir, filename);
  float* sound = doReadSound(name.c_str(), numFrames, rate);
  if (sound) return sound;

  // try filename as is
  sound = doReadSound(filename.c_str(), numFrames, rate);
  if (sound) return sound;

#if defined(INSTALL_DATA_DIR)
  // try standard-mediaDir/filename
  name = makePath(INSTALL_DATA_DIR, filename);
  sound = doReadSound(name.c_str(), numFrames, rate);
  if (sound) return sound;
#endif

  // try mediaDir/filename with replaced extension
  name = replaceExtension(makePath(mediaDir, filename), getSoundExtension());
  sound = doReadSound(name.c_str(), numFrames, rate);
  if (sound) return sound;

  // try filename with replaced extension
  name = replaceExtension(filename, getSoundExtension());
  sound = doReadSound(name.c_str(), numFrames, rate);
  if (sound) return sound;

#if defined(INSTALL_DATA_DIR)
  // try mediaDir/filename with replaced extension
  name = makePath(INSTALL_DATA_DIR, filename);
  name = replaceExtension(name, getSoundExtension());
  sound = doReadSound(name.c_str(), numFrames, rate);
  if (sound) return sound;
#endif

  return NULL;
}

std::string		BzfMedia::makePath(const std::string& dir,
				const std::string& filename) const
{
  if ((dir.length() == 0) || filename[0] == '/') return filename;
  std::string path = dir;
  path += "/";
  path += filename;
  return path;
}

std::string		BzfMedia::replaceExtension(
				const std::string& pathname,
				const std::string& extension) const
{
  std::string newName;

  const int extPosition = findExtension(pathname);
  if (extPosition == 0)
    newName = pathname;
  else
    newName = pathname.substr(0, extPosition);

  newName += ".";
  newName += extension;
  return newName;
}

int			BzfMedia::findExtension(const std::string& pathname) const
{
  const char* string = pathname.c_str();
  int scan = pathname.length();
  while (--scan > 0) {
    if (string[scan] == '.')
      return scan;
    if (string[scan] == '/')
      break;
  }
  return 0;
}

std::string		BzfMedia::getImageExtension() const
{
  return std::string("rgb");
}

std::string		BzfMedia::getSoundExtension() const
{
  return std::string("wav");
}

unsigned char*		BzfMedia::doReadImage(const char* filename,
				int& dx, int& dy, int& dz) const
{
  // open file
  FILE* file = fopen(filename, "rb");
  if (!file) return NULL;

  // read header
  unsigned char header[512];
  const size_t numItems = fread(header, sizeof(header), 1, file);
  if (numItems != 1 ||
	getShort(header + 0) != 474 ||
	(header[2] != 0 && header[2] != 1) ||
	(header[3] < 1 || header[3] > 1) ||
	(getUShort(header + 4) < 1 || getUShort(header + 4) > 3) ||
	(getLong(header + 104) != 0)) {
    fclose(file);
    return NULL;
  }

  // get dimensions
  uint16_t width, height, depth;
  width  = getUShort(header + 6);
  height = (getUShort(header + 4) < 2) ? 1 : getUShort(header + 8);
  depth  = (getUShort(header + 4) < 3) ? 1 : getUShort(header + 10);
  dx = (int)width;
  dy = (int)height;
  dz = (int)depth;

  // make image buffer
  unsigned char* image = new unsigned char[4 * dx * dy];

  // read scan lines
  bool okay;
  if (header[2] == 0)
    okay = doReadVerbatim(file, dx, dy, dz, image);
  else
    okay = doReadRLE(file, dx, dy, dz, image);

  // done with file
  fclose(file);

  if (!okay) {
    delete[] image;
    image = NULL;
  }

  // handle different image depths
  if (dz == 1) {
    // r=g=b, a=max
    int n = dx * dy;
    unsigned char* scan = image;
    for (; n > 0; --n) {
      scan[2] = scan[1] = scan[0];
      scan[3] = 0xff;
      scan += 4;
    }
  }
  else if (dz == 2) {
    // r=g=b, move alpha channel
    int n = dx * dy;
    unsigned char* scan = image;
    for (; n > 0; --n) {
      scan[3] = scan[1];
      scan[2] = scan[1] = scan[0];
      scan += 4;
    }
  }
  else if (dz == 3) {
    // a=max
    int n = dx * dy;
    unsigned char* scan = image + 3;
    for (; n > 0; --n) {
      *scan = 0xff;
      scan += 4;
    }
  }

  return image;
}

int16_t			BzfMedia::getShort(const void* ptr)
{
  const unsigned char* data = (const unsigned char*)ptr;
  return ((int16_t)data[0] << 8) + (int16_t)data[1];
}

uint16_t		BzfMedia::getUShort(const void* ptr)
{
  const unsigned char* data = (const unsigned char*)ptr;
  return ((uint16_t)data[0] << 8) + (uint16_t)data[1];
}

int32_t			BzfMedia::getLong(const void* ptr)
{
  const unsigned char* data = (const unsigned char*)ptr;
  return ((int32_t)data[0] << 24) + ((int32_t)data[1] << 16) +
	((int32_t)data[2] << 8) + (int32_t)data[3];
}

bool			BzfMedia::doReadVerbatim(FILE* file,
				int dx, int dy, int dz,
				unsigned char* image)
{
  // zero image data
  memset(image, 0, 4 * dx * dy);

  // how many channels to read?
  if (dz > 4)
    dz = 4;

  // read each channel one after the other
  unsigned char row[4096];
  for (int k = 0; k < dz; ++k) {
    unsigned char* dst = image + k;
    for (int j = 0; j < dy; ++j) {
      // read raw data
      if (fread(row, dx, 1, file) != 1)
	return false;

      // reformat
      for (int i = 0; i < dx; ++i) {
	*dst = row[i];
	dst += 4;
      }
    }
  }
  return true;
}

bool			BzfMedia::doReadRLE(FILE* file,
				int dx, int dy, int dz,
				unsigned char* image)
{
  // zero image data
  memset(image, 0, 4 * dx * dy);

  // how many channels to read?
  if (dz > 4)
    dz = 4;

  // read offset tables
  const int tableSize = dy * dz;
  int32_t* startTable  = new int32_t[tableSize];
  int32_t* lengthTable = new int32_t[tableSize];
  if (fread(startTable, 4 * tableSize, 1, file) != 1) {
    return false;
  }
  if (fread(lengthTable, 4 * tableSize, 1, file) != 1) {
    delete[] startTable;
    return false;
  }

  // convert offset tables to proper endianness
  for (int n = 0; n < tableSize; ++n) {
    startTable[n]  = getLong(startTable + n);
    lengthTable[n] = getLong(lengthTable + n);
  }

  // read each channel one after the other
  unsigned char row[4096];
  for (int k = 0; k < dz; ++k) {
    unsigned char* dst = image + k;
    for (int j = 0; j < dy; ++j) {
      // read raw data
      const int32_t length = lengthTable[j + k * dy];
      if (fseek(file, startTable[j + k * dy], SEEK_SET) < 0 ||
	  fread(row, length, 1, file) != 1) {
	delete[] startTable;
	delete[] lengthTable;
	return false;
      }

      // decode
      unsigned char* src = row;
      while (1) {
	// check for error in image
	if (src - row >= length) {
	  delete[] startTable;
	  delete[] lengthTable;
	  return false;
	}

	// get next code
	const unsigned char type = *src++;
	int count = (int)(type & 0x7f);

	// zero code means end of row
	if (count == 0)
	  break;

	if (type & 0x80) {
	  // copy count pixels
	  while (count--) {
	    *dst = *src++;
	    dst += 4;
	  }
	}
	else {
	  // repeat pixel count times
	  const unsigned char pixel = *src++;
	  while (count--) {
	    *dst = pixel;
	    dst += 4;
	  }
	}
      }
    }
  }

  delete[] startTable;
  delete[] lengthTable;
  return true;
}

float*			BzfMedia::doReadSound(const char* filename,
				int& numFrames, int& rate) const
{
  short format, channels, width;
  long speed;
  FILE* file;
  char *rawdata, *rawpos;
  int i;
  float *data;

  file = openWavFile(filename, &format, &speed, &numFrames, &channels, &width);
  if (!file) return 0;
  rate=speed;
  rawdata=new char[numFrames*width*channels];
  if (readWavData(file, rawdata, numFrames*channels, width)) {
    fprintf(stderr, "Failed to read the wav data\n");
    delete [] rawdata;
    closeWavFile(file);
    return 0;
  }
  closeWavFile(file);

#ifdef HALF_RATE_AUDIO
  numFrames/=2;
  rate/=2;
#endif
  data = new float[2*numFrames];
  rawpos=rawdata;
  for (i=0; i<numFrames; i++) {
    if (channels==1) {
      if (width==1) {
	data[i*2]=data[i*2+1] = (*(signed char*)rawpos)*256.0f;
	rawpos++;
#ifdef HALF_RATE_AUDIO
	rawpos++;
#endif
      } else {
	data[i*2]=data[i*2+1] = *(short*)rawpos;
	rawpos+=2;
#ifdef HALF_RATE_AUDIO
	rawpos+=2;
#endif
      }
    } else {
      if (width==1) {
	data[i*2] = (*(signed char*)rawpos)*256.0f;
	rawpos++;
	data[i*2+1] = (*(signed char*)rawpos)*256.0f;
	rawpos++;
#ifdef HALF_RATE_AUDIO
	rawpos+=2;
#endif
      } else {
	data[i*2] = *(short*)rawpos;
	rawpos+=2;
	data[i*2+1] = *(short*)rawpos;
	rawpos+=2;
#ifdef HALF_RATE_AUDIO
	rawpos+=4;
#endif

      }
    }
  }
  delete [] rawdata;
  return data;
}
// ex: shiftwidth=2 tabstop=8
