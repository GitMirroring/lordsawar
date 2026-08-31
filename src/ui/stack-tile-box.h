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

#pragma once
#ifndef STACK_TILE_BOX_H
#define STACK_TILE_BOX_H

#include <sigc++/trackable.h>
#include <gtkmm.h>
#include <glibmm.h>
#include "configuration.h"
#include "player-list.h"
#include "stack-army-button.h"
#include "stack-tile.h"
#include "player.h"
#include "stack.h"
#include "game-map.h"

class StackTile;
class Stack;
class Army;

// shows the listing of army units that coexist on a single map tile.
class StackTileBox: public Gtk::Box
{
public:
    static StackTileBox * create ()
      {
        Glib::RefPtr<Gtk::Builder> xml =
          Gtk::Builder::create_from_resource
          (std::string (RESOURCE) + "stack-tile-box.ui");
        StackTileBox *box =
          Gtk::Builder::get_widget_derived<StackTileBox> (xml, "box");
        return box;
      }

    StackTileBox (BaseObjectType* base, const Glib::RefPtr<Gtk::Builder> &xml)
      : Gtk::Box (base), m_builder (xml)
      {
        m_stack_info_box = xml->get_widget <Gtk::Box> ("stack_info_box");
        m_stack_info_container =
          xml->get_widget <Gtk::Box> ("stack_info_container");
        m_group_moves_label =
          xml->get_widget <Gtk::Label> ("group_moves_label");
        m_group_ungroup_toggle =
          xml->get_widget <Gtk::ToggleButton> ("group_togglebutton");
        m_terrain_image = xml->get_widget<Gtk::Image> ("terrain_image");
      }

    ~StackTileBox ()
      {
      }

    void on_stack_info_changed (Stack *s)
      {
        set_selected_stack (s);

        if (s->getOwner ()->getType () == Player::HUMAN)
          {
            StackTile *stile = GameMap::getStacks (s->getPos ());
            stile->setDefending (s->getOwner (), false);
            stile->setParked (s->getOwner (), false);
            show_stack (stile);
          }
      }

    Stack * get_currently_selected_stack () const
      {
        return m_currently_selected_stack;
      }

    void show_stack (StackTile *s)
      {
        reset_army_buttons ();
        Player *p = Playerlist::getActiveplayer ();
        std::vector<Stack *> stks = s->getFriendlyStacks (p);
        unsigned int count= 0;

        guint32 color_id = 0;
        if (color_id == p->getId ())
          color_id = Shield::get_next_shield (color_id);
        for (auto j = stks.begin (); j != stks.end (); ++j)
          {
            bool first = true;
            for (auto i = (*j)->begin (); i != (*j)->end (); ++i)
              {
                Stack *stack = NULL;
                if (first == true)
                  {
                    first = false;
                    stack = *j;
                  }
                if (count >= MAX_ARMIES_ON_A_SINGLE_TILE)
                  break;

                StackArmyButton *button = m_stack_army_buttons[count];
                button->set (stack, *i, color_id,
                             (*j) == m_currently_selected_stack);
                m_army_conn[count].disconnect ();
                Stack *jstack = *j;
                Army *iarmy = *i;
                m_army_conn[count] = button->signal_army_toggled ().connect
                  ([this, button, jstack, iarmy] ()
                   {
                     on_army_toggled (button, jstack, iarmy);
                   });
                m_stack_conn[count].disconnect ();
                m_stack_conn[count] = button->signal_stack_clicked ().connect
                  ([this, jstack] ()
                   {
                     on_stack_toggled (jstack);
                   });
                count++;
              }

            color_id = Shield::get_next_shield (color_id);
            if (color_id== p->getId ())
              color_id = Shield::get_next_shield (color_id);
          }

        fill_in_group_info (s, m_currently_selected_stack);
      }

    void clear_selected_stack ()
      {
        m_currently_selected_stack = NULL;
      }

    void set_selected_stack (Stack*s)
      {
        m_currently_selected_stack = s;
      }

    void toggle_group_ungroup ()
      {
        m_group_ungroup_toggle->set_active
          (!m_group_ungroup_toggle->get_active ());
      }

    void setup ()
      {
        m_inhibit = false;
        m_group_ungroup_toggle->signal_toggled ().connect
          ([this] ()
           {
             Gtk::ToggleButton *toggle = m_group_ungroup_toggle;
             if (m_inhibit_group_toggle)
               return;
             if (toggle->get_sensitive () == false)
               return;
             bool active = toggle->get_active ();

             StackTile *s =
               GameMap::getStacks (m_currently_selected_stack->getPos ());
             m_stack_tile_group_toggle.emit (true);
             if (active)
               {
                 s->group (Playerlist::getActiveplayer (),
                           m_currently_selected_stack);
                 m_currently_selected_stack->sortForViewing (true);
               }
             else
               s->ungroup (Playerlist::getActiveplayer ());
             m_stack_tile_group_toggle.emit (false);

             on_stack_info_changed (m_currently_selected_stack);
             m_stack_composition_modified.emit (m_currently_selected_stack);
           });

        //okay let's make our army buttons.
        for (unsigned int i = 0; i < MAX_ARMIES_ON_A_SINGLE_TILE; i++)
          {
            StackArmyButton *button = StackArmyButton::create ();
            m_stack_army_buttons.push_back (button);
            m_stack_info_box->append (*button);
            button->setup ();
          }
        m_inhibit_group_toggle = false;
      }

    //! Signals
    sigc::signal<void (Stack*)> signal_stack_composition_modified ()
      {
        return m_stack_composition_modified;
      }

    sigc::signal<void (bool)> signal_stack_tile_group_toggle ()
      {
        return m_stack_tile_group_toggle;
      }

protected:

private:
    Glib::RefPtr<Gtk::Builder> m_builder;
    bool m_inhibit = false;
    Stack *m_currently_selected_stack = NULL;
    typedef std::vector<StackArmyButton *> stack_army_buttons_type;
    stack_army_buttons_type m_stack_army_buttons;
    Gtk::Box *m_stack_info_box = NULL;
    Gtk::Box *m_stack_info_container = NULL;
    Gtk::Label *m_group_moves_label = NULL;
    Gtk::Image *m_terrain_image = NULL;
    Gtk::ToggleButton *m_group_ungroup_toggle = NULL;
    bool m_inhibit_group_toggle = false;
    sigc::signal<void (Stack*)> m_stack_composition_modified;
    sigc::signal<void (bool)> m_stack_tile_group_toggle;

    sigc::connection m_army_conn[MAX_ARMIES_ON_A_SINGLE_TILE];
    sigc::connection m_stack_conn[MAX_ARMIES_ON_A_SINGLE_TILE];

    void fill_in_group_info (StackTile *stile, Stack *s)
      {
        guint32 bonus = s->calculateMoveBonus ();
        ImageCache *gc = ImageCache::instance ();

        auto pixbuf =
          gc->getMoveBonusPic (GameMap::getTileset ()->getId (),
                               bonus)->to_pixbuf ();
        m_terrain_image->set (pixbuf);
        m_terrain_image->set_pixel_size (LW_BUTTON_SIZE);
        m_group_moves_label->set_text (String::ucompose ("%1", s->getMoves ()));
        m_group_ungroup_toggle->set_sensitive (false);
        m_inhibit_group_toggle = true;
        if (stile->getFriendlyStacks (s->getOwner ()).size () != 1)
          m_group_ungroup_toggle->set_active (false);
        else
          m_group_ungroup_toggle->set_active (true);
        if (m_group_ungroup_toggle->get_active () == true)
          m_group_ungroup_toggle->set_label (_("UnGrp"));
        else
          m_group_ungroup_toggle->set_label (_("Grp"));
        m_group_ungroup_toggle->set_sensitive (true);
        m_inhibit_group_toggle = false;
      }

    void on_army_toggled (StackArmyButton *toggle, Stack *stack, Army *army)
      {
        if (m_inhibit || m_inhibit_group_toggle)
          return;
        for (auto i = m_stack_army_buttons.begin (),
             end = m_stack_army_buttons.end (); i != end; ++i)
          (*i)->update_stack_button (toggle == *i);
        Player *p = Playerlist::getActiveplayer ();
        Stack *s = p->getActivestack ();
        m_group_ungroup_toggle->set_sensitive (false);
        if (toggle->get_active () == true)
          {
            if (stack->size () > 1)
              {
                Stack *new_stack = p->stackSplitArmy (stack, army);
                if (new_stack)
                  p->stackJoin (m_currently_selected_stack, new_stack);
              }
            else
              p->stackJoin (m_currently_selected_stack, stack);
            m_currently_selected_stack->sortForViewing (true);
          }
        else
          {
            p->stackSplitArmy (stack, army);
            stack->sortForViewing (true);
          }
        on_stack_info_changed (s);
        m_group_ungroup_toggle->set_sensitive (true);
        m_stack_composition_modified.emit (s);
      }

    void on_stack_toggled (Stack *stack)
      {
        if (m_inhibit || m_inhibit_group_toggle)
          return;
        if (stack == m_currently_selected_stack)
          return;
        m_currently_selected_stack = stack;
        Playerlist::getActiveplayer ()->stackSelect (stack);
        on_stack_info_changed (stack);
        m_stack_composition_modified.emit (stack);
      }

    void on_group_toggled (Gtk::ToggleButton *toggle)
      {
        if (m_inhibit_group_toggle)
          return;
        if (toggle->get_sensitive () == false)
          return;
        bool active = toggle->get_active ();

        StackTile *s = GameMap::getStacks (m_currently_selected_stack->getPos ());
        m_stack_tile_group_toggle.emit (true);
        if (active)
          {
            s->group (Playerlist::getActiveplayer (), m_currently_selected_stack);
            m_currently_selected_stack->sortForViewing (true);
          }
        else
          s->ungroup (Playerlist::getActiveplayer ());
        m_stack_tile_group_toggle.emit (false);

        on_stack_info_changed (m_currently_selected_stack);
        m_stack_composition_modified.emit (m_currently_selected_stack);
      }

    void reset_army_buttons ()
      {
        for (unsigned int i = 0; i < m_stack_army_buttons.size (); i++)
          m_stack_army_buttons[i]->reset ();
        for (unsigned int i = 0; i < MAX_ARMIES_ON_A_SINGLE_TILE; i++)
          {
            m_army_conn[i].disconnect ();
            m_stack_conn[i].disconnect ();
          }
      }
};

#endif
