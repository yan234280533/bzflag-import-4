/* bzflag
 * Copyright 1993-1999, Chris Schoeneman
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 *
 */

#ifndef	BZF_MAIN_WINDOW_H
#define	BZF_MAIN_WINDOW_H

#define	USE_GL_STEREO

#include "common.h"

class BzfWindow;

class MainWindow {
  public:
    enum Quadrant {
			FullWindow,
			UpperLeft,
			UpperRight,
			LowerLeft,
			LowerRight,
			ZoomRegion,
			UpperHalf,
			LowerHalf
    };

			MainWindow(BzfWindow*);
			~MainWindow();

    BzfWindow*		getWindow() const { return window; }

    boolean		getQuit() const;
    int			getOriginX() const;
    int			getOriginY() const;
    int			getWidth() const;
    int			getHeight() const;
    int			getPanelHeight() const;
    int			getViewHeight() const;
    boolean		getFullscreen();

    void		setQuit();
    void		setPosition(int x, int y);
    void		setSize(int width, int height);
    void		setMinSize(int width, int panelHeight, int viewHeight);
    void		setPanelRatio(float);
    void		setFullscreen();
    void		setNoMouseGrab();

    void		setZoomFactor(int);
    void		setQuadrant(Quadrant);

    void		showWindow(boolean = True);
    void		warpMouse();
    void		grabMouse();
    void		ungrabMouse();

    // FIXME -- try to get rid of this
    void		getMousePosition(int& mx, int& my) const;

  private:
    // no copying
			MainWindow(const MainWindow&);
    MainWindow&		operator=(const MainWindow&);

    void		resize();
    static void		resize(void*);

  private:
    BzfWindow*		window;
    boolean		quit;
    Quadrant		quadrant;
    float		panelRatio;
    boolean		isFullscreen;
    boolean		allowMouseGrab;
    int			zoomFactor;
    int			trueWidth, trueHeight;
    int			xOrigin, yOrigin;
    int			width;
    int			panelHeight;
    int			viewHeight;
    int			minWidth;
    int			minPanelHeight;
    int			minViewHeight;
};

//
// MainWindow
//

inline boolean		MainWindow::getQuit() const
{
  return quit;
}

inline int		MainWindow::getOriginX() const
{
  return xOrigin;
}

inline int		MainWindow::getOriginY() const
{
  return yOrigin;
}

inline int		MainWindow::getWidth() const
{
  return width;
}

inline int		MainWindow::getHeight() const
{
  return panelHeight + viewHeight;
}

inline int		MainWindow::getPanelHeight() const
{
  return panelHeight;
}

inline int		MainWindow::getViewHeight() const
{
  return viewHeight;
}

#endif // BZF_MAIN_WINDOW_H
