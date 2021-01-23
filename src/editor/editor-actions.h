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
#ifndef EDITOR_ACTIONS_H
#define EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "defs.h"
#include "rectangle.h"
#include "maptile.h"
#include "Tile.h"

//! A record of an event in the shieldset editor
/** 
 * The purpose of these classes is to implement undo/redo in the shieldset
 * editor.
 */

class Scenario;
class GameScenario;
class Tileset;
class Cityset;
class Shieldset;
class Armyset;

class EditorAction
{
    public:

	//! An Editor Action can be one of the following kinds.
        enum Type {
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
                /** All cities are given random contents */
                RAND_ALL_CITIES = 37,
                /** All unnamed cities are given random contents */
                RAND_UNNAMED_CITIES = 38,
                /** All ruins are given random contents */
                RAND_ALL_RUINS = 39,
                /** All unnamed ruins are given random contents */
                RAND_UNNAMED_RUINS = 40,
                /** All temples are given random details */
                RAND_ALL_TEMPLES = 41,
                /** All unnamed temples are given random details */
                RAND_UNNAMED_TEMPLES = 42,
                /** All signs are given random messages */
                RAND_ALL_SIGNS = 43,
                /** All empty signs are given random messages */
                RAND_UNNAMED_SIGNS = 44,
                /** Players have cities assigned as capitals */
                ASSIGN_CAPITALS = 45,
                /** Change active player */
                ACTIVE_PLAYER = 46,
                /** A new backpack has been placed on the map */
                BACKPACK = 47,
        };

	//! Default constructor.
        EditorAction(Type type) : d_type (type) {}

	//! Destructor.
        virtual ~EditorAction() {}

        //! Get the name of this action for the undo/redo menuitem.
        virtual Glib::ustring getActionName () const {return "";}

        //! Returns the Action::Type for this action.
        Type getType() const {return d_type;}

    protected:

        Type d_type;
};

//-----------------------------------------------------------------------------

//! A record of the shieldset's properties changing in the editor.
/**
 * The purpose of the EditorAction_Properties class is to record
 * when a map's name, description, copyright and license have changed.
 */
class EditorAction_Properties: public EditorAction
{
    public:
	//! Make a new change properties action
	/**
         * Populate the properties action with the new name, description,
         * copyright, and license text.
         */
        EditorAction_Properties (Glib::ustring n, Glib::ustring d, Glib::ustring c, Glib::ustring l)
          :EditorAction(EditorAction::CHANGE_PROPERTIES), d_name (n),
          d_desc (d), d_copyright (c), d_license (l) {}
	//! Destroy a change properties action.
        ~EditorAction_Properties () {}

        Glib::ustring getActionName () const {return _("Properties");}

        Glib::ustring getName () {return d_name;}
        Glib::ustring getDescription () {return d_desc;}
        Glib::ustring getCopyright () {return d_copyright;}
        Glib::ustring getLicense () {return d_license;}

    private:
        Glib::ustring d_name;
        Glib::ustring d_desc;
        Glib::ustring d_copyright;
        Glib::ustring d_license;
};

//-----------------------------------------------------------------------------

//! A helper class for events that require saving the whole map

/**
 * Several actions require saving the whole tar file because it's the
 * easiest way to implement undo/redo.
 */
class EditorAction_Save: public EditorAction
{
    public:
        EditorAction_Save (Type t, GameScenario *g);
        ~EditorAction_Save ();

        Glib::ustring getScenarioFilename () const {return d_filename;}
        Scenario *getScenario () const {return d_scenario;}
    private:
        Glib::ustring d_filename;
        Scenario *d_scenario;
};

//-----------------------------------------------------------------------------

//! A record of the scenario media being modified
/**
 * The purpose of the EditorAction_ScenarioMedia class is to record
 * when an image or sound file in the scenario media is changed.
 *
 * We take a copy of the whole scenario.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class EditorAction_ScenarioMedia: public EditorAction_Save
{
    public:
	//! Make a new scenario media action
	/**
         * Populate the scenario media action with the game scenario.
         */
        EditorAction_ScenarioMedia (GameScenario *g)
          :EditorAction_Save(EditorAction::SCENARIO_MEDIA, g) {}
	//! Destroy a scenario media action, and delete the file.
        ~EditorAction_ScenarioMedia () {}

        Glib::ustring getActionName () const {return _("Scenario Media");}
};

//-----------------------------------------------------------------------------

//! helper class for undoing regions on the map
class EditorAction_ChangeMap: public EditorAction
{
    public:
        EditorAction_ChangeMap (Type t, LwRectangle r, bool grow);
        EditorAction_ChangeMap (Type t, LwRectangle r1, LwRectangle r2, bool grow);
        ~EditorAction_ChangeMap ();

        LwRectangle getArea () const {return rects.front ();}
        std::list<LwRectangle> getRectangles () const {return rects;}
        std::list<Maptile *> getMaptiles () const {return maptiles;}
        std::list<UniquelyIdentified *> getObjects () const {return objects;}
        void clearObjects ();
    private:
        std::list<LwRectangle> rects;
        std::list<Maptile *> maptiles;
        std::list<UniquelyIdentified *> objects;
};

//-----------------------------------------------------------------------------

//! A record of the terrain being modified
/**
 * The purpose of the EditorAction_Terrain class is to record when the map has
 * changed.  The region changed is specified by rect.
 *
 * We capture everything about the enclosed tiles:  cities, ports, stacks,
 * tile type, etc.
 *
 * We actually grab a region one tile larger than rect on all sides.
 *
 */
class EditorAction_Terrain: public EditorAction_ChangeMap
{
    public:
	//! Make a new terrain action
	/**
         * Populate the terrain action with the area modified.
         */
        EditorAction_Terrain (Tile::Type t, LwRectangle r)
          : EditorAction_ChangeMap (TERRAIN, r, true), d_tile (t) {};
	//! Destroy a terrain action.
        ~EditorAction_Terrain () {};

        Glib::ustring getActionName () const;
        Tile::Type getTile () const {return d_tile;}
    private:
        Tile::Type d_tile;
};

//-----------------------------------------------------------------------------

//! A record of a tile on the map being erased
/**
 * The purpose of the EditorAction_Erase class is to record when the map has
 * had game objects removed from a region. e.g. stacks, ports, roads, etc.
 *
 */
class EditorAction_Erase: public EditorAction_ChangeMap
{
    public:
	//! Make a new erase action
	/**
         * Populate the erase action with the region that was erased.
         */
        EditorAction_Erase (LwRectangle r)
          : EditorAction_ChangeMap (ERASE, r, false) {};
	//! Destroy an erase action.
        ~EditorAction_Erase () {};

        Glib::ustring getActionName () const {return _("Erase");}
};

//-----------------------------------------------------------------------------

//! A record of an object moving from one place to another on the map.
/**
 * The purpose of the EditorAction_Move class is to record when the map has
 * had an object moved on it. e.g. a stack, a city, etc.
 *
 */
class EditorAction_Move: public EditorAction_ChangeMap
{
    public:
	//! Make a new move action
	/**
         * Populate the move action with the source and destination regions.
         */
        EditorAction_Move (LwRectangle r1, LwRectangle r2)
          : EditorAction_ChangeMap (MOVE, r1, r2, false) {};
	//! Destroy a move action.
        ~EditorAction_Move () {};

        Glib::ustring getActionName () const {return _("Move");}
};

//-----------------------------------------------------------------------------

//! A record of a stack being added to the map.
/**
 * The purpose of the EditorAction_Stack class is to record when a stack
 * is placed on the map.  This is only for new stacks.
 *
 */
class EditorAction_Stack: public EditorAction_ChangeMap
{
    public:
	//! Make a new stack action
	/**
         * Populate the stack action with a rectangle of the stack's position.
         */
        EditorAction_Stack (LwRectangle r)
          : EditorAction_ChangeMap (STACK, r, false) {};
	//! Destroy a stack action.
        ~EditorAction_Stack() {};

        Glib::ustring getActionName () const {return _("Stack");}
};

//-----------------------------------------------------------------------------

//! A record of a city being added to the map.
/**
 * The purpose of the EditorAction_City class is to record when a city
 * is placed on the map.  This is only for new cities.
 *
 */
class EditorAction_City: public EditorAction_ChangeMap
{
    public:
	//! Make a new city action
	/**
         * Populate the city action with a rectangle of the city's position.
         * The dimensions need to be set to the city width.
         */
        EditorAction_City (LwRectangle r)
          : EditorAction_ChangeMap (CITY, r, true) {};
	//! Destroy a city action.
        ~EditorAction_City () {};

        Glib::ustring getActionName () const {return _("City");}
};

//-----------------------------------------------------------------------------

//! A record of a ruin being added to the map.
/**
 * The purpose of the EditorAction_Ruin class is to record when a ruin
 * is placed on the map.  This is only for new ruins.
 *
 */
class EditorAction_Ruin: public EditorAction_ChangeMap
{
    public:
	//! Make a new ruin action
	/**
         * Populate the ruin action with a rectangle of the ruin's position.
         * The dimensions need to be set to the ruin width.
         */
        EditorAction_Ruin (LwRectangle r)
          : EditorAction_ChangeMap (RUIN, r, true) {};
	//! Destroy a ruin action.
        ~EditorAction_Ruin () {};

        Glib::ustring getActionName () const {return _("Ruin");}
};

//-----------------------------------------------------------------------------

//! A record of a temple being added to the map.
/**
 * The purpose of the EditorAction_Temple class is to record when a temple
 * is placed on the map.  This is only for new temples.
 *
 */
class EditorAction_Temple: public EditorAction_ChangeMap
{
    public:
	//! Make a new temple action
	/**
         * Populate the temple action with a rectangle of the temple's position.
         * The dimensions need to be set to the temple width.
         */
        EditorAction_Temple (LwRectangle r)
          : EditorAction_ChangeMap (TEMPLE, r, true) {};
	//! Destroy a temple action.
        ~EditorAction_Temple () {};

        Glib::ustring getActionName () const {return _("Temple");}
};

//-----------------------------------------------------------------------------

//! A record of a standing stone being added to the map.
/**
 * The purpose of the EditorAction_Stone class is to record when a standing
 * stone is placed on the map.  This is only for new standing stones.
 *
 */
class EditorAction_Stone: public EditorAction_ChangeMap
{
    public:
	//! Make a new stone action
	/**
         * Populate the stone action with a rectangle of the its position.
         */
        EditorAction_Stone (LwRectangle r)
          : EditorAction_ChangeMap (STONE, r, true) {};
	//! Destroy a stone action.
        ~EditorAction_Stone () {};

        Glib::ustring getActionName () const {return _("Stone");}
};

//-----------------------------------------------------------------------------

//! A record of a signpost being added to the map.
/**
 * The purpose of the EditorAction_Signpost class is to record when a signpost
 * is placed on the map.  This is only for new signposts.
 *
 */
class EditorAction_Signpost: public EditorAction_ChangeMap
{
    public:
	//! Make a new signpost action
	/**
         * Populate the signpost action with a rectangle of the its position.
         */
        EditorAction_Signpost (LwRectangle r)
          : EditorAction_ChangeMap (SIGNPOST, r, true) {};
	//! Destroy a signpost action.
        ~EditorAction_Signpost () {};

        Glib::ustring getActionName () const {return _("Sign");}
};

//-----------------------------------------------------------------------------

//! A record of a port being added to the map.
/**
 * The purpose of the EditorAction_Port class is to record when a port is
 * placed on the map.  This is only for new ports.
 *
 */
class EditorAction_Port: public EditorAction_ChangeMap
{
    public:
	//! Make a new port action
	/**
         * Populate the port action with a rectangle of the its position.
         */
        EditorAction_Port (LwRectangle r)
          : EditorAction_ChangeMap (PORT, r, true) {};
	//! Destroy a port action.
        ~EditorAction_Port () {};

        Glib::ustring getActionName () const {return _("Port");}
};

//-----------------------------------------------------------------------------

//! A record of a road being added to the map.
/**
 * The purpose of the EditorAction_Road class is to record when a road is
 * placed on the map.  This is only for new road objects.
 *
 */
class EditorAction_Road: public EditorAction_ChangeMap
{
    public:
	//! Make a new road action
	/**
         * Populate the road action with a rectangle of the its position.
         */
        EditorAction_Road (LwRectangle r)
          : EditorAction_ChangeMap (ROAD, r, true) {};
	//! Destroy a road action.
        ~EditorAction_Road () {};

        Glib::ustring getActionName () const {return _("Road");}
};

//-----------------------------------------------------------------------------

//! A record of a bridge being added to the map.
/**
 * The purpose of the EditorAction_Bridge class is to record when a bridge is
 * placed on the map.  This is only for new bridge objects.
 *
 */
class EditorAction_Bridge: public EditorAction_ChangeMap
{
    public:
	//! Make a new bridge action
	/**
         * Populate the bridge action with a rectangle of the its position.
         */
        EditorAction_Bridge (LwRectangle r)
          : EditorAction_ChangeMap (BRIDGE, r, true) {};
	//! Destroy a bridge action.
        ~EditorAction_Bridge () {};

        Glib::ustring getActionName () const {return _("Bridge");}
};

//-----------------------------------------------------------------------------

//! A record of a flag being added to the map.
/**
 * The purpose of the EditorAction_Flag class is to record when a flag is
 * placed on the map.  This is only for new flag objects.
 *
 */
class EditorAction_Flag: public EditorAction_ChangeMap
{
    public:
	//! Make a new flag action
	/**
         * Populate the flag action with a rectangle of the its position.
         */
        EditorAction_Flag (LwRectangle r)
          : EditorAction_ChangeMap (FLAG, r, false) {};
	//! Destroy a flag action.
        ~EditorAction_Flag () {};

        Glib::ustring getActionName () const {return _("Flag");}
};

//-----------------------------------------------------------------------------

//! A record of a flag being modified on the map.
/**
 * The purpose of the EditorAction_EditFlag class is to record when a flag is
 * changed on the map.  This is only for existing flag objects, where you 
 * might change the owner, etc.
 *
 */
class EditorAction_EditFlag: public EditorAction_ChangeMap
{
    public:
	//! Make a new edit-flag action
	/**
         * Populate the edit-flag action with a rectangle of the its position.
         */
        EditorAction_EditFlag (LwRectangle r)
          : EditorAction_ChangeMap (EDIT_FLAG, r, false) {};
	//! Destroy an edit-flag action.
        ~EditorAction_EditFlag () {};

        Glib::ustring getActionName () const {return _("Edit Flag");}
};

//-----------------------------------------------------------------------------

//! A record of a stack being modified on the map.
/**
 * The purpose of the EditorAction_EditStack class is to record when a stack
 * is changed on the map.  This is only for existing stack objects, where you 
 * might change its owner or its armies.
 *
 */
class EditorAction_EditStack: public EditorAction_ChangeMap
{
    public:
	//! Make a new edit-stack action
	/**
         * Populate the edit-stack  action with a rectangle of the its position.
         */
        EditorAction_EditStack (LwRectangle r)
          : EditorAction_ChangeMap (EDIT_STACK, r, false) {};
	//! Destroy an edit-stack action.
        ~EditorAction_EditStack () {};

        Glib::ustring getActionName () const {return _("Edit Stack");}
};

//-----------------------------------------------------------------------------

//! A record of a city being modified on the map.
/**
 * The purpose of the EditorAction_EditCity class is to record when a city is
 * changed on the map.  This is only for existing city objects, where you 
 * might change its owner, name, production, etc.
 *
 */
class EditorAction_EditCity: public EditorAction_ChangeMap
{
    public:
	//! Make a new edit-city action
	/**
         * Populate the edit-city action with a rectangle of the its position.
         */
        EditorAction_EditCity (LwRectangle r)
          : EditorAction_ChangeMap (EDIT_CITY, r, false) {};
	//! Destroy an edit-city action.
        ~EditorAction_EditCity () {};

        Glib::ustring getActionName () const {return _("Edit City");}
};

//-----------------------------------------------------------------------------

//! A record of a ruin being modified on the map.
/**
 * The purpose of the EditorAction_EditRuin class is to record when a ruin is
 * changed on the map.  This is only for existing ruin objects, where you 
 * might change its name, keeper, etc.
 *
 */
class EditorAction_EditRuin: public EditorAction_ChangeMap
{
    public:
	//! Make a new edit-ruin action
	/**
         * Populate the edit-ruin action with a rectangle of the its position.
         */
        EditorAction_EditRuin (LwRectangle r)
          : EditorAction_ChangeMap (EDIT_RUIN, r, false) {};
	//! Destroy an edit-ruin action.
        ~EditorAction_EditRuin () {};

        Glib::ustring getActionName () const {return _("Edit Ruin");}
};

//-----------------------------------------------------------------------------

//! A record of a temple being modified on the map.
/**
 * The purpose of the EditorAction_EditTemple class is to record when a temple 
 * is changed on the map.  This is only for existing temple objects, where you 
 * might change its name or type.
 *
 */
class EditorAction_EditTemple: public EditorAction_ChangeMap
{
    public:
	//! Make a new edit-temple action
	/**
         * Populate the edit-temple action with a rectangle of the its position.
         */
        EditorAction_EditTemple (LwRectangle r)
          : EditorAction_ChangeMap (EDIT_TEMPLE, r, false) {};
	//! Destroy an edit-temple action.
        ~EditorAction_EditTemple () {};

        Glib::ustring getActionName () const {return _("Edit Temple");}
};

//-----------------------------------------------------------------------------

//! A record of a standing stone being modified on the map.
/**
 * The purpose of the EditorAction_EditStone class is to record when a 
 * standing stone is changed on the map.  This is only for existing standing
 * stones where you might change its type.
 *
 */
class EditorAction_EditStone: public EditorAction_ChangeMap
{
    public:
	//! Make a new edit-stone action
	/**
         * Populate the edit-stone action with a rectangle of the its position.
         */
        EditorAction_EditStone (LwRectangle r)
          : EditorAction_ChangeMap (EDIT_STONE, r, false) {};
	//! Destroy an edit-stone action.
        ~EditorAction_EditStone () {};

        Glib::ustring getActionName () const {return _("Edit Stone");}
};

//-----------------------------------------------------------------------------

//! A record of a signpost being modified on the map.
/**
 * The purpose of the EditorAction_EditSignpost class is to record when a 
 * sign is changed on the map.  This is only for existing signs where you
 * might change its text.
 *
 */
class EditorAction_EditSignpost: public EditorAction_ChangeMap
{
    public:
	//! Make a new edit-signpost action
	/**
         * Populate the edit-signpost action with a rectangle of the its
         * position.
         */
        EditorAction_EditSignpost (LwRectangle r)
          : EditorAction_ChangeMap (EDIT_SIGNPOST, r, false) {};
	//! Destroy an edit-signpost action.
        ~EditorAction_EditSignpost () {};

        Glib::ustring getActionName () const {return _("Edit Sign");}
};

//-----------------------------------------------------------------------------

//! A record of a backpack being modified on the map.
/**
 * The purpose of the EditorAction_EditBackpack class is to record when a 
 * pack is changed on the map, for example its items are changed.
 *
 */
class EditorAction_EditBackpack: public EditorAction_ChangeMap
{
    public:
	//! Make a new edit-backpack action
	/**
         * Populate the edit-backpack action with a rectangle of the its
         * position.
         */
        EditorAction_EditBackpack (LwRectangle r)
          : EditorAction_ChangeMap (EDIT_BACKPACK, r, false) {};
	//! Destroy an edit-backpack action.
        ~EditorAction_EditBackpack () {};

        Glib::ustring getActionName () const {return _("Edit Backpack");}
};
//-----------------------------------------------------------------------------

//! A record of a road being modified on the map.
/**
 * The purpose of the EditorAction_EditRoad class is to record when a road is
 * changed on the map.  e.g. its type.
 *
 */
class EditorAction_EditRoad: public EditorAction_ChangeMap
{
    public:
	//! Make a new edit-road action
	/**
         * Populate the edit-road action with a rectangle of the its
         * position.
         */
        EditorAction_EditRoad (LwRectangle r)
          : EditorAction_ChangeMap (EDIT_ROAD, r, false) {};
	//! Destroy an edit-road action.
        ~EditorAction_EditRoad () {};

        Glib::ustring getActionName () const {return _("Edit Road");}
};

class EditorAction_MiniMap: public EditorAction_Save
{
    public:
        EditorAction_MiniMap (GameScenario *g)
          :EditorAction_Save(EditorAction::MINIMAP, g) {}
        ~EditorAction_MiniMap () {}
        Glib::ustring getActionName () const {return _("Miniature Map");}
};
class EditorAction_Players: public EditorAction_Save
{
    public:
        EditorAction_Players (GameScenario *g)
          :EditorAction_Save(EditorAction::PLAYERS, g) {}
        ~EditorAction_Players () {}
        Glib::ustring getActionName () const {return _("Players");}
};
class EditorAction_Items: public EditorAction_Save
{
    public:
        EditorAction_Items (GameScenario *g)
          :EditorAction_Save(EditorAction::ITEMS, g) {}
        ~EditorAction_Items () {}
        Glib::ustring getActionName () const {return _("Items");}
};
class EditorAction_Rewards: public EditorAction_Save
{
    public:
        EditorAction_Rewards (GameScenario *g)
          :EditorAction_Save(EditorAction::REWARDS, g) {}
        ~EditorAction_Rewards () {}
        Glib::ustring getActionName () const {return _("Rewards");}
};
class EditorAction_Smooth: public EditorAction_Save
{
    public:
        EditorAction_Smooth (GameScenario *g)
          :EditorAction_Save(EditorAction::SMOOTH, g) {}
        ~EditorAction_Smooth () {}
        Glib::ustring getActionName () const {return _("Smooth Map");}
};
class EditorAction_SwitchSets: public EditorAction_Save
{
    public:
        EditorAction_SwitchSets (GameScenario *g)
          :EditorAction_Save(EditorAction::SWITCH_SETS, g) {}
        ~EditorAction_SwitchSets () {}
        Glib::ustring getActionName () const {return _("Switch Sets");}
};
class EditorAction_TileSet: public EditorAction
{
    public:
        EditorAction_TileSet (Tileset *t);
        ~EditorAction_TileSet ();
        Glib::ustring getActionName () const {return _("Tile Set");}
        Glib::ustring getTileSetFilename () const {return d_filename;}
        Tileset *getTileSet () const {return d_tileset;}
    private:
        Glib::ustring d_filename;
        Tileset *d_tileset;
};
class EditorAction_CitySet: public EditorAction
{
    public:
        EditorAction_CitySet (Cityset *t);
        ~EditorAction_CitySet ();
        Glib::ustring getActionName () const {return _("City Set");}
        Glib::ustring getCitySetFilename () const {return d_filename;}
        Cityset *getCitySet () const {return d_cityset;}
    private:
        Glib::ustring d_filename;
        Cityset *d_cityset;
};
class EditorAction_ShieldSet: public EditorAction
{
    public:
        EditorAction_ShieldSet (Shieldset *t);
        ~EditorAction_ShieldSet ();
        Glib::ustring getActionName () const {return _("Shield Set");}
        Glib::ustring getShieldSetFilename () const {return d_filename;}
        Shieldset *getShieldSet () const {return d_shieldset;}
    private:
        Glib::ustring d_filename;
        Shieldset *d_shieldset;
};
class EditorAction_ArmySet: public EditorAction
{
    public:
        EditorAction_ArmySet (Armyset *t);
        ~EditorAction_ArmySet ();
        Glib::ustring getActionName () const {return _("Army Set");}
        Glib::ustring getArmySetFilename () const {return d_filename;}
        Armyset *getArmySet () const {return d_armyset;}
    private:
        Glib::ustring d_filename;
        Armyset *d_armyset;
};
class EditorAction_FightOrder: public EditorAction_Save
{
    public:
        EditorAction_FightOrder (GameScenario *g)
          :EditorAction_Save(EditorAction::FIGHT_ORDER, g) {}
        ~EditorAction_FightOrder () {}
        Glib::ustring getActionName () const {return _("Fight Order");}
};
class EditorAction_RemoveStacks: public EditorAction_Save
{
    public:
        EditorAction_RemoveStacks (GameScenario *g)
          :EditorAction_Save(EditorAction::REMOVE_STACKS, g) {}
        ~EditorAction_RemoveStacks () {}
        Glib::ustring getActionName () const {return _("Remove All Stacks");}
};
class EditorAction_RandAllCities: public EditorAction_Save
{
    public:
        EditorAction_RandAllCities (GameScenario *g)
          :EditorAction_Save(EditorAction::RAND_ALL_CITIES, g) {}
        ~EditorAction_RandAllCities () {}
        Glib::ustring getActionName () const {return _("Random All Cities");}
};
class EditorAction_RandUnnamedCities: public EditorAction_Save
{
    public:
        EditorAction_RandUnnamedCities (GameScenario *g)
          :EditorAction_Save(EditorAction::RAND_UNNAMED_CITIES, g) {}
        ~EditorAction_RandUnnamedCities () {}
        Glib::ustring getActionName () const {return _("Random Unnamed Cities");}
};
class EditorAction_RandAllRuins: public EditorAction_Save
{
    public:
        EditorAction_RandAllRuins (GameScenario *g)
          :EditorAction_Save(EditorAction::RAND_ALL_RUINS, g) {}
        ~EditorAction_RandAllRuins () {}
        Glib::ustring getActionName () const {return _("Random All Ruins");}
};
class EditorAction_RandUnnamedRuins: public EditorAction_Save
{
    public:
        EditorAction_RandUnnamedRuins (GameScenario *g)
          :EditorAction_Save(EditorAction::RAND_UNNAMED_RUINS, g) {}
        ~EditorAction_RandUnnamedRuins () {}
        Glib::ustring getActionName () const {return _("Random Unnamed Ruins");}
};
class EditorAction_RandAllTemples: public EditorAction_Save
{
    public:
        EditorAction_RandAllTemples (GameScenario *g)
          :EditorAction_Save(EditorAction::RAND_ALL_TEMPLES, g) {}
        ~EditorAction_RandAllTemples () {}
        Glib::ustring getActionName () const {return _("Random All Temples");}
};
class EditorAction_RandUnnamedTemples: public EditorAction_Save
{
    public:
        EditorAction_RandUnnamedTemples (GameScenario *g)
          :EditorAction_Save(EditorAction::RAND_UNNAMED_TEMPLES, g) {}
        ~EditorAction_RandUnnamedTemples () {}
        Glib::ustring getActionName () const {return _("Random Unnamed Temples");}
};
class EditorAction_RandAllSigns: public EditorAction_Save
{
    public:
        EditorAction_RandAllSigns (GameScenario *g)
          :EditorAction_Save(EditorAction::RAND_ALL_SIGNS, g) {}
        ~EditorAction_RandAllSigns () {}
        Glib::ustring getActionName () const {return _("Random All Signs");}
};
class EditorAction_RandUnnamedSigns: public EditorAction_Save
{
    public:
        EditorAction_RandUnnamedSigns (GameScenario *g)
          :EditorAction_Save(EditorAction::RAND_UNNAMED_SIGNS, g) {}
        ~EditorAction_RandUnnamedSigns () {}
        Glib::ustring getActionName () const {return _("Random Unnamed Signs");}
};
class EditorAction_AssignCapitals: public EditorAction_Save
{
    public:
        EditorAction_AssignCapitals (GameScenario *g)
          :EditorAction_Save(EditorAction::ASSIGN_CAPITALS, g) {}
        ~EditorAction_AssignCapitals () {}
        Glib::ustring getActionName () const {return _("Assign Capital Cities");}
};

//-----------------------------------------------------------------------------

//! A record of the active player changing in the editor.
/**
 * The purpose of the EditorAction_ActivePlayer class is to record when the
 * active player changes in the editor.  This relates to the row of shield
 * toggle buttons.
 */
class EditorAction_ActivePlayer: public EditorAction
{
    public:
	//! Make a new active-player action
	/**
         * Populate the active-player action with the player id.
         */
        EditorAction_ActivePlayer (guint32 id)
          :EditorAction(EditorAction::ACTIVE_PLAYER), d_id (id) {}
	//! Destroy an active-player action.
        ~EditorAction_ActivePlayer () {}

        Glib::ustring getActionName () const {return _("Select Player");}

        guint32 getId () const {return d_id;}

    private:
        guint32 d_id;
};

//-----------------------------------------------------------------------------

//! A record of a backpack being added to the map.
/**
 * The purpose of the EditorAction_Backpack class is to record when a backpack
 * is placed on the map.  This is only for new backpack objects.
 *
 */
class EditorAction_Backpack: public EditorAction_ChangeMap
{
    public:
	//! Make a new backpack action
	/**
         * Populate the backpack action with a rectangle of the its position.
         */
        EditorAction_Backpack (LwRectangle r)
          : EditorAction_ChangeMap (BACKPACK, r, false) {};
	//! Destroy a backpack action.
        ~EditorAction_Backpack () {};

        Glib::ustring getActionName () const {return _("Backpack");}
};
#endif //EDITOR_ACTIONS_H
