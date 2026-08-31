//  Copyright (C) 2021, 2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

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

class FightOrderUndoAction: public UndoAction
{
public:

    enum Type
      {
        ORDER = 1,
        MAKE_SAME = 2,
        OWNER = 3,
      };

    FightOrderUndoAction (Type type, bool agg = false)
      : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                    UndoAction::AGGREGATE_NONE), m_type (type)
        {
        }

    Type get_type () const
      {
        return m_type;
      }

protected:

    Type m_type;
};

class FightOrderUndoAction_Order: public FightOrderUndoAction
{
public:
    FightOrderUndoAction_Order (int row, std::list<guint32> o)
      : FightOrderUndoAction (ORDER), m_row (row), m_fight_order (o)
      {
      }

    ~FightOrderUndoAction_Order ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Order";
      }

    std::list<guint32> get_fight_order () const
      {
        return m_fight_order;
      }

    int get_row () const
      {
        return m_row;
      }
private:
    int m_row;
    std::list<guint32> m_fight_order;
};

class FightOrderUndoAction_MakeSame: public FightOrderUndoAction
{
public:
    FightOrderUndoAction_MakeSame (int r,
                                   std::list<std::list<guint32> > o)
      : FightOrderUndoAction (MAKE_SAME), m_row (r), m_orders (o)
      {
      }

    ~FightOrderUndoAction_MakeSame ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "MakeSame";
      }

    int get_row () const
      {
        return m_row;
      }

    std::list<std::list<guint32> >get_fight_orders () const
      {
        return m_orders;
      }
private:
    int m_row;
    std::list<std::list<guint32> > m_orders;
};

class FightOrderUndoAction_Owner: public FightOrderUndoAction
{
public:
    FightOrderUndoAction_Owner (int r)
      : FightOrderUndoAction (OWNER), m_row (r)
      {
      }

    ~FightOrderUndoAction_Owner ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Owner";
      }

    int get_row () const
      {
        return m_row;
      }

private:
    int m_row;
};

#endif
