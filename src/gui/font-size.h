//  Copyright (C) 2020 Ben Asselstine
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
#ifndef FONT_SIZE_H
#define FONT_SIZE_H

#include <map>
#include <gtkmm.h>

//! A utility class to get the height of the font
/**
 * we need to know how tall a font is so that we can properly size
 * some buttons and graphics later on.
 *
 */
class FontSize
{
    public:
        //! Returns the singleton instance. Creates a new one if required.
        static FontSize* getInstance();

        //! Explicitly deletes the singleton instance.
        static void deleteInstance();

        //! Return the height of the default font in pixels.
        double get_height () { return d_height; }

        //! Return the width in pixels of one character in the default font.
        double get_width () { return d_width; }

        //! Return true if it changed.
        bool recalculate ();

    protected:    

	// Constructor.
        FontSize();

	//! Destructor.
        ~FontSize() {};

    private:

        //! Calculate the height of the default font in pixels.
        void calculate_default_font_height_and_width ();

	// DATA
        double d_height;
        double d_width;

        static FontSize * s_instance;
};

#endif // FONT_SIZE_H

// End of file
