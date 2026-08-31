//  Copyright (C) 2010, 2012, 2014 Ben Asselstine
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
#ifndef USE_ITEM_ON_PLAYER_DIALOG_H
#define USE_ITEM_ON_PLAYER_DIALOG_H

#include <memory>
#include <vector>
#include <gtkmm.h>

#include "citymap.h"
#include "lw-dialog.h"

class Player;

// dialog for targetting a player when using an item.
class UseItemOnPlayerDialog: public LwDialog
{
 public:
    UseItemOnPlayerDialog(Gtk::Window &parent);
    ~UseItemOnPlayerDialog() {delete citymap;};

    void hide() {dialog->hide();};
    Player *run();
    
 private:
    CityMap* citymap;

    Gtk::TreeView *player_treeview;

    class PlayersColumns: public Gtk::TreeModelColumnRecord {
    public:
	PlayersColumns() 
        { add(image); add(name); add(player);}
	
	Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > image;
	Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Player*> player;
    };
    const PlayersColumns players_columns;
    Glib::RefPtr<Gtk::ListStore> players_list;

    Gtk::Image *map_image;
    Gtk::Button *continue_button;
    
    void on_map_changed(Cairo::RefPtr<Cairo::Surface> map);
    void addPlayer(Player *player);
    Player *grabSelectedPlayer();

    void on_player_selected();
};

#endif
