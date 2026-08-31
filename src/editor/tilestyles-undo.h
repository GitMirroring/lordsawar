//  Copyright (C) 2026 Ben Asselstine
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
#ifndef TILESTYLES_UNDO_H
#define TILESTYLES_UNDO_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "tile.h"
#include "small-tile.h"
#include "tile-style.h"
#include "defs.h"
#include "undo-action.h"
#include "undo-mgr.h"

class Tileset;

//! A record of an event in the tilestyles dialog
/** 
 * The purpose of these classes is to implement undo/redo in the tilestyles
 * dialog.
 */

class TileStylesUndoAction: public UndoAction
{
public:

    //! A TileStyles Undo Action can be one of the following kinds.
    enum Type
      {
        ADD_TILESTYLESET = 1,
        REMOVE_TILESTYLESET = 2,
        TYPE = 3,
      };

    //! Default constructor.
    TileStylesUndoAction(Type type, bool agg = false)
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

//-----------------------------------------------------------------------------

//! A helper class for events that require saving the whole tileset

/**
 * We're putting images inside the tar file, or removing them so we
 * need to save the whole tar file.
 */
class TileStylesUndoAction_Save: public TileStylesUndoAction
{
public:
    TileStylesUndoAction_Save (Tileset *s, Type t)
      :TileStylesUndoAction (t)
      {
        m_tileset = new Tileset (*s);
        m_filename = File::get_tmp_file () + TILESET_EXT;
        s->save (m_filename, TILESET_EXT);
      }

    ~TileStylesUndoAction_Save ()
      {
        File::erase (m_filename);
        delete m_tileset;
      }

    Glib::ustring get_tileset_filename () const
      {
        return m_filename;
      }

    Tileset *get_tileset () const
      {
        return m_tileset;
      }
private:
    Glib::ustring m_filename;
    Tileset *m_tileset;
};


//-----------------------------------------------------------------------------

//! A record of a new tile being added to the tileset
/**
 * The purpose of the TileStylesUndoAction_AddSet class is to record
 * when a new blank tile has been added to the set.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileStylesUndoAction_AddSet: public TileStylesUndoAction_Save
{
public:
    //! Make a new add set action
    /**
     * Populate the add set action with the tileset.
     */
    TileStylesUndoAction_AddSet (Tileset *t)
      :TileStylesUndoAction_Save (t, TileStylesUndoAction::ADD_TILESTYLESET)
      {
      }

    //! Destroy an add set action, and delete the file.
    ~TileStylesUndoAction_AddSet ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add Tile Style Set";
      }
};

//-----------------------------------------------------------------------------

//! A record of a set being erased from the tileset
/**
 * The purpose of the TileStylesUndoAction_RemoveSet class is to record
 * when a tile style set has been deleted from the tileset.
 *
 */
class TileStylesUndoAction_RemoveSet: public TileStylesUndoAction_Save
{
public:
    //! Make a new remove set action
    /**
     * Populate the remove set action with the tileset.
     */
    TileStylesUndoAction_RemoveSet (Tileset *t)
      :TileStylesUndoAction_Save (t, TileStylesUndoAction::REMOVE_TILESTYLESET)
      {
      }

    //! Destroy a remove tile action, and delete the file.
    ~TileStylesUndoAction_RemoveSet ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Remove Tile Style Set";
      }
};

//-----------------------------------------------------------------------------
//
class TileStylesUndoAction_Type: public TileStylesUndoAction_Save
{
public:
    //! Make a new type action
    /**
     * Populate the type action with the tileset.
     * this is for one or more tilestyles getting their type changed.
     */
    TileStylesUndoAction_Type (Tileset *t)
      :TileStylesUndoAction_Save (t, TileStylesUndoAction::TYPE)
      {
      }

    //! Destroy a type action, and delete the file.
    ~TileStylesUndoAction_Type ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Type";
      }
};

#endif
