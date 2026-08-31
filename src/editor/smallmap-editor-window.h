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

#include <gtkmm.h>
#ifndef SMALLMAP_EDITOR_WINDOW_H
#define SMALLMAP_EDITOR_WINDOW_H

#include "editable-small-map.h"
#include "smallmap-editor-undo-actions.h"

class SmallmapEditorWindow: public Gtk::ApplicationWindow
{
public:

    SmallmapEditorWindow (Gtk::Window *parent)
      : m_actions (Gio::SimpleActionGroup::create ())
      {
        set_modal (true);
        set_transient_for (*parent);
        present ();
        m_pointer = EditableSmallMap::NONE;
        m_pointer_size = 1;
      }

    ~SmallmapEditorWindow ()
      {
        delete m_smallmap;
        m_dark_style_handler.disconnect ();
        disconnect_signals ();
        disconnect_terrain_button_signals ();
        delete m_umgr;
      }

    void setup ()
      {
        setup_header_bar ();

        populate ();
        add_terrain_buttons ();
        connect_terrain_button_signals ();

        setup_smallmap ();

        connect_signals ();

        setup_dark_mode_change ();

        setup_undo ();
        setup_quit ();
        m_smallmap->resize ();
        update ();

        if (m_terrain_buttons.empty () == false)
          m_terrain_buttons[m_terrain_buttons.size () - 1]->set_active (true);
      }

    bool is_changed ()
      {
        //some of the undo events here aren't changes to the model
        //e.g. we can put a start road pin on the map but it shouldn't
        //report that we changed anything
        guint32 count = 0;
        auto names = m_umgr->get_undo_names ();
        for (auto name : names)
          {
            if (name == "ClearRoad" ||
                name == "PlaceStart" ||
                name == "PlaceFinish")
              count++;
          }
        return count != names.size () && names.empty () == false;
      }

    sigc::signal<void(Gtk::ResponseType)> signal_response ()
      {
        return m_signal_response;
      }
private:
    Glib::RefPtr<Gio::SimpleActionGroup> m_actions;
    Gtk::HeaderBar *m_header_bar;
    Gtk::Button *m_close_button;
    Gtk::ToggleButton *m_erase_button;
    Gtk::ToggleButton *m_city_button;
    Gtk::ToggleButton *m_ruin_button;
    Gtk::ToggleButton *m_temple_button;
    Gtk::ToggleButton *m_start_road_button;
    Gtk::ToggleButton *m_finish_road_button;
    Gtk::Button *m_clear_road_button;
    Gtk::Button *m_build_road_button;

    Gtk::FlowBox *m_button_box;
    std::vector<Gtk::ToggleButton *>m_terrain_buttons;

    Gtk::DrawingArea *m_map_drawing_area;
    PointerSizeMenuButton *m_pointer_size_menu_button;
    ShieldMenuButton *m_shield_menu_button;

    bool m_dark;
    sigc::connection m_dark_style_handler;
    enum EditableSmallMap::Pointer m_pointer;

    guint32 m_pointer_size;
    EditableSmallMap *m_smallmap;

    UndoMgr *m_umgr;
    Cairo::RefPtr<Cairo::Surface> m_smallmap_surface;
    struct MouseMotionEvent m_smallmap_mouse_motion = {};

    std::list<sigc::connection> m_connections;
    std::list<sigc::connection> m_terrain_button_connections;
    std::list<sigc::connection> m_action_connections;

    sigc::signal<void(Gtk::ResponseType)> m_signal_response;

    void setup_header_bar ()
      {
        m_header_bar = Gtk::make_managed<Gtk::HeaderBar>();
        m_header_bar->set_show_title_buttons (true);
        auto m_title = Gtk::make_managed<Gtk::Label> (_("Mini-Map Editor"));
        m_title->add_css_class ("title");
        m_header_bar->set_title_widget (*m_title);

        m_shield_menu_button = Gtk::make_managed<ShieldMenuButton> ();
        m_header_bar->pack_end (*m_shield_menu_button);

        m_pointer_size_menu_button =
          Gtk::make_managed<PointerSizeMenuButton> ();
        m_header_bar->pack_end (*m_pointer_size_menu_button);

        set_titlebar (*m_header_bar);
      }

    void draw_pointer_buttons ()
      {
        m_pointer_size_menu_button->redraw ();
      }

    void draw_buttons ()
      {
        if (Lw::get_dark ())
          set_button_image (m_erase_button,
                            File::getEditorFile ("button_erase_dark"));
        else
          set_button_image (m_erase_button,
                            File::getEditorFile ("button_erase"));

        set_button_image (m_city_button,
                          File::getEditorFile ("button_castle"));
        set_button_image (m_ruin_button,
                          File::getEditorFile ("button_ruin"));
        set_button_image (m_temple_button,
                          File::getEditorFile ("button_temple"));

        m_start_road_button->set_icon_name ("go-next-symbolic");
        m_start_road_button->add_css_class ("editor-control-button");

        m_finish_road_button->set_icon_name ("go-last-symbolic");
        m_finish_road_button->add_css_class ("editor-control-button");

        m_clear_road_button->set_icon_name ("edit-clear-symbolic");
        m_clear_road_button->add_css_class ("editor-control-button");

        auto image = Gtk::make_managed<Gtk::Image> ();
        image->set (File::getEditorFile ("button_road"));
        image->set_pixel_size (get_editor_button_size ());
        m_build_road_button->set_child (*image);
        m_build_road_button->add_css_class ("editor-control-button");
      }

    void
    add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    void block_terrain_button_signals ()
      {
        for (auto c : m_terrain_button_connections)
          c.block ();
      }

    void unblock_terrain_button_signals ()
      {
        for (auto c : m_terrain_button_connections)
          c.unblock ();
      }

    void block_signals ()
      {
        for (auto c : m_connections)
          c.block ();
      }

    void unblock_signals ()
      {
        for (auto c : m_connections)
          c.unblock ();
      }

    void connect_signals ()
      {
        add_connection
          (m_smallmap->signal_map_water_changed ().connect
           ([this] ()
            {
              m_smallmap->resize ();
            }));

        add_connection
          (m_close_button->signal_clicked ().connect
           ([this] ()
            {
              m_signal_response (Gtk::ResponseType::ACCEPT);
            }));

        add_connection
          (m_smallmap->signal_map_changed ().connect
           ([this] (Cairo::RefPtr<Cairo::Surface> map, Gdk::Rectangle)
            {
              std::shared_ptr<Cairo::ImageSurface> img_surface =
                std::static_pointer_cast<Cairo::ImageSurface>(map);
              m_smallmap_surface = map;
              m_map_drawing_area->set_size_request
                (img_surface->get_width (), img_surface->get_height ());
              map->flush ();

              m_map_drawing_area->queue_draw ();
            }));

        add_connection
          (m_pointer_size_menu_button->signal_pointer_size_selected ().connect
           ([this] (guint32 size)
            {
              m_pointer_size = size;
              m_smallmap->set_pointer (m_pointer, m_pointer_size,
                                       get_selected_terrain ());
              update_cursor ();
              update ();
            }));

        add_connection
          (m_shield_menu_button->signal_shield_selected ().connect
           ([this] (Shield::Color shield)
            {
              Playerlist::instance ()->setActiveplayer (shield);
              update ();
            }));

        add_connection
          (m_erase_button->signal_toggled ().connect
           ([this] ()
            {
              if (m_erase_button->get_active ())
                  {
                    set_pointer_button (EditableSmallMap::ERASE);
                    update ();
                  }
            }));

        add_connection
          (m_city_button->signal_toggled ().connect
           ([this] ()
            {
              if (m_city_button->get_active ())
                {
                  set_pointer_button (EditableSmallMap::CITY);
                  update ();
                }
            }));

        add_connection
          (m_ruin_button->signal_toggled ().connect
           ([this] ()
            {
              if (m_ruin_button->get_active ())
                {
                  set_pointer_button (EditableSmallMap::RUIN);
                  update ();
                }
            }));

        add_connection
          (m_temple_button->signal_toggled ().connect
           ([this] ()
            {
              if (m_temple_button->get_active ())
                {
                  set_pointer_button (EditableSmallMap::TEMPLE);
                  update ();
                }
            }));

        add_connection
          (m_start_road_button->signal_toggled ().connect
           ([this] ()
            {
              if (m_start_road_button->get_active ())
                {
                  set_pointer_button (EditableSmallMap::PICK_NEW_ROAD_START);
                  update ();
                }
            }));

        add_connection
          (m_finish_road_button->signal_toggled ().connect
           ([this] ()
            {
              if (m_finish_road_button->get_active ())
                {
                  set_pointer_button (EditableSmallMap::PICK_NEW_ROAD_FINISH);
                  update ();
                }
            }));

        add_connection
          (m_clear_road_button->signal_clicked ().connect
           ([this] ()
            {
              m_umgr->add
                (new SmallMapEditorUndoAction_ClearRoad
                 (m_smallmap->get_road_start (),
                  m_smallmap->get_road_finish ()));
              m_smallmap->clear_road ();
              update ();
            }));

        add_connection
          (m_build_road_button->signal_clicked ().connect
           ([this] ()
            {
              auto action =
                new SmallMapEditorUndoAction_BuildRoads
                (GameMap::get_boundary (),
                 m_smallmap->get_road_start (),
                 m_smallmap->get_road_finish ());
              if (m_smallmap->create_road ())
                {
                  m_umgr->add (action);
                  m_smallmap->clear_road ();
                  update ();
                }
              else
                delete action;
            }));

        add_connection
          (m_smallmap->signal_road_start_placed ().connect
           ([this] (Vector<int>)
            {
              update ();
            }));

        add_connection
          (m_smallmap->signal_road_finish_placed ().connect
           ([this] (Vector<int>)
            {
              update ();
            }));

        add_connection
          (m_smallmap->signal_road_finish_placed ().connect
           ([this] (Vector<int>)
            {
              update ();
            }));

        add_connection
          (m_smallmap->signal_undo_map ().connect
           ([this] (UndoAction *action)
            {
              m_umgr->add (action);
            }));
      }

    void update ()
      {
        m_smallmap->draw ();
        update_buttons ();
      }

    void populate ()
      {
        auto root = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::VERTICAL);
        root->set_spacing (6);
        root->set_margin (6);
        auto box = Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
        box->set_spacing (6);
        m_map_drawing_area = Gtk::make_managed<Gtk::DrawingArea> ();
        m_map_drawing_area->set_valign (Gtk::Align::FILL);
        m_map_drawing_area->set_halign (Gtk::Align::CENTER);
        box->append (*m_map_drawing_area);
        m_smallmap = new EditableSmallMap ();
        auto button_box = create_button_box ();
        box->append (*button_box);
        root->append (*box);

        auto action_area =
          Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
        action_area->set_spacing (6);
        action_area->set_hexpand (true);
        action_area->set_halign (Gtk::Align::END);
        m_close_button = Gtk::make_managed<Gtk::Button> ();
        m_close_button->set_label (_("Close"));
        action_area->append (*m_close_button);
        root->append (*action_area);

        set_child (*root);
      }

    Gtk::ToggleButton* setup_toggle_button (Glib::ustring tooltip)
      {
        auto button = Gtk::make_managed<Gtk::ToggleButton> ();
        button->add_css_class ("editor-toggle-button");
        button->set_valign (Gtk::Align::START);
        button->set_group (*m_erase_button);
        button->set_tooltip_text (tooltip);
        return button;
      }

    Gtk::FlowBox *create_button_box ()
      {
        m_button_box = Gtk::make_managed<Gtk::FlowBox> ();
        m_button_box->set_valign (Gtk::Align::START);
        m_button_box->set_orientation (Gtk::Orientation::HORIZONTAL);
        m_button_box->set_selection_mode (Gtk::SelectionMode::NONE);
        m_button_box->set_min_children_per_line (2);
        m_button_box->set_max_children_per_line (2);
        auto box = m_button_box;

        m_erase_button = Gtk::make_managed<Gtk::ToggleButton> ();
        m_erase_button->add_css_class ("editor-toggle-button");
        m_erase_button->set_valign (Gtk::Align::START);
        m_erase_button->set_tooltip_text ("Remove object");
        box->append (*m_erase_button);

        m_city_button = setup_toggle_button (_("Add a city"));
        box->append (*m_city_button);

        m_ruin_button = setup_toggle_button (_("Add a ruin"));
        box->append (*m_ruin_button);

        m_temple_button = setup_toggle_button (_("Add a temple"));
        box->append (*m_temple_button);

        m_start_road_button =
          setup_toggle_button (_("Set a road starting point"));
        box->append (*m_start_road_button);

        m_finish_road_button =
          setup_toggle_button (_("Set a road ending point"));
        box->append (*m_finish_road_button);

        m_clear_road_button = Gtk::make_managed<Gtk::Button> ();
        m_clear_road_button->set_tooltip_text (_("Clear road points"));
        box->append (*m_clear_road_button);

        m_build_road_button = Gtk::make_managed<Gtk::Button> ();
        m_build_road_button->set_tooltip_text (_("Build road"));
        box->append (*m_build_road_button);

        draw_buttons ();
        return box;
      }

    void setup_dark_mode_change ()
      {
        m_dark = Lw::get_dark ();
        auto iface = Gio::Settings::create ("org.gnome.desktop.interface",
                                            "/org/gnome/desktop/interface/");
        if (iface)
          {
            m_dark_style_handler =
              iface->signal_changed ().connect
              ([this, iface] (const Glib::ustring &key)
               {
                 (void)key;
                 auto scheme = iface->get_string ("color-scheme");
                 if (scheme == "prefer-dark")
                   m_dark = true;
                 else
                   m_dark = false;
                 draw_pointer_buttons ();
                 draw_buttons ();
               });
          }
      }

    void set_button_image (Gtk::Button *button, std::string file)
      {
        if (button->get_child ())
          button->unset_child ();
        auto image = Gtk::make_managed<Gtk::Image> ();
        image->set (file);
        image->set_pixel_size (get_editor_button_size ());
        button->set_child (*image);
        button->add_css_class ("editor-control-button");
      }

    void add_terrain_buttons ()
      {
        auto box = m_button_box;

        int i = 0;
        int active = -1;
        for (auto button : m_terrain_buttons)
          {
            if (button->get_active ())
              active = i;
            box->remove (*button);
          }

        m_terrain_buttons.clear ();

        auto tileset = GameMap::getTileset ();
        for (auto it = tileset->rbegin (); it != tileset->rend (); ++it)
          {
            Tile *tile = (*it);
            if (tile->empty ())
              continue;
            auto button = Gtk::make_managed<Gtk::ToggleButton>();
            button->add_css_class ("editor-control-button");
            button->set_group (*m_erase_button);
            button->set_valign (Gtk::Align::START);
            button->set_tooltip_text
              (String::ucompose (_("Draw %1"), tile->getName ()));
            if (button->get_child ())
              button->unset_child ();
            auto image = Gtk::make_managed<Gtk::Image> ();
            auto pix = tile->front ()->front ()->getImage ();
            image->set_pixel_size (get_editor_button_size ());
            image->set (pix->to_texture ());
            button->set_child (*image);
            m_terrain_buttons.push_back (button);
            box->prepend (*button);
          }
        if (active >= 0)
          m_terrain_buttons[tileset->size () - active - 1]->set_active (true);

      }

    void connect_terrain_button_signals ()
      {
        int tile_idx = GameMap::getTileset ()->size ();
        for (auto button : m_terrain_buttons)
          {
            tile_idx--;
            add_terrain_button_connection
              (button->signal_toggled ().connect
               ([this, button, tile_idx] ()
                {
                  if (button->get_active ())
                    {
                      set_pointer_button (EditableSmallMap::TERRAIN);
                      m_smallmap->set_pointer (m_pointer, m_pointer_size,
                                               tile_idx);
                    }
                }));
          }
      }

    void add_terrain_button_connection (sigc::connection c)
      {
        m_terrain_button_connections.push_back (c);
      }

    void disconnect_terrain_button_signals ()
      {
        for (auto conn : m_terrain_button_connections)
          conn.disconnect ();
        m_terrain_button_connections.clear ();
      }

    void set_pointer_button (enum EditableSmallMap::Pointer pointer)
      {
        block_signals ();
        block_terrain_button_signals ();

        //update the pointer menu button
        switch (pointer)
          {
          case EditableSmallMap::ERASE:
          case EditableSmallMap::TERRAIN:
            m_pointer_size_menu_button->set_sensitive (true);
            m_pointer_size_menu_button->set_active (1);
            break;

          case EditableSmallMap::CITY:
          case EditableSmallMap::RUIN:
          case EditableSmallMap::TEMPLE:
          case EditableSmallMap::PICK_NEW_ROAD_START:
          case EditableSmallMap::PICK_NEW_ROAD_FINISH:
            m_pointer_size_menu_button->set_sensitive (false);
            break;

          case EditableSmallMap::NONE:
            break;
          }

        //update the shield menu button
        switch (pointer)
          {
          case EditableSmallMap::CITY:
            m_shield_menu_button->set_sensitive (true);
            m_shield_menu_button->set_active (Shield::WHITE);
            break;

          case EditableSmallMap::ERASE:
          case EditableSmallMap::TERRAIN:
          case EditableSmallMap::RUIN:
          case EditableSmallMap::TEMPLE:
          case EditableSmallMap::PICK_NEW_ROAD_START:
          case EditableSmallMap::PICK_NEW_ROAD_FINISH:
            m_shield_menu_button->set_sensitive (false);
            break;

          case EditableSmallMap::NONE:
            break;
          }
        unblock_terrain_button_signals ();
        unblock_signals ();
        m_pointer = pointer;

        m_smallmap->set_pointer (m_pointer, m_pointer_size, -1);

        update_cursor ();
      }

    void update_cursor ()
      {
        Vector<int> hotspot;
        auto im = m_smallmap->get_cursor (hotspot);
        auto cursor = Gdk::Cursor::create (im->to_texture (),
                                           hotspot.x, hotspot.y);
        m_map_drawing_area->set_cursor (cursor);
      }

    void setup_smallmap ()
      {
        struct MouseMotionEvent empty_event{};
        m_smallmap_mouse_motion = empty_event;

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this] (double x, double y)
           {
             struct MouseMotionEvent *ev = &m_smallmap_mouse_motion;
             ev->pos = Vector<int>(std::round (x), std::round (y));
             m_smallmap->mouse_motion_event (*ev);
           });
        m_map_drawing_area->add_controller (motion);

        auto click = Gtk::GestureClick::create ();
        click->signal_pressed ().connect
          ([this] (int n, double x, double y)
           {
             struct MouseButtonEvent ev = to_input_event (n, x, y, true);
             struct MouseMotionEvent *evmotion = &m_smallmap_mouse_motion;
             evmotion->pressed[ev.button] = true;
             m_smallmap->mouse_button_event (ev);
           });

        click->signal_released ().connect
          ([this] (int n, double x, double y)
           {
             struct MouseButtonEvent ev = to_input_event (n, x, y, false);
             struct MouseMotionEvent *evmotion = &m_smallmap_mouse_motion;
             evmotion->pressed[ev.button] = false;
             m_smallmap->mouse_button_event (ev);
           });
        m_map_drawing_area->add_controller (click);

        m_map_drawing_area->set_draw_func
          ([this](const Cairo::RefPtr<Cairo::Context>& cr, int, int)
           {
             if (!m_smallmap_surface)
               return;

             cr->set_source (m_smallmap_surface, 0, 0);
             cr->paint ();
           });
      }

    guint32 get_editor_button_size ()
      {
        return LW_BUTTON_SIZE / 1.6667;
      }

    void setup_quit ()
      {
        auto controller = Gtk::EventControllerKey::create ();
        controller->signal_key_pressed ().connect
          ([this] (guint keyval, guint, Gdk::ModifierType)
           {
             if (keyval == GDK_KEY_Escape)
               {
                 m_signal_response.emit (Gtk::ResponseType::CLOSE);
                 return true;
               }
             return false;
           }, false);
        add_controller (controller);

        signal_close_request ().connect
          ([this] () -> bool
           {
             m_signal_response.emit (Gtk::ResponseType::CLOSE);
             return true;
           }, false);
      }

    void update_buttons ()
      {
        m_build_road_button->set_sensitive (m_smallmap->check_road ());
        m_clear_road_button->set_sensitive (m_smallmap->is_start_set () ||
                                            m_smallmap->is_finish_set ());
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &SmallmapEditorWindow::execute_action));
        m_actions->add_action ("undo",
           ([this] ()
            {
              m_umgr->undo ();
              m_smallmap->resize ();
              update ();
            }));
        insert_action_group ("win", m_actions);
        auto shortcuts = Gtk::ShortcutController::create();
        shortcuts->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);

        shortcuts->add_shortcut
          (Gtk::Shortcut::create
           (Gtk::ShortcutTrigger::parse_string ("<Control>z"),
            Gtk::NamedAction::create ("win.undo")));
        add_controller (shortcuts);
      }


    void add_action_connection (sigc::connection c)
      {
        m_action_connections.push_back (c);
      }

    UndoAction * execute_action (UndoAction *action2)
      {
        auto *action = dynamic_cast<SmallMapEditorUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case SmallMapEditorUndoAction::TERRAIN:
              {
                auto *a =
                  dynamic_cast<SmallMapEditorUndoAction_Terrain*>(action);
                out =
                  new SmallMapEditorUndoAction_Terrain
                  (get_selected_terrain (), a->get_area ());

                do_change_map (a);
              }
            break;

          case SmallMapEditorUndoAction::ERASE:
              {
                auto a = dynamic_cast<SmallMapEditorUndoAction_Erase*>(action);
                out = new SmallMapEditorUndoAction_Erase (a->get_area ());
                do_change_map (a);
              }
            break;

          case SmallMapEditorUndoAction::CITY:
              {
                auto a = dynamic_cast<SmallMapEditorUndoAction_City*>(action);
                out = new SmallMapEditorUndoAction_City (a->get_area ());
                do_change_map (a);
              }
            break;

          case SmallMapEditorUndoAction::RUIN:
              {
                auto a = dynamic_cast<SmallMapEditorUndoAction_Ruin*>(action);
                out = new SmallMapEditorUndoAction_Ruin (a->get_area ());
                do_change_map (a);
              }
            break;

          case SmallMapEditorUndoAction::TEMPLE:
              {
                auto a = dynamic_cast<SmallMapEditorUndoAction_Temple*>(action);
                out = new SmallMapEditorUndoAction_Temple (a->get_area ());
                do_change_map (a);
              }
            break;

          case SmallMapEditorUndoAction::BUILD_ROAD:
              {
                auto a =
                  dynamic_cast<SmallMapEditorUndoAction_BuildRoads*>(action);
                out =
                  new SmallMapEditorUndoAction_BuildRoads
                  (GameMap::get_boundary (),
                   m_smallmap->get_road_start (),
                   m_smallmap->get_road_finish ());

                do_change_map (a);
                m_smallmap->setRoadStart (a->get_src ());
                m_smallmap->setRoadFinish (a->get_dest ());
              }
            break;

          case SmallMapEditorUndoAction::CLEAR_ROAD:
              {
                auto a =
                  dynamic_cast<SmallMapEditorUndoAction_ClearRoad*>(action);
                out =
                  new SmallMapEditorUndoAction_ClearRoad
                  (m_smallmap->get_road_start (),
                   m_smallmap->get_road_finish ());

                m_smallmap->setRoadStart (a->get_src ());
                m_smallmap->setRoadFinish (a->get_dest ());
              }
            break;

          case SmallMapEditorUndoAction::PLACE_START:
              {
                auto a =
                  dynamic_cast<SmallMapEditorUndoAction_PlaceStart*>(action);
                out =
                  new SmallMapEditorUndoAction_PlaceStart
                  (m_smallmap->get_road_start ());
                m_smallmap->setRoadStart (a->get_pos ());
              }
            break;

          case SmallMapEditorUndoAction::PLACE_FINISH:
              {
                auto a =
                  dynamic_cast<SmallMapEditorUndoAction_PlaceFinish*>(action);
                out =
                  new SmallMapEditorUndoAction_PlaceFinish
                  (m_smallmap->get_road_finish ());
                m_smallmap->setRoadFinish (a->get_pos ());
              }
            break;
          }
        return out;
      }

    void do_change_map (SmallMapEditorUndoAction_ChangeMap *action)
      {
        GameMap::instance ()->updateMaptiles (action->get_map_tiles ());

        if (action->get_only_map_tiles ())
          return;

        GameMap::instance ()->updateObjects (action->get_objects (),
                                             action->get_rectangles ());
        action->clear_objects ();
      }

    int get_selected_terrain ()
      {
        int found = -1;
        int i = 0;

        for (auto it = m_terrain_buttons.rbegin ();
             it != m_terrain_buttons.rend (); ++it, i++)
          {
            if ((*it)->get_active ())
              {
                found = i;
                break;
              }
          }
        return found;
      }
};
#endif
