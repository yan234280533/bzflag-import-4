/* bzflag
 * Copyright (c) 1993 - 2001 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* XWindow:
 *	OpenGL X window.
 */

#ifndef BZF_XWINDOW_H
#define	BZF_XWINDOW_H

#include "BzfWindow.h"
#include "XDisplay.h"
#include <X11/Xlib.h>
#include <GL/glx.h>

class XVisual;

class XWindow : public BzfWindow {
  public:
			XWindow(const XDisplay*, XVisual*);
			~XWindow();

    boolean		isValid() const;

    void		showWindow(boolean);

    void		getPosition(int& x, int& y);
    void		getSize(int& width, int& height) const;

    void		setTitle(const char*);
    void		setPosition(int x, int y);
    void		setSize(int width, int height);
    void		setMinSize(int width, int height);
    void		setFullscreen();

    void		warpMouse(int x, int y);
    void		getMouse(int& x, int& y) const;
    void		grabMouse();
    void		ungrabMouse();
    void		showMouse();
    void		hideMouse();

    void		setGamma(float);
    float		getGamma() const;
    boolean		hasGammaControl() const;

    void		makeCurrent();
    void		swapBuffers();
    void		makeContext();
    void		freeContext();

    // other X stuff
    static XWindow*	lookupWindow(Window);

    static void		reactivateAll();
    static void		deactivateAll();

  private:
    void		loadColormap();
    unsigned short	getIntensityValue(float i) const;
    static float	pixelField(int i, int bits, int offset);
    static void		countBits(unsigned long mask, int& num, int& offset);

  private:
    XDisplay::Rep*	display;
    Window		window;
    Colormap		colormap;
    GLXContext		context;
    boolean		noWM;
    boolean		defaultColormap;
    XWindow*		prev;
    XWindow*		next;
    XVisualInfo		visual;
    unsigned long*	colormapPixels;
    float		gammaVal;
    static XWindow*	first;
};

#endif // BZF_XWINDOW_H
