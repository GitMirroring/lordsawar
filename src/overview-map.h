// Copyright (C) 2006 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012, 2014, 2015, 2017,
// 2020 Ben Asselstine
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
#ifndef OVERVIEWMAP_H
#define OVERVIEWMAP_H

#include <sigc++/trackable.h>
#include <gtkmm.h>
#include "vector.h"
#include "rectangle.h"
#include "Tile.h"
#include "SmallTile.h"

class Maptile;
class Player;
class City;

//! Generates a miniature graphic of the game map.
/**
 * This is a base class that draws the terrain, ruins, temples, and roads 
 * onto a Gdk::Pixmap.
 * This class is responsible for drawing terrain features using the correct
 * pattern, as specified in Tile::Pattern.  Ruins and Temples are drawn as 
 * white dots, and roads are drawn as brown lines.
 * If a tile on the game map is obscured due to fog of war on a hidden map,
 * it appears as black.
 * This class provides a method for optionally drawing cities onto the map.
 * Derived classes can add their own stuff to the map by overriding the 
 * after_draw method that is called by OverviewMap::draw.
 */
class OverviewMap : public sigc::trackable
{
 public:
     //! Default constructor.
     /**
      * Make a new overview map.  This constructor doesn't do anything, the
      * real work is done in the OverviewMap::resize and OverviewMap::draw
      * methods.
      * This constructor is not normally called by itself, usually it is
      * called by a derived class.
      */
     OverviewMap(bool headless = false);

     //! Destructor.
     virtual ~OverviewMap() {};


    //! Draw and scale the miniature map graphic to the given size.
    /**
     * This method is responsible for drawing the terrain (e.g. grass, water, 
     * etc), but not the terrain features (roads, ruins, etc).
     * It will draw a map that is max_dimensions large, but will keep it's
     * aspect ratio.
     * This method should be called before the draw method.  
     *
     * @param max_dimensions  Two integers; the first of which dictates the
     *                        width of the map graphic, and the second dictates
     *                        the height.
     * @param scale A real number representing how zoomed out the graphic
     *              should appear.  1.0 means not zoomed out at all.  2 means
     *              zoomed out a little more.  this sets the map_tiles_per_tile
     *              member.
     *
     */
    void resize(Vector<int> max_dimensions, float scale = 1.0);

    //! Draw and scale the mini map graphic to the correct size of the game map.
    /**
     * @note This method depends on the map being one of the 3 sizes as 
     * defined in the GamePreferencesDialog.
     * This method should be called before the draw method.
     */
    void resize();

    //! Draw the terrain features (roads, ruins, etc) on the map.
    /**
     * Draws the roads, cities, ruins, temples and roads onto the map in the
     * correct aspect ratio.
     *
     * The aspect ratio for the terrain features is governed by the dimensions 
     * passed to the OverviewMap::resize method.
     *
     * This method calls the after_draw method from the derived classes.
     */
    void draw();

    //! Redraw a portion of the map graphic.
    /**
     * This method draws the terrain (water, grass, etc) for the given 
     * portion of the screen.
     *
     * @note This method redraws all terrain features (roads, ruins, etc), 
     *       including the features outside of the given portion.
     *
     * @param tiles  The rectangle to redraw.
     */
    void redraw_tiles(LwRectangle tiles);

    //! Returns the map graphic.
    /**
     * It only makes sense to get the surface after OverviewMap::resize and 
     * OverviewMap::draw have been called.
     */
    Cairo::RefPtr<Cairo::Surface> get_surface();


    static void draw_terrain_tile (Cairo::RefPtr<Cairo::Context> gc,
				SmallTile::Pattern pattern,
				Gdk::RGBA first, 
				Gdk::RGBA second,
				Gdk::RGBA third,
				int i, int j, bool shadowed);

    static void draw_pixel(Cairo::RefPtr<Cairo::Context> gc, int x, int y, const Gdk::RGBA color);


    void draw_filled_rect(int x, int y, int width, int height, const Gdk::RGBA color);

    void draw_rect(int x, int y, int width, int height, const Gdk::RGBA color);

    void draw_line( int src_x, int src_y, int dst_x, int dst_y, const Gdk::RGBA color);

    //! Make the map go black.
    void blank(bool on);

    static int calculatePixelsPerTile(int width, int height);
    static int calculatePixelsPerTile();

    static Vector<int> calculate_smallmap_size();

    static void draw_radial_gradient(Cairo::RefPtr<Cairo::Surface> surface, Gdk::RGBA inner, Gdk::RGBA outer, int width, int height);

    int get_width();
    int get_height();

 private:

    //! Returns whether or not the given pixel appears sunken (Tile::SUNKEN).
    /**
     * The leftmost and bottommost pixels of a sunken terrain type are shaded
     * in a different color.  This method returns whether or not a given pixel
     * should be shaded.
     *
     * @param type  The tile associated with the pixel location must be of
     *              this type.  If not, then this method always returns false.
     * @param i     The pixel on the horizontal axis on the map graphic that 
     *              we're querying.
     * @param j     The pixel on the vertical axis on the map graphic that
     *              we're querying.
     *
     * @return True if the given pixel location should be shaded, false if not.
     */
    bool isShadowed(Tile::Type type, int i, int j);

    //! Draw the given tile at the given pixel location on the map.
    /**
     * This method draws a square that is Overviewmap::pixels_per_tile pixels
     * in height and width.  It draws it at position (i,j) on the graphic.
     * This method uses the pattern assocaited the given Maptile to know 
     * what this box should look like.
     *
     * @param tile  The tile of the game map we're trying to draw.
     * @param i     The pixel on the horizontal axis of the map graphic.  This
     *              value must be a multiple of OverviewMap::pixels_per_tile.
     * @param j     The pixel on the vertical axis of the map graphic.  This
     *              value must be a multiple of OverviewMap::pixels_per_tile.
     */
    void draw_terrain_tile (Maptile *tile, int i, int j);


    void choose_surface(bool front, Cairo::RefPtr<Cairo::Surface> &surf,
				 Cairo::RefPtr<Cairo::Context> &gc);
    void draw_filled_rect(bool front, int x, int y, int width, int height, Gdk::RGBA color);

    void draw_rect(bool front, int x, int y, int width, int height, Gdk::RGBA color);

    void draw_line(bool front, int src_x, int src_y, int dst_x, int dst_y, Gdk::RGBA color);
 protected:

    //! Every pixel on the graphic is this wide and tall.  2 is normal.
    /**
     * The minimum value for this variable is 2, so that we can draw stipples
     * and other effects per terrain type.
     */
    double pixels_per_tile;

    //! The number of map tiles that each overview map tile represents.
    /**
     * Each tile on the graphic represents this many map tiles tall and wide.
     * 1x1 is normal.  2x2 skips 1 row and column per tile. 
     * 3x3 skips 2 rows and columns per tile.
     * The minimum value for this variable is 1.
     */
    double map_tiles_per_tile;

    //! Maps the given point in graphic coordinates to a game map coordinate.
    Vector<int> mapFromScreen(Vector<int> pos);

    //! And the other way round. Map a map coordinate to a surface pixel.
    Vector<int> mapToSurface(Vector<int> pos);

    //! A hook method for derived classes to put features on the map.
    virtual void after_draw();

    //! This method draws the cities onto the map.
    /**
     * Scan through all of the cities in the citylist, and draw each one with
     * a small shield graphic belonging to the owner of that city.
     *
     * Derived classes may call this method in their after_draw method.
     *
     * @param all_razed  Show each city as if it were razed.
     */
    void draw_cities(bool all_razed);

    //! Draw a hero icon at the given location.  white or black.
    void draw_hero(Vector<int> pos, bool white);

    //! Redraw the specified region.
    void draw_terrain_tiles(LwRectangle r);

    //! Returns a maptile, but takes map_tiles_per_tile into account.
    Maptile* getTile(int x, int y);

    //! An SDL surface of the terrain without the features.
    /**
     * This is the cached surface after the resize method was called.
     * It is cached so that we don't have recalculate it.
     */
    Cairo::RefPtr<Cairo::Surface> static_surface;
    Cairo::RefPtr<Cairo::Context> static_surface_gc;

    //! The surface containing the drawn map.
    Cairo::RefPtr<Cairo::Surface> surface;
    Cairo::RefPtr<Cairo::Context> surface_gc;

    void draw_target_box(Vector<int> pos, const Gdk::RGBA color);
    void draw_square_around_city(City *c, const Gdk::RGBA color);
    void draw_radial_gradient(Gdk::RGBA inner, Gdk::RGBA outer, int width, int height);

    bool blank_screen;
    bool d_headless;

};

#endif // OVERVIEWMAP_H
