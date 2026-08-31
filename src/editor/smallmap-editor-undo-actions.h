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
#ifndef SMALLMAP_EDITOR_UNDO_ACTIONS_H
#define SMALLMAP_EDITOR_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "rectangle.h"
#include "map-tile.h"
#include "tile.h"
#include "undo-action.h"

//! A record of an event in the smallmap editor
/** 
 * The purpose of these classes is to implement undo/redo in the miniature
 * map editor.
 */

class SmallMapEditorUndoAction: public UndoAction
{
public:

    //! An Editor Action can be one of the following kinds.
    enum Type
      {
        TERRAIN = 1,
        ERASE = 2,
        CITY = 3,
        RUIN = 4,
        TEMPLE = 5,
        BUILD_ROAD = 6,
        CLEAR_ROAD = 7,
        PLACE_START = 8,
        PLACE_FINISH = 9,
      };

    //! Default constructor.
    SmallMapEditorUndoAction
      (Type type, UndoAction::AggregateType a = UndoAction::AGGREGATE_NONE)
      : UndoAction (a), m_type (type)
        {
        }

    //! Destructor.
    virtual ~SmallMapEditorUndoAction ()
      {
      }

    Type get_type() const
      {
        return m_type;
      }

protected:

    Type m_type;
};

class SmallMapEditorUndoAction_ChangeMap: public SmallMapEditorUndoAction
{
public:
    SmallMapEditorUndoAction_ChangeMap (Type t, LwRectangle r, bool grow,
                                        bool agg, bool only_maptiles)
      : SmallMapEditorUndoAction (t, agg ? AGGREGATE_BLANK : AGGREGATE_NONE),
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

    SmallMapEditorUndoAction_ChangeMap (Type t, LwRectangle r1, LwRectangle r2,
                                        bool grow, bool agg,
                                        bool only_maptiles)
      : SmallMapEditorUndoAction (t, agg ? AGGREGATE_BLANK : AGGREGATE_NONE),
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

    ~SmallMapEditorUndoAction_ChangeMap ()
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

    bool get_only_map_tiles () const
      {
        return m_only_maptiles;
      }
private:
    std::list<LwRectangle> m_rects;
    bool m_only_maptiles;
    std::list<Maptile *> m_maptiles;
    std::list<UniquelyIdentified *> m_objects;
};

class SmallMapEditorUndoAction_Terrain:
    public SmallMapEditorUndoAction_ChangeMap
{
public:
    SmallMapEditorUndoAction_Terrain (int t, LwRectangle r)
      : SmallMapEditorUndoAction_ChangeMap (TERRAIN, r, true, true, true),
      m_tile (t)
  {
  }

    ~SmallMapEditorUndoAction_Terrain ()
      {
      }

    Glib::ustring get_action_name () const
      {
        Tileset *ts = GameMap::getTileset ();
        return (*ts)[m_tile]->getName ();
      }

    int get_tile () const
      {
        return m_tile;
      }
private:
    int m_tile;
};

class SmallMapEditorUndoAction_Erase: public SmallMapEditorUndoAction_ChangeMap
{
public:
    SmallMapEditorUndoAction_Erase (LwRectangle r)
      : SmallMapEditorUndoAction_ChangeMap (ERASE, r, false, true, false)
      {
      }

    ~SmallMapEditorUndoAction_Erase ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Erase";
      }
};

class SmallMapEditorUndoAction_City: public SmallMapEditorUndoAction_ChangeMap
{
public:
    SmallMapEditorUndoAction_City (LwRectangle r)
      : SmallMapEditorUndoAction_ChangeMap (CITY, r, true, false, false)
      {
      }

    ~SmallMapEditorUndoAction_City ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "City";
      }
};

class SmallMapEditorUndoAction_Ruin: public SmallMapEditorUndoAction_ChangeMap
{
public:
    SmallMapEditorUndoAction_Ruin (LwRectangle r)
      : SmallMapEditorUndoAction_ChangeMap (RUIN, r, true, false, false)
      {
      }

    ~SmallMapEditorUndoAction_Ruin ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Ruin";
      }
};

class SmallMapEditorUndoAction_Temple: public SmallMapEditorUndoAction_ChangeMap
{
public:
    SmallMapEditorUndoAction_Temple (LwRectangle r)
      : SmallMapEditorUndoAction_ChangeMap (TEMPLE, r, true, false, false)
      {
      }

    ~SmallMapEditorUndoAction_Temple ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Temple";
      }
};

class SmallMapEditorUndoAction_BuildRoads:
    public SmallMapEditorUndoAction_ChangeMap
{
public:
    SmallMapEditorUndoAction_BuildRoads (LwRectangle r, Vector<int> s,
                                         Vector<int> d)
      : SmallMapEditorUndoAction_ChangeMap (BUILD_ROAD, r, true, false, false),
      m_src (s), m_dst (d)
  {
  }

    ~SmallMapEditorUndoAction_BuildRoads ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "BuildRoads";
      }

    Vector<int> get_src () const
      {
        return m_src;
      }

    Vector<int> get_dest () const
      {
        return m_dst;
      }
private:
    Vector<int> m_src;
    Vector<int> m_dst;
};

class SmallMapEditorUndoAction_ClearRoad: public SmallMapEditorUndoAction
{
public:
    SmallMapEditorUndoAction_ClearRoad (Vector<int> src, Vector<int> dst)
      : SmallMapEditorUndoAction (CLEAR_ROAD), m_src (src), m_dst (dst)
      {
      }

    ~SmallMapEditorUndoAction_ClearRoad ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "ClearRoad";
      }

    Vector<int> get_src () const
      {
        return m_src;
      }

    Vector<int> get_dest () const
      {
        return m_dst;
      }
private:
    Vector<int> m_src;
    Vector<int> m_dst;
};

class SmallMapEditorUndoAction_Pos: public SmallMapEditorUndoAction
{
public:
    SmallMapEditorUndoAction_Pos (Type t, Vector<int> pos)
      : SmallMapEditorUndoAction (t), m_pos (pos)
      {
      }

    ~SmallMapEditorUndoAction_Pos ()
      {
      }

    Vector<int> get_pos () const
      {
        return m_pos;
      }

private:
    Vector<int> m_pos;
};

class SmallMapEditorUndoAction_PlaceStart: public SmallMapEditorUndoAction_Pos
{
public:
    SmallMapEditorUndoAction_PlaceStart (Vector<int> pos)
      : SmallMapEditorUndoAction_Pos (PLACE_START, pos)
      {
      }

    ~SmallMapEditorUndoAction_PlaceStart ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "PlaceStart";
      }
};

class SmallMapEditorUndoAction_PlaceFinish: public SmallMapEditorUndoAction_Pos
{
public:
    SmallMapEditorUndoAction_PlaceFinish (Vector<int> pos)
      : SmallMapEditorUndoAction_Pos (PLACE_FINISH, pos)
      {
      }

    ~SmallMapEditorUndoAction_PlaceFinish ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "PlaceFinish";
      }
};
#endif
