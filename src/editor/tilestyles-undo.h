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
#ifndef TILESTYLE_ORGANIZER_ACTIONS_H
#define TILESTYLE_ORGANIZER_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include <vector>
#include "tilestyle.h"

//! A record of an event in the tilestyle organizer
/** 
 * The purpose of these classes is to implement undo/redo in the tilestyle
 * organizer.
 */

class TileStyleOrganizerAction: public UndoAction
{
public:

    enum Type {
      MOVE = 1,
    };

    TileStyleOrganizerAction(Type type)
     : UndoAction (UndoAction::AGGREGATE_NONE), d_type (type) {}

    virtual ~TileStyleOrganizerAction() {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class TileStyleOrganizerAction_Move: public TileStyleOrganizerAction
{
    public:
        TileStyleOrganizerAction_Move
          (std::list<std::pair<guint32, TileStyle::Type> >l)
          : TileStyleOrganizerAction (MOVE), d_list (l) {}
        ~TileStyleOrganizerAction_Move () {}

        Glib::ustring getActionName () const {return "Move";}

        std::list<std::pair<guint32, TileStyle::Type> > getTileStyleTypes()
          {return d_list;}

    private:
        std::list<std::pair<guint32, TileStyle::Type> > d_list;
};

#endif //TILESTYLE_ORGANIZER_ACTIONS_H
