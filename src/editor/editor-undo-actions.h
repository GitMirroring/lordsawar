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
#ifndef EDITOR_UNDO_ACTIONS_H
#define EDITOR_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "defs.h"
#include "rectangle.h"
#include "map-tile.h"
#include "tile.h"
#include "undo-action.h"
#include "scenario.h"
#include "army-set.h"
#include "city-set.h"
#include "shield-set.h"
#include "tile-set.h"

//! A record of an event in the scenario builder
/**
 * The purpose of these classes is to implement undo/redo in the scenario
 * builder.
 */

class Scenario;
class GameScenario;
class Tileset;
class Cityset;
class Shieldset;
class Armyset;

class EditorUndoAction: public UndoAction
{
public:

    //! An Editor Action can be one of the following kinds.
    enum Type
      {
        /** Modify description/copyright/license. */
        CHANGE_PROPERTIES = 1,
        /** The scenario media has been modified */
        SCENARIO_MEDIA = 2,
        /** A section of the map has been modified */
        TERRAIN = 3,
        /** Some game objects have been removed */
        ERASE = 4,
        /** An object was moved from one place to another */
        MOVE = 5,
        /** A stack has been placed on the map */
        STACK = 6,
        /** A city has been placed on the map */
        CITY = 7,
        /** A ruin has been placed on the map */
        RUIN = 8,
        /** A temple has been placed on the map */
        TEMPLE = 9,
        /** A standing stone has been placed on the map */
        STONE = 10,
        /** A signpost has been placed on the map */
        SIGNPOST = 11,
        /** A port has been placed on the map */
        PORT = 12,
        /** A road has been placed on the map */
        ROAD = 13,
        /** A bridge has been placed on the map */
        BRIDGE = 14,
        /** A flag has been placed on the map */
        FLAG = 15,
        /** An existing flag has modified on the map */
        EDIT_FLAG = 16,
        /** An existing stack has modified on the map */
        EDIT_STACK = 17,
        /** An existing city has modified on the map */
        EDIT_CITY = 18,
        /** An existing ruin has modified on the map */
        EDIT_RUIN = 19,
        /** An existing temple has modified on the map */
        EDIT_TEMPLE = 20,
        /** An existing stone has modified on the map */
        EDIT_STONE = 21,
        /** An existing signpost has modified on the map */
        EDIT_SIGNPOST = 22,
        /** A backpack has modified on the map */
        EDIT_BACKPACK = 23,
        /** A road has modified on the map */
        EDIT_ROAD = 24,
        /** The minimap editor modified the map */
        MINIMAP = 25,
        /** One or more players were modified */
        PLAYERS = 26,
        /** One or more items were modified */
        ITEMS = 27,
        /** One or more rewards were modified */
        REWARDS = 28,
        /** The map was partially or completely smoothed */
        SMOOTH = 29,
        /** The scenario modified which tileset, etc. it uses */
        SWITCH_SETS = 30,
        /** The scenario's tileset has changed */
        TILESET = 31,
        /** The scenario's cityset has changed */
        CITYSET = 32,
        /** The scenario's shieldset has changed */
        SHIELDSET = 33,
        /** One of the scenario's armysets have changed */
        ARMYSET = 34,
        /** The fight order has changed */
        FIGHT_ORDER = 35,
        /** All stacks have been removed from the map */
        REMOVE_STACKS = 36,
        /** Randomize some objects on the map */
        RANDOMIZE_OBJECTS = 37,
        /** Players have cities assigned as capitals */
        ASSIGN_CAPITALS = 38,
        /** A particular tilestyle was edited on a tile */
        TILESTYLE = 39,
      };

    //! Default constructor.
    EditorUndoAction (Type type,
                      UndoAction::AggregateType a = UndoAction::AGGREGATE_NONE)
      : UndoAction (a), m_type (type)
      {
      }

    //! Destructor.
    virtual ~EditorUndoAction ()
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

//! A record of the shieldset's properties changing in the editor.
/**
 * The purpose of the EditorUndoAction_Properties class is to record
 * when a map's name, description, copyright and license have changed.
 */
class EditorUndoAction_Properties: public EditorUndoAction
{
public:
    //! Make a new change properties action
    /**
     * Populate the properties action with the new name, description,
     * copyright, and license text.
     */
    EditorUndoAction_Properties (Glib::ustring n, Glib::ustring d,
                                 Glib::ustring c, Glib::ustring l)
      :EditorUndoAction (EditorUndoAction::CHANGE_PROPERTIES), m_name (n),
      m_desc (d), m_copyright (c), m_license (l)
  {
  }
    //! Destroy a change properties action.
    ~EditorUndoAction_Properties ()
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

    Glib::ustring get_license ()
      {
        return m_license;
      }

private:
    Glib::ustring m_name;
    Glib::ustring m_desc;
    Glib::ustring m_copyright;
    Glib::ustring m_license;
};

//-----------------------------------------------------------------------------

//! A helper class for events that require saving the whole map

/**
 * Several actions require saving the whole tar file because it's the
 * easiest way to implement undo/redo.
 */
class EditorUndoAction_Save: public EditorUndoAction
{
public:
    EditorUndoAction_Save (Type t, GameScenario *g)
      :EditorUndoAction (t)
      {
        m_scenario = new Scenario (g);

        m_filename = File::get_tmp_file () + SAVE_EXT;
        m_scenario->get_game_scenario ()->saveGame (m_filename);
      }

    ~EditorUndoAction_Save ()
      {
        File::erase (m_filename);
        delete m_scenario;
      }

    Glib::ustring get_scenario_filename () const
      {
        return m_filename;
      }

    Scenario *get_scenario () const
      {
        return m_scenario;
      }
private:
    Glib::ustring m_filename;
    Scenario *m_scenario;
};

//-----------------------------------------------------------------------------

//! A record of the scenario media being modified
/**
 * The purpose of the EditorUndoAction_ScenarioMedia class is to record
 * when an image or sound file in the scenario media is changed.
 *
 * We take a copy of the whole scenario.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class EditorUndoAction_ScenarioMedia: public EditorUndoAction_Save
{
public:
    //! Make a new scenario media action
    /**
     * Populate the scenario media action with the game scenario.
     */
    EditorUndoAction_ScenarioMedia (GameScenario *g)
      :EditorUndoAction_Save (EditorUndoAction::SCENARIO_MEDIA, g)
      {
      }

    //! Destroy a scenario media action, and delete the file.
    ~EditorUndoAction_ScenarioMedia ()
      {
      }

    Glib::ustring get_action_nName () const
      {
        return "Scenario Media";
      }
};

//-----------------------------------------------------------------------------

//! helper class for undoing regions on the map
class EditorUndoAction_ChangeMap: public EditorUndoAction
{
public:
    EditorUndoAction_ChangeMap (Type t, LwRectangle r, bool grow, bool agg,
                                bool only_maptiles)
      : EditorUndoAction (t, agg ? AGGREGATE_BLANK : AGGREGATE_NONE),
      m_rects (std::list<LwRectangle>()), m_only_maptiles (only_maptiles)
        {
          m_rects.push_back (r);
          LwRectangle rr = r;
          if (grow)
            rr.grow (1);
          std::list<LwRectangle> grown_rr;
          grown_rr.push_back (rr);

          if (!only_maptiles)
            m_objects = GameMap::instance ()->copyObjects (grown_rr);
          m_maptiles = GameMap::instance ()->copyMaptiles (grown_rr);
        }

    EditorUndoAction_ChangeMap (Type t, LwRectangle r1, LwRectangle r2,
                                bool grow, bool agg, bool only_maptiles)
      : EditorUndoAction (t, agg ? AGGREGATE_BLANK : AGGREGATE_NONE),
      m_rects (std::list<LwRectangle>()), m_only_maptiles (only_maptiles)
        {
          m_rects.push_back (r1);
          m_rects.push_back (r2);

          LwRectangle rr1 = r1;
          if (grow)
            rr1.grow (1);

          LwRectangle rr2 = r2;
          if (grow)
            rr2.grow (1);

          std::list<LwRectangle> grown_rr;
          grown_rr.push_back (rr1);
          grown_rr.push_back (rr2);

          if (!only_maptiles)
            m_objects = GameMap::instance ()->copyObjects (grown_rr);
          m_maptiles = GameMap::instance ()->copyMaptiles (grown_rr);
        }

    ~EditorUndoAction_ChangeMap ()
      {
        for (auto m : m_maptiles)
          delete m;
        for (auto o : m_objects)
          delete o;
      }

    LwRectangle get_area () const
      {
        return m_rects.front ();
      }

    std::list<LwRectangle> get_rectangles () const
      {
        return m_rects;
      }

    std::list<Maptile *> get_map_tiles () const
      {
        return m_maptiles;
      }

    std::list<UniquelyIdentified *> get_objects () const
      {
        return m_objects;
      }

    void clear_objects ()
      {
        m_objects = std::list<UniquelyIdentified*>();
      }

    bool get_only_maptiles () const
      {
        return m_only_maptiles;
      }
private:
    std::list<LwRectangle> m_rects;
    bool m_only_maptiles;
    std::list<Maptile *> m_maptiles;
    std::list<UniquelyIdentified *> m_objects;
};

//-----------------------------------------------------------------------------

//! A record of the terrain being modified
/**
 * The purpose of the EditorUndoAction_Terrain class is to record when the map has
 * changed.  The region changed is specified by rect.
 *
 * We capture everything about the enclosed tiles:  cities, ports, stacks,
 * tile type, etc.
 *
 * We actually grab a region one tile larger than rect on all sides.
 *
 */
class EditorUndoAction_Terrain: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new terrain action
    /**
     * Populate the terrain action with the area modified.
     */
    EditorUndoAction_Terrain (int tile_idx, LwRectangle r)
      : EditorUndoAction_ChangeMap (TERRAIN, r, true, true, true),
      m_tile_idx (tile_idx)
      {
      }

    //! Destroy a terrain action.
    ~EditorUndoAction_Terrain ()
      {
      }

    Glib::ustring get_action_name () const
      {
        Tileset *ts = GameMap::getTileset ();
        if (m_tile_idx < 0)
          return "";
        return (*ts)[m_tile_idx]->getName ();
      }

    int get_tile_index () const
      {
        return m_tile_idx;
      }
private:
    int m_tile_idx;
};

//-----------------------------------------------------------------------------

//! A record of a tile on the map being erased
/**
 * The purpose of the EditorUndoAction_Erase class is to record when the map has
 * had game objects removed from a region. e.g. stacks, ports, roads, etc.
 *
 */
class EditorUndoAction_Erase: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new erase action
    /**
     * Populate the erase action with the region that was erased.
     */
    EditorUndoAction_Erase (LwRectangle r)
      : EditorUndoAction_ChangeMap (ERASE, r, false, true, false)
      {
      }

    //! Destroy an erase action.
    ~EditorUndoAction_Erase ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Erase";
      }
};

//-----------------------------------------------------------------------------

//! A record of an object moving from one place to another on the map.
/**
 * The purpose of the EditorUndoAction_Move class is to record when the map has
 * had an object moved on it. e.g. a stack, a city, etc.
 *
 */
class EditorUndoAction_Move: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new move action
    /**
     * Populate the move action with the source and destination regions.
     */
    EditorUndoAction_Move (LwRectangle r1, LwRectangle r2)
      : EditorUndoAction_ChangeMap (MOVE, r1, r2, false, false, false)
      {
      }

    //! Destroy a move action.
    ~EditorUndoAction_Move ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Move";
      }
};

//-----------------------------------------------------------------------------

//! A record of a stack being added to the map.
/**
 * The purpose of the EditorUndoAction_Stack class is to record when a stack
 * is placed on the map.  This is only for new stacks.
 *
 */
class EditorUndoAction_Stack: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new stack action
    /**
     * Populate the stack action with a rectangle of the stack's position.
     */
    EditorUndoAction_Stack (LwRectangle r)
      : EditorUndoAction_ChangeMap (STACK, r, false, false, false)
      {
      }

    //! Destroy a stack action.
    ~EditorUndoAction_Stack ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Stack";
      }
};

//-----------------------------------------------------------------------------

//! A record of a city being added to the map.
/**
 * The purpose of the EditorUndoAction_City class is to record when a city
 * is placed on the map.  This is only for new cities.
 *
 */
class EditorUndoAction_City: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new city action
    /**
     * Populate the city action with a rectangle of the city's position.
     * The dimensions need to be set to the city width.
     */
    EditorUndoAction_City (LwRectangle r)
      : EditorUndoAction_ChangeMap (CITY, r, true, false, false)
      {
      }

    //! Destroy a city action.
    ~EditorUndoAction_City ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "City";
      }
};

//-----------------------------------------------------------------------------

//! A record of a ruin being added to the map.
/**
 * The purpose of the EditorUndoAction_Ruin class is to record when a ruin
 * is placed on the map.  This is only for new ruins.
 *
 */
class EditorUndoAction_Ruin: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new ruin action
    /**
     * Populate the ruin action with a rectangle of the ruin's position.
     * The dimensions need to be set to the ruin width.
     */
    EditorUndoAction_Ruin (LwRectangle r)
      : EditorUndoAction_ChangeMap (RUIN, r, true, false, false)
      {
      }

    //! Destroy a ruin action.
    ~EditorUndoAction_Ruin ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Ruin";
      }
};

//-----------------------------------------------------------------------------

//! A record of a temple being added to the map.
/**
 * The purpose of the EditorUndoAction_Temple class is to record when a temple
 * is placed on the map.  This is only for new temples.
 *
 */
class EditorUndoAction_Temple: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new temple action
    /**
     * Populate the temple action with a rectangle of the temple's position.
     * The dimensions need to be set to the temple width.
     */
    EditorUndoAction_Temple (LwRectangle r)
      : EditorUndoAction_ChangeMap (TEMPLE, r, true, false, false)
      {
      }

    //! Destroy a temple action.
    ~EditorUndoAction_Temple ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Temple";
      }
};

//-----------------------------------------------------------------------------

//! A record of a standing stone being added to the map.
/**
 * The purpose of the EditorUndoAction_Stone class is to record when a standing
 * stone is placed on the map.  This is only for new standing stones.
 *
 */
class EditorUndoAction_Stone: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new stone action
    /**
     * Populate the stone action with a rectangle of the its position.
     */
    EditorUndoAction_Stone (LwRectangle r)
      : EditorUndoAction_ChangeMap (STONE, r, true, true, false)
      {
      }

    //! Destroy a stone action.
    ~EditorUndoAction_Stone ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Stone";
      }
};

//-----------------------------------------------------------------------------

//! A record of a signpost being added to the map.
/**
 * The purpose of the EditorUndoAction_Signpost class is to record when a signpost
 * is placed on the map.  This is only for new signposts.
 *
 */
class EditorUndoAction_Signpost: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new signpost action
    /**
     * Populate the signpost action with a rectangle of the its position.
     */
    EditorUndoAction_Signpost (LwRectangle r)
      : EditorUndoAction_ChangeMap (SIGNPOST, r, true, false, false)
      {
      }

    //! Destroy a signpost action.
    ~EditorUndoAction_Signpost ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Sign";
      }
};

//-----------------------------------------------------------------------------

//! A record of a port being added to the map.
/**
 * The purpose of the EditorUndoAction_Port class is to record when a port is
 * placed on the map.  This is only for new ports.
 *
 */
class EditorUndoAction_Port: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new port action
    /**
     * Populate the port action with a rectangle of the its position.
     */
    EditorUndoAction_Port (LwRectangle r)
      : EditorUndoAction_ChangeMap (PORT, r, true, false, false)
      {
      }

    //! Destroy a port action.
    ~EditorUndoAction_Port ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Port";
      }
};

//-----------------------------------------------------------------------------

//! A record of a road being added to the map.
/**
 * The purpose of the EditorUndoAction_Road class is to record when a road is
 * placed on the map.  This is only for new road objects.
 *
 */
class EditorUndoAction_Road: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new road action
    /**
     * Populate the road action with a rectangle of the its position.
     */
    EditorUndoAction_Road (LwRectangle r)
      : EditorUndoAction_ChangeMap (ROAD, r, true, true, false)
      {
      }

    //! Destroy a road action.
    ~EditorUndoAction_Road ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Road";
      }
};

//-----------------------------------------------------------------------------

//! A record of a bridge being added to the map.
/**
 * The purpose of the EditorUndoAction_Bridge class is to record when a bridge is
 * placed on the map.  This is only for new bridge objects.
 *
 */
class EditorUndoAction_Bridge: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new bridge action
    /**
     * Populate the bridge action with a rectangle of the its position.
     */
    EditorUndoAction_Bridge (LwRectangle r)
      : EditorUndoAction_ChangeMap (BRIDGE, r, true, false, false)
      {
      }

    //! Destroy a bridge action.
    ~EditorUndoAction_Bridge ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Bridge";
      }
};

//-----------------------------------------------------------------------------

//! A record of a flag being added to the map.
/**
 * The purpose of the EditorUndoAction_Flag class is to record when a flag is
 * placed on the map.  This is only for new flag objects.
 *
 */
class EditorUndoAction_Flag: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new flag action
    /**
     * Populate the flag action with a rectangle of the its position.
     */
    EditorUndoAction_Flag (LwRectangle r)
      : EditorUndoAction_ChangeMap (FLAG, r, false, false, false)
      {
      }

    //! Destroy a flag action.
    ~EditorUndoAction_Flag ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Flag";
      }
};

//-----------------------------------------------------------------------------

//! A record of a flag being modified on the map.
/**
 * The purpose of the EditorUndoAction_EditFlag class is to record when a flag is
 * changed on the map.  This is only for existing flag objects, where you
 * might change the owner, etc.
 *
 */
class EditorUndoAction_EditFlag: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new edit-flag action
    /**
     * Populate the edit-flag action with a rectangle of the its position.
     */
    EditorUndoAction_EditFlag (LwRectangle r)
      : EditorUndoAction_ChangeMap (EDIT_FLAG, r, false, false, false)
      {
      }

    //! Destroy an edit-flag action.
    ~EditorUndoAction_EditFlag ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Edit Flag";
      }
};

//-----------------------------------------------------------------------------

//! A record of a stack being modified on the map.
/**
 * The purpose of the EditorUndoAction_EditStack class is to record when a stack
 * is changed on the map.  This is only for existing stack objects, where you
 * might change its owner or its armies.
 *
 */
class EditorUndoAction_EditStack: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new edit-stack action
    /**
     * Populate the edit-stack  action with a rectangle of the its position.
     */
    EditorUndoAction_EditStack (LwRectangle r)
      : EditorUndoAction_ChangeMap (EDIT_STACK, r, false, false, false)
      {
      }

    //! Destroy an edit-stack action.
    ~EditorUndoAction_EditStack ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Edit Stack";
      }
};

//-----------------------------------------------------------------------------

//! A record of a city being modified on the map.
/**
 * The purpose of the EditorUndoAction_EditCity class is to record when a city is
 * changed on the map.  This is only for existing city objects, where you
 * might change its owner, name, production, etc.
 *
 */
class EditorUndoAction_EditCity: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new edit-city action
    /**
     * Populate the edit-city action with a rectangle of the its position.
     */
    EditorUndoAction_EditCity (LwRectangle r)
      : EditorUndoAction_ChangeMap (EDIT_CITY, r, false, false, false)
      {
      }

    //! Destroy an edit-city action.
    ~EditorUndoAction_EditCity ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Edit City";
      }
};

//-----------------------------------------------------------------------------

//! A record of a ruin being modified on the map.
/**
 * The purpose of the EditorUndoAction_EditRuin class is to record when a ruin is
 * changed on the map.  This is only for existing ruin objects, where you
 * might change its name, keeper, etc.
 *
 */
class EditorUndoAction_EditRuin: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new edit-ruin action
    /**
     * Populate the edit-ruin action with a rectangle of the its position.
     */
    EditorUndoAction_EditRuin (LwRectangle r)
      : EditorUndoAction_ChangeMap (EDIT_RUIN, r, false, false, false)
      {
      }

    //! Destroy an edit-ruin action.
    ~EditorUndoAction_EditRuin ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Edit Ruin";
      }
};

//-----------------------------------------------------------------------------

//! A record of a temple being modified on the map.
/**
 * The purpose of the EditorUndoAction_EditTemple class is to record when a temple
 * is changed on the map.  This is only for existing temple objects, where you
 * might change its name or type.
 *
 */
class EditorUndoAction_EditTemple: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new edit-temple action
    /**
     * Populate the edit-temple action with a rectangle of the its position.
     */
    EditorUndoAction_EditTemple (LwRectangle r)
      : EditorUndoAction_ChangeMap (EDIT_TEMPLE, r, false, false, false)
      {
      }

    //! Destroy an edit-temple action.
    ~EditorUndoAction_EditTemple ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Edit Temple";
      }
};

//-----------------------------------------------------------------------------

//! A record of a standing stone being modified on the map.
/**
 * The purpose of the EditorUndoAction_EditStone class is to record when a
 * standing stone is changed on the map.  This is only for existing standing
 * stones where you might change its type.
 *
 */
class EditorUndoAction_EditStone: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new edit-stone action
    /**
     * Populate the edit-stone action with a rectangle of the its position.
     */
    EditorUndoAction_EditStone (LwRectangle r)
      : EditorUndoAction_ChangeMap (EDIT_STONE, r, false, false, false)
      {
      }

    //! Destroy an edit-stone action.
    ~EditorUndoAction_EditStone ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Edit Stone";
      }
};

//-----------------------------------------------------------------------------

//! A record of a signpost being modified on the map.
/**
 * The purpose of the EditorUndoAction_EditSignpost class is to record when a
 * sign is changed on the map.  This is only for existing signs where you
 * might change its text.
 *
 */
class EditorUndoAction_EditSignpost: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new edit-signpost action
    /**
     * Populate the edit-signpost action with a rectangle of the its
     * position.
     */
    EditorUndoAction_EditSignpost (LwRectangle r)
      : EditorUndoAction_ChangeMap (EDIT_SIGNPOST, r, false, false, false)
      {
      }

    //! Destroy an edit-signpost action.
    ~EditorUndoAction_EditSignpost ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Edit Sign";
      }
};

//-----------------------------------------------------------------------------

//! A record of a backpack being modified on the map.
/**
 * The purpose of the EditorUndoAction_EditBackpack class is to record when a
 * pack is changed on the map, for example its items are changed.
 *
 */
class EditorUndoAction_EditBackpack: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new edit-backpack action
    /**
     * Populate the edit-backpack action with a rectangle of the its
     * position.
     */
    EditorUndoAction_EditBackpack (LwRectangle r)
      : EditorUndoAction_ChangeMap (EDIT_BACKPACK, r, false, false, false)
      {
      }

    //! Destroy an edit-backpack action.
    ~EditorUndoAction_EditBackpack ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Edit Backpack";
      }
};
//-----------------------------------------------------------------------------

//! A record of a road being modified on the map.
/**
 * The purpose of the EditorUndoAction_EditRoad class is to record when a road is
 * changed on the map.  e.g. its type.
 *
 */
class EditorUndoAction_EditRoad: public EditorUndoAction_ChangeMap
{
public:
    //! Make a new edit-road action
    /**
     * Populate the edit-road action with a rectangle of the its
     * position.
     */
    EditorUndoAction_EditRoad (LwRectangle r)
      : EditorUndoAction_ChangeMap (EDIT_ROAD, r, false, false, false)
      {
      }

    //! Destroy an edit-road action.
    ~EditorUndoAction_EditRoad ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Edit Road";
      }
};

class EditorUndoAction_MiniMap: public EditorUndoAction_Save
{
public:
    EditorUndoAction_MiniMap (GameScenario *g)
      :EditorUndoAction_Save (EditorUndoAction::MINIMAP, g)
      {
      }

    ~EditorUndoAction_MiniMap ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Miniature Map";
      }
};

class EditorUndoAction_Players: public EditorUndoAction_Save
{
public:
    EditorUndoAction_Players (GameScenario *g)
      :EditorUndoAction_Save (EditorUndoAction::PLAYERS, g)
      {
      }

    ~EditorUndoAction_Players ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Players";
      }
};

class EditorUndoAction_Items: public EditorUndoAction_Save
{
public:
    EditorUndoAction_Items (GameScenario *g)
      :EditorUndoAction_Save (EditorUndoAction::ITEMS, g)
      {
      }

    ~EditorUndoAction_Items ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Items";
      }
};

class EditorUndoAction_Rewards: public EditorUndoAction_Save
{
    public:
        EditorUndoAction_Rewards (GameScenario *g)
          :EditorUndoAction_Save (EditorUndoAction::REWARDS, g)
          {
          }

        ~EditorUndoAction_Rewards ()
          {
          }

        Glib::ustring get_action_name () const
          {
            return "Rewards";
          }
};

class EditorUndoAction_Smooth: public EditorUndoAction_ChangeMap
{
public:
    EditorUndoAction_Smooth (LwRectangle r)
      :EditorUndoAction_ChangeMap (EditorUndoAction::SMOOTH, r, false, false,
                                   true)
  {
  }

    ~EditorUndoAction_Smooth ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Smooth Map";
      }
};

class EditorUndoAction_SwitchSets: public EditorUndoAction_Save
{
public:
    EditorUndoAction_SwitchSets (GameScenario *g)
      :EditorUndoAction_Save (EditorUndoAction::SWITCH_SETS, g)
      {
      }

    ~EditorUndoAction_SwitchSets ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Switch Sets";
      }
};

class EditorUndoAction_TileSet: public EditorUndoAction
{
public:
    EditorUndoAction_TileSet (Tileset *t)
      :EditorUndoAction (EditorUndoAction::TILESET)
      {
        m_tileset = new Tileset (*t);
        m_filename = File::get_tmp_file () + TILESET_EXT;
        t->save (m_filename, TILESET_EXT);
      }

    ~EditorUndoAction_TileSet ()
      {
        File::erase (m_filename);
        delete m_tileset;
      }

    Glib::ustring get_action_name () const
      {
        return "Tile Set";
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

class EditorUndoAction_CitySet: public EditorUndoAction
{
public:
    EditorUndoAction_CitySet (Cityset *c)
      :EditorUndoAction (EditorUndoAction::CITYSET)
      {
        m_cityset = new Cityset (*c);
        m_filename = File::get_tmp_file () + CITYSET_EXT;
        c->save (m_filename, CITYSET_EXT);
      }

    ~EditorUndoAction_CitySet ()
      {
        File::erase (m_filename);
        delete m_cityset;
      }

    Glib::ustring get_action_name () const
      {
        return "City Set";
      }

    Glib::ustring get_cityset_filename () const
      {
        return m_filename;
      }

    Cityset *get_cityset () const
      {
        return m_cityset;
      }
private:
    Glib::ustring m_filename;
    Cityset *m_cityset;
};

class EditorUndoAction_ShieldSet: public EditorUndoAction
{
public:
    EditorUndoAction_ShieldSet (Shieldset *s)
      :EditorUndoAction (EditorUndoAction::SHIELDSET)
      {
        m_shieldset = new Shieldset (*s);
        m_filename = File::get_tmp_file () + SHIELDSET_EXT;
        s->save (m_filename, SHIELDSET_EXT);
      }

    ~EditorUndoAction_ShieldSet ()
      {
        File::erase (m_filename);
        delete m_shieldset;
      }

    Glib::ustring get_action_name () const
      {
        return "Shield Set";
      }

    Glib::ustring get_shieldset_filename () const
      {
        return m_filename;
      }

    Shieldset *get_shieldset () const
      {
        return m_shieldset;
      }
private:
    Glib::ustring m_filename;
    Shieldset *m_shieldset;
};

class EditorUndoAction_ArmySet: public EditorUndoAction
{
public:
    EditorUndoAction_ArmySet (Armyset *a)
      :EditorUndoAction (EditorUndoAction::ARMYSET)
      {
        m_armyset = new Armyset (*a);
        m_filename = File::get_tmp_file () + ARMYSET_EXT;
        a->save (m_filename, ARMYSET_EXT);
      }

    ~EditorUndoAction_ArmySet ()
      {
        File::erase (m_filename);
        delete m_armyset;
      }

    Glib::ustring get_action_name () const
      {
        return "Army Set";
      }

    Glib::ustring get_armyset_filename () const
      {
        return m_filename;
      }

    Armyset *get_armyset () const
      {
        return m_armyset;
      }
private:
    Glib::ustring m_filename;
    Armyset *m_armyset;
};

class EditorUndoAction_FightOrder: public EditorUndoAction_Save
{
public:
    EditorUndoAction_FightOrder (GameScenario *g)
      :EditorUndoAction_Save (EditorUndoAction::FIGHT_ORDER, g)
      {
      }

    ~EditorUndoAction_FightOrder ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Fight Order";
      }
};

class EditorUndoAction_RemoveStacks: public EditorUndoAction_Save
{
    public:
        EditorUndoAction_RemoveStacks (GameScenario *g)
          :EditorUndoAction_Save (EditorUndoAction::REMOVE_STACKS, g)
          {
          }

        ~EditorUndoAction_RemoveStacks ()
          {
          }

        Glib::ustring get_action_name () const
          {
            return "Remove All Stacks";
          }
};

class EditorUndoAction_RandomizeObjects: public EditorUndoAction_Save
{
public:
    EditorUndoAction_RandomizeObjects (GameScenario *g)
      :EditorUndoAction_Save (EditorUndoAction::RANDOMIZE_OBJECTS, g)
      {
      }

    ~EditorUndoAction_RandomizeObjects ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Randomize Objects ";
      }
};

class EditorUndoAction_AssignCapitals: public EditorUndoAction_Save
{
public:
    EditorUndoAction_AssignCapitals (GameScenario *g)
      :EditorUndoAction_Save (EditorUndoAction::ASSIGN_CAPITALS, g)
      {
      }

    ~EditorUndoAction_AssignCapitals ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Assign Capital Cities";
      }
};

class EditorUndoAction_TileStyle: public EditorUndoAction
{
public:
    EditorUndoAction_TileStyle (Vector<int> pos, guint32 id)
      : EditorUndoAction (TILESTYLE), m_pos (pos), m_id (id)
      {
      }

    ~EditorUndoAction_TileStyle ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "TileStyle";
      }

    Vector<int> get_pos () const
      {
        return m_pos;
      }

    guint32 get_tilestyle_id () const
      {
        return m_id;
      }
private:
    Vector<int> m_pos;
    guint32 m_id;
};
#endif
