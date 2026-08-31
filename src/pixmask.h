//  Copyright (C) 2009, 2010, 2011, 2012, 2014, 2015, 2017, 2020, 2021,
//  2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#pragma once
#ifndef PIXMASK_H
#define PIXMASK_H

#include <gtkmm.h>
#include "vector.h"
#include "rectangle.h"
#include <cairomm/cairomm.h>


//! A pixmap and bitmask pair.
/** 
 */
class PixMask
{
 public:
     enum DimensionType
       {
         DIMENSION_ANY,
         DIMENSION_SAME_HEIGHT_AND_WIDTH,
         DIMENSION_WIDTH_IS_MULTIPLE_OF_HEIGHT,
         DIMENSION_WIDTH_IS_MULTIPLE_OF_ROW_HEIGHT,
         DIMENSION_WIDTH_IS_FIXED_MAX_PLAYERS,
         DIMENSION_HEIGHT_IS_MULTIPLE_OF_WIDTH,
         DIMENSION_HEIGHT_IS_MARKED,
       };
     Cairo::RefPtr<Cairo::Surface> get_pixmap() {return pixmap;};
     Cairo::RefPtr<Cairo::Surface> get_mask() {return mask;};
     Cairo::RefPtr<Cairo::Context> get_gc() {return gc;};
     int get_width() {return width;};
     int get_height() {return height;};
     int get_depth();

     static PixMask* create(Glib::ustring file, bool &broken);
     static PixMask* create(Glib::RefPtr<Gdk::Pixbuf> buf);
     static PixMask* create(Cairo::RefPtr<Cairo::Surface> pixmap,
					 Cairo::RefPtr<Cairo::Surface> mask);
     static PixMask* create(Cairo::RefPtr<Cairo::Surface> pixmap);
     static PixMask* create (guint32 tilesize);
     static bool checkDimension (Glib::ustring file, DimensionType t, guint32 rows = 0);
     static bool checkFormat (Glib::ustring file);
     PixMask* copy();

     PixMask* scale (guint32 w, guint32 h);

     //! convert this pixmask to a pixbuf.
     Glib::RefPtr<Gdk::Pixbuf> to_pixbuf() const;
     Glib::RefPtr<Gdk::Texture> to_texture() const;

     //! draw a pixbuf onto this pixmask.
     void draw_pixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, int src_x, int src_y, int dest_x, int dest_y, int width, int height);

     //! draw this pixmask onto a pixmap.
     void blit(Cairo::RefPtr<Cairo::Surface> pixmap, int dest_x, int dest_y);
     void blit(Cairo::RefPtr<Cairo::Surface> pixmap, Vector<int> pos = Vector<int>(0,0));
     void blit_centered(Cairo::RefPtr<Cairo::Surface> pixmap, Vector<int> pos);
     void blit_center_tile(Cairo::RefPtr<Cairo::Surface> pixmap, guint32 tilesize, Vector<int> pos);
      //blit a tile's worth of imagery from this pixmask to a pixmap.
     void blit(Vector<int> tile, int ts, Cairo::RefPtr<Cairo::Surface> pixmap, Vector<int> dest = Vector<int>(0,0));
     void blit(Vector<int> tile, int span, int ts, Cairo::RefPtr<Cairo::Surface> pixmap, Vector<int> dest = Vector<int>(0,0));
     Vector<int> get_dim() const;

     //! Take the left half of this PixMask and make a new one containing it.
     PixMask* cropLeftHalf () const;

     //! Take the right half of this PixMask and make a new one containing it.
     PixMask* cropRightHalf () const;

     //! Take the center half horizontally from this PixMask and make a new one containing it.
     PixMask* cropCenterHalf () const;

     //! Take the leftmost two thirds of this PixMask and make a new one containing it.
     PixMask* cropLeftTwoThirds () const;

     //! Take the rightmost two thirds of this PixMask and make a new one containing it.
     PixMask* cropRightTwoThirds () const;

     //! Destructor.
    ~PixMask();
 protected:
     //! Default constructor.
     PixMask(Glib::RefPtr<Gdk::Pixbuf> pixbuf);

     //! Alternative constructor.
     PixMask(Cairo::RefPtr<Cairo::Surface> pixmap, Cairo::RefPtr<Cairo::Surface> mask);

     //! Copy constructor.
     PixMask(const PixMask&);

     //! Loading constructor.
     /**
      * Load the pixmask from a file.
      *
      */
     PixMask(Glib::ustring filename, bool &broken);

 private:
     Cairo::RefPtr<Cairo::Surface> pixmap;
     Cairo::RefPtr<Cairo::Surface> mask;
     Cairo::RefPtr<Cairo::Context> gc;
    int width;
    int height;

    //blast this to a surface
    void blit(LwRectangle src, Cairo::RefPtr<Cairo::Surface> pixmap, Vector<int> dest);
};

#endif
