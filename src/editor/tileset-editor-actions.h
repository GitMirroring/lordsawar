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
#ifndef TILESET_EDITOR_ACTIONS_H
#define TILESET_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "Tile.h"
#include "SmallTile.h"
#include "tilestyle.h"
#include "defs.h"
#include "undo-action.h"
#include "undo-mgr.h"

class Tileset;

//! A record of an event in the tileset editor
/** 
 * The purpose of these classes is to implement undo/redo in the tileset
 * editor.
 */

class TileSetEditorAction: public UndoAction
{
public:

    //! A TileSet Editor Action can be one of the following kinds.
    enum Type {
      CHANGE_PROPERTIES = 1,
      NAME = 2,
      TYPE = 3,
      PATTERN = 4,
      MOVES = 5,
      COLOUR = 6,
      ADD_TILESTYLESET = 7,
      REMOVE_TILESTYLESET = 8,
      TILESTYLE = 9,
      ADD_TILE = 10,
      REMOVE_TILE = 11,
      SELECTOR = 12,
      EXPLOSION = 13,
      ROADS = 14,
      STONES = 15,
      BRIDGES = 16,
      FOG = 17,
      FLAGS = 18,
      TILESTYLES = 19,
      BUILDING_COLOURS = 20,
      MOVE_BONUS = 21,
    };

    //! Default constructor.
    TileSetEditorAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

//-----------------------------------------------------------------------------

//! A record of the tileset's properties changing in the editor.
/**
 * The purpose of the TileSetEditorAction_Properties class is to record
 * when a tileset's name, description, copyright, license, and tilesize have
 * changed.
 */
class TileSetEditorAction_Properties: public TileSetEditorAction
{
    public:
	//! Make a new change properties action
	/**
         * Populate the properties action with the new name, description,
         * copyright, license text, and tile size.
         */
        TileSetEditorAction_Properties (Glib::ustring n, Glib::ustring d, Glib::ustring c, Glib::ustring l, guint32 ts)
          : TileSetEditorAction (CHANGE_PROPERTIES), d_name (n), d_desc (d),
          d_copyright (c), d_license (l), d_tile_size (ts) {}
	//! Destroy a change properties action.
        ~TileSetEditorAction_Properties () {}

        Glib::ustring getActionName () const {return _("Properties");}

        Glib::ustring getName () {return d_name;}
        Glib::ustring getDescription () {return d_desc;}
        Glib::ustring getCopyright () {return d_copyright;}
        Glib::ustring getLicense () {return d_license;}
        guint32 getTileSize () {return d_tile_size;}

    private:
        Glib::ustring d_name;
        Glib::ustring d_desc;
        Glib::ustring d_copyright;
        Glib::ustring d_license;
        guint32 d_tile_size;
};

//-----------------------------------------------------------------------------

//! A helper class for events that require referencing the tile's place
//in the set. this equates to the position in the treeview.

class TileSetEditorAction_TileIndex: public TileSetEditorAction
{
    public:
        TileSetEditorAction_TileIndex (Type t, guint32 i, bool agg = false)
          : TileSetEditorAction (t, agg), d_index (i) {}
        ~TileSetEditorAction_TileIndex () {}

        guint32 getIndex () {return d_index;}
    private:
        guint32 d_index;
};

//-----------------------------------------------------------------------------

//! A record of a tile's name being changed
/**
 * The purpose of the TileSetEditorAction_Name class is to record
 * when we change the tile's name.  This happens letter by letter.
 *
 */
class TileSetEditorAction_Name: public TileSetEditorAction_TileIndex, public UndoCursor
{
    public:
	//! Make a new name action
	/**
         * Populate the action with the name of the tile.
         * Also supply the index of the tile whose name we're modifying.
         */
        TileSetEditorAction_Name (guint32 i, Glib::ustring n, UndoMgr *u,
                                  Gtk::Entry *e)
          : TileSetEditorAction_TileIndex (NAME, i, true), UndoCursor (u, e),
          d_name (n) {}
	//! Destroy a name action.
        ~TileSetEditorAction_Name () {}

        Glib::ustring getActionName () const {return _("Name");}

        Glib::ustring getName () {return d_name;}

    private:
        Glib::ustring d_name;
};

//-----------------------------------------------------------------------------

//! A record of a tile's type being changed
/**
 * The purpose of the TileSetEditorAction_Type class is to record
 * when we change the tile's type.  e.g. grass, water, forest, etc.
 *
 */
class TileSetEditorAction_Type: public TileSetEditorAction_TileIndex
{
    public:
	//! Make a new type action
	/**
         * Populate the action with the type of the tile.
         * Also supply the index of the tile whose type we're modifying.
         */
        TileSetEditorAction_Type (guint32 i, Tile::Type t)
          : TileSetEditorAction_TileIndex (TYPE, i), d_tile_type (t) {}
	//! Destroy a type action.
        ~TileSetEditorAction_Type () {}

        Glib::ustring getActionName () const {return _("Type");}

        Tile::Type getTileType () {return d_tile_type;}

    private:
        Tile::Type d_tile_type;
};

//-----------------------------------------------------------------------------

//! A record of a tile's pattern being changed
/**
 * The purpose of the TileSetEditorAction_Pattern class is to record
 * when we change the tile's pattern.  e.g. solid, stippled, etc.
 *
 */
class TileSetEditorAction_Pattern: public TileSetEditorAction_TileIndex
{
    public:
	//! Make a new pattern action
	/**
         * Populate the action with the pattern of the tile.
         * Also supply the index of the tile whose pattern we're modifying.
         */
        TileSetEditorAction_Pattern (guint32 i, SmallTile::Pattern p)
          : TileSetEditorAction_TileIndex (PATTERN, i), d_pattern (p) {}
	//! Destroy a pattern action.
        ~TileSetEditorAction_Pattern () {}

        Glib::ustring getActionName () const {return _("Pattern");}

        SmallTile::Pattern getPattern () {return d_pattern;}

    private:
        SmallTile::Pattern d_pattern;
};

//-----------------------------------------------------------------------------

//! A record of a tile's moves being changed
/**
 * The purpose of the TileSetEditorAction_Moves class is to record
 * when we change the tile's moves.  e.g. how many movement points it takes
 * to cross this kind of terrain.
 *
 */
class TileSetEditorAction_Moves: public TileSetEditorAction_TileIndex
{
    public:
	//! Make a new moves action
	/**
         * Populate the action with the moves of the tile.
         * Also supply the index of the tile whose moves we're modifying.
         */
        TileSetEditorAction_Moves (guint32 i, guint32 mp)
          : TileSetEditorAction_TileIndex (MOVES, i, true), d_moves (mp) {}
	//! Destroy a moves action.
        ~TileSetEditorAction_Moves () {}

        Glib::ustring getActionName () const {return _("Moves");}

        guint32 getMoves () {return d_moves;}

    private:
        guint32 d_moves;
};

//-----------------------------------------------------------------------------

//! A record of the tile's colour changing in the tileset editor.
/**
 * The purpose of the TileSetEditorAction_Colour class is to record
 * when a tile's first, second or third colour has been modified.
 */
class TileSetEditorAction_Colour: public TileSetEditorAction_TileIndex
{
    public:
	//! Make a new colour action
	/**
         * Populate the colour action with the index of the tile, the
         * colour number (e.g. 1, 2 or 3), and finally the actual colour.
         */
        TileSetEditorAction_Colour (guint32 i, guint32 n, Gdk::RGBA colour)
          : TileSetEditorAction_TileIndex (COLOUR, i), d_colour_number (n),
          d_colour (colour) {}
	//! Destroy a colour action.
        ~TileSetEditorAction_Colour () {}

        Glib::ustring getActionName () const 
          {
            switch (d_colour_number)
              {
              case 0:
                return _("First Colour");
              case 1:
                return _("Second Colour");
              case 2:
                return _("Third Colour");
              default:
                break;
              }
            return "";
          }
        guint32 getColourNumber () {return d_colour_number;}
        Gdk::RGBA getColour () const {return d_colour;}

    private:
        guint32 d_colour_number;
        Gdk::RGBA d_colour;
};

//-----------------------------------------------------------------------------

//! A helper class for events that require saving the whole tileset

/**
 * Several actions require saving the whole tar file because it's the
 * easiest way to implement undo/redo.
 */
class TileSetEditorAction_Save: public TileSetEditorAction
{
    public:
        TileSetEditorAction_Save (Tileset *s, Type t);
        ~TileSetEditorAction_Save ();

        Glib::ustring getTilesetFilename () const {return d_filename;}
        Tileset *getTileset () const {return d_tileset;}
    private:
        Glib::ustring d_filename;
        Tileset *d_tileset;
};

//-----------------------------------------------------------------------------

//! A record of a new tilestyle set being added to the tileset
/**
 * The purpose of the TileSetEditorAction_AddTileStyleSet class is to record
 * when a new image is added to the set, which is called a "tile style set".
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetEditorAction_AddTileStyleSet: public TileSetEditorAction_Save
{
    public:
	//! Make a new add tilestyleset action
	/**
         * Populate the add tilestyleset action with the tileset.
         */
        TileSetEditorAction_AddTileStyleSet (Tileset *t)
          :TileSetEditorAction_Save (t, TileSetEditorAction::ADD_TILESTYLESET) {}
	//! Destroy an add tilestyleset action, and delete the file.
        ~TileSetEditorAction_AddTileStyleSet () {}

        Glib::ustring getActionName () const {return _("Add TileStyle Set");}
};

//-----------------------------------------------------------------------------

//! A record of a new tilestyle set being erased from the tileset
/**
 * The purpose of the TileSetEditorAction_RemoveTileStyleSet class is to record
 * when a "tile style set" image is deleted from the set.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetEditorAction_RemoveTileStyleSet: public TileSetEditorAction_Save
{
    public:
	//! Make a new remove tilestyleset action
	/**
         * Populate the remove tilestyleset action with the tileset.
         */
        TileSetEditorAction_RemoveTileStyleSet (Tileset *t)
          :TileSetEditorAction_Save
           (t, TileSetEditorAction::REMOVE_TILESTYLESET) {}
	//! Destroy a remove tilestyleset action, and delete the file.
        ~TileSetEditorAction_RemoveTileStyleSet () {}

        Glib::ustring getActionName () const {return _("Remove TileStyle Set");}
};

//-----------------------------------------------------------------------------

//! A record of a tilestyle being changed
/**
 * The purpose of the TileSetEditorAction_TileStyle class is to record
 * when we change a tilestyle's type.
 *
 */
class TileSetEditorAction_TileStyle: public TileSetEditorAction_TileIndex
{
    public:
	//! Make a new tilestyle action
	/**
         * Populate the action with the index of the tile whose tilestyle
         * we're modifying, the index of the tilestyle, and lastly the new
         * tilestyle type.
         */
        TileSetEditorAction_TileStyle (guint32 i, guint32 v, TileStyle::Type t)
          : TileSetEditorAction_TileIndex (TILESTYLE, i), d_tilestyle_index (v),
          d_tilestyle_type (t) {}
	//! Destroy a tilestyle action.
        ~TileSetEditorAction_TileStyle () {}

        Glib::ustring getActionName () const {return _("TileStyle Type");}

        guint32 getTileStyleIndex () {return d_tilestyle_index;}
        TileStyle::Type getTileStyleType () {return d_tilestyle_type;}
    private:
        guint32 d_tilestyle_index;
        TileStyle::Type d_tilestyle_type;
};

//-----------------------------------------------------------------------------

//! A record of a new tile being added to the tileset
/**
 * The purpose of the TileSetEditorAction_AddTile class is to record
 * when a new blank tile has been added to the set.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetEditorAction_AddTile: public TileSetEditorAction_Save
{
    public:
	//! Make a new add tile action
	/**
         * Populate the add tile action with the tileset.
         */
        TileSetEditorAction_AddTile (Tileset *t)
          :TileSetEditorAction_Save (t, TileSetEditorAction::ADD_TILE) {}
	//! Destroy an add tile action, and delete the file.
        ~TileSetEditorAction_AddTile () {}

        Glib::ustring getActionName () const {return _("Add Tile");}
};

//-----------------------------------------------------------------------------

//! A record of a tile being erased to the tileset
/**
 * The purpose of the TileSetEditorAction_RemoveTile class is to record
 * when a tile has been deleted from the set.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetEditorAction_RemoveTile: public TileSetEditorAction_Save
{
    public:
	//! Make a new remove tile action
	/**
         * Populate the remove tile action with the tileset.
         */
        TileSetEditorAction_RemoveTile (Tileset *t)
          :TileSetEditorAction_Save (t, TileSetEditorAction::REMOVE_TILE) {}
	//! Destroy a remove tile action, and delete the file.
        ~TileSetEditorAction_RemoveTile () {}

        Glib::ustring getActionName () const {return _("Remove Tile");}
};

//-----------------------------------------------------------------------------

//! A record of the selector images being modified in the tileset
/**
 * The purpose of the TileSetEditorAction_Selector class is to record
 * when the tileset's selector images have been changed.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetEditorAction_Selector: public TileSetEditorAction_Save
{
    public:
	//! Make a new selector action
	/**
         * Populate the selector action with the tileset.
         */
        TileSetEditorAction_Selector (Tileset *t)
          :TileSetEditorAction_Save (t, TileSetEditorAction::SELECTOR) {}
	//! Destroy a selector action, and delete the file.
        ~TileSetEditorAction_Selector () {}
        Glib::ustring getActionName () const {return _("Selector");}
};

//-----------------------------------------------------------------------------

//! A record of the explosion image being modified in the tileset
/**
 * The purpose of the TileSetEditorAction_Explosion class is to record
 * when the tileset's explosion image has been changed.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetEditorAction_Explosion: public TileSetEditorAction_Save
{
    public:
	//! Make a new explosion action
	/**
         * Populate the explosion action with the tileset.
         */
        TileSetEditorAction_Explosion (Tileset *t)
          :TileSetEditorAction_Save (t, TileSetEditorAction::EXPLOSION) {}
	//! Destroy a explosion action, and delete the file.
        ~TileSetEditorAction_Explosion () {}
        Glib::ustring getActionName () const {return _("Explosion");}
};

//-----------------------------------------------------------------------------

//! A record of the road images being modified in the tileset
/**
 * The purpose of the TileSetEditorAction_Roads class is to record
 * when the tileset's road images have been changed.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetEditorAction_Roads: public TileSetEditorAction_Save
{
    public:
	//! Make a new roads action
	/**
         * Populate the roads action with the tileset.
         */
        TileSetEditorAction_Roads (Tileset *t)
          :TileSetEditorAction_Save (t, TileSetEditorAction::ROADS) {}
	//! Destroy a roads action, and delete the file.
        ~TileSetEditorAction_Roads () {}
        Glib::ustring getActionName () const {return _("Roads");}
};

//-----------------------------------------------------------------------------

//! A record of the standing stone images being modified in the tileset
/**
 * The purpose of the TileSetEditorAction_Stones class is to record
 * when the tileset's standing stone images have been changed.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetEditorAction_Stones: public TileSetEditorAction_Save
{
    public:
	//! Make a new stones action
	/**
         * Populate the stones action with the tileset.
         */
        TileSetEditorAction_Stones (Tileset *t)
          :TileSetEditorAction_Save (t, TileSetEditorAction::STONES) {}
	//! Destroy a stones action, and delete the file.
        ~TileSetEditorAction_Stones () {}
        Glib::ustring getActionName () const {return _("Stones");}
};

//-----------------------------------------------------------------------------

//! A record of the bridge images being modified in the tileset
/**
 * The purpose of the TileSetEditorAction_Bridges class is to record
 * when the tileset's bridge images have been changed.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetEditorAction_Bridges: public TileSetEditorAction_Save
{
    public:
	//! Make a new bridges action
	/**
         * Populate the bridges action with the tileset.
         */
        TileSetEditorAction_Bridges (Tileset *t)
          :TileSetEditorAction_Save (t, TileSetEditorAction::BRIDGES) {}
	//! Destroy a bridges action, and delete the file.
        ~TileSetEditorAction_Bridges () {}
        Glib::ustring getActionName () const {return _("Bridges");}
};

//-----------------------------------------------------------------------------

//! A record of the fog images being modified in the tileset
/**
 * The purpose of the TileSetEditorAction_Fog class is to record
 * when the tileset's fog images have been changed.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetEditorAction_Fog: public TileSetEditorAction_Save
{
    public:
	//! Make a new fog action
	/**
         * Populate the fog action with the tileset.
         */
        TileSetEditorAction_Fog (Tileset *t)
          :TileSetEditorAction_Save (t, TileSetEditorAction::FOG) {}
	//! Destroy a fog action, and delete the file.
        ~TileSetEditorAction_Fog () {}
        Glib::ustring getActionName () const {return _("Fog");}
};

//-----------------------------------------------------------------------------

//! A record of the flag images being modified in the tileset
/**
 * The purpose of the TileSetEditorAction_Flag class is to record
 * when the tileset's flag images have been changed.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetEditorAction_Flag: public TileSetEditorAction_Save
{
    public:
	//! Make a new flag action
	/**
         * Populate the flag action with the tileset.
         */
        TileSetEditorAction_Flag (Tileset *t)
          :TileSetEditorAction_Save (t, TileSetEditorAction::FLAGS) {}
	//! Destroy a flag action, and delete the file.
        ~TileSetEditorAction_Flag () {}
        Glib::ustring getActionName () const {return _("Flags");}
};

//-----------------------------------------------------------------------------

//! A record of the tilestyles being organized within the tileset
/**
 * The purpose of the TileSetEditorAction_TileStyles class is to record
 * when the tileset's tilestyles have been changed en masse.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetEditorAction_TileStyles: public TileSetEditorAction_Save
{
    public:
	//! Make a new tilestyles action
	/**
         * Populate the tilestyles action with the tileset.
         */
        TileSetEditorAction_TileStyles (Tileset *t)
          :TileSetEditorAction_Save (t, TileSetEditorAction::TILESTYLES) {}
	//! Destroy a tilestyles action, and delete the file.
        ~TileSetEditorAction_TileStyles () {}
        Glib::ustring getActionName () const {return _("TileStyles");}
};

//-----------------------------------------------------------------------------

//! A record of the building colours being modified in the tileset
/**
 * The purpose of the TileSetEditorAction_BuildingColours class is to record
 * when the tileset's building colours have been changed.  e.g. the colour of
 * the roads on the smallmap, the colour of the temple dots, etc.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetEditorAction_BuildingColours: public TileSetEditorAction_Save
{
    public:
	//! Make a new building colours action
	/**
         * Populate the building colours action with the tileset.
         */
        TileSetEditorAction_BuildingColours (Tileset *t)
          :TileSetEditorAction_Save
           (t, TileSetEditorAction::BUILDING_COLOURS) {}
	//! Destroy a building colours action, and delete the file.
        ~TileSetEditorAction_BuildingColours () {}
        Glib::ustring getActionName () const {return _("Building Colours");}
};

//-----------------------------------------------------------------------------

//! A record of the move bonus images being modified in the tileset
/**
 * The purpose of the TileSetEditorAction_MoveBonus class is to record
 * when the tileset's move bonus images have been changed.  e.g. flight,
 * faster in forests, faster in hills, etc.
 *
 * We take a copy of the whole tileset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class TileSetEditorAction_MoveBonus: public TileSetEditorAction_Save
{
    public:
	//! Make a new move bonus action
	/**
         * Populate the move bonus action with the tileset.
         */
        TileSetEditorAction_MoveBonus (Tileset *t)
          :TileSetEditorAction_Save (t, TileSetEditorAction::MOVE_BONUS) {}
	//! Destroy a move bonus action, and delete the file.
        ~TileSetEditorAction_MoveBonus () {}
        Glib::ustring getActionName () const {return _("Move Bonus");}
};
#endif //TILESET_EDITOR_ACTIONS_H
