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

#ifndef EDITOR_MAP_WIDGET_H
#define EDITOR_MAP_WIDGET_H
#include <gtkmm.h>
class PixMask;
class Stack;
class PathCalculator;
class City;
class Ruin;
class Stack;
class Signpost;
class Temple;
class EditorUndoAction;
#include "vector.h"
#include "rectangle.h"
#include "image-cache.h"
#include "input-events.h"
#include "editor-undo-actions.h"

class EditorMapWidget : public Gtk::Widget
{
public:
    typedef std::vector<UniquelyIdentified*> map_selection_seq;

    enum Pointer
      {
        UNKNOWN = -1,
        POINTER,
        TERRAIN,
        STACK,
        CITY,
        RUIN,
        TEMPLE,
        SIGNPOST,
        ROAD,
        ERASE,
        MOVE,
        PORT,
        BRIDGE,
        BAG,
        FIGHT,
        STONE,
        FLAG,
        TILESTYLE,
      };

    EditorMapWidget ();
    ~EditorMapWidget ();
    void center_on_smallmap_pos (int x, int y);

    LwRectangle get_view ()
      {
        return to_logical_extents ();
      }

    void set_view (LwRectangle rect)
      {
        center_on_smallmap_pos (rect.pos.x + (rect.dim.x / 2),
                                rect.pos.y + (rect.dim.y / 2));
      }

    void set_pointer (Pointer p)
      {
        m_pointer = p;
      }

    void set_pointer_size (guint32 size)
      {
        m_pointer_size = size;
      }

    void set_pointer_terrain (int tile_idx)
      {
        m_pointer_terrain = tile_idx;
      }

    void set_pointer_tile_style (int tile_style_id)
      {
        m_pointer_tile_style_id = tile_style_id;
      }

    void set_show_grid (bool active)
      {
        m_grid_toggled = active;
      }

    bool get_show_grid () const
      {
        return m_grid_toggled;
      }

    sigc::signal<void(LwRectangle)> signal_view_changed ()
      {
        return m_view_changed;
      }

    sigc::signal<void(int, int)> signal_size_allocated ()
      {
        return m_size_allocated;
      }

    sigc::signal<void(Vector<int>)> signal_pointing_at_new_tile ()
      {
        return m_pointing_at_new_tile;
      }

    sigc::signal<void(EditorUndoAction*)> signal_undo_generated ()
      {
        return m_undo_map;
      }

    sigc::signal<void(map_selection_seq,Vector<int>)> signal_object_select_choice ()
      {
        return m_objects_selected;
      }

    sigc::signal<void(LwRectangle)> signal_map_tiles_changed ()
      {
        return m_map_tiles_changed;
      }

    sigc::signal<void()> signal_map_water_changed ()
      {
        return m_map_water_changed;
      }

    sigc::signal<void(Vector<int>)> signal_bag_selected ()
      {
        return m_bag_selected;
      }

    sigc::signal<void(Vector<int>)> signal_flag_selected ()
      {
        return m_flag_selected;
      }

    sigc::signal<void(Vector<int>,Vector<int>)> signal_tilestyle_tile_selected ()
      {
        return m_tilestyle_tile_selected;
      }

    sigc::signal<void(Stack*,Vector<int>)> signal_stack_selected_for_battle_calculator ()
      {
        return m_stack_selected_for_battle_calculator;
      }

    sigc::signal<void(Vector<int>)> signal_create_new_stack ()
      {
        return m_create_new_stack;
      }

    sigc::signal<void(ImageCache::CursorType)> cursor_changed;

protected:

    Pointer m_pointer = UNKNOWN;
    guint32 m_pointer_size = 1;
    int m_pointer_terrain = 0;
    int m_pointer_tile_style_id = -1;
    double m_offset_x = 0.0;
    double m_offset_y = 0.0;
    double m_drag_start_x = 0.0;
    double m_drag_start_y = 0.0;
    double m_scale = 1.0;
    bool m_grid_toggled = false;
    ImageCache::CursorType m_cursor;
    enum mouse_state_enum
      {
	NONE,
        MAP_DRAGGING,
        MOVE_DRAGGING,
        TERRAIN_DRAGGING,
      };
    enum mouse_state_enum m_mouse_state = NONE;
    MapBackpack *m_moving_bag = NULL;
    Vector<int> m_moving_objects_from = Vector<int>(-1,-1);
    bool m_left_shift_down = false;
    bool m_right_shift_down = false;
    bool m_left_control_down = false;
    bool m_right_control_down = false;
    Vector<int> m_current_tile = Vector<int> (-1, -1);
    bool m_redraw_without_cursor = false;
    Vector<int> m_prev_mouse_pos = Vector<int> (-1, -1);
    sigc::signal<void(LwRectangle)> m_view_changed;
    sigc::signal<void(int, int)> m_size_allocated;
    sigc::signal<void(Vector<int>)> m_pointing_at_new_tile;
    sigc::signal<void(EditorUndoAction*)> m_undo_map;
    sigc::signal<void(map_selection_seq,Vector<int>)> m_objects_selected;
    sigc::signal<void(LwRectangle)> m_map_tiles_changed;
    sigc::signal<void()> m_map_water_changed;
    sigc::signal<void(Vector<int>)> m_bag_selected;
    sigc::signal<void(Vector<int>)> m_flag_selected;
    sigc::signal<void(Vector<int>,Vector<int>)> m_tilestyle_tile_selected;
    sigc::signal<void(Stack*,Vector<int>)> m_stack_selected_for_battle_calculator;
    sigc::signal<void(Vector<int>)> m_create_new_stack;

    void clamp_offset ();
    void snapshot_vfunc (const Glib::RefPtr<Gtk::Snapshot>& snapshot) override;
    void size_allocate_vfunc (int width, int height, int baseline) override;
    struct MouseMotionEvent m_mouse_motion = {};

    PixMask * lookup_tile_graphics (Vector<int> tile);
    LwRectangle to_logical_extents ();
    Vector<int> to_logical_coords (double x, double y);
    Vector<int> mouse_pos_to_tile (Vector<int> pos)
      {
        return to_logical_coords (pos.x, pos.y);
      }

    void determine_mouse_cursor (Stack *stack, Vector<int> tile);

    bool is_shift_key_down ()
      {
        return m_left_shift_down || m_right_shift_down;
      }

    bool is_control_key_down ()
      {
        return m_left_control_down || m_right_control_down;
      }

    void update_key_state (guint keyval, bool pressed);

    void mouse_button_event (MouseButtonEvent e);

    std::vector<Vector<int> > get_cursor_tiles ();
    void change_map_under_cursor (Vector<int> mouse_position);
    void bring_up_details (Vector<int>);
    int tile_to_bridge_type (Vector<int> t);
    LwRectangle get_cursor_rectangle ();

    void after_draw (Cairo::RefPtr<Cairo::Context> &cr);
    void rounded_rectangle (Cairo::RefPtr<Cairo::Context> cr, double x,
                            double y, double w, double h);
    void set_source_rgb (Cairo::RefPtr<Cairo::Context> cr, Gdk::RGBA c);
    void display_moving_building (Cairo::RefPtr<Cairo::Context> cr, Vector<int> src, Vector<int> dest);

    void mouse_motion_event (MouseMotionEvent e);

    bool is_acceptable_building_for_stone (Maptile::Building b);
};
#endif
