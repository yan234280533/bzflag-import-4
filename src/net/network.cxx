/* bzflag
 * Copyright (c) 1993 - 2000 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if !defined(_WIN32)

#include "network.h"
#include "ErrorHandler.h"
#include "Address.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#if defined(_old_linux_)
#define hstrerror(x) sys_errlist[x]
#elif defined(sun)
#define hstrerror(x) "<network error>"
#endif

extern "C" {

void			nerror(const char* msg)
{
  if (msg)
    printError("%s: %s", msg, strerror(errno));
  else
    printError("%s", strerror(errno));
}

void			bzfherror(const char* msg)
{
  if (msg)
    printError("%s: %s", msg, hstrerror(h_errno));
  else
    printError("%s", hstrerror(h_errno));
}

int			getErrno()
{
  return errno;
}

}

int			BzfNetwork::setNonBlocking(int fd)
{
  int mode = fcntl(fd, F_GETFL, 0);
  if (mode == -1 || fcntl(fd, F_SETFL, mode | O_NDELAY) < 0)
    return -1;
  return 0;
}

#else /* defined(_WIN32) */

#include "network.h"
#include "ErrorHandler.h"
#include "Address.h"
#include <stdio.h>
#include <string.h>

extern "C" {

int			inet_aton(const char* cp, struct in_addr* pin)
{
  unsigned long a = inet_addr(cp);
  if (a == (unsigned long)-1) {
    // could be an error or cp could be a broadcast address.
    // FIXME -- this check is a little simplistic.
    if (strcmp(cp, "255.255.255.255") != 0) return 0;
  }

  pin->s_addr = a;
  return 1;
}

void			nerror(const char* msg)
{
  if (msg)
    printError("%s: %d (0x%x)", msg, getErrno(), getErrno());
  else
    printError("%d (0x%x)", getErrno(), getErrno());
}

void			herror(const char* msg)
{
  if (msg)
    printError("%s: error code %d", msg, getErrno());
  else
    printError("error code %d", getErrno());
}

int			getErrno()
{
  return WSAGetLastError();
}

}

int			BzfNetwork::setNonBlocking(int fd)
{
  int on = 1;
  return ioctl(fd, FIONBIO, &on);
}

#endif /* defined(_WIN32) */

// valid bzflag url's are:
//   http://<hostname>[:<port>][<path-to-text-file>]
//     connect to above HTTP server requesting named file.  response
//     must be another url of any valid form.  a 302 response code
//     (object moved) is understood and redirection is automatic.
//
//   file:<pathname>
//     reads from file.  contents must be a url of any valid form.
//     note: pathname can include a drive specifier on windows.
//     note: pathname must be absolute.
//
//   bzflist://<hostname>[:<port>]
//     named host must be a bzflag list server listening on port
//     (if not supplied, port is ServerPort + 1).  hostname can
//     be a hostname or IP address.
//
// note that partially formed urls are not allowed.  for example, the
// http:// cannot be elided.

BzfString		BzfNetwork::dereferenceHTTP(
				const BzfString& hostname, int port,
				const BzfString& pathname)
{
  // lookup server address
  Address address = Address::getHostAddress(hostname);
  if (address.isAny())
    return BzfString();

  // create socket
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    nerror("creating socket for HTTP");
    return BzfString();
  }

  // connect to HTTP server
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr = address;
  if (connect(fd, (CNCTType*)&addr, sizeof(addr)) < 0) {
    nerror("connecting to HTTP server");
    close(fd);
    return BzfString();
  }

  // form request
  BzfString data("GET ");
  data += pathname;			// FIXME -- escape special characters
  data += " HTTP/1.0\r\nHost: ";
  data += hostname;
  data += "\r\nAccept: */*\r\n\r\n";

  // send request
  if (send(fd, (const char*)data, data.getLength(), 0) < 0) {
    nerror("sending request to HTTP server");
    close(fd);
    return BzfString();
  }

  // slurp up reply
  char line[256];
  data = "";
  int size;
  while ((size = recv(fd, line, sizeof(line), 0)) > 0)
    data.append(line, size);

  // fail if there was an error receiving
  if (size < 0) {
    nerror("receiving reply from HTTP server");
    close(fd);
    return BzfString();
  }

  // close socket
  close(fd);

  // parse reply.  first check for HTTP response and get the result code.
  const char* scan = data;
  char code[4];
  if (strncmp(data, "HTTP/1.", 7) != 0)
    return BzfString();
  scan += 7;
  while (*scan && !isspace(*scan)) ++scan;
  while (*scan &&  isspace(*scan)) ++scan;
  memcpy(code, scan, 3);
  code[3] = '\0';

  // what was the result?  we only accept results 200 (OK) and 302
  // (object moved).
  if (strcmp(code, "200") == 0) {
    // skip past headers to body
    const char* body = strstr(scan, "\r\n\r\n");
    if (body)
      body += 4;
    else {
      body = strstr(scan, "\n\n");
      if (body)
	body += 2;
      else
	return BzfString();
    }

    // data is entire body
    return data(body - data);
  }
  else if (strcmp(code, "302") == 0) {
    // find Location: header
    const char* location = strstr(scan, "Location:");
    if (!location)
      return BzfString();

    // data is rest of Location: header line sans leading/trailing whitespace.
    // skip to beginning of url.
    location += 9;
    while (*location && isspace(*location))
      ++location;

    // find end of header line, minus whitespace
    const char* end = strchr(location, '\n');
    if (!end)
      return BzfString();
    while (end > location && isspace(end[-1]))
      --end;

    // get length
    const int urlLength = end - location;
    if (urlLength == 0)
      return BzfString();

    // copy url
    return data(location - data, urlLength);
  }

  return BzfString();
}

BzfString		BzfNetwork::dereferenceFile(
				const BzfString& pathname)
{
  // open file
  FILE* file = fopen(pathname, "r");
  if (!file)
    return BzfString();

  // slurp up file
  char line[256];
  BzfString data;
  while (fgets(line, sizeof(line), file))
    data += line;

  // close file
  fclose(file);

  return data;
}

void			BzfNetwork::insertLines(BzfStringAList& list,
				int index, const BzfString& data)
{
    const char* start, *end, *tail;

    start = data;
    while (*start) {
	// skip leading whitespace
	while (*start && isspace(*start))
	    start++;

	// find end of line or end of string
	end = start;
	while (*end && *end != '\r' && *end != '\n')
	    end++;

	// back up over trailing whitespace
	tail = end;
	while (tail > start && isspace(tail[-1]))
	    tail--;

	// if non-empty and not beginning with # then add to list
	if (end > start && *start != '#')
	    list.insert(data(start - data, end - start), index++);

	// go to next line
	start = end;
	while (*start == '\r' || *start == '\n')
	    start++;
    }
}

boolean			BzfNetwork::dereferenceURLs(
				BzfStringAList& list, int max,
				BzfStringAList& failedList)
{
    int i = 0;

    while (i < list.getLength() && i < max) {
	BzfString protocol, hostname, pathname;
	int port = 0;

	// parse next url
	parseURL(list[i], protocol, hostname, port, pathname);

	// dereference various protocols
	if (protocol == "http") {
	    // get data
	    if (port == 0) port = 80;
	    BzfString data = dereferenceHTTP(hostname, port, pathname);

	    // insert new URLs
	    if (data.getLength() == 0)
		failedList.append(list[i]);
	    else
		insertLines(list, i + 1, data);

	    // done with this URL
	    list.remove(i);
	}

	else if (protocol == "file") {
	    // get data
	    BzfString data = dereferenceFile(pathname);

	    // insert new URLs
	    if (data.getLength() == 0)
		failedList.append(list[i]);
	    else
		insertLines(list, i + 1, data);

	    // done with this URL
	    list.remove(i);
	}

	else if (protocol == "bzflist") {
	    // leave it alone
	    i++;
	}

	else {
	    // invalid protocol or url
	    failedList.append(list[i]);
	    list.remove(i);
	}
    }

    // remove any urls we didn't get to
    while (list.getLength() > max)
	list.remove(list.getLength() - 1);

    return (list.getLength() > 0);
}

// parse a url into its parts
boolean			BzfNetwork::parseURL(const BzfString& url,
				BzfString& protocol,
				BzfString& hostname,
				int& port,
				BzfString& pathname)
{
  static const char* defaultHostname = "localhost";

  // scan for :
  const char* base = url;
  const char* scan = base;
  while (*scan != '\0' && *scan != ':' && !isspace(*scan))
    ++scan;

  // url is bad if delimiter not found or is first character or whitespace
  // found.
  if (*scan == '\0' || scan == base || isspace(*scan))
    return False;

  // set defaults
  hostname = defaultHostname;

  // store protocol
  protocol = url(base - url, scan - base);
  scan++;

  // store hostname and optional port for some protocols
  if (protocol == "http" || protocol == "bzflist") {
    if (scan[0] == '/' && scan[1] == '/') {
      // scan over hostname and store it
      base = scan + 2;
      scan = base;
      while (*scan != '\0' && *scan != ':' &&
			*scan != '/' && *scan != '\\' && !isspace(*scan))
	++scan;
      if (isspace(*scan))
	return False;
      if (scan != base)
	hostname = url(base - url, scan - base);

      // scan over and store port number
      if (*scan == ':') {
	scan++;
	base = scan;
	while (isdigit(*scan))
	  ++scan;
	port = atoi(base);
      }

      // next character must be / or \ or there must be no next character
      if (*scan != '\0' && *scan != '/' && *scan != '\\')
	return False;
    }
  }
  base = scan;

  // store pathname
  if (*base != 0)
    pathname = url(base - url);
  else
    pathname = "";

  return True;
}
