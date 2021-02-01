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
#ifndef ARMYSET_INFO_ACTIONS_H
#define ARMYSET_INFO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"

class ArmySetInfoAction: public UndoAction
{
public:

    enum Type {
      DESCRIPTION = 1,
      COPYRIGHT = 2,
      LICENSE = 3,
      NAME = 4,
      TILE_SIZE = 5,
    };

    ArmySetInfoAction(Type type)
     : UndoAction (UndoAction::AGGREGATE_DELAY), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class ArmySetInfoAction_Message: public ArmySetInfoAction
{
    public:
        ArmySetInfoAction_Message (Type t, Glib::ustring m)
          : ArmySetInfoAction (t), d_message (m) {}
        Glib::ustring getMessage () {return d_message;}
    private:
        Glib::ustring d_message;
};

class ArmySetInfoAction_Description: public ArmySetInfoAction_Message
{
    public:
        ArmySetInfoAction_Description (Glib::ustring m)
          : ArmySetInfoAction_Message (DESCRIPTION, m) {}
        ~ArmySetInfoAction_Description () {}

        Glib::ustring getActionName () const {return "Description";}
};

class ArmySetInfoAction_Copyright: public ArmySetInfoAction_Message
{
    public:
        ArmySetInfoAction_Copyright (Glib::ustring m)
          : ArmySetInfoAction_Message (COPYRIGHT, m) {}
        ~ArmySetInfoAction_Copyright () {}

        Glib::ustring getActionName () const {return "Copyright";}
};

class ArmySetInfoAction_License: public ArmySetInfoAction_Message
{
    public:
        ArmySetInfoAction_License (Glib::ustring m)
          : ArmySetInfoAction_Message (LICENSE, m) {}
        ~ArmySetInfoAction_License () {}

        Glib::ustring getActionName () const {return "License";}
};

class ArmySetInfoAction_Name: public ArmySetInfoAction
{
    public:
        ArmySetInfoAction_Name (Glib::ustring n)
          : ArmySetInfoAction (NAME), d_name (n)
          {}
        ~ArmySetInfoAction_Name () {};

        Glib::ustring getActionName () const {return "Name";}
        Glib::ustring getName () {return d_name;}

    private:
        Glib::ustring d_name;
};

class ArmySetInfoAction_TileSize: public ArmySetInfoAction
{
    public:
        ArmySetInfoAction_TileSize (int ts)
          : ArmySetInfoAction (TILE_SIZE), d_tile_size(ts) {};
        ~ArmySetInfoAction_TileSize () {};

        Glib::ustring getActionName () const {return "TileSize";}
        int getTileSize () {return d_tile_size;}

    private:
        int d_tile_size;
};
#endif //ARMYSET_INFO_ACTIONS_H
