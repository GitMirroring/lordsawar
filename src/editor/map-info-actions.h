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
#ifndef MAP_INFO_ACTIONS_H
#define MAP_INFO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"

class MapInfoAction: public UndoAction
{
public:

    enum Type {
      DESCRIPTION = 1,
      COPYRIGHT = 2,
      LICENSE = 3,
      NAME = 4,
    };

    MapInfoAction(Type type)
     : UndoAction (UndoAction::AGGREGATE_DELAY), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class MapInfoAction_Message: public MapInfoAction
{
    public:
        MapInfoAction_Message (Type t, Glib::ustring m)
          : MapInfoAction (t), d_message (m) {}
        Glib::ustring getMessage () {return d_message;}
    private:
        Glib::ustring d_message;
};

class MapInfoAction_Description: public MapInfoAction_Message
{
    public:
        MapInfoAction_Description (Glib::ustring m)
          : MapInfoAction_Message (DESCRIPTION, m) {}
        ~MapInfoAction_Description () {}

        Glib::ustring getActionName () const {return "Description";}
};

class MapInfoAction_Copyright: public MapInfoAction_Message
{
    public:
        MapInfoAction_Copyright (Glib::ustring m)
          : MapInfoAction_Message (COPYRIGHT, m) {}
        ~MapInfoAction_Copyright () {}

        Glib::ustring getActionName () const {return "Copyright";}
};

class MapInfoAction_License: public MapInfoAction_Message
{
    public:
        MapInfoAction_License (Glib::ustring m)
          : MapInfoAction_Message (LICENSE, m) {}
        ~MapInfoAction_License () {}

        Glib::ustring getActionName () const {return "License";}
};

class MapInfoAction_Name: public MapInfoAction
{
    public:
        MapInfoAction_Name (Glib::ustring n)
          : MapInfoAction (NAME), d_name (n) {};
        ~MapInfoAction_Name () {};

        Glib::ustring getActionName () const {return "Name";}
        Glib::ustring getName () {return d_name;}

    private:
        Glib::ustring d_name;
};
#endif //MAP_INFO_ACTIONS_H
