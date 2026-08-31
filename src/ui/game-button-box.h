//  Copyright (C) 2011, 2014, 2015, 2020, 2026 Ben Asselstine
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
#ifndef GAME_BUTTON_BOX_H
#define GAME_BUTTON_BOX_H
#include "image-cache.h"
class GameButtonBox: public Gtk::Grid
{
public:

    void
    disable_end_turn_button ()
      {
        m_end_turn_button->set_sensitive (false);
      }

    GameButtonBox ()
      {
        set_row_spacing (6);
        set_column_spacing (6);
        set_halign (Gtk::Align::CENTER);
        set_margin_top (12);
        set_vexpand (true);
        set_valign (Gtk::Align::START);

        m_move_button = Gtk::make_managed<Gtk::Button> ();
        setup_button
          (m_move_button,
           ImageCache::STACK_MOVE,
           "lw.order.move",
           _("Move this stack along its path."));

        m_next_movable_button = Gtk::make_managed<Gtk::Button> ();
        setup_button
          (m_next_movable_button,
           ImageCache::NEXT_MOVABLE_STACK,
           "lw.order.next",
           _("Select the next stack that can move."));

        m_park_button = Gtk::make_managed<Gtk::Button> ();
        setup_button
          (m_park_button,
           ImageCache::STACK_PARK,
           "lw.order.stay-here",
           _("Finish moving this stack."));

        m_deselect_button = Gtk::make_managed<Gtk::Button> ();
        setup_button
          (m_deselect_button,
           ImageCache::STACK_DESELECT,
           "lw.order.deselect",
           _("Deselect this stack."));

        m_move_all_button = Gtk::make_managed<Gtk::Button> ();
        setup_button
          (m_move_all_button,
           ImageCache::MOVE_ALL_STACKS,
           "lw.order.move-all",
           _("Move all stacks along their routes."));

        m_center_button = Gtk::make_managed<Gtk::Button> ();
        setup_button
          (m_center_button,
           ImageCache::CENTER_ON_STACK,
           "lw.view.center",
           _("Center the map on this stack."));

        m_diplomacy_button = Gtk::make_managed<Gtk::Button> ();
        setup_button
          (m_diplomacy_button,
           ImageCache::DIPLOMACY_NO_PROPOSALS,
           "lw.view.diplomacy",
           _("Diplomacy"));

        m_defend_button = Gtk::make_managed<Gtk::Button> ();
        setup_button
          (m_defend_button,
           ImageCache::STACK_DEFEND,
           "lw.order.defend",
           _("Put this stack in a defensive posture. (takes 1 turn)"));

        m_search_button = Gtk::make_managed<Gtk::Button> ();
        setup_button
          (m_search_button,
           ImageCache::STACK_SEARCH,
           "lw.hero.search",
           _("Search a temple or ruin."));

        m_end_turn_button = Gtk::make_managed<Gtk::Button> ();
        setup_button
          (m_end_turn_button,
           ImageCache::END_TURN,
           "lw.turn.end-turn",
           _("End your turn."));

        int row = 0;
        attach (*m_move_button, 0, row);
        attach (*m_next_movable_button, 1, row);
        attach (*m_park_button, 2, row);
        attach (*m_deselect_button, 3, row);
        attach (*m_move_all_button, 4, row);

        row++;
        attach (*m_center_button, 0, row);
        attach (*m_diplomacy_button, 1, row);
        attach (*m_defend_button, 2, row);
        attach (*m_search_button, 3, row);
        attach (*m_end_turn_button, 4, row);
      }

    void update_diplomacy_button (bool new_proposals)
      {
        int image_id = ImageCache::DIPLOMACY_NO_PROPOSALS;
        if (new_proposals)
          image_id = ImageCache::DIPLOMACY_NEW_PROPOSALS;

        auto pixmask = ImageCache::instance ()->getGameButtonPic (image_id);
        auto image = Gtk::make_managed<Gtk::Image> (pixmask->to_texture ());
        image->set_pixel_size (LW_BUTTON_SIZE);
        m_diplomacy_button->set_child (*image);
      }

    ~GameButtonBox ()
      {
      }

private:
    Gtk::Button *m_move_button;
    Gtk::Button *m_next_movable_button;
    Gtk::Button *m_park_button;
    Gtk::Button *m_deselect_button;
    Gtk::Button *m_move_all_button;
    Gtk::Button *m_center_button;
    Gtk::Button *m_diplomacy_button;
    Gtk::Button *m_defend_button;
    Gtk::Button *m_search_button;
    Gtk::Button *m_end_turn_button;

    void setup_button (Gtk::Button *button, int image_id,
                       Glib::ustring action, Glib::ustring tooltip)
      {
        auto pixmask = ImageCache::instance ()->getGameButtonPic (image_id);
        auto image = Gtk::make_managed<Gtk::Image> (pixmask->to_texture ());
        image->set_pixel_size (LW_BUTTON_SIZE);
        button->set_size_request (LW_BUTTON_SIZE, LW_BUTTON_SIZE);
        image->set_margin (3);
        button->set_tooltip_text (tooltip);
        button->set_action_name (action);
        button->set_child (*image);
      }
};
#endif
