// Copyright (C) 2009-2012, 2014, 2015, 2017 Ben Asselstine
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
     Cairo::RefPtr<Cairo::Surface> get_pixmap() {return pixmap;};
     Cairo::RefPtr<Cairo::Surface> get_mask() {return mask;};
     Cairo::RefPtr<Cairo::Context> get_gc() {return gc;};
     int get_width() {return width;};
     int get_height() {return height;};
     int get_unscaled_width() {return unscaled_width;};
     int get_unscaled_height() {return unscaled_height;};
     int get_depth();

     static PixMask* create(Glib::ustring file, bool &broken);
     static PixMask* create(Glib::RefPtr<Gdk::Pixbuf> buf);
     static PixMask* create(Cairo::RefPtr<Cairo::Surface> pixmap,
					 Cairo::RefPtr<Cairo::Surface> mask);
     PixMask* copy();

     //! convert this pixmask to a pixbuf.
     Glib::RefPtr<Gdk::Pixbuf> to_pixbuf();

     //! draw a pixbuf onto this pixmask.
     void draw_pixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, int src_x, int src_y, int dest_x, int dest_y, int width, int height);

     //! scale a pixmask in place (alters pixmask)
     static void scale(PixMask*& pixmask, int xsize, int ysize, Gdk::InterpType intper = Gdk::INTERP_BILINEAR);

     //! draw this pixmask onto a pixmap.
     void blit(Cairo::RefPtr<Cairo::Surface> pixmap, int dest_x, int dest_y);
     void blit(Cairo::RefPtr<Cairo::Surface> pixmap, Vector<int> pos = Vector<int>(0,0));
     void blit_centered(Cairo::RefPtr<Cairo::Surface> pixmap, Vector<int> pos);
      //blit a tile's worth of imagery from this pixmask to a pixmap.
     void blit(Vector<int> tile, int ts, Cairo::RefPtr<Cairo::Surface> pixmap, Vector<int> dest = Vector<int>(0,0));

     Vector<int> get_dim() const;
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

     void set_unscaled_width(guint32 w) {unscaled_width = w;};
     void set_unscaled_height(guint32 h) {unscaled_height = h;};
    
 private:
     Cairo::RefPtr<Cairo::Surface> pixmap;
     Cairo::RefPtr<Cairo::Surface> mask;
     Cairo::RefPtr<Cairo::Context> gc;
    int width;
    int height;
    int unscaled_width;
    int unscaled_height;

     //! return a stretched copy of this pixmask.
     PixMask* scale(int xsize, int ysize, 
		    Gdk::InterpType interp = Gdk::INTERP_NEAREST);
     
     void blit(Rectangle src, Cairo::RefPtr<Cairo::Surface> pixmap, Vector<int> dest);
};

#endif
