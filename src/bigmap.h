// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004, 2005 Bryan Duff
// Copyright (C) 2004, 2005, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2012, 2014, 2015, 2017, 2020,
// 2021 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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
#ifndef BIGMAP_H
#define BIGMAP_H

#include <sigc++/signal.h>
#include <sigc++/trackable.h>
#include <sigc++/connection.h>
#include <gtkmm.h>

#include "vector.h"
#include "map-tip-position.h"
#include "rectangle.h"
#include "PixMask.h"
#include "LocationBox.h"

class Player;
class Stack;
class MapRenderer;
class Location;

//! Draws the game objects and terrain on the large map.
/** The large map
  * 
  * The detailed map in which all the action takes place. Handles scrolling
  * and mouse clicks via signals.
  *
  * Draws everything to a buffer to simplify scrolling, the buffer is then
  * blitted to the screen. The current view of the map is kept track of both
  * approximately (in tiles) and more precisely in pixels.
  */
class BigMap: public sigc::trackable
{
 public:
    static bool s_show_hidden_ruins;

    BigMap(bool headless);
    virtual ~BigMap();

    // draw everything
    void draw(bool redraw_buffer = true);

    bool get_toggled () const {return d_grid_toggled;}
    LwRectangle get_view () const {return view;} // in tiles

    // view the rectangle, measured in tiles
    void set_view(LwRectangle rect);
    void screen_size_changed(Gtk::Allocation box);
    Gtk::Allocation get_allocation() {return image;};

    // return a good position of a map tip given that it should be close to the
    // tiles in tile_area without covering them
    MapTipPosition map_tip_position(LwRectangle tile_area);
    MapTipPosition map_tip_position(Vector<int> tile);

    // emitted when the view has changed because of user interactions
    sigc::signal<void, LwRectangle> view_changed;

    // Emitted after a call to SmallMap::Draw.
    /**
     * Classes that use BigMap must catch this signal to display the map.
     */
    sigc::signal<void, Cairo::RefPtr<Cairo::Surface> > map_changed;
    void blank(bool on);

    //the game object sets this when the active stack is fighting so we can 
    //draw a fight graphic, or not
    void setFighting(LocationBox ruckus) {d_fighting = ruckus;};

    //! Save the whole map as one big image (bmp file).
    bool saveAsBitmap(Glib::ustring filename);

    void toggle_grid();
    bool scroll(GdkEventScroll *event);
    
    Cairo::RefPtr<Cairo::Surface> get_surface() const {return outgoing;}

    //! Calculate the zoom factor.
    /**
     */
    static double get_default_zoom_scale (int screen_height);

 protected:
    bool d_headless;
    MapRenderer* d_renderer;

    LwRectangle view;		// approximate view of screen, in tiles
    Vector<int> view_pos; 	// precise position of view in pixels

    Cairo::RefPtr<Cairo::Surface> buffer;	// the buffer we draw things in
    Cairo::RefPtr<Cairo::Surface> outgoing; //goes out to the gtk::image
    Cairo::RefPtr<Cairo::Context> buffer_gc;
    LwRectangle buffer_view;	// current view of the buffer, in tiles

    bool input_locked;
    bool blank_screen;

    bool d_grid_toggled;
    Gtk::Allocation image;
    double deltax; //for smooth scrolling
    double deltay;

    // helpers
    Vector<int> mouse_pos_to_tile(Vector<int> pos);
    Vector<int> tile_to_buffer_pos(Vector<int> tile);
    Vector<int> get_view_pos_from_view();
    void draw_buffer();  

    virtual void after_draw() { }

 protected:
    void draw_stack(Stack *s, Cairo::RefPtr<Cairo::Surface> surface);
    LocationBox d_fighting;
 private:
    void draw_buffer(const LwRectangle &map_view, Cairo::RefPtr<Cairo::Surface> surface);
    void draw_buffer_tiles(const LwRectangle &map_view, Cairo::RefPtr<Cairo::Surface> surface);

    void draw_buffer_tile(Vector<int> tile, Cairo::RefPtr<Cairo::Surface> surface);
    void clip_viewable_buffer(Cairo::RefPtr<Cairo::Surface> pixmap, Vector<int> pos, Cairo::RefPtr<Cairo::Surface> out);
};

#endif
