//  Copyright (C) 2017 Ben Asselstine
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
#ifndef TARTAN_PROGRESS_BAR_H
#define TARTAN_PROGRESS_BAR_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include <glibmm.h>
#include "player.h"

//! A progress bar that shows an image of a tartan
/**
 * The idea here is that we're going to show a player-specific image of
 * a progressbar.
 * The whole image is generated in the ImageCache, and here in this class
 * we choose how much of it to show, and how wide it should be overall.
 *
 */
class TartanProgressBar: public Gtk::Image
{
 public:

    // use this percent of the width available to us.
    const double TARTAN_PERCENT_WIDTH = 1.00;

    //always show this much of the tartan.
    const double MIN_PERCENT = 0.10;

    //! Constructor.  Player determines the color.
    TartanProgressBar(Player *p);

    //! Destructor.
    ~TartanProgressBar();

    //! Draw the progress bar for the given player.
    void pulse (Player *p);

 protected:

    virtual bool on_draw (const Cairo::RefPtr<Cairo::Context> &cr);

 private:

    // Data
    Player *player;
    double percent;

    // Helpers
    double calculate_percentage () const;
};

#endif // TARTAN_PROGRESS_BAR
