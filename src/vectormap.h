//  Copyright (C) 2007, 2008, 2009, 2012, 2015 Ben Asselstine
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Library General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#pragma once
#ifndef VECTORMAP_H
#define VECTORMAP_H

#include <sigc++/signal.h>

#include "overviewmap.h"
#include "input-events.h"

class City;

//! Draw city production on a miniature map graphic.
/**
 * 
 * This is a map where you can select a city's vector, i.e. the position where
 * freshly produced units automatically go to.  They show up at the destination
 * in 2 turns.
 * VectorMap is the image in the CityDialog which provides a detailed view of  
 * the production of a Player.  It draws City objects on top of the graphic
 * that the OverviewMap class provides, but changes the icons over the City
 * objects to depict production.  It is interactive in that it handles 
 * mouse clicks on City objects and planted standard objects, but the 
 * behaviour of what the click does can change based on other calls to methods
 * in this class.
 * It can show vectoring by drawing lines between cities, and it can show
 * which cities are producing Army production bases, and which are not.
 * Clicking can select that City object, or clicking can set a vectoring
 * destination for a selected City, or it can change all City objects vectoring
 * to the currently selected city by vectoring them all to the newly 
 * selected city.
 */
class VectorMap : public OverviewMap
{
public:
    //! The different ways of depicting vectoring on a VectorMap.
    enum ShowVectoring
      {
	//! Don't depict any vectoring at all.
	SHOW_NO_VECTORING,
	//! Only depict vectoring that involves the selected city.
	SHOW_ORIGIN_CITY_VECTORING,
	//! Depict vectoring for all City objects that the Player owns.
	SHOW_ALL_VECTORING
      };

    //! The different effects that a mouse click can have on a VectorMap.
    enum ClickAction
      {
	//! Clicking a City object makes it the newly selected City.
	CLICK_SELECTS,
	//! Clicking on a City or Planted standard vectors to the selected City.
	CLICK_VECTORS,
	//! Changes vectoring from the old selected city to the new city.
	/**
	 * Changes all cities vectoring to the currently selected city, to 
	 * vector to the newly selected city.  This makes the newly clicked 
	 * City object the selected city..
	 */
	CLICK_CHANGES_DESTINATION
      };

    //! Default constructor.
    /**
     * Make a new VectorMap.
     *
     * @param city    The City object that is initially selected.
     * @param vector  How much vectoring we're going to depict.
     * @param see_opponents_production  Whether or not we can click City 
     *                                  objects that belong to other Players.
     *                                  This is only useful when the 
     *                                  ClickAction is CLICK_SELECTS.
     */
    VectorMap(City *city, enum ShowVectoring vector, 
	      bool see_opponents_production);

    //! Realize a mouse button event.
    void mouse_button_event(MouseButtonEvent e);

    // Emitted whenever something is drawn on to the miniature map graphic.
    sigc::signal<void, Cairo::RefPtr<Cairo::Surface> > map_changed;

    //! Change what kind of vectoring is depicted on the VectorMap.
    void setShowVectoring (enum ShowVectoring v) { show_vectoring = v;}

    //! Return the currently selected City object.
    City* getCity() {return city;}

    //! show vectoring from a different city.
    void setCity(City *c);

    //! Change what happens when a City object is clicked on.
    void setClickAction (enum ClickAction a) { click_action = a;}

    //! Return what happens when a City object is clicked on.
    enum ClickAction getClickAction () { return click_action;}

private:
    //! The currently selected city object.
    City *city;

    //! The Player's planted standard.
    /**
     * This value is (-1,-1) if the Player does not have a standard planted
     * anywhere on the game map.
     */
    Vector<int> planted_standard;

    //! The current amount of vectoring that the VectorMap is showing.
    enum ShowVectoring show_vectoring;

    //! The current behaviour of a click on a City object.
    enum ClickAction click_action;

    //! Whether or not City objects belonging to other players can be selected.
    bool d_see_opponents_production;

    //! Depict the vectoring and production onto the miniature map graphic.
    /**
     * This method is automatically called by the VectorMap::draw method.
     */
    virtual void after_draw();

    //! Draw a city icon in the given fashion.
    /**
     * Draw an icon on top of a City object on the miniature map.
     *
     * @param city  The city to depict production or vectoring for.
     * @param type  When type is 0 it means it is the currently selected city.
     *              When type is 1 it means that the given city is not being
     *              vectored to or from.  When type is 2 it means that the 
     *              city is being vectored to.  When type is 3 it means that 
     *              the city is being vectored from.
     * @param prod  Whether or not we're showing production or vectoring.
     */
    void draw_city (City *city, guint32 &type, bool &prod);

    //! Draw a list of city objects in the given fashion.
    /**
     * @param citylist  The list of city objects to depict.
     * @param type  When type is 0 it means it is the currently selected city.
     *              When type is 1 it means that the given city is not being
     *              vectored to or from.  When type is 2 it means that the 
     *              city is being vectored to.  When type is 3 it means that 
     *              the city is being vectored from.
     */
    void draw_cities (std::list<City*> citylist, guint32 type);

    //! Draw lines from src to dests according to each city's vectoring policy.
    void draw_lines (std::list<City*> srcs, std::list<City*> dests);

    //! Draw a hero icon on the spot where a Hero has planted the standard.
    /**
     * @param pos  The position of a tile on the map to draw a hero icon.
     */
    void draw_planted_standard(Vector<int> pos);

    //! Draw a yellow line to a place on the map.
    /**
     * Draw a yellow line from the currently selected city to the given 
     * position on the map.
     *
     * @param dest  The position of a tile on the map to draw a line to.
     */
    void draw_vectoring_line_from_here_to (Vector<int> dest);

    //! Draw an orange line to a place on the map.
    /**
     * Draw an orange line from the given position on the map to the currently 
     * selected city.
     *
     * @param src   The position of a tile on the map to draw a line from.
     */
    void draw_vectoring_line_to_here_from (Vector<int> src);

    //! Draw a line between the given points, in a given color.
    /**
     * @param src   The position of a tile on the map to draw a line from.
     * @param dest  The position of a tile on the map to draw a line to.
     * @param to    Whether or not the line will be drawn in a yellow color
     *              or an orange color.  If true, then yellow.  Otherwise
     *              orange.
     */
    void draw_vectoring_line(Vector<int> src, Vector<int> dest, bool to);

    void draw_square_around_active_city();
};

#endif
