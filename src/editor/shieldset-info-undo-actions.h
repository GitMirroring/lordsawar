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
#ifndef SHIELDSET_INFO_UNDO_ACTIONS_H
#define SHIELDSET_INFO_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"

class UndoMgr;

class ShieldSetInfoUndoAction: public UndoAction
{
public:

    enum Type
      {
        DESCRIPTION = 1,
        COPYRIGHT,
        LICENSE,
        NAME,
      };

    ShieldSetInfoUndoAction (Type type, bool agg = true)
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

class ShieldSetInfoUndoAction_Message: public ShieldSetInfoUndoAction, public UndoCursor
{
public:
    ShieldSetInfoUndoAction_Message (Type t, Glib::ustring m, UndoMgr *u,
                                     Gtk::TextView *v)
      : ShieldSetInfoUndoAction (t), UndoCursor (u->get_pos (v), v), m_message (m)
      {
      }

    Glib::ustring get_message () const
      {
        return m_message;
      }
private:
    Glib::ustring m_message;
};

class ShieldSetInfoUndoAction_Description: public ShieldSetInfoUndoAction_Message
{
public:
    ShieldSetInfoUndoAction_Description (Glib::ustring m, UndoMgr *u,
                                         Gtk::TextView *v)
      : ShieldSetInfoUndoAction_Message (DESCRIPTION, m, u, v)
      {
      }

    ~ShieldSetInfoUndoAction_Description ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Description";
      }
};

class ShieldSetInfoUndoAction_Copyright: public ShieldSetInfoUndoAction_Message
{
public:
    ShieldSetInfoUndoAction_Copyright (Glib::ustring m, UndoMgr *u,
                                       Gtk::TextView *v)
      : ShieldSetInfoUndoAction_Message (COPYRIGHT, m, u, v)
      {
      }

    ~ShieldSetInfoUndoAction_Copyright ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Copyright";
      }
};

class ShieldSetInfoUndoAction_License: public ShieldSetInfoUndoAction_Message
{
public:
    ShieldSetInfoUndoAction_License (Glib::ustring m, UndoMgr *u,
                                     Gtk::TextView *v)
      : ShieldSetInfoUndoAction_Message (LICENSE, m, u, v)
      {
      }

    ~ShieldSetInfoUndoAction_License ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "License";
      }
};

class ShieldSetInfoUndoAction_Name: public ShieldSetInfoUndoAction, public UndoCursor
{
public:
    ShieldSetInfoUndoAction_Name (Glib::ustring n, UndoMgr *u, Gtk::Entry *e)
      : ShieldSetInfoUndoAction (NAME), UndoCursor (u->get_pos (e), e), m_name (n)
      {
      }

    ~ShieldSetInfoUndoAction_Name ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Name";
      }

    Glib::ustring get_name () const
      {
        return m_name;
      }

private:
    Glib::ustring m_name;
};

#endif
