//  Copyright (C) 2021 Ben Asselstine
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
#ifndef TILESET_INFO_UNDO_H
#define TILESET_INFO_UNDO_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"

class TileSetInfoUndoAction: public UndoAction
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

    TileSetInfoUndoAction (Type type)
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

class TileSetInfoUndoAction_Message: public TileSetInfoUndoAction,
    public UndoCursor
{
public:
    TileSetInfoUndoAction_Message (Type t, Glib::ustring m, UndoMgr *u,
                                   Gtk::TextView *v)
      : TileSetInfoUndoAction (t), UndoCursor (u->get_pos (v), v), m_message (m)
      {
      }

    Glib::ustring get_message ()
      {
        return m_message;
      }

private:
    Glib::ustring m_message;
};

class TileSetInfoUndoAction_Description: public TileSetInfoUndoAction_Message
{
public:
    TileSetInfoUndoAction_Description (Glib::ustring m, UndoMgr *u,
                                       Gtk::TextView *v)
      : TileSetInfoUndoAction_Message (DESCRIPTION, m, u, v)
      {
      }

    ~TileSetInfoUndoAction_Description ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Description";
      }
};

class TileSetInfoUndoAction_Copyright: public TileSetInfoUndoAction_Message
{
public:
    TileSetInfoUndoAction_Copyright (Glib::ustring m, UndoMgr *u,
                                     Gtk::TextView *v)
      : TileSetInfoUndoAction_Message (COPYRIGHT, m, u, v)
      {
      }

    ~TileSetInfoUndoAction_Copyright ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Copyright";
      }
};

class TileSetInfoUndoAction_License: public TileSetInfoUndoAction_Message
{
public:
    TileSetInfoUndoAction_License (Glib::ustring m, UndoMgr *u,
                                   Gtk::TextView *v)
      : TileSetInfoUndoAction_Message (LICENSE, m, u, v)
      {
      }

    ~TileSetInfoUndoAction_License ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "License";
      }
};

class TileSetInfoUndoAction_Name: public TileSetInfoUndoAction, public UndoCursor
{
public:
    TileSetInfoUndoAction_Name (Glib::ustring n, UndoMgr *u, Gtk::Entry *e)
      : TileSetInfoUndoAction (NAME), UndoCursor (u->get_pos (e), e), m_name (n)
      {
      }

    ~TileSetInfoUndoAction_Name ()
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

class TileSetInfoUndoAction_TileSize: public TileSetInfoUndoAction
{
public:
    TileSetInfoUndoAction_TileSize (int ts)
      : TileSetInfoUndoAction (TILE_SIZE), m_tile_size (ts)
      {
      }

    ~TileSetInfoUndoAction_TileSize ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "TileSize";
      }

    int get_tile_size () const
      {
        return m_tile_size;
      }

private:
    int m_tile_size;
};
#endif
