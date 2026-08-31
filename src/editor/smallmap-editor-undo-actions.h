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
#ifndef SMALLMAP_EDITOR_ACTIONS_H
#define SMALLMAP_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "rectangle.h"
#include "maptile.h"
#include "Tile.h"
#include "undo-action.h"

//! A record of an event in the smallmap editor
/** 
 * The purpose of these classes is to implement undo/redo in the miniature
 * map editor.
 */


class SmallmapEditorAction: public UndoAction
{
    public:

	//! An Editor Action can be one of the following kinds.
        enum Type {
                TERRAIN = 1,
                ERASE = 2,
                CITY = 3,
                RUIN = 4,
                TEMPLE = 5,
                BUILD_ROAD = 6,
                CLEAR_ROAD = 7,
                BLANK = 8,
        };

	//! Default constructor.
        SmallmapEditorAction
          (Type type, UndoAction::AggregateType a = UndoAction::AGGREGATE_NONE)
          : UndoAction (a), d_type (type) {}

	//! Destructor.
        virtual ~SmallmapEditorAction() {}

        Type getType() const {return d_type;}

    protected:

        Type d_type;
};

class SmallmapEditorAction_ChangeMap: public SmallmapEditorAction
{
    public:
        SmallmapEditorAction_ChangeMap (Type t, LwRectangle r, bool grow, bool agg, bool only_maptiles);
        SmallmapEditorAction_ChangeMap (Type t, LwRectangle r1, LwRectangle r2, bool grow, bool agg, bool only_maptiles);
        ~SmallmapEditorAction_ChangeMap ();

        LwRectangle getArea () const {return rects.front ();}
        std::list<LwRectangle> getRectangles () const {return rects;}
        std::list<Maptile *> getMaptiles () const {return maptiles;}
        std::list<UniquelyIdentified *> getObjects () const {return objects;}
        void clearObjects ();
        bool getOnlyMaptiles () const {return d_only_maptiles;}
    private:
        std::list<LwRectangle> rects;
        bool d_only_maptiles;
        std::list<Maptile *> maptiles;
        std::list<UniquelyIdentified *> objects;
};

class SmallmapEditorAction_Terrain: public SmallmapEditorAction_ChangeMap
{
    public:
        SmallmapEditorAction_Terrain (Tile::Type t, LwRectangle r)
          : SmallmapEditorAction_ChangeMap (TERRAIN, r, true, true, true),
          d_tile (t) {}
        ~SmallmapEditorAction_Terrain () {}

        Glib::ustring getActionName () const;
        Tile::Type getTile () const {return d_tile;}
    private:
        Tile::Type d_tile;
};

class SmallmapEditorAction_Erase: public SmallmapEditorAction_ChangeMap
{
    public:
        SmallmapEditorAction_Erase (LwRectangle r)
          : SmallmapEditorAction_ChangeMap (ERASE, r, false, true, false) {}
        ~SmallmapEditorAction_Erase () {}

        Glib::ustring getActionName () const {return "Erase";}
};

class SmallmapEditorAction_City: public SmallmapEditorAction_ChangeMap
{
    public:
        SmallmapEditorAction_City (LwRectangle r)
          : SmallmapEditorAction_ChangeMap (CITY, r, true, false, false) {}
        ~SmallmapEditorAction_City () {}

        Glib::ustring getActionName () const {return "City";}
};

class SmallmapEditorAction_Ruin: public SmallmapEditorAction_ChangeMap
{
    public:
        SmallmapEditorAction_Ruin (LwRectangle r)
          : SmallmapEditorAction_ChangeMap (RUIN, r, true, false, false) {}
        ~SmallmapEditorAction_Ruin () {}

        Glib::ustring getActionName () const {return "Ruin";}
};
class SmallmapEditorAction_Temple: public SmallmapEditorAction_ChangeMap
{
    public:
        SmallmapEditorAction_Temple (LwRectangle r)
          : SmallmapEditorAction_ChangeMap (TEMPLE, r, true, false, false) {}
        ~SmallmapEditorAction_Temple () {}

        Glib::ustring getActionName () const {return "Temple";}
};
class SmallmapEditorAction_BuildRoads: public SmallmapEditorAction_ChangeMap
{
    public:
        SmallmapEditorAction_BuildRoads (LwRectangle r, Vector<int> s,
                                         Vector<int> d)
          : SmallmapEditorAction_ChangeMap (BUILD_ROAD, r, true, false, false),
          d_src (s), d_dst (d) {}
        ~SmallmapEditorAction_BuildRoads () {}

        Glib::ustring getActionName () const {return "BuildRoads";}
        Vector<int> getSrc () const {return d_src;}
        Vector<int> getDest () const {return d_dst;}
    private:
        Vector<int> d_src;
        Vector<int> d_dst;
};
class SmallmapEditorAction_ClearRoad: public SmallmapEditorAction
{
public:
    SmallmapEditorAction_ClearRoad (Vector<int> src, Vector<int> dst)
      : SmallmapEditorAction (CLEAR_ROAD), d_src (src), d_dst (dst) {}
    ~SmallmapEditorAction_ClearRoad () {}

    Glib::ustring getActionName () const {return "ClearRoad";}

    Vector<int> getSrc () const {return d_src;}
    Vector<int> getDest () const {return d_dst;}
private:
    Vector<int> d_src;
    Vector<int> d_dst;
};

class SmallmapEditorAction_Blank: public SmallmapEditorAction
{
public:
    SmallmapEditorAction_Blank ()
      : SmallmapEditorAction (BLANK, UndoAction::AGGREGATE_BLANK) {}
    ~SmallmapEditorAction_Blank () {}

    Glib::ustring getActionName () const {return "";}
};
#endif //SMALLMAP_EDITOR_ACTIONS_H
