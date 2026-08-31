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
#ifndef STACK_EDITOR_ACTIONS_H
#define STACK_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"
#include "stack.h"
#include "hero.h"

class Player;

//! A record of an event in the stack editor
/** 
 * The purpose of these classes is to implement undo/redo in the stack
 * editor.
 */

class StackEditorAction: public UndoAction
{
public:

    enum Type
      {
        OWNER = 1,
        FORTIFY = 2,
        ADD = 3,
        REMOVE = 4,
        COPY = 5,
        STRENGTH = 6,
        MOVES = 7,
        UPKEEP = 8,
        HERO_DETAILS = 9,
      };

    StackEditorAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class StackEditorAction_Fortify: public StackEditorAction
{
    public:
        StackEditorAction_Fortify (bool state)
          : StackEditorAction (FORTIFY, false), d_active (state) {}
        ~StackEditorAction_Fortify () {}

        Glib::ustring getActionName () const {return "Fortify";}
        bool getFortify () const {return d_active;}
    private:
        bool d_active;
};
class StackEditorAction_Owner: public StackEditorAction
{
    public:
        StackEditorAction_Owner (Player *o, bool fort)
          : StackEditorAction (OWNER, false), d_owner (o), d_active (fort) {}
        ~StackEditorAction_Owner () {}

        Glib::ustring getActionName () const {return "Owner";}

        Player *getOwner () const {return d_owner;}
        bool getFortify () const {return d_active;}
    private:
        Player *d_owner;
        bool d_active;
};
class StackEditorAction_Index: public StackEditorAction
{
    public:
        StackEditorAction_Index (Type t, guint32 i, bool agg = false)
          : StackEditorAction (t, agg), d_index (i) {}
        ~StackEditorAction_Index () {}

        guint32 getIndex () {return d_index;}
    private:
        guint32 d_index;
};

class StackEditorAction_Strength: public StackEditorAction_Index
{
    public:
        StackEditorAction_Strength (guint32 i, guint32 s)
          : StackEditorAction_Index (STRENGTH, i), d_strength (s) {}
        ~StackEditorAction_Strength () {}

        Glib::ustring getActionName () const {return "Strength";}

        guint32 getStrength  () {return d_strength;}

    private:
        guint32 d_strength;
};

class StackEditorAction_Moves: public StackEditorAction_Index
{
    public:
        StackEditorAction_Moves (guint32 i, guint32 m)
          : StackEditorAction_Index (MOVES, i), d_moves (m) {}
        ~StackEditorAction_Moves () {}

        Glib::ustring getActionName () const {return "Moves";}

        guint32 getMoves () {return d_moves;}

    private:
        guint32 d_moves;
};

class StackEditorAction_Upkeep: public StackEditorAction_Index
{
    public:
        StackEditorAction_Upkeep (guint32 i, guint32 u)
          : StackEditorAction_Index (UPKEEP, i), d_upkeep (u) {}
        ~StackEditorAction_Upkeep () {}

        Glib::ustring getActionName () const {return "Upkeep";}

        guint32 getUpkeep () {return d_upkeep;}

    private:
        guint32 d_upkeep;
};

class StackEditorAction_HeroDetails: public StackEditorAction_Index
{
    public:
        StackEditorAction_HeroDetails (guint32 i, Hero *h)
          : StackEditorAction_Index (HERO_DETAILS, i),
          d_hero(new Hero (*h)) {}
        ~StackEditorAction_HeroDetails () {delete d_hero;}

        Glib::ustring getActionName () const {return "HeroDetails";}

        Hero *getHero () const {return d_hero;}

    private:
        Hero *d_hero;
};

class StackEditorAction_Save : public StackEditorAction
{
    public:
        StackEditorAction_Save (Type t, Stack *s)
          :StackEditorAction (t, false), d_stack (s) {}
        ~StackEditorAction_Save ()
          {
            delete d_stack;
          }

        void clearStack () {d_stack = NULL;}
        Stack* getStack () const {return d_stack;}
    private:
        Stack *d_stack;
};

class StackEditorAction_Add: public StackEditorAction_Save
{
    public:
        StackEditorAction_Add (Stack *s)
          :StackEditorAction_Save (ADD, s) {}
        ~StackEditorAction_Add () {}

        Glib::ustring getActionName () const {return "Add";}
};

class StackEditorAction_Remove: public StackEditorAction_Save
{
    public:
        StackEditorAction_Remove (Stack *s)
          :StackEditorAction_Save (REMOVE, s) {}
        ~StackEditorAction_Remove () {}

        Glib::ustring getActionName () const {return "Remove";}
};

class StackEditorAction_Copy: public StackEditorAction_Save
{
    public:
        StackEditorAction_Copy (Stack *s)
          :StackEditorAction_Save (COPY, s) {}
        ~StackEditorAction_Copy () {}

        Glib::ustring getActionName () const {return "Copy";}
};
#endif //STACK_EDITOR_ACTIONS_H
