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
#ifndef TEMPLE_EDITOR_ACTIONS_H
#define TEMPLE_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"
#include "temple.h"

//! A record of an event in the temple editor
/** 
 * The purpose of these classes is to implement undo/redo in the temple
 * editor.
 */

class TempleEditorAction: public UndoAction
{
public:

    enum Type {
      NAME = 1,
      RANDOMIZE_NAME = 2,
      DESCRIPTION = 3,
      TYPE = 4,
    };

    TempleEditorAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class TempleEditorAction_Name: public TempleEditorAction, public UndoCursor
{
    public:
        TempleEditorAction_Name (Glib::ustring n, UndoMgr *u, Gtk::Entry *e)
          : TempleEditorAction (NAME, true), UndoCursor (u, e),
          d_name (n) {}
        ~TempleEditorAction_Name () {}

        Glib::ustring getActionName () const {return "Name";}

        Glib::ustring getName () {return d_name;}

    private:
        Glib::ustring d_name;
};

class TempleEditorAction_RandomizeName: public TempleEditorAction
{
    public:
        TempleEditorAction_RandomizeName (Glib::ustring n)
          : TempleEditorAction (RANDOMIZE_NAME), d_name (n) {}
        ~TempleEditorAction_RandomizeName () {}

        Glib::ustring getActionName () const {return "RandomizeName";}

        Glib::ustring getName () const {return d_name;}

    private:
        Glib::ustring d_name;
};

class TempleEditorAction_Description: public TempleEditorAction, public UndoCursor
{
    public:
        TempleEditorAction_Description (Glib::ustring d, UndoMgr *u,
                                        Gtk::Entry *e)
          : TempleEditorAction (DESCRIPTION, true), UndoCursor (u, e),
          d_description (d) {}
        ~TempleEditorAction_Description () {}

        Glib::ustring getActionName () const {return "Description";}

        Glib::ustring getDescription () {return d_description;}

    private:
        Glib::ustring d_description;
};

class TempleEditorAction_Type: public TempleEditorAction
{
    public:
        TempleEditorAction_Type (Temple::Type ty)
          : TempleEditorAction (TYPE, false), d_temple_type (ty) {}
        ~TempleEditorAction_Type () {}

        Glib::ustring getActionName () const {return "Type";}

        Temple::Type getTempleType () const {return d_temple_type;}

    private:
        Temple::Type d_temple_type;
};
#endif //TEMPLE_EDITOR_ACTIONS_H
