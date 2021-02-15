// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003, 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2014, 2015, 2020,
// 2021 Ben Asselstine
// Copyright (C) 2008 Janek Kozicki
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
#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <sigc++/trackable.h>

#include <vector>

#include <gtkmm.h>
#include "vector.h"
#include "rectangle.h"
#include "maptile.h"

class ArmyProto;
class Movable;
class Stack;
class Location;
class Shieldset;
class Cityset;
class MapGenerator;
class XML_Helper;
class Port;
class Road;
class Stone;
class City;
class Temple;
class Bridge;
class Ruin;
class Armyset;
class Signpost;
class LocationBox;
class Tileset;
class Army;

//! The model of the map and an interface to interacting with everything on it.
/** Class representing the map in the game
  * 
  * GameMap represents a single map. In most cases this will be the map that is
  * currently played, but it might be used to preview a map in a mapeditor too.
  * Notes: GameMap was prefered over Map, because of the potential confusion 
  * with the std::map from the STL.
  */

class GameMap: public sigc::trackable
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! The xml tag of the itemstack subobject in a saved-game file.
	static Glib::ustring d_itemstack_tag; 

        //! Makes a copy of the citylist.
        GameMap* copy () {return new GameMap (*this);}

        /** Singleton function to get the GameMap instance
          * 
          * @return singleton instance
          */
        static GameMap* getInstance() {
          return s_instance ? s_instance : getInstance("", "", ""); };


        /** Returns singleton instance or creates a new one using the tileset
          * 
          * @param TilesetName      the name of the tileset to be used
          * @return singleton instance
          */
        static GameMap* getInstance(Glib::ustring TilesetName,
				    Glib::ustring Shieldsetname,
				    Glib::ustring Citysetname);

        /** Creates a new singleton instance from a savegame file
          * 
          * @param helper           see XML_Helper for an explanation
          *
          * \note This function deletes an existing instance!
          */
        static GameMap* getInstance(XML_Helper* helper);

        //! Explicitly deletes the singleton instance
        static void deleteInstance();

        //! Set the width of the game map
        static void setWidth(int width){s_width = width;}
 
        //! Set the height of the game map
        static void setHeight(int height){s_height = height;}

        //! Returns the width of the map
        static int getWidth() {return s_width;}
 
        //! Returns the height of the map
        static int getHeight() {return s_height;}

        //! Returns the dimensions of the map, as a vector.
	static Vector<int> get_dim() { return Vector<int>(s_width, s_height); }
	
        //! Returns the dimensions of the map, as a Rectangle.
	static LwRectangle get_boundary()
	    { return LwRectangle(0, 0, s_width, s_height); }

        //! Returns a pointer to the current Tileset for the map.
        static Tileset* getTileset();

        //! Returns a pointer to the current Cityset for the map.
        static Cityset* getCityset();

        //! Returns all of the Armysets.
        static std::vector<Armyset*> getArmysets ();

        //! Returns a pointer to the current Shieldset for the map.
        static Shieldset* getShieldset();

        /** Change the map's Tileset.
          * 
          * @param tileset The name of the Tileset to change to.
          *
          * \note This just changes the pointer that GameMap returns when
          * asked for the current Tileset.  switchTileset is more comprehensive.
          */
        void setTileset(Glib::ustring tileset);

        //! Return the width of a tile on the BigMap in pixels after scaling.
        guint32 getTileSize() const;

        //! Return the width of a tile on the BigMap in pixels before scaling.
        guint32 getUnscaledTileSize() const;

        //! Return the id of the current Tileset.  Returns zero if a valid tileset hasn't been set yet.
        guint32 getTilesetId() const;

        //! Return the id of the current Cityset.  Returns zero if a valid cityset hasn't been set yet.
        guint32 getCitysetId() const;

        //! Return the id of the current Shieldset.  Returns zero if a valid shieldset hasn't been set yet.
        guint32 getShieldsetId() const;

        //! Return the basename of the current Tileset.
        Glib::ustring getTilesetBaseName() const;

        //! Return the basename of the current Cityset.
        Glib::ustring getCitysetBaseName() const;

        //! Return the basename of the current Shieldset.
        Glib::ustring getShieldsetBaseName() const;

        /** Change the map's Shieldset.
          * 
          * @param shieldset The name of the Shieldset to change to.
          *
          * \note This just changes the pointer that GameMap returns when
          * asked for the current Shieldset.  switchShieldset is more 
          * comprehensive.
          */
        void setShieldset(Glib::ustring shieldset);

        /** Change the map's Cityset.
          * 
          * @param cityset The name of the Cityset to change to.
          *
          * \note This just changes the pointer that GameMap returns when
          * asked for the current Cityset.  switchCityset is more 
          * comprehensive.
          */
        void setCityset(Glib::ustring cityset);

        /** Change a Maptile on the map.
          * 
          * @param x The horizontal index of the map.
          * @param y The vertical index of the map.
          * @param tile The new Maptile to install at x,y.
          *
          * \note This method deletes the old tile if one is already present.
          * \note A new TileStyle is automatically assigned to the Maptile.
          */
	//void setTile(int x, int y, Maptile *tile);
	void setTileIndex(int x, int y, guint32 new_index);

        /** Change a Maptile on the map.
          * 
          * @param p The position on the map to modify.
          * @param tile The new Maptile to install at p.
          *
          * \note This method deletes the old tile if one is already present.
          * \note A new TileStyle is automatically assigned to the Maptile.
          */
        //void setTile(Vector<int> p, Maptile *t) {return setTile(p.x, p.y, t);}
        void setTileIndex(Vector<int> p, guint32 new_index) {return setTileIndex(p.x, p.y, new_index);}

        /** Return a pointer to the City at the given position on the map.
          * 
          * @param pos The position on the map to look for a City at.
          *
          * @return Returns NULL if a City is not at the given position.
          */
	static City* getCity(Vector<int> pos);

        /** Return a pointer to the City at the position of the given Movable.
         *
         * @param m The Movable object to look for a City under.
         *
         * \note Stack objects are Movable objects.  This method is used to 
         * return the city that a stack is sitting on.
         * @return Returns NULL if no City is found.
         */
        static City* getCity(Movable *m);

        /** Return a pointer to the enemy City at the given position on the map.
         *
         * @param pos The position on the map to look for an enemy City at.
         *
         * \note Enemy cities are defined as cities not owned by the 
         * Playerlist::getActiveplayer().
         * @return Returns NULL if a City owned by the enemy is not found.
         */
	static City* getEnemyCity(Vector<int> pos);

        /** Return a pointer to a Ruin at the given position on the map.
         *
         * @param pos The position on the map to look for a Ruin at.
         *
         * @return Returns NULL if a Ruin is not found.
         */
	static Ruin* getRuin(Vector<int> pos);

        /** Return a pointer to the Ruin at the position of the given Movable.
         *
         * @param m The Movable object to look for a Ruin under.
         *
         * @return Returns NULL if a Ruin is not found.
         */
        static Ruin* getRuin(Movable *m);

        /** Return a pointer to a Temple at the given position on the map.
         *
         * @param pos The position on the map to look for a Temple at.
         *
         * @return Returns NULL if a Temple is not found.
         */
	static Temple* getTemple(Vector<int> pos);

        /** Return a pointer to the Temple at the position of the given Movable.
         *
         * @param m The Movable object to look for a Temple under.
         *
         * \note Stack objects are Movable objects.  This method is used to 
         * return the Temple that a Stack is sitting on.
         *
         * @return Returns NULL if a Temple is not found.
         */
        static Temple* getTemple(Movable *m);

        /** Return a pointer to a Port at the given position on the map.
         *
         * @param pos The position on the map to look for a Port at.
         *
         * @return Returns NULL if a Port is not found.
         */
	static Port* getPort(Vector<int> pos);

        /** Return a pointer to a Road at the given position on the map.
         *
         * @param pos The position on the map to look for a Road at.
         *
         * @return Returns NULL if a Road is not found.
         */
	static Road* getRoad(Vector<int> pos);

        /** Return a pointer to a Bridge at the given position on the map.
         *
         * @param pos The position on the map to look for a Bridge at.
         *
         * @return Returns NULL if a Bridge is not found.
         */
	static Bridge* getBridge(Vector<int> pos);

        /** Return a pointer to a Signpost at the given position on the map.
         *
         * @param pos The position on the map to look for a Signpost at.
         *
         * @return Returns NULL if a Signpost is not found.
         */
	static Signpost* getSignpost(Vector<int> pos);

        /** Return a pointer to a Signpost at the position of the given Movable.
         *
         * @param m The Movable object to look for a Signpost under.
         *
         * \note Stack objects are Movable objects.  This method is used to 
         * return the Signpost that a Stack is sitting on.
         *
         * @return Returns NULL if a Signpost is not found.
         */
        static Signpost* getSignpost(Movable *m);

        /** Return a pointer to the Stack at the given position on the map.
         *
         * @param pos The position on the map to look for a stack at.
         *
         * \note This method does not take into account the 
         * Playerlist::getActiveplayer().  It simply returns a pointer to the
         * first stack it finds at the given location.
         * \note More than one stack can sit on a tile.  This method just 
         * returns a pointer to a single stack.
         *
         * @return Returns NULL if a Stack is not found.
         */
	static Stack* getStack(Vector<int> pos);

        /** Return a pointer to the Stacktile object at the given position on the map.
         *
         * @param pos The position on the map to return the Stacktile for.
         *
         * \note Every tile has a Stacktile object on it, even if it does not
         * contain any stacks.
         * \note This method is one way to return all of the stacks on a tile.
         *
         * @return A pointer to the Stacktile object, or NULL if the position 
         * is out of range.
         */
	static StackTile* getStacks(Vector<int> pos);

        /** Merge all the stacks at the given position on the map into a single Stack.
         *
         * @param pos The position on the map to merge Stack objects on.
         *
         * \note Only stacks belonging to the Playerlist::getActiveplayer() are
         * merged.
         *
         * @return When friendly stacks are found at the given position, this 
         * method returns a pointer to the Stack object that holds all of the
         * army units.  Returns NULL if no stacks are present at the given 
         * position, or if the position is out of range.
         */
	static Stack *groupStacks(Vector<int> pos);

        /** Merge all the stacks at the position of the given Stack into a single Stack.
         *
         * @param s A pointer to the Stack whose position will be used to 
         * merge Stack objects on.
         *
         * \note Only stacks belonging to the Playerlist::getActiveplayer() are
         * merged.
         * @return When friendly stacks are found at the given position, this 
         * method returns a pointer to the Stack object that holds all of the
         * army units.  Returns NULL if no stacks are present at the given 
         * position.
         */
	static void groupStacks(Stack *s);

        /** Merge the stacks owned by Player at the given position into a single Stack.
         * @param pos The position on the map to merge Stack objects on.
         * @param player Merge the stacks belonging to this Player.
         *
         * @return When stacks belonging to the Player are found at the given 
         * position, this method returns a pointer to the Stack object that 
         * holds all of the army units.  Returns NULL if no stacks belonging 
         * to Player are present at the given position, or if the position is
         * out of range.
         */
        Stack *groupStacks(Vector<int> pos, Player *player);

        /** Check whether a Stack can join another one.
         *
         * @param src  The source Stack object.
         * @param dest The destination Stack object that is checked to see if 
         * the source Stack can join.
         *
         *  \note Only Stack objects belonging to the owner of the src Stack
         *  are considered.
         *
         *  @return Returns True if the number of Army units in the new Stack
         *  would not exceed MAX_STACK_SIZE.   Returns False if the number of 
         *  Army units in the new Stack would exceed MAX_STACK_SIZE.
         */
	static bool canJoin(const Stack *src, Stack *dest);

        /** Check whether a Stack can join stacks at the given position.
         *
         * @param stack  The Stack object to see if we can join elsewhere.
         * @param pos  The position on the map.
         *
         *  \note Only Stack objects belonging to the owner of the src Stack 
         *  are considered.
         *
         *  @return Returns True if there are no stacks at the given position, 
         *  or if the number of Army units in the new Stack would not 
         *  exceed MAX_STACK_SIZE.  Returns False if the number of Army units 
         *  in the new Stack would exceed MAX_STACK_SIZE.
         */
	static bool canJoin(const Stack *stack, Vector<int> pos);
        
        /** Check to see if an Army can be added to the given position.
         *
         * @param pos  The position on the map.
         *
         * @return Returns True if the number of Army units at the given 
         * position is less than MAX_ARMIES_ON_A_SINGLE_TILE.  Otherwise False 
         * is returned.
         */
	static bool canAddArmy(Vector<int> pos);

        /** Check to see if Armies can be added to the given position.
         *
         * @param dest  The position on the map to check if armies can be added at.
         * @param stackSize The number of Army units to check if can be added.
         *
         * @return Returns True if the number of Army units at the given 
         * position is less than MAX_ARMIES_ON_A_SINGLE_TILE.  Otherwise False
         * is returned.
         */
	static bool canAddArmies(Vector<int> dest, guint32 stackSize);

        /** Returns a pointer to a Stack belonging to the active player at the given position.
         * 
         * @param pos The position on the map to check for a friendly Stack at.
         *
         * \note Friendly Stack objects are defined as stacks owned by the 
         * Playerlist::getActiveplayer().
         * \note More than one stack can sit on a tile.  This method just 
         * returns a pointer to a single stack.
         *
         * @return Returns NULL if a Stack is not found.
         */
	static Stack* getFriendlyStack(Vector<int> pos);

        /** Returns all of the stacks at the given position belonging to the given Player.
         * 
         * @param pos The position on the map to check for Stack objects at.
         * @param player Stacks owned by this Player will be returned.
         *
         * @return Returns an empty list if no stacks are found, otherwise a 
         * vector of pointers to Stack objects is returned.
         */
	static std::vector<Stack*> getFriendlyStacks(Vector<int> pos, Player *player = NULL);

        /** Return a pointer to a Stack at the given position not belonging to the active player.
         *
         * @param pos The position on the map to check for a Stack at.
         *
         * \note Enemy Stack objects are defined as stacks not owned by the 
         * Playerlist::getActiveplayer().
         *
         * @return Returns NULL if a Stack is not found.
         */
	static Stack* getEnemyStack(Vector<int> pos);

        /** Returns all of the stacks at the given position not belonging to the given Player.
         * 
         * @param pos The position on the map to check for Stack objects at.
         * @param player Stacks not owned by this Player will be returned.
         *
         * @return Returns an empty list if no stacks are found, otherwise a 
         * vector of pointers to Stack objects is returned.
         */
	static std::vector<Stack*> getEnemyStacks(Vector<int> pos, Player *player = NULL);

        /** Returns all stacks not belonging to the active player at the given positions.
         *
         * @param posns A list of positions on the map to check for enemy stacks on.
         * \note Enemy Stack objects are defined as stacks not owned by the 
         * Playerlist::getActiveplayer().
         * @return Returns an empty list if no stacks are found, otherwise a 
         * list of pointers to Stack objects is returned.
         */
	static std::vector<Stack*> getEnemyStacks(std::list<Vector<int> > posns);

        /** Returns all stacks belonging to the active player that are within a given distance from a given position.
         *
         * @param pos The center of the box to find friendly Stack objects in.
         * @param dist Any tile this far from pos is deemed to be in the box.
         *
         * \note When dist is 1, the size of the box checked is 3x3.
         * \note Friendly Stack objects are defined as stacks owned by the 
         * Playerlist::getActiveplayer().
         * \note The returned list of Stack objects is sorted by their distance
         * away from pos.
         * @return Returns an empty list if no stacks are found, otherwise a 
         * vector of pointers to Stack objects is returned.
         */
	static std::vector<Stack*> getNearbyFriendlyStacks(Vector<int> pos, int dist);

        /** Returns all stacks not belonging to the active player that are within a given distance from a given position.
         *
         * @param pos The center of the box to find enemy Stack objects in.
         * @param dist Any tile this far from pos is deemed to be in the box.
         *
         * \note When dist is 1, the size of the box checked is 3x3.
         * \note Enemy Stack objects are defined as stacks not owned by the 
         * Playerlist::getActiveplayer().
         * \note The returned list of Stack objects is sorted by their distance
         * away from pos.
         *
         * @return Returns an empty list if no stacks are found, otherwise a 
         * list of pointers to Stack objects is returned.
         */
	static std::vector<Stack*> getNearbyEnemyStacks(Vector<int> pos, int dist);

        /** Returns a list of positions that are within a given distance from a given position.
         *
         * @param pos The center of the box to return positions for.
         * @param dist Any tile this far from pos is deemed to be in the box.
         *
         * \note When dist is 1, the size of the box is 3x3.
         * \note The returned list of positions is sorted by their distance away from pos (the center).
         *
         * @return Returns a vector of valid map positions.
         */
        static std::list<Vector<int> > getNearbyPoints(Vector<int> pos, int dist);

        //! Gather up the points on the map specified by the list of rectangles
        std::list<Vector<int> > getPoints (std::list<LwRectangle> rects);

        /** Returns the number of Army units at a given position on the map.
         *
         * @param pos The position on the map to count Army units for.
         *
         * \note Only Army units in stacks belonging to the 
         * Playerlist::getActiveplayer are counted.
         *
         * @return Returns the number of army units, or zero if the position is
         * out of range.
         */
	static guint32 countArmyUnits(Vector<int> pos);

        /** Returns a pointer to the Backpack object at the given position.
         *
         * @param pos The position on the map to get the Backpack object for.
         *
         * \note There is a Backpack object on every tile.  However it is 
         * usually empty of Item objects.
         *
         * @return Returns NULL if pos is out of range.  Otherwise a pointer 
         * to a MapBackpack object is returned.
         */
	static MapBackpack *getBackpack(Vector<int> pos);

        //! Return how many bags of stuff there are on the map.
        static guint32 countBags ();

        /** Check if the given Stack is able to search the Maptile it is on.
         *
         * @param stack A pointer to the stack to check if it can search.
         *
         * \note This involves checking if the Stack is on a Temple, or if a
         * Hero is on a Ruin.
         *
         * @return Returns True if the stack can search, otherwise False is 
         * returned.
         */
        static bool can_search(Stack *stack);

        /** Check if the Stack can plant a standard on the Maptile it is on.
         *
         * @param stack A pointer to the stack to check if it can plant a 
         * standard.
         *
         * \note This involves checking if the Stack contains a Hero, who has 
         * the standard Item, and isn't on Maptile that has a building.
         *
         * \note Standards are special items that can be vectored to, when 
         * planted in the ground.
         *
         * @return Returns True if the stack can plant the standard.  Otherwise 
         * false is returned.
         */
        static bool can_plant_flag(Stack *stack);

        /** Check if the Stack can go into defend mode.
         *
         * @param stack A pointer to the stack to check if it can go into 
         * defend mode.
         *
         * \note This involves checking if the Stack is on a suitable tile,
         * namely, not on water, or in a building.
         * \note If the stack is already in defend mode, this method will 
         * still return True.
         *
         * Returns True if the stack can go into defend mode.  Otherwise false 
         * is returned.
         */
        static bool can_defend(Stack *stack);

        /** Return a list of all MapBackpack objects that contain Item objects.
         *
         * \note This refers to the "bags of stuff" that can appear on the map.
         *
         * \note Planted Standards are a special case of a MapBackpack object.
         *
         * @return A list of pointers to MapBackpack objects that contain one 
         * or more items.
         */
        std::list<MapBackpack*> getBackpacks() const;

        /** Return a pointer to the Maptile object at position (x,y).
         *
         * @param x The horizontal index of the map.
         * @param y The vertical index of the map.
         *
         * \note Every square of the map contains a MapTile object.
         *
         * @return Returns NULL when the given position is out of range.  
         * Otherwise a pointer to a Maptile object is returned.
         */
        inline Maptile* getTile(int x, int y) const { return &d_map[y*s_width + x]; };

        /** Return a pointer to the Maptile object at the given position.
         *
         * @param pos The position on the map to get a Maptile object for.
         *
         * \note Every square of the map contains a MapTile object.
         *
         * @return Returns NULL when the given position is out of range.  
         * Otherwise a pointer to a Maptile object is returned.
         */
        inline Maptile* getTile(Vector<int> p) const {return getTile(p.x, p.y);}

        /** Try to insert an Army in this Location on the map.
         *
         * @param l A pointer to the Location to insert the Army into.
         * @param a A pointer to the Army object to insert.
         *
         * \note The tiles of the Location are checked-for in a left-to-right,
         * top-to-bottom fashion.
         *
         * @return Returns a pointer to the Stack that the Army is added to.  
         * Otherwise, if the Location is too full too accept another Army, 
         * NULL is returned.
         */
	Stack* addArmy(Location *l, Army *a);

        /** Try to insert an Army on or near the given position on the map.
         *
         * @param pos The position on the map to insert an Army at.
         * @param a A pointer to the Army object to insert.
         *
         * \note If the given position cannot accept another Army, the next 
         * closest position is tried.  It must be on land if pos is on land, 
         * and water if pos is on water.
         *
         * @return Returns a pointer to the Stack that the Army is added to.  
         * Otherwise, if there is no suitable place for the Army unit, NULL 
         * is returned.
         */
        Stack* addArmyAtPos(Vector<int> pos, Army *a);

        /** Try to insert an Army on or near a building at the given position on map.
         *
         * @param pos The position on the map to insert an Army at.
         * @param a A pointer to the Army object to insert.
         *
         * \note If the Army cannot be inserted into a building at the given 
         * position, addArmyAtPos is called as a last resort.
         *
         * @return Returns a pointer to the Stack that the Army is added to.  
         * Otherwise, if there is no suitable place for the Army unit, NULL 
         * is returned.
         */
        Stack* addArmy(Vector<int> pos, Army *a);

        /** Insert a given number of Army units at a given position.
         *
         * @param a  A pointer to the ArmyProto object describing the kind of
         * Army unit to insert.
         * @param num_allies The number of army units to try to insert.
         * @param pos The position on the map to add the Army units to.
         *
         * \note Not all of the Army units are guaranteed to be inserted.
         */
        void addArmies(const ArmyProto *a, guint32 num_allies, Vector<int> pos);

        /** Returns the position of the planted standard owned by the given Player.
         *
         * @param p A pointer to the Player object to find the planted standard
         * for.
         *
         * @return Returns the position of the planted standard on the map, or 
         * otherwise a position of -1,-1 is returned if the standard is not 
         * planted anywhere.
         */
        Vector<int> findPlantedStandard(Player *p);

        /** Fill the map using the data supplied by a map generator.
          * 
          * @param generator A pointer to the MapGenrator which supplies the 
          * terrain data
          *
          * \note This method assigns new Maptile objects for every square of
          * the map.
          *
          * @return Returns True on success, False on error.
          */
        bool fill(MapGenerator* generator);

        /** Fills the whole map with a single terrain.
          *
          * @param type The type of the terrain to fill the map with.  This is
          * an index in the tileset.
          *
          * \note This method assigns new Maptile objects for every square of
          * the map.
          *
          * @return Returns True on success, False on error.
          */
        bool fill(guint32 type);

        /** Save the contents of the map.
          * 
          * @param helper A pointer to an XML file opened for writing.
          *
          * @return Returns True if saving went well, False otherwise.
          */
        bool save(XML_Helper* helper) const;

        /** Calculate what tiles can be traveled to by a non-flying Stack.
         *
         * For each tile on the map, check to see where land and water meet,
         * along with cities, ports and bridges.  This method updates the 
         * internal state of which tile can be reached from an adjacent tile.
         */
        void calculateBlockedAvenues();

        /** Calculate what tiles can be traveled to by a non-flying Stack for the given position.
         *
         * This method updates the internal state of which of the adjacent 
         * tiles can be traveled to.  For example, the way is not blocked when 
         * a stack is on a bridge and the adjacent tile is water.
         */
	void calculateBlockedAvenue(int i, int j);

        /** Load the Stack objects from Stacklist objects into StackTile objects.
         * Loop over all players and all of their stacks, adding the stacks to
         * the state of the StackTile objects associated with every square of
         * map.
         */
	void updateStackPositions();

        /** Forget the state of all of the StackTile objects.
         */
        void clearStackPositions();

	/** Smooth a portion of the terrain on the big map. 
	 *
         * @param r The area of the map to modify.
         * @param smooth_terrain Whether or not we want to demote lone tiles 
         * to be similar to the terrain of nearby tiles.
         *
	 * Give each tile in the prescribed area the preferred picture for 
	 * the underlying terrain tile.
         *
         * \note This method changes the TileStyle associated with a tile's 
         * Maptile object.
	 */
	void applyTileStyles (LwRectangle r, bool smooth_terrain);

	/** Smooth a portion of the terrain on the big map. 
	 *
         * @param minx The top left horizontal coordinate.
         * @param miny The top left vertical coordinate.
         * @param maxx The bottom right horizontal coordinate.
         * @param maxy  The bototm right vertical coordinate.
         * @param smooth_terrain Whether or not we want to demote lone tiles 
         * to be similar to the terrain of nearby tiles.
         *
	 * Give each tile in the prescribed area the preferred picture for 
	 * the underlying terrain tile.
         *
         * \note This method changes the TileStyle associated with a tile's 
         * Maptile object.
	 */
	void applyTileStyles (int minx, int miny, int maxx, int maxy,
			      bool smooth_terrain);

	/** Change how the terrain looks at given position on the big map.
	 *
         * @param i The horizontal index of the map.
         * @param j The vertical index of the map.
         *
         * This method determines what the appropriate TileStyle is for the 
         * given position on the map.  It's easy to figure out for regular old
         * grass tiles, but it's more difficult for forest tiles.
         *
         * \note This method changes the TileStyle associated with a tile's 
         * Maptile object.
	 */
	void applyTileStyle (int i, int j);

        /** Make the mountains look right by making them transition to hills.
         * Mountains are treated differently than every other terrain types 
         * because unlike all the other terrain types, they transition to 
         * hills, not grass.  This method ensures that mountain tiles 
         * transition to hill tiles.
         * \note This method changes the TileStyle associated with a tile's 
         * Maptile object.
         */
	void surroundMountains(int minx, int miny, int maxx, int maxy);

	/** Returns the positions of all of the items on the game map (in bags).
         *
         * \note This does not include the position of items that heroes are 
         * carrying around.  It only includes dropped items, including planted
         * standards.
         *
         * @return Returns a vector of positions of bags on the map.  An empty 
         * list is returned when there are no dropped items on the map.
         */
	std::vector<Vector<int> > getItems();

        /** Find the closest road, bridge, city, ruin, or temple to the north of the given position.
         *
         * @pos The position to search for an object from.
         *
         * @return Returns the position of the closest object, or if there are 
         * none, -1,-1 is returned.
         */
	Vector<int> findNearestObjectToTheNorth(Vector<int> pos);

        /** Find the closest road, bridge, city, ruin, or temple to the south of the given position.
         *
         * @pos The position to search for an object from.
         *
         * @return Returns the position of the closest object, or if there are 
         * none, -1,-1 is returned.
         */
	Vector<int> findNearestObjectToTheSouth(Vector<int> pos);

        /** Find the closest road, bridge, city, ruin, or temple to the east of the given position.
         *
         * @pos The position to search for an object from.
         *
         * @return Returns the position of the closest object, or if there are 
         * none, -1,-1 is returned.
         */
	Vector<int> findNearestObjectToTheEast(Vector<int> pos);

        /** Find the closest road, bridge, city, ruin, or temple to the west of the given position.
         *
         * @pos The position to search for an object from.
         *
         * @return Returns the position of the closest object, or if there are 
         * none, -1,-1 is returned.
         */
	Vector<int> findNearestObjectToTheWest(Vector<int> pos);

        /** Change the current Armyset to a different one.
         *
         * @param armyset A pointer to the Armyset to change to.
         *
         * Loops through all of the players and changes their armyset to given
         * one.  All Army units are updated, including ones in ruins, and the
         * city production slots.  If the new armyset doesn't have an army 
         * type, army units of that type are erased.
         *
         */
	void switchArmysets(Armyset *armyset);
        void switchArmysets(Player *p, Armyset *armyset);
        
        /** Change the current Cityset to a different one.
         *
         * @param cityset A pointer to the Cityset to change to.
         *
         * Loops through all of the cities, temples and ruins to change the
         * way they look.  These buildings may be moved to different tiles on 
         * the map because the number of tiles a city takes up, e.g. 2x2 can 
         * be changed to a bigger or smaller footprint.
         */
	void switchCityset(Cityset *cityset);

        /** Change the current Shieldset to a different one.
         *
         * @param shieldset A pointer to the Shieldset to change to.
         *
         * Loops through all Shields and change the way they look.  This
         * changes the color of army units, flags, shields, and selector, as
         * well as the graphics of the shields.
         */
	void switchShieldset(Shieldset *shieldset);

        /** Change the current Tileset to a different one.
         *
         * @param tileset A pointer to the Tileset to change to.
         *
         * Change the way the terrain looks on the big map and the small map.
         * Also change the way the flags look, the explosion, the roads, the 
         * fog, and the army unit selector animation.
         * 
         * \note This method changes the TileStyle objects associated with 
         * every tile's associated Maptile object.
         */
	void switchTileset(Tileset *tileset);

        /** Load the current Shieldset again.
         * Throw away the state of the current Shieldset, and load it up from
         * the shieldset file.  Includes loading images.
         */
        void reloadShieldset();

        /** Load the current Cityset again.
         * Throw away the state of the current Cityset, and load it up from
         * the cityset file.  Includes loading images.
         */
        void reloadCityset();

        /** Move a building from one place to another on the map.
         *
         * @param from The source position of a building.
         * @param to The destination position for the building.
         * @param new_width How many tiles the building should occupy.
         *
         * Move a city, ruin, temple, port, signpost, road tile or bridge tile
         * on the map.
         *
         * \note The whole road or bridge does not get moved.  Just a single 
         * tile.  Whole cities, ruins, temples, etc get moved.
         *
         * \note The new_width is also the height in tiles.  0 means ignore.
         *
         * @return Returns True if successful.  Otherwise False.
         */
	bool moveBuilding(Vector<int> from, Vector<int> to, guint32 new_width = 0);
        /** Check if we can put a given building type, of a given size at a given location.
         * @param bldg  The type of building to check if we can put down.
         * @param size  The width in tiles of the building.  e.g. 2 means 2x2.
         * @param to  The position on the map to check if we can put a building on.
         * @param making_islands  Whether or not to create grass underneath a 
         * city, ruin, temple, orsignpost.  False means to create grass.
         *
         * @return Returns True if the desired building can be put down.  
         * Otherwise False.
         */
	bool canPutBuilding(Maptile::Building bldg, guint32 size, Vector<int> to, bool making_islands = true);


        /** Check if we can put some items here.
         *
         * @param pos  where on the map we're dropping the bag
         *
         * @return Returns True if we can drop it, otherwise false.
         */
        bool canDropBag (Vector<int> pos);

        /** Check if a stack of the given size, owned by the given player, can be added to the given position.
         * @param size The number of army units to check if we can add.
         * @param p Only stacks owned by this player are considered.
         * @param to The position on the map to check if we can add them at.
         * @return Returns True if the desired stack can be put down, otherwise
         * False.
         */
	bool canPutStack(guint32 size, Player *p, Vector<int> to);

        /** Move a given stack to a given position on the map.
         *
         * @param stack A pointer to the stack to be moved.
         * @param to The destination position to move it to.
         *
         * \note This method is used to move a stack far distances, without
         * losing any movement points.
         * \note When moving a stack into an enemy city, it changes ownership 
         * of the stack to be the same owner as the city.
         *
         * @return Returns True if the stack is moved successfully, otherwise 
         * False.
         */
	bool moveStack(Stack *stack, Vector<int> to);

        /** Move a bag of stuff from one position on the map, to another.
         *
         * @param bag The bag of stuff we're moving.
         * @param to The destination position.
         *
         * \note Every square has a Backpack object, but only some of them
         * contain Item objects.
         *
         * If there are items in the Backpack located at the source position, 
         * they are removed and added to the destination position.
         */
	void moveBackpack(MapBackpack *bag, Vector<int> to);

        /** Returns the size of the building at the given position on the map.
         *
         * @param tile  The position on the map of a building to get the size for.
         * 
         * This method gets the width in tiles of any of the building objects 
         * specified by Maptile::Building.  e.g. a size of 2 means 2x2 tiles.
         *
         * @return Returns the size of the building in tiles, or zero if there
         * isn't a building at that position.
         */
	guint32 getBuildingSize(Vector<int> tile);

        /** Get the building type at a given position on the map.
         *
         * @param tile The position on the map to get a building type of.
         *
         * @return Returns a building type if one is present at the given 
         * position, or Maptile::NONE if one is not present, or if the given
         * position is out of range.
         */
	inline Maptile::Building getBuilding(Vector<int> tile) const
          {
            Maptile *t = getTile(tile);
            return t ? t->getBuilding() : Maptile::NONE;
          }

        /** Count the number of tiles occupied by buildings of a given type.
         *
         * @param building_type A Maptile::Building representing the kind of 
         * constructed entities that are to be counted.
         *
         * @return Returns the number of tiles occupied by building_type;
         */
        guint32 countBuildings(Maptile::Building building_type);

        /** Return the kind of terrain at a given position on the map.
         *
         * @param tile A position on the map to get the terrain kind for.
         *
         * @return Returns a Tile::Type for the given position.  Tile::NONE is
         * returned if the given position is out of bounds.
         */
	inline Tile::Type getTerrainType(Vector<int> tile) const
          {
            Maptile *t = getTile(tile);
            return t ? t->getType() : Tile::NONE;
          }

        /** Change the building type of a tile.
         *
         * @param tile A position on the map to change the building type of.
         *
         * \note This is merely changing a lookup flag of what building is 
         * present on this tile.  It isn't making a new City or Ruin.
         */
	void setBuilding(Vector<int> tile, Maptile::Building building);

        /** Drop a new City at the given position on the map.
         *
         * @param tile The position to create a new City at.
         *
         * \note The terrain under the City is changed to grass.
         * \note The new City is owned by Playerlist::getActiveplayer().
         *
         * @return Returns True if a City was created.  Otherwise False if a
         * City is not placed at the given position.
         */
        bool putNewCity(Vector<int> tile);

        /** Put the given City on the map.
         *
         * @param c A pointer to the City to add to the map.
         * @param keep_owner Whether or not to reassign ownership to 
         * Playerlist::getActiveplayer().
         *
         * \note The position of the City is held in the City object.
         *
         * \note This method doesn't do any checking if the City can be 
         * added to the given position or not.  Callers are expected to do
         * this check beforehand.
         *
         * @return Always returns True.
         */
	bool putCity(City *c, bool keep_owner = false);

        /** Erase a City from the given position from the map.
         *
         * @param pos The position on the map to erase a City from.
         *
         * @return Returns True if a City was erased.  Otherwise, False.
         */
	bool removeCity(Vector<int> pos);

        /** Add a given Ruin to the map.
         *
         * @param r A pointer to the Ruin to add to the map.
         *
         * \note If the terrain under the Ruin is water, it is changed to grass.
         *
         * \note The position of the Ruin is held in the Ruin object.
         *
         * \note This method doesn't do any checking if the Ruin can be 
         * added to the given position or not.  Callers are expected to do
         * this check beforehand.
         *
         * @return Always returns True.
         */
	bool putRuin(Ruin *r);

        /** Drop a new Ruin at the given position on the map.
         *
         * @param tile The position to create a new Ruin at.
         *
         * @return Returns True if a Ruin was created.  Otherwise False if a
         * Ruin is not placed at the given position.
         */
        bool putNewRuin(Vector<int> tile);
        
        /** Erase a ruin from the given position from the map.
         *
         * @param pos The position on the map to erase a Ruin from.
         *
         * @return Returns True if a Ruin was erased.  Otherwise, False.
         */
	bool removeRuin(Vector<int> pos);

        /** Add a given Temple to the map.
         *
         * @param t A pointer to the Temple to add to the map.
         *
         * \note If the terrain under the Temple is water, it is changed to 
         * grass.
         *
         * \note The position of the Temple is held in the Temple object.
         *
         * \note This method doesn't do any checking if the Temple can be 
         * added to the given position or not.  Callers are expected to do
         * this check beforehand.
         *
         * @return Always returns True.
         */
	bool putTemple(Temple *t);

        /** Drop a new Temple at the given position on the map.
         *
         * @param tile The position on the map to create a new Temple at.
         *
         * @return Returns True if a Temple was created.  Otherwise False if a
         * Temple is not placed at the given position.
         */
        bool putNewTemple(Vector<int> tile);

        /** Erase a Temple from the given position from the map.
         *
         * @param pos The position on the map to erase a Temple from.
         *
         * @return Returns True if a Temple was erased.  Otherwise, False.
         */
	bool removeTemple(Vector<int> pos);

	static Stone* getStone(Vector<int> pos);
	bool putStone(Stone *t);
	bool removeStone(Vector<int> pos);

        /** Add a given Road tile to the map.
         *
         * @param t A pointer to the Road tile to add to the map.
         *
         * @param smooth whether or not surrounding road types should be
         * changed to align.
         *
         * The road type is calculated and changed according to the other road
         * and bridge tiles nearby.
         *
         * \note If the terrain under the Road is water, it is changed to 
         * grass.
         *
         * \note The position of the Road is held in the Road object.
         *
         * \note This method doesn't do any checking if the Road can be 
         * added to the given position or not.  Callers are expected to do
         * this check beforehand.
         *
         * @return Always returns True.
         */
	bool putRoad(Road *r, bool smooth=true);

        /** Drop a new Road tile at the given position on the map.
         *
         * @param tile The position on the map to create a new Road at.
         *
         * The road type is calculated and changed according to the other road
         * and bridge tiles nearby.
         *
         * @return Returns True if a Road was created.  Otherwise False if a
         * Road is not placed at the given position.
         */
        bool putNewRoad(Vector<int> tile);

        /** Erase a Road tile from the given position from the map.
         *
         * @param pos The position on the map to erase a Road from.
         *
         * \note The type of other nearby Road tiles are not modified as a 
         * result of erasing this Road.
         *
         * @return Returns True if a Road was erased.  Otherwise, False.
         */
	bool removeRoad(Vector<int> pos);

        /** Add a given Bridge tile to the map.
         *
         * @param t A pointer to the Bridge tile to add to the map.
         *
         * The bridge type is calculated and changed according to the other 
         * road and bridge tiles nearby.
         *
         * \note If the terrain under the Bridge is water, it is changed to 
         * grass.
         *
         * \note The position of the Bridge is held in the Bridge object.
         *
         * \note This method doesn't do any checking if the Bridge can be 
         * added to the given position or not.  Callers are expected to do
         * this check beforehand.
         *
         * @return Always returns True.
         */
	bool putBridge(Bridge *b);

        /** Erase a Bridge tile from the given position from the map.
         *
         * @param pos The position on the map to erase a Bridge from.
         *
         * @return Returns True if a Bridge was erased.  Otherwise, False.
         */
	bool removeBridge(Vector<int> pos);

        /** Destroy both halves of a bridge.
         *
         * @param pos The position on the map to erase a Bridge from.
         *
         * Both halves of the bridge are removed, and the type of the 
         * connecting Road tiles are recalculated. 
         *
         * @return Returns True if a Bridge was burned.  Otherwise, False.
         */
        bool burnBridge(Vector<int> pos);

        /** Add a given Sign to the map.
         *
         * @param t A pointer to the Signpost to add to the map.
         *
         * \note If the terrain under the Sign is water, it is changed to 
         * grass.
         *
         * \note The position of the Signpost is held in the Signpost object.
         *
         * \note This method doesn't do any checking if the Signpost can be 
         * added to the given position or not.  Callers are expected to do
         * this check beforehand.
         *
         * @return Always returns True.
         */
	bool putSignpost(Signpost *s);

        /** Erase a Signpost tile from the given position from the map.
         *
         * @param pos The position on the map to erase a Signpost from.
         *
         * @return Returns True if a Signpost was erased.  Otherwise, False.
         */
	bool removeSignpost(Vector<int> pos);

        /** Add a given Port to the map.
         *
         * @param t A pointer to the Port to add to the map.
         *
         * \note If the terrain under the Port is water, it is changed to 
         * grass.
         *
         * \note The position of the Port is held in the Port object.
         *
         * \note This method doesn't do any checking if the Port can be 
         * added to the given position or not.  Callers are expected to do
         * this check beforehand.
         *
         * @return Always returns True.
         */
	bool putPort(Port *p);

        /** Erase a Port tile from the given position from the map.
         *
         * @param pos The position on the map to erase a Port from.
         *
         * @return Returns True if a Port was erased.  Otherwise, False.
         */
	bool removePort(Vector<int> pos);

        /** Add the given stack to the map.
         *
         * @param s     A pointer to the stack to add to the map.
         * @param owner if true, is added to the stacklist of the owner of the
         *              stack
         *
         * \note This method doesn't do any checking if the stack can be 
         * added to the given position or not.  Callers are expected to do
         * this check beforehand.
         *
         * \note When owner is false the Stack is added to the Stacklist of the 
         * Playerlist::getActiveplayer().
         * @return Always returns True.
         */
	bool putStack(Stack *s, bool owner = false);

        /** Remove the given stack from the map.
         *
         * @param s A pointer to the stack to be removed from the map.
         *
         */
	void removeStack(Stack *s);

        /** Erase a building at the given location from the map.
         *
         * @param pos A position on the map to remove a building from.
         *
         * This method erases a City, Road, Ruin, Temple, Port, Bridge, or 
         * a Signpost from the map.
         *
         * @return Returns True if a building was removed.  Otherwise, False.
         */
        bool removeLocation (Vector<int> pos);

        /** Erases everything except the terrain from the given position on the map.
         *
         * @param pos A position on the map to remove stuff from.
         *
         * This method erases a City, Road, Ruin, Temple, Port, Bridge, 
         * Signpost, Stack or Backpack from the given position on the map.
         *
         * \note Every tile has a MapBackpack object.  It is not removed, but
         * the Item objects held within it are.
         *
         * @return Returns True if anything was removed.  Otherwise, False.
         */
        bool eraseTile(Vector<int> pos);

        /** Erase everything from a region of the map.
         *
         * @param r A Rectangle specifying the region to remove stuff from.
         *
         * This method erases any City, Road, Ruin, Temple, Port, Bridge, 
         * Signpost, Stack or Backpack objects from the given region of the map.
         *
         * \note Every tile has a MapBackpack object.  It is not removed, but
         * the Item objects held within it are.
         *
         * @return Returns True if anything was removed.  Otherwise, False.
         */
        bool eraseTiles(LwRectangle r);

        /** Returns a Location from the given position on the map.
         *
         * @param pos The position on the map to check a building for.
         *
         * All buildings are also Location objects.
         *
         * @return Returns a pointer to a Location if a building a present, 
         * otherwise NULL is returned.
         */
	Location *getLocation(Vector<int> pos);

        LwRectangle getBoundingBox (Vector<int> pos);

        /** Check if all City objects are reachable.
         *
         * Loop over all cities to see if they can all be reached via a 
         * non-flying Stack.
         *
         * @return Returns True if all cities are accessible.  Otherwise, False.
         */
	bool checkCityAccessibility();

        /** Check if buildings are on land or water.
         *
         * @param b The kind of building to check for.
         * @param land Whether or not the building should be on land or not.
         *
         * @return Returns True if the building was found to be on land, or
         * water, anywhere on the map.  Otherwise, False.
         */
        static bool checkBuildingTerrain(Maptile::Building b, bool land);

        /** Returns the center of the map.
         *
         * @return Returns the centermost tile of the map.
         */
        static Vector<int> getCenterOfMap();

        /** Change the terrain on a region of the map.
         *
         * @param r The region of the map to alter.
         * @param type  The type of terrain to change it to.
         * @param tile_style_id  The TileStyle id to change it to.  -1 means 
         * automatically pick the id of a suitable TileStyle.
         * @param always_alter_tilestyles Reassign TileStyle ids even if the
         * terrain is already of the given type.
         *
         * \note This method will not change land to water under buildings.
         * \note This method will change Stack objects on land to be Stack 
         * objects in water (e.g. in a boat).
         *
         * @return Returns the region altered as a Rectangle.
         */
        LwRectangle putTerrain(LwRectangle r, Tile::Type type, 
                             int tile_style_id = -1, 
                             bool always_alter_tilestyles = false);


        std::list<UniquelyIdentified*> copyObjects(std::list<LwRectangle> rects);
        std::list<Maptile*> copyMaptiles (std::list<LwRectangle> rects);

        void updateMaptiles (std::list<Maptile *> maptiles);
        void updateObjects (std::list<UniquelyIdentified*> objects,
                            std::list<LwRectangle> rects);

        static int calculateTilesPerOverviewMapTile(int width, int height);
        static int calculateTilesPerOverviewMapTile();
        //! Replace the current GameMap with another.
        static void reset (GameMap *m);

        Vector<int> findNearestAreaForBuilding(Maptile::Building building_type, Vector<int> pos, guint32 width);

        static bool friendlyCitiesPresent();
        static bool enemyCitiesPresent();
        static bool neutralCitiesPresent();
        static Stack* getStrongestStack(Vector<int> pos);

        static void resetCityset ();
        static void resetTileset ();
        static void resetShieldset ();
        //! Destructor
        ~GameMap();
        
    protected:
        //! Create the map with the given tileset
        GameMap(Glib::ustring TilesetName = "", Glib::ustring ShieldsetName = "",
		Glib::ustring Citysetname = "");

        //! Copy constructor
        GameMap (const GameMap &m);

        //! Load the map using the given XML_Helper
        GameMap(XML_Helper* helper);

    private:
        //! Callback for item loading used during loading.
        bool loadItems(Glib::ustring tag, XML_Helper* helper);
        bool containsWater (LwRectangle rect);
        bool containsForest (LwRectangle rect);
        bool isBlockedAvenue(bool mountains, int x, int y, int destx, int desty);
        bool isDock(Vector<int> pos);
	void close_circles (int minx, int miny, int maxx, int maxy);
	void processStyles(Glib::ustring styles, int chars_per_style);
	int determineCharsPerStyle(Glib::ustring styles);

	TileStyle *calculatePreferredStyle(int i, int j);
	void demote_lone_tile(int minx, int miny, int maxx, int maxy, 
			      Tile::Type intype, Tile::Type outtype);

	int tile_is_connected_to_other_like_tiles (Tile::Type tile, int i, 
						   int j);
	bool are_those_tiles_similar(Tile::Type outer_tile,Tile::Type inner_tile, bool checking_loneliness);
	Vector<int> findNearestObjectInDir(Vector<int> pos, Vector<int> dir);
	void putBuilding(LocationBox *b, Maptile::Building building);
        void clearBuilding(Vector<int> pos, guint32 width);
	void removeBuilding(LocationBox *b);

	void updateShips(Vector<int> pos);
        void updateTowers (Vector<int> pos);

        static void changeFootprintToSmallerCityset(Location *location, Maptile::Building building_type, guint32 old_tile_width);

        static void relocateLocation(Location *location, Maptile::Building building_type, guint32 tile_width);

	static std::vector<Stack*> getNearbyStacks(Vector<int> pos, int dist, bool friendly);

	static bool offmap(int x, int y);

        static bool compareStackStrength(Stack *lhs, Stack *rhs);

        // Data
        static GameMap* s_instance;
        static int s_width;
        static int s_height;
        static Tileset* s_tileset; //not saved
        static Cityset* s_cityset; //not saved
        static Shieldset* s_shieldset; //not saved

        Glib::ustring d_tileset; //the basename, not the friendly name.
        Glib::ustring d_shieldset; //the basename, not the friendly name.
        Glib::ustring d_cityset; //the basename, not the friendly name.

        Maptile* d_map;
};

#endif

// End of file
