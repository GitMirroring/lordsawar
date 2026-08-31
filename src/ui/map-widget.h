//  Copyright (C) 2003 Michael Bartl
//  Copyright (C) 2003, 2004, 2005, 2006, 2007 Ulf Lorenz
//  Copyright (C) 2004, 2005 Bryan Duff
//  Copyright (C) 2004, 2005, 2006 Andrea Paternesi
//  Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2016, 2017,
//  2020, 2021, 2026 Ben Asselstine
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

#ifndef MAP_WIDGET_H
#define MAP_WIDGET_H
#include <gtkmm.h>
class PixMask;
class Stack;
class PathCalculator;
class City;
class Ruin;
class Stack;
class Signpost;
class Temple;
#include "vector.h"
#include "rectangle.h"
#include "image-cache.h"
#include "input-events.h"
#include "map-tip-position.h"
#include "player-list.h"

class MapWidget : public Gtk::Widget
{
public:
    static bool s_show_hidden_ruins;
    static constexpr double MIN_ZOOM_FACTOR = 0.5;
    static constexpr double MAX_ZOOM_FACTOR = 3.0;

    MapWidget ();
    ~MapWidget ();

    void select_active_stack ();
    void unselect_active_stack ();

    void mouse_button_event(MouseButtonEvent e);
    void mouse_motion_event (MouseMotionEvent e);

    void set_shift_key_down (bool down);
    void set_control_key_down (bool down);
    bool is_shift_key_down ()
      {
        return m_left_shift_down || m_right_shift_down;
      }

    bool is_control_key_down ()
      {
        return m_left_control_down || m_right_control_down;
      }

    void update_mouse_cursor ()
      {
        Vector<int> tile = mouse_pos_to_tile (m_prev_mouse_pos);
        m_current_tile = tile;
        determine_mouse_cursor
          (Playerlist::getActiveplayer ()->getActivestack (), m_current_tile);
      }

    void set_input_locked (bool locked)
      {
        m_input_locked = locked;
      }

    sigc::signal<void(double)> signal_zoom_changed ()
      {
        return m_signal_zoom_changed;
      }

    sigc::signal<void(LwRectangle)> signal_view_changed ()
      {
        return m_view_changed;
      }

    sigc::signal<void(int, int)> signal_size_allocated ()
      {
        return m_size_allocated;
      }

    sigc::signal<void(Stack*)> signal_stack_selected ()
      {
        return m_stack_selected;
      }

    sigc::signal<void(Stack*)> signal_stack_grouped_or_ungrouped ()
      {
        return m_stack_grouped_or_ungrouped;
      }

    sigc::signal<void(City*)> signal_city_visited ()
      {
        return m_city_visited;
      }

    sigc::signal<void(Vector<int>, City*)> signal_city_queried ()
      {
        return m_city_queried;
      }

    sigc::signal<void()> signal_city_unqueried ()
      {
        return m_city_unqueried;
      }

    sigc::signal<void(Ruin*,Vector<int>)> signal_ruin_queried ()
      {
        return m_ruin_queried;
      }

    sigc::signal<void(Ruin*)> signal_ruin_visited ()
      {
        return m_ruin_visited;
      }

    sigc::signal<void()> signal_ruin_unqueried ()
      {
        return m_ruin_unqueried;
      }

    sigc::signal<void(Signpost*,Vector<int>)> signal_signpost_queried ()
      {
        return m_signpost_queried;
      }

    sigc::signal<void()> signal_signpost_unqueried ()
      {
        return m_signpost_unqueried;
      }

    sigc::signal<void(Stack*, Vector<int>)> signal_stack_queried ()
      {
        return m_stack_queried;
      }

    sigc::signal<void()> signal_stack_unqueried ()
      {
        return m_stack_unqueried;
      }

    sigc::signal<void(Temple*,Vector<int>)> signal_temple_queried ()
      {
        return m_temple_queried;
      }

    sigc::signal<void(Temple*)> signal_temple_visited ()
      {
        return m_temple_visited;
      }

    sigc::signal<void()> signal_temple_unqueried ()
      {
        return m_temple_unqueried;
      }

    sigc::signal<void(Vector<int>, guint32)> signal_path_turns ()
      {
        return m_path_turns;
      }

    sigc::signal<void()> signal_path_set ()
      {
        return m_path_set;
      }

    sigc::signal<void(ImageCache::CursorType)> signal_cursor_changed ()
      {
        return m_cursor_changed;
      }

    void reset_path_calculator (Stack *s);
  
    void set_view (LwRectangle rect)
      {
        center_on_smallmap_pos (rect.pos.x + (rect.dim.x / 2),
                                rect.pos.y + (rect.dim.y / 2));
      }

    LwRectangle get_view ()
      {
        return to_logical_extents ();
      }

    MapTipPosition map_tip_position(LwRectangle tile_area);
    MapTipPosition map_tip_position(Vector<int> tile);

    void blank (bool value)
      {
        m_blank_screen = value;
      }

    void set_fighting (LocationBox ruckus) {m_fighting = ruckus;}

    bool get_toggled () const {return m_grid_toggled;}

    void toggle_grid ()
      {
        m_grid_toggled = !m_grid_toggled;
        queue_draw ();
      }

    void zoom_in ()
      {
        m_zoom_in_progress = true;
        on_zoom_scale_changed (m_scale + 0.1);
        m_zoom_in_progress = false;
      }

    void zoom_out ()
      {
        m_zoom_in_progress = true;
        on_zoom_scale_changed (m_scale - 0.1);
        m_zoom_in_progress = false;
      }

    void reset_zoom ()
      {
        m_zoom_in_progress = true;
        on_zoom_scale_reset ();
        m_zoom_in_progress = false;
      }

    void center_on_smallmap_pos (int x, int y);

    void popup_stack_actions_menu (Stack *stack, Vector<int> pos);
protected:

    double m_offset_x = 0.0;
    double m_offset_y = 0.0;
    double m_drag_start_x = 0.0;
    double m_drag_start_y = 0.0;
    double m_scale = 1.0;

    bool m_zoom_in_progress = false;
    double m_zoom_start_scale = 1.0;

    bool m_grid_toggled = false;

    ImageCache::CursorType m_cursor = ImageCache::CursorType::POINTER;
    enum mouse_state_enum
      {
        NONE,
        DRAGGING_MAP,
        SHOWING_CITY,
        SHOWING_RUIN,
        SHOWING_TEMPLE,
        SHOWING_SIGNPOST,
        SHOWING_STACK,
        DRAGGING_STACK,
        DRAGGING_ENDPOINT
      } m_mouse_state = NONE;

    bool m_left_shift_down = false;
    bool m_right_shift_down = false;
    bool m_left_control_down = false;
    bool m_right_control_down = false;
    bool m_input_locked = false;

    Vector<int> m_current_tile = Vector<int> (-1, -1);

    bool m_blank_screen = false;

    LocationBox m_fighting = LocationBox (Vector<int> (-1, -1));

    Vector<int> m_prev_mouse_pos = Vector<int> (0, 0);

    struct MouseMotionEvent m_mouse_motion = {};

    Glib::DateTime m_last_clicked = Glib::DateTime::create_now_local ();

    int m_current_large_selector_image = 0;
    int m_current_small_selector_image = 0;
    int m_num_large_selector_images = 0;
    int m_num_small_selector_images = 0;

    PathCalculator *m_path_calculator = NULL;

    sigc::connection m_selector;

    sigc::signal<void(double)> m_signal_zoom_changed;
    sigc::signal<void(LwRectangle)> m_view_changed;
    sigc::signal<void(int, int)> m_size_allocated;
    sigc::signal<void(Stack*)> m_stack_selected;
    sigc::signal<void(Stack*)> m_stack_grouped_or_ungrouped;
    sigc::signal<void(City*)> m_city_visited;  //for citywindow
    sigc::signal<void(Vector<int>, City*)> m_city_queried;  //for city-info-tip
    sigc::signal<void()> m_city_unqueried;
    sigc::signal<void(Ruin*)> m_ruin_visited;
    sigc::signal<void(Ruin*,Vector<int>)> m_ruin_queried; // the map tip
    sigc::signal<void()> m_ruin_unqueried;
    sigc::signal<void(Signpost*,Vector<int>)> m_signpost_queried;
    sigc::signal<void()> m_signpost_unqueried;
    sigc::signal<void(Stack *,Vector<int>)> m_stack_queried;
    sigc::signal<void()> m_stack_unqueried;
    sigc::signal<void(Temple*)> m_temple_visited;
    sigc::signal<void(Temple*,Vector<int>)> m_temple_queried;
    sigc::signal<void()> m_temple_unqueried;
    sigc::signal<void(Vector<int>, guint32)> m_path_turns;
    sigc::signal<void()> m_path_set; // emitted when a path for a stack is set
    sigc::signal<void(ImageCache::CursorType)> m_cursor_changed; // emitted when the cursor changes

    void setup_gestures ();

    void snapshot_vfunc (const Glib::RefPtr<Gtk::Snapshot>& snapshot) override;
    void draw_active_tile (const Glib::RefPtr<Gtk::Snapshot> &snapshot);
    void draw_stack(Stack *s, const Glib::RefPtr<Gtk::Snapshot> &snapshot);
    PixMask * lookup_tile_graphics (Vector<int> tile);

    void clamp_offset ();
    void on_zoom_scale_changed (double scale_delta);
    void on_zoom_scale_reset ();

    void size_allocate_vfunc (int width, int height, int baseline) override;

    void determine_mouse_cursor (Stack *stack, Vector<int> tile);

    void get_selector_frame_limits (Stack *s, int &limitbig, int &limitsmall);
    void update_selector_limits (Stack *s);

    void update_key_state (guint keyval, bool pressed);


    bool is_tile_visible (Vector<int> pos);
    LwRectangle to_logical_extents ();
    Vector<int> to_logical_coords (double x, double y);
    Vector<int> mouse_pos_to_tile (Vector<int> pos)
      {
        return to_logical_coords (pos.x, pos.y);
      }
    Vector<int> get_view_pos_from_view ();
    Vector<int> tile_to_buffer_pos (Vector<int> pos);
    void get_tile_rect (Vector<int> pos, double& x, double& y, double& w, double& h);

    void blit_pixbuf (const Glib::RefPtr<Gtk::Snapshot> &snapshot, Vector<int> pos, Glib::RefPtr<Gdk::Pixbuf> pixbuf);

    void draw_explosion (const Glib::RefPtr<Gtk::Snapshot>& snapshot);
    void release_left_button ();
};
#endif
