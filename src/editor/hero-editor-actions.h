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
#ifndef HERO_EDITOR_ACTIONS_H
#define HERO_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"

//! A record of an event in the hero editor
/** 
 * The purpose of these classes is to implement undo/redo in the hero
 * editor.
 */

class HeroEditorAction: public UndoAction
{
public:

    enum Type {
      NAME = 1,
      GENDER = 2,
      BACKPACK = 3,
    };

    HeroEditorAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class HeroEditorAction_Name: public HeroEditorAction, public UndoCursor
{
    public:
        HeroEditorAction_Name (Glib::ustring n, UndoMgr *u, Gtk::Entry *e)
          : HeroEditorAction (NAME, true), UndoCursor (u, e),
          d_name (n) {}
        ~HeroEditorAction_Name () {}

        Glib::ustring getActionName () const {return "Name";}

        Glib::ustring getName () {return d_name;}

    private:
        Glib::ustring d_name;
};

class HeroEditorAction_Gender: public HeroEditorAction
{
    public:
        HeroEditorAction_Gender (Hero::Gender g)
          : HeroEditorAction (GENDER), d_gender (g) {}
        ~HeroEditorAction_Gender () {}

        Glib::ustring getActionName () const {return "Gender";}

        Hero::Gender getGender () {return d_gender;}

    private:
        Hero::Gender d_gender;
};

class HeroEditorAction_Backpack: public HeroEditorAction
{
    public:
        HeroEditorAction_Backpack (Backpack *b)
          : HeroEditorAction (BACKPACK), d_backpack (new Backpack (*b)) {}
        ~HeroEditorAction_Backpack () {delete d_backpack;}

        Glib::ustring getActionName () const {return "Backpack";}

        Backpack * getBackpack () {return d_backpack;}

    private:
        Backpack *d_backpack;
};
#endif //HERO_EDITOR_ACTIONS_H
