//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015 Ben Asselstine
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
#ifndef HERO_OFFER_DIALOG_H
#define HERO_OFFER_DIALOG_H

#include <gtkmm.h>

#include "heromap.h"
#include "player.h"
#include "hero.h"
#include "heroproto.h"
#include "lw-dialog.h"

//! dialog for accepting/rejecting a hero
class HeroOfferDialog: public LwDialog
{
 public:
    HeroOfferDialog(Gtk::Window &parent, Player *player, HeroProto *hero, City *city, int gold);
    ~HeroOfferDialog() {delete heromap;};

    bool run();
    void hide() {dialog->hide();};
    
 private:
    //! The smallmap that shows where the Hero is emerging.
    HeroMap* heromap;

    Gtk::Image *map_image;
    Gtk::Image *hero_image;
    Gtk::RadioButton *male_radiobutton;
    Gtk::RadioButton *female_radiobutton;
    Gtk::Entry *name_entry;
    Gtk::Button *accept_button;
    Gtk::Button *close_button;
    
    HeroProto *hero;
    City *city;

    void on_toggled();
    void on_map_changed(Cairo::RefPtr<Cairo::Surface> map);
    void update_buttons();
    void on_name_changed();

};

#endif
