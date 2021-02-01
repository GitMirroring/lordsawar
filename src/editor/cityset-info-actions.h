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
#ifndef CITYSET_INFO_ACTIONS_H
#define CITYSET_INFO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"

class CitySetInfoAction: public UndoAction
{
public:

    enum Type {
      DESCRIPTION = 1,
      COPYRIGHT = 2,
      LICENSE = 3,
      NAME = 4,
      TILE_SIZE = 5,
    };

    CitySetInfoAction(Type type)
     : UndoAction (UndoAction::AGGREGATE_DELAY), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class CitySetInfoAction_Message: public CitySetInfoAction
{
    public:
        CitySetInfoAction_Message (Type t, Glib::ustring m)
          : CitySetInfoAction (t), d_message (m) {}
        Glib::ustring getMessage () {return d_message;}
    private:
        Glib::ustring d_message;
};

class CitySetInfoAction_Description: public CitySetInfoAction_Message
{
    public:
        CitySetInfoAction_Description (Glib::ustring m)
          : CitySetInfoAction_Message (DESCRIPTION, m) {}
        ~CitySetInfoAction_Description () {}

        Glib::ustring getActionName () const {return "Description";}
};

class CitySetInfoAction_Copyright: public CitySetInfoAction_Message
{
    public:
        CitySetInfoAction_Copyright (Glib::ustring m)
          : CitySetInfoAction_Message (COPYRIGHT, m) {}
        ~CitySetInfoAction_Copyright () {}

        Glib::ustring getActionName () const {return "Copyright";}
};

class CitySetInfoAction_License: public CitySetInfoAction_Message
{
    public:
        CitySetInfoAction_License (Glib::ustring m)
          : CitySetInfoAction_Message (LICENSE, m) {}
        ~CitySetInfoAction_License () {}

        Glib::ustring getActionName () const {return "License";}
};

class CitySetInfoAction_Name: public CitySetInfoAction
{
    public:
        CitySetInfoAction_Name (Glib::ustring n)
          : CitySetInfoAction (NAME), d_name (n) {};
        ~CitySetInfoAction_Name () {};

        Glib::ustring getActionName () const {return "Name";}
        Glib::ustring getName () {return d_name;}

    private:
        Glib::ustring d_name;
};

class CitySetInfoAction_TileSize: public CitySetInfoAction
{
    public:
        CitySetInfoAction_TileSize (int ts)
          : CitySetInfoAction (TILE_SIZE), d_tile_size(ts) {};
        ~CitySetInfoAction_TileSize () {};

        Glib::ustring getActionName () const {return "TileSize";}
        int getTileSize () {return d_tile_size;}

    private:
        int d_tile_size;
};
#endif //CITYSET_INFO_ACTIONS_H
