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
#ifndef CITYSET_INFO_UNDO_ACTIONS_H
#define CITYSET_INFO_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
class UndoMgr;

class CitySetInfoUndoAction: public UndoAction
{
public:

    enum Type
      {
        DESCRIPTION = 1,
        COPYRIGHT,
        LICENSE,
        NAME,
        TILE_SIZE,
      };

    CitySetInfoUndoAction (Type type)
     : UndoAction (UndoAction::AGGREGATE_DELAY), m_type (type)
      {
      }

    Type get_type () const
      {
        return m_type;
      }

protected:

    Type m_type;
};

class CitySetInfoUndoAction_Message: public CitySetInfoUndoAction, public UndoCursor
{
public:
    CitySetInfoUndoAction_Message (Type t, Glib::ustring m, UndoMgr *u,
                                   Gtk::TextView *v)
      : CitySetInfoUndoAction (t), UndoCursor (u->get_pos (v), v), m_message (m)
      {
      }

    Glib::ustring get_message ()
      {
        return m_message;
      }
private:
    Glib::ustring m_message;
};

class CitySetInfoUndoAction_Description: public CitySetInfoUndoAction_Message
{
public:
    CitySetInfoUndoAction_Description (Glib::ustring m, UndoMgr *u,
                                       Gtk::TextView *v)
      : CitySetInfoUndoAction_Message (DESCRIPTION, m, u, v)
      {
      }

    ~CitySetInfoUndoAction_Description ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Description";
      }
};

class CitySetInfoUndoAction_Copyright: public CitySetInfoUndoAction_Message
{
public:
    CitySetInfoUndoAction_Copyright (Glib::ustring m, UndoMgr *u,
                                     Gtk::TextView *v)
      : CitySetInfoUndoAction_Message (COPYRIGHT, m, u, v)
      {
      }

    ~CitySetInfoUndoAction_Copyright ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Copyright";
      }
};

class CitySetInfoUndoAction_License: public CitySetInfoUndoAction_Message
{
public:
    CitySetInfoUndoAction_License (Glib::ustring m, UndoMgr *u,
                                   Gtk::TextView *v)
      : CitySetInfoUndoAction_Message (LICENSE, m, u, v)
      {
      }

    ~CitySetInfoUndoAction_License ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "License";
      }
};

class CitySetInfoUndoAction_Name: public CitySetInfoUndoAction, public UndoCursor
{
public:
    CitySetInfoUndoAction_Name (Glib::ustring n, UndoMgr *u, Gtk::Entry *e)
      : CitySetInfoUndoAction (NAME), UndoCursor (u->get_pos (e), e), m_name (n)
      {
      }

    ~CitySetInfoUndoAction_Name ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Name";
      }

    Glib::ustring get_name ()
      {
        return m_name;
      }

private:
    Glib::ustring m_name;
};

class CitySetInfoUndoAction_TileSize: public CitySetInfoUndoAction
{
public:
    CitySetInfoUndoAction_TileSize (int ts)
      : CitySetInfoUndoAction (TILE_SIZE), m_tile_size (ts)
      {
      }

    ~CitySetInfoUndoAction_TileSize ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "TileSize";
      }

    int get_tile_size ()
      {
        return m_tile_size;
      }

private:
    int m_tile_size;
};
#endif
