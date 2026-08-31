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
#ifndef FIGHT_ORDER_EDITOR_ACTIONS_H
#define FIGHT_ORDER_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"

//! A record of an event in the fight order editor
/** 
 * The purpose of these classes is to implement undo/redo in the fight order
 * editor.
 */

class FightOrderEditorAction: public UndoAction
{
public:

    enum Type {
      ORDER = 1,
      OWNER = 2,
      MAKE_SAME = 3,
    };

    FightOrderEditorAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class FightOrderEditorAction_Order: public FightOrderEditorAction
{
    public:
        FightOrderEditorAction_Order (guint32 id, std::list<guint32> o)
          : FightOrderEditorAction (ORDER), d_player_id (id), d_fight_order (o)
          {}
        ~FightOrderEditorAction_Order () {}

        Glib::ustring getActionName () const {return "Order";}

        std::list<guint32> getFightOrder () const {return d_fight_order;}

        guint32 getPlayerId () const {return d_player_id;}
    private:
        guint32 d_player_id;
        std::list<guint32> d_fight_order;
};

class FightOrderEditorAction_Owner: public FightOrderEditorAction
{
    public:
        FightOrderEditorAction_Owner (int r)
          : FightOrderEditorAction (OWNER), d_row (r) {}
        ~FightOrderEditorAction_Owner () {}

        Glib::ustring getActionName () const {return "Owner";}

        int getRow () const {return d_row;}

    private:
        int d_row;
};

class FightOrderEditorAction_MakeSame: public FightOrderEditorAction
{
    public:
        FightOrderEditorAction_MakeSame (int r,
                                         std::list<std::list<guint32> > o)
          : FightOrderEditorAction (MAKE_SAME), d_row (r), d_orders (o) {}
        ~FightOrderEditorAction_MakeSame () {}

        Glib::ustring getActionName () const {return "MakeSame";}

        int getRow () const {return d_row;}
        std::list<std::list<guint32> >getFightOrders () const {return d_orders;}
    private:
        int d_row;
        std::list<std::list<guint32> > d_orders;
};
#endif //FIGHT_ORDER_EDITOR_ACTIONS_H
