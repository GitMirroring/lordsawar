// Copyright (C) 2021 Ben Asselstine
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
#ifndef TILESET_INFO_ACTIONS_H
#define TILESET_INFO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"

class UndoMgr;

class TileSetInfoAction: public UndoAction
{
public:

    enum Type {
      DESCRIPTION = 1,
      COPYRIGHT = 2,
      LICENSE = 3,
      NAME = 4,
      TILE_SIZE = 5,
    };

    TileSetInfoAction(Type type)
     : UndoAction (UndoAction::AGGREGATE_DELAY), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class TileSetInfoAction_Message: public TileSetInfoAction, public UndoCursor
{
    public:
        TileSetInfoAction_Message (Type t, Glib::ustring m,
                                   UndoMgr *u, Gtk::TextView *v)
          : TileSetInfoAction (t), UndoCursor (u, v), d_message (m) {}
        Glib::ustring getMessage () {return d_message;}
    private:
        Glib::ustring d_message;
};

class TileSetInfoAction_Description: public TileSetInfoAction_Message
{
    public:
        TileSetInfoAction_Description (Glib::ustring m, UndoMgr *u, 
                                       Gtk::TextView *v)
          : TileSetInfoAction_Message (DESCRIPTION, m, u, v) {}
        ~TileSetInfoAction_Description () {}

        Glib::ustring getActionName () const {return "Description";}
};

class TileSetInfoAction_Copyright: public TileSetInfoAction_Message
{
    public:
        TileSetInfoAction_Copyright (Glib::ustring m, UndoMgr *u, 
                                     Gtk::TextView *v)
          : TileSetInfoAction_Message (COPYRIGHT, m, u, v) {}
        ~TileSetInfoAction_Copyright () {}

        Glib::ustring getActionName () const {return "Copyright";}
};

class TileSetInfoAction_License: public TileSetInfoAction_Message
{
    public:
        TileSetInfoAction_License (Glib::ustring m, UndoMgr *u, 
                                   Gtk::TextView *v)
          : TileSetInfoAction_Message (LICENSE, m, u, v) {}
        ~TileSetInfoAction_License () {}

        Glib::ustring getActionName () const {return "License";}
};

class TileSetInfoAction_Name: public TileSetInfoAction, public UndoCursor
{
    public:
        TileSetInfoAction_Name (Glib::ustring n, UndoMgr *u, Gtk::Entry *e)
          : TileSetInfoAction (NAME), UndoCursor (u, e), d_name (n) {}
        ~TileSetInfoAction_Name () {}

        Glib::ustring getActionName () const {return "Name";}
        Glib::ustring getName () {return d_name;}

    private:
        Glib::ustring d_name;
};

class TileSetInfoAction_TileSize: public TileSetInfoAction
{
    public:
        TileSetInfoAction_TileSize (int ts)
          : TileSetInfoAction (TILE_SIZE), d_tile_size(ts) {};
        ~TileSetInfoAction_TileSize () {};

        Glib::ustring getActionName () const {return "TileSize";}
        int getTileSize () {return d_tile_size;}

    private:
        int d_tile_size;
};
#endif //TILESET_INFO_ACTIONS_H
