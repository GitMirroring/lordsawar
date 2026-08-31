//  Copyright (C) 2010, 2014, 2015, 2020, 2021, 2026 Ben Asselstine
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
#ifndef EDITABLE_SMALLMAP_H
#define EDITABLE_SMALLMAP_H

#include <sigc++/signal.h>
#include <sigc++/connection.h>
#include <sigc++/trackable.h>

#include "overview-map.h"

#include "input-events.h"
#include "undo-action.h"

//! Draw a miniature map graphic and let it be changeable.
/**
 */
class EditableSmallMap: public OverviewMap
{
public:
    enum Pointer
      {
        NONE = 0,
        TERRAIN,
        CITY,
        RUIN,
        TEMPLE,
        ERASE,
        PICK_NEW_ROAD_START,
        PICK_NEW_ROAD_FINISH
      };

    //! Default constructor.  Make a new EditableSmallMap.
    EditableSmallMap ();

    //! Destructor.
    ~EditableSmallMap ()
      {
      }


    // Get Methods

    //! Get an image of the mouse cursor.
    PixMask* get_cursor (Vector<int> &hotspot) const;

    // Set Methods

    //! Set the pointer characteristics.
    void set_pointer (Pointer pointer, int size, int terrain);

    void set_pointer_size (int size)
      {
        m_pointer_size = size;
      }

    // Methods that operate on the class data and modify the class.

    void setRoadFinish (Vector<int> p);

    void setRoadStart (Vector<int> p);

    //! Realize the given mouse button event.
    void mouse_button_event (MouseButtonEvent e);

    //! Realize the given mouse motion event.
    void mouse_motion_event (MouseMotionEvent e);

    //! erase the target points
    void clear_road ();

    Vector<int> get_road_start () const
      {
        return m_road_start;
      }

    Vector<int> get_road_finish () const
      {
        return m_road_finish;
      }

    bool is_start_set () const
      {
        return m_road_start != Vector<int>(-1, -1);
      }

    bool is_finish_set () const
      {
        return m_road_finish != Vector<int>(-1, -1);
      }

    //! make a road from road_start to road_finish.
    bool create_road ();

    //! check to see if the road can be made.
    bool check_road ();

    //! a hack to force a redraw
    void update ();

    // Signals

    // Emitted after a call to EditableSmallMap::Draw.
    /**
     * Classes that use EditableSmallMap must catch this signal to display the map.
     */
    sigc::signal<void(Cairo::RefPtr<Cairo::Surface>, Gdk::Rectangle)> signal_map_changed ()
      {
        return m_map_changed;
      }
                
    sigc::signal<void()> signal_map_water_changed ()
      {
        return m_map_water_changed;
      }

    sigc::signal<void()> signal_map_edited ()
      {
        return  m_map_edited;
      }

    sigc::signal<void(UndoAction *)> signal_undo_map ()
      {
        return m_undo_map;
      }

    sigc::signal<void(Vector<int>)> signal_road_start_placed ()
      {
        return m_road_start_placed;
      }

    sigc::signal<void(Vector<int>)> signal_road_finish_placed ()
      {
        return m_road_finish_placed;
      }


private:

    //! Draw the City objects and little white box onto the mini-map graphic.
    /**
     * This method is automatically called by the EditableSmallMap::draw method.
     */
    virtual void after_draw ();

    void change_map (Vector<int> pos);

    LwRectangle get_cursor_rectangle (Vector<int> current_tile);

    // DATA

    Pointer m_pointer;
    int m_pointer_terrain;
    int m_pointer_size;
    Vector<int> m_road_start;
    Vector<int> m_road_finish;
    std::list<UndoAction*> m_undo_actions;

    sigc::signal<void(Cairo::RefPtr<Cairo::Surface>, Gdk::Rectangle)> m_map_changed;
    sigc::signal<void()> m_map_edited;
    sigc::signal<void(UndoAction *)> m_undo_map;
    sigc::signal<void(Vector<int>)> m_road_start_placed;
    sigc::signal<void(Vector<int>)> m_road_finish_placed;
    sigc::signal<void()> m_map_water_changed;
};

#endif
