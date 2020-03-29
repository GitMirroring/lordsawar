//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2009, 2011, 2012 Ben Asselstine
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
#ifndef IMAGE_HELPERS_H
#define IMAGE_HELPERS_H

#include <vector>
#include <gdkmm/pixbuf.h>
#include "PixMask.h"

// convert a file containing one large image with subimages, each of the same
// width, to an array of pixbufs corresponding to the subimages
std::vector<PixMask*>
disassemble_row(const Glib::ustring &file, int no, bool &broken);
std::vector<PixMask*>
disassemble_row(const Glib::ustring &file, int no, bool first_half_height, bool &broken);
std::vector<PixMask*>
disassemble_row(Glib::RefPtr<Gdk::Pixbuf> p, int no, bool first_half_height);

//Cairo::RefPtr<Cairo::Surface> scale (Cairo::RefPtr<Cairo::Surface> pixmap, int w, int h);
bool image_width_is_multiple_of_image_height(const Glib::ustring file);
void get_image_width_and_height (const Glib::ustring &file, guint32 &width, guint32 &height, bool &broken);
#endif
