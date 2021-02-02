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
#ifndef HEROES_EDITOR_ACTIONS_H
#define HEROES_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "heroproto.h"
#include "herotemplates.h"
#include "undo-mgr.h"

//! A record of an event in the heroes editor
/** 
 * The purpose of these classes is to implement undo/redo in the heroes
 * editor.
 */

class HeroesEditorAction: public UndoAction
{
public:

    enum Type {
      NAME = 1,
      GENDER = 2,
      ADD = 3,
      REMOVE = 4,
    };

    HeroesEditorAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class HeroesEditorAction_Index: public HeroesEditorAction
{
    public:
        HeroesEditorAction_Index (Type t, guint32 i, bool agg = false)
          : HeroesEditorAction (t, agg), d_index (i) {}
        ~HeroesEditorAction_Index () {}

        guint32 getIndex () {return d_index;}
    private:
        guint32 d_index;
};

class HeroesEditorAction_Name: public HeroesEditorAction_Index, public UndoCursor
{
    public:
        HeroesEditorAction_Name (guint32 i, Glib::ustring n, UndoMgr *u,
                                 Gtk::Entry *e)
          : HeroesEditorAction_Index (NAME, i, true), UndoCursor (u, e),
          d_name (n) {}
        ~HeroesEditorAction_Name () {}

        Glib::ustring getActionName () const {return "Name";}

        Glib::ustring getName () {return d_name;}

    private:
        Glib::ustring d_name;
};

class HeroesEditorAction_Gender: public HeroesEditorAction_Index
{
    public:
        HeroesEditorAction_Gender (guint32 i, Hero::Gender g)
          : HeroesEditorAction_Index (GENDER, i), d_gender (g) {}
        ~HeroesEditorAction_Gender () {}

        Glib::ustring getActionName () const {return "Gender";}

        Hero::Gender getGender () {return d_gender;}

    private:
        Hero::Gender d_gender;
};

class HeroesEditorAction_Save : public HeroesEditorAction
{
    public:
        HeroesEditorAction_Save (Type t, HeroTemplates *h)
          :HeroesEditorAction (t, true), d_heroes (h) {}
        ~HeroesEditorAction_Save ()
          {
            delete d_heroes;
          }

        void clearHeroes () {d_heroes = NULL;}
        HeroTemplates * getHeroes () const {return d_heroes;}
    private:
        HeroTemplates *d_heroes;
};

class HeroesEditorAction_Add: public HeroesEditorAction_Save
{
    public:
        HeroesEditorAction_Add (HeroTemplates *t)
          :HeroesEditorAction_Save (ADD, t) {}
        ~HeroesEditorAction_Add () {}

        Glib::ustring getActionName () const {return "Add";}
};

class HeroesEditorAction_Remove: public HeroesEditorAction_Save
{
    public:
        HeroesEditorAction_Remove (HeroTemplates *t)
          :HeroesEditorAction_Save (REMOVE, t) {}
        ~HeroesEditorAction_Remove () {}

        Glib::ustring getActionName () const {return "Remove";}
};
#endif //HEROES_EDITOR_ACTIONS_H
