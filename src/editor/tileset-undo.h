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
#ifndef TILESET_UNDO_H
#define TILESET_UNDO_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "tile.h"
#include "small-tile.h"
#include "tile-style.h"
#include "defs.h"
#include "undo-action.h"
#include "undo-mgr.h"
#include "tile-set.h"
#include "file.h"

class Tileset;

//! A record of an event in the tileset editor
/** 
 * The purpose of these classes is to implement undo/redo in the tileset
 * editor.
 */

class TileSetUndoAction: public UndoAction
{
public:

    //! A TileSet Undo Action can be one of the following kinds.
    enum Type
      {
        CHANGE_PROPERTIES = 1,
        NAME = 2,
        TYPE = 3,
        PATTERN = 4,
        MOVES = 5,
        COLOR = 6,
        ADD_TILE = 7,
        REMOVE_TILE = 8,
        ADD_IMAGE = 9,
        CLEAR_IMAGE = 10,
        TILESTYLES = 11,
        BUILDING_COLORS = 12,
      };

    //! Default constructor.
    TileSetUndoAction(Type type, bool agg = false)
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

//! A record of the tileset's properties changing in the editor.
/**
 * The purpose of the TileSetUndoAction_Properties class is to record
 * when a tileset's name, description, copyright, license, and tilesize have
 * changed.
 */
class TileSetUndoAction_Properties: public TileSetUndoAction
{
public:
    //! Make a new change properties action
    /**
     * Populate the properties action with the new name, description,
     * copyright, license text, and tile size.
     */
    TileSetUndoAction_Properties (Glib::ustring n, Glib::ustring d,
                                  Glib::ustring c, Glib::ustring l,
                                  guint32 ts)
      : TileSetUndoAction (CHANGE_PROPERTIES), m_name (n), m_desc (d),
      m_copyright (c), m_license (l), m_tile_size (ts)
  {
  }
    //! Destroy a change properties action.
    ~TileSetUndoAction_Properties ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Properties";
      }

    Glib::ustring get_name () const
      {
        return m_name;
      }

    Glib::ustring get_description () const
      {
        return m_desc;
      }

    Glib::ustring get_copyright () const
      {
        return m_copyright;
      }

    Glib::ustring get_license () const
      {
        return m_license;
      }

    guint32 get_tile_size () const
      {
        return m_tile_size;
      }

private:
    Glib::ustring m_name;
    Glib::ustring m_desc;
    Glib::ustring m_copyright;
    Glib::ustring m_license;
    guint32 m_tile_size;
};

//-----------------------------------------------------------------------------

//! A helper class for events that require referencing the tile's place
//in the set. this equates to the position in the treeview.

class TileSetUndoAction_TileIndex: public TileSetUndoAction
{
    public:
        TileSetUndoAction_TileIndex (Type t, guint32 i, bool agg = false)
          : TileSetUndoAction (t, agg), m_index (i)
          {
          }

        ~TileSetUndoAction_TileIndex ()
          {
          }

        guint32 get_index () const
          {
            return m_index;
          }
    private:
        guint32 m_index;
};

//-----------------------------------------------------------------------------

//! A record of a tile's name being changed
/**
 * The purpose of the TileSetUndoAction_Name class is to record
 * when we change the tile's name.  This happens letter by letter.
 *
 */
class TileSetUndoAction_Name: public TileSetUndoAction_TileIndex, public UndoCursor
{
public:
    //! Make a new name action
    /**
     * Populate the action with the name of the tile.
     * Also supply the index of the tile whose name we're modifying.
     */
    TileSetUndoAction_Name (guint32 i, Glib::ustring n, UndoMgr *u,
                            Gtk::Entry *e)
      : TileSetUndoAction_TileIndex (NAME, i, true),
      UndoCursor (u->get_pos (e), e), m_name (n)
  {
  }
    //! Destroy a name action.
    ~TileSetUndoAction_Name ()
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

//-----------------------------------------------------------------------------

//! A record of a tile's type being changed
/**
 * The purpose of the TileSetUndoAction_Type class is to record
 * when we change the tile's type.  e.g. grass, water, forest, etc.
 *
 */
class TileSetUndoAction_Type: public TileSetUndoAction_TileIndex
{
public:
    //! Make a new type action
    /**
     * Populate the action with the type of the tile.
     * Also supply the index of the tile whose type we're modifying.
     */
    TileSetUndoAction_Type (guint32 i, Tile *t)
      : TileSetUndoAction_TileIndex (TYPE, i), m_tile (new Tile (*t))
      {
      }

    //! Destroy a type action.
    ~TileSetUndoAction_Type ()
      {
        delete m_tile;
      }

    Glib::ustring get_action_name () const
      {
        return "Type";
      }

    Tile * get_tile () const
      {
        return m_tile;
      }

private:
    Tile *m_tile;
};

//-----------------------------------------------------------------------------

//! A record of a tile's pattern being changed
/**
 * The purpose of the TileSetUndoAction_Pattern class is to record
 * when we change the tile's pattern.  e.g. solid, stippled, etc.
 *
 */
class TileSetUndoAction_Pattern: public TileSetUndoAction_TileIndex
{
public:
    //! Make a new pattern action
    /**
     * Populate the action with the pattern of the tile.
     * Also supply the index of the tile whose pattern we're modifying.
     */
    TileSetUndoAction_Pattern (guint32 i, SmallTile::Pattern p)
      : TileSetUndoAction_TileIndex (PATTERN, i), m_pattern (p)
      {
      }

    //! Destroy a pattern action.
    ~TileSetUndoAction_Pattern ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Pattern";
      }

    SmallTile::Pattern get_pattern () const
      {
        return m_pattern;
      }

private:
    SmallTile::Pattern m_pattern;
};

//-----------------------------------------------------------------------------

//! A record of a tile's moves being changed
/**
 * The purpose of the TileSetUndoAction_Moves class is to record
 * when we change the tile's moves.  e.g. how many movement points it takes
 * to cross this kind of terrain.
 *
 */
class TileSetUndoAction_Moves: public TileSetUndoAction_TileIndex
{
public:
    //! Make a new moves action
    /**
     * Populate the action with the moves of the tile.
     * Also supply the index of the tile whose moves we're modifying.
     */
    TileSetUndoAction_Moves (guint32 i, guint32 mp)
      : TileSetUndoAction_TileIndex (MOVES, i, true), m_moves (mp)
      {
      }

    //! Destroy a moves action.
    ~TileSetUndoAction_Moves ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Moves";
      }

    guint32 get_moves () const
      {
        return m_moves;
      }

private:
    guint32 m_moves;
};

//-----------------------------------------------------------------------------

//! A record of the tile's color changing in the tileset editor.
/**
 * The purpose of the TileSetUndoAction_Color class is to record
 * when a tile's first, second or third color has been modified.
 */
class TileSetUndoAction_Color: public TileSetUndoAction_TileIndex
{
public:
    //! Make a new color action
    /**
     * Populate the color action with the index of the tile, the
     * color number (e.g. 1, 2 or 3), and finally the actual color.
     */
    TileSetUndoAction_Color (guint32 i, guint32 n, Gdk::RGBA color)
      : TileSetUndoAction_TileIndex (COLOR, i), m_color_number (n),
      m_color (color)
  {
  }
    //! Destroy a color action.
    ~TileSetUndoAction_Color ()
      {
      }

    Glib::ustring get_action_name () const 
      {
        switch (m_color_number)
          {
          case 0:
            return "First Color";

          case 1:
            return "Second Color";

          case 2:
            return "Third Color";

          default:
            break;
          }
        return "";
      }

    guint32 get_color_number () const
      {
        return m_color_number;
      }

    Gdk::RGBA get_color () const
      {
        return m_color;
      }

private:
    guint32 m_color_number;
    Gdk::RGBA m_color;
};

//-----------------------------------------------------------------------------

//! A helper class for events that require saving the whole tileset

/**
 * Several actions require saving the whole tar file because it's the
 * easiest way to implement undo/redo.
 */
class TileSetUndoAction_Save: public TileSetUndoAction
{
public:
    TileSetUndoAction_Save (Tileset *s, Type t)
      :TileSetUndoAction (t)
      {
        m_tileset = new Tileset (*s);
        m_filename = File::get_tmp_file () + TILESET_EXT;
        s->save (m_filename, TILESET_EXT);
      }

    ~TileSetUndoAction_Save ()
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
 * The purpose of the TileSetUndoAction_AddTile class is to record
 * when a new blank tile has been added to the set.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetUndoAction_AddTile: public TileSetUndoAction_Save
{
public:
    //! Make a new add tile action
    /**
     * Populate the add tile action with the tileset.
     */
    TileSetUndoAction_AddTile (Tileset *t)
      :TileSetUndoAction_Save (t, TileSetUndoAction::ADD_TILE)
      {
      }

    //! Destroy an add tile action, and delete the file.
    ~TileSetUndoAction_AddTile ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add Tile";
      }
};

//-----------------------------------------------------------------------------

//! A record of a tile being erased to the tileset
/**
 * The purpose of the TileSetUndoAction_RemoveTile class is to record
 * when a tile has been deleted from the set.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetUndoAction_RemoveTile: public TileSetUndoAction_Save
{
public:
    //! Make a new remove tile action
    /**
     * Populate the remove tile action with the tileset.
     */
    TileSetUndoAction_RemoveTile (Tileset *t)
      :TileSetUndoAction_Save (t, TileSetUndoAction::REMOVE_TILE)
      {
      }

    //! Destroy a remove tile action, and delete the file.
    ~TileSetUndoAction_RemoveTile ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Remove Tile";
      }
};

//-----------------------------------------------------------------------------

//! A record of an image being modified in the tileset
/**
 */
class TileSetUndoAction_AddImage: public TileSetUndoAction_Save
{
public:

    TileSetUndoAction_AddImage (Tileset *t)
      :TileSetUndoAction_Save (t, TileSetUndoAction::ADD_IMAGE)
      {
      }

    ~TileSetUndoAction_AddImage ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add Image";
      }
};

//-----------------------------------------------------------------------------

//! A record of an image being cleared in the tileset
/**
 */
class TileSetUndoAction_ClearImage: public TileSetUndoAction_Save
{
public:

    TileSetUndoAction_ClearImage (Tileset *t)
      :TileSetUndoAction_Save (t, TileSetUndoAction::CLEAR_IMAGE)
      {
      }

    ~TileSetUndoAction_ClearImage ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Clear Image";
      }
};

//-----------------------------------------------------------------------------

//! A record of the tilestyles being organized within the tileset
/**
 * The purpose of the TileSetUndoAction_TileStyles class is to record
 * when the tileset's tilestyles have been changed en masse.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetUndoAction_TileStyles: public TileSetUndoAction_Save
{
public:
    //! Make a new tilestyles action
    /**
     * Populate the tilestyles action with the tileset.
     */
    TileSetUndoAction_TileStyles (Tileset *t)
      :TileSetUndoAction_Save (t, TileSetUndoAction::TILESTYLES)
      {
      }

    //! Destroy a tilestyles action, and delete the file.
    ~TileSetUndoAction_TileStyles ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "TileStyles";
      }
};

//-----------------------------------------------------------------------------

//! A record of the building colors being modified in the tileset
/**
 * The purpose of the TileSetUndoAction_BuildingColors class is to record
 * when the tileset's building colors have been changed.  e.g. the color of
 * the roads on the smallmap, the color of the temple dots, etc.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetUndoAction_BuildingColors: public TileSetUndoAction_Save
{
public:
    //! Make a new building colors action
    /**
     * Populate the building colors action with the tileset.
     */
    TileSetUndoAction_BuildingColors (Tileset *t)
      :TileSetUndoAction_Save (t, TileSetUndoAction::BUILDING_COLORS)
      {
      }

    //! Destroy a building colors action, and delete the file.
    ~TileSetUndoAction_BuildingColors ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Building Colors";
      }
};

#endif
