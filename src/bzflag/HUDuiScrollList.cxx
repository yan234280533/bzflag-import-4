/* bzflag
 * Copyright (c) 1993 - 2008 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface headers
#include "HUDuiScrollList.h"

// common implementation headers
#include "BundleMgr.h"
#include "Bundle.h"
#include "FontManager.h"
#include "LocalFontFace.h"
#include "HUDui.h"

#include <iostream>

//
// HUDuiScrollList
//

HUDuiScrollList::HUDuiScrollList() : HUDuiControl(), index(-1), visiblePosition(0), numVisibleItems(1), pagedList(false), pageLabel(NULL)
{
  // do nothing
}

HUDuiScrollList::HUDuiScrollList(bool paged) : HUDuiControl(), index(-1), visiblePosition(0), numVisibleItems(1), pagedList(paged), pageLabel(new HUDuiLabel)
{
  // do nothing
}

HUDuiScrollList::~HUDuiScrollList()
{
  // do nothing
}

int HUDuiScrollList::getSelected() const
{
  return index;
}

void HUDuiScrollList::clear()
{
	items.clear();
	setSelected(0);
}

void HUDuiScrollList::setSelected(int _index)
{
	// Ensure the index is not negative, or past the end of our list
	if (_index < 0)
		_index = 0;
	else if (_index >= (int)items.size())
		_index = (int)items.size() - 1;
	
	if (pagedList) {
		// Figure out what page the new index is on
		int newPage = (_index/numVisibleItems) + 1;
		visiblePosition = _index - ((newPage - 1)*numVisibleItems);
	} else {
		// The new index falls within the portion of the list already on screen
		if ((_index >= index - visiblePosition)&&(_index < (index + (numVisibleItems - visiblePosition)))) {
			visiblePosition = visiblePosition + (_index - index);
		// Moving one down outside of list range
		} else if (_index == (index + (numVisibleItems - visiblePosition))) {
			visiblePosition = numVisibleItems - 1;
		// Moving one up outside of list range
		} else if (_index == ((index - visiblePosition - 1))) {
			visiblePosition = 0;
		// Jumping to a different part of the list
		} else {
			// Jump to that part of the list and set the new index as first
			visiblePosition = 0;
		}
	}
	
	index = _index;
}

// Adds a new item to our scrollable list
void HUDuiScrollList::addItem(HUDuiLabel* item)
{
	// We need to update both the stringList and the labelList
	//items.push_back(new HUDuiScrollListItem(item));
	update();
}

void HUDuiScrollList::addItem(std::string item)
{
	//HUDuiLabel* newLabel = new HUDuiLabel;
	//newLabel->setFontFace(getFontFace());
	//newLabel->setString(item);
	
	//addItem(newLabel);
	//items.push_back(new HUDuiScrollListItem(item));
	HUDuiScrollListItem* newItem = new HUDuiScrollListItem(item);
	items.push_back(newItem); // SEEMS TO BE THE PROBLEM CODE RIGHT HERE
	update();
}

void HUDuiScrollList::update()
{
	setSelected(index);
}

void HUDuiScrollList::setPaged(bool paged)
{
	pagedList = paged;
	if (pagedList) {
		pageLabel = new HUDuiLabel;
	} else {
		pageLabel = NULL;
	}
	resizeLabels();
	update();
}

bool HUDuiScrollList::doKeyPress(const BzfKeyEvent& key)
{
	// Figure out what page the user is on
	int currentPage = (index/numVisibleItems) + 1;

	if (key.chr == 0)
		switch (key.button) {
			case BzfKeyEvent::Up:
				if (index != -1) {
					setSelected(index - 1);
					doCallback();
				}
				break;
				
			case BzfKeyEvent::Down:
				if (index != -1) {
					setSelected(index + 1);
					doCallback();
				}
				break;
				
			case BzfKeyEvent::PageUp:
				if ((pagedList)&&(index != -1)) {
					//  Jump back to the previous page
					setSelected((currentPage - 2)*numVisibleItems);
					doCallback();
				}
				break;
				
			case BzfKeyEvent::PageDown:
				if ((pagedList)&&(index != -1)) {
					//  Skip to the next page
					setSelected((currentPage)*numVisibleItems);
					doCallback();
				}
				break;
				
			// Testing purposes only
			case BzfKeyEvent::Home:
				setPaged(!pagedList);
				break;

			default:
				return false;
	}

	switch (key.chr) {
		case 13:
		case 27:
			return false;
	}

	return true;
}

bool HUDuiScrollList::doKeyRelease(const BzfKeyEvent&)
{
  // ignore key releases
  return false;
}

// Update our scrollable list when the size is changed
void HUDuiScrollList::setSize(float width, float height)
{
	HUDuiControl::setSize(width, height);
	resizeLabels();
}

// Update our scrollable list whe the font size is changed
void HUDuiScrollList::setFontSize(float size)
{
	HUDuiControl::setFontSize(size);
	resizeLabels();
}

// Change our label sizes to match any changes to our scrollable list
void HUDuiScrollList::resizeLabels()
{
	// Determine how many items are visible
	FontManager &fm = FontManager::instance();
	float itemHeight = fm.getStringHeight(getFontFace()->getFMFace(), getFontSize());
	float listHeight = getHeight();
	numVisibleItems = listHeight/itemHeight;
	
	// If it's a paged list make it one item shorter so we can fit the page label
	if (pagedList) {
		numVisibleItems = numVisibleItems - 1;
	}
	
	//HUDuiScrollListItem item;
	
	std::list<HUDuiScrollListItem*>::iterator it;
	
	// Iterate through our list of strings
	for (it=items.begin(); it != items.end(); it++)	{
		//tempString = stringList.at(i);
		HUDuiScrollListItem* item = *it;
		item->shorten(getWidth());
		
		// Shorten the label to fit within the scrollable list
		//if (tempString.length() > numVisibleChars) {
		//	tempString = tempString.substr(0, numVisibleChars);
		//}
		
		//item->setString(tempString);
	}
	
	/*
	// Trim strings to fit our available space
	float width = getWidth();



	std::string tempString;
	HUDuiLabel* item;

	for (size_t i = 0; i < stringList.size(); ++i) {


		item = labelList.at(i);
		std::string tempStr = stringList.at(i);

		// skip if it already fits
		if (fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), tempStr.c_str()) > width)
			continue;

		// expensive
		UTF8StringItr lastPos = stringList.at(i).c_str();
		for (UTF8StringItr str = lastPos; str.getBufferFromHere() != NULL; ++str) {
			// is it too big yet?
			if (fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), 
							tempStr.substr(str.getBufferFromHere() - tempStr.c_str()).c_str()) > width) {
				break;
			}
			lastPos = str;
		}
		item->setString(tempStr.substr(0, lastPos.getBufferFromHere() - tempStr.c_str()));
	}
	*/
}

void HUDuiScrollList::doRender()
{
	FontManager &fm = FontManager::instance();
	float itemHeight = fm.getStringHeight(getFontFace()->getFMFace(), getFontSize());
	
	std::list<HUDuiScrollListItem*>::iterator it;
	
	// Draw the list items
	for (int i = (getSelected() - visiblePosition); i<((numVisibleItems - visiblePosition) + getSelected()); i++) {
		if (i < items.size()) {
			std::advance(it, i);
			HUDuiScrollListItem* item = *it;
			item->setFontSize(getFontSize());
			item->setPosition(getX(), (getY() - itemHeight*(i-(getSelected() - visiblePosition))));
			//item->setDarker(i != getSelected());
			item->render();
		}
	}
	
	// Draw the page label
	if (pagedList) {
		int numPages = ((items.size() - 1)/numVisibleItems) + 1;
		int currentPage = (getSelected()/numVisibleItems) + 1;
		
		std::vector<std::string> args;
		char msg[50];
		
		sprintf(msg, "%ld", currentPage);
		args.push_back(msg);
		sprintf(msg, "%ld", numPages);
		args.push_back(msg);
		
		pageLabel->setString("Page: {1}/{2}", &args);
		
		float labelWidth = fm.getStringWidth(getFontFace()->getFMFace(), getFontSize(), pageLabel->getString().c_str());
		
		pageLabel->setFontFace(getFontFace());
		pageLabel->setFontSize(getFontSize());
		pageLabel->setPosition((getX() +(getWidth() - labelWidth)), (getY() - itemHeight*(numVisibleItems + 1)));
		pageLabel->render();
	}
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
