//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2012, 2014, 2015, 2016, 2020,
//  2021, 2026 Ben Asselstine
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
#ifndef STACK_INFO_TIP_H
#define STACK_INFO_TIP_H

#include <gtkmm.h>
#include <memory>
#include "ucompose.hpp"
#include "vector.h"
#include "image-cache.h"
#include "stack.h"
#include "army.h"
#include "player-list.h"

// shows a tooltip like window with information about an army

class StackInfoTip : public Gtk::Popover
{
public:
    StackInfoTip ()
      {
        set_has_arrow (false);

        m_vbox = new Gtk::Box (Gtk::Orientation::VERTICAL);
        set_child (*m_vbox);

        //row 1
        m_hero_label = new Gtk::Label ("");
        m_hero_label->set_justify (Gtk::Justification::CENTER);
        m_hero_label->set_halign (Gtk::Align::FILL);
        m_vbox->append (*m_hero_label);

        //row 2
        m_image_hbox = new Gtk::Box (Gtk::Orientation::HORIZONTAL);
        m_vbox->append (*m_image_hbox);

        //row 3
        m_hero_description_label = new Gtk::Label ("");
        m_hero_description_label->set_justify (Gtk::Justification::CENTER);
        m_hero_description_label->set_halign (Gtk::Align::FILL);
        m_vbox->append (*m_hero_description_label);

        set_autohide (false);
        set_position (Gtk::PositionType::TOP);
      }

    ~StackInfoTip ()
      {
        for (auto i : m_images)
          delete i;
        delete m_image_hbox;
        delete m_hero_label;
        delete m_hero_description_label;
        delete m_vbox;
      }

    void get_hero (StackTile *stile, Glib::ustring &name, Glib::ustring &desc)
      {
        std::vector<Stack *> stacks = stile->getStacks ();
        std::list<Hero*> heroes;
        for (auto s : stacks)
          {
            if (s->getStrongestHero ())
              {
                Hero *hero = dynamic_cast<Hero*>(s->getStrongestHero ());
                heroes.push_back (hero);
              }
          }

        auto active = Playerlist::getActiveplayer ();
        for (auto hero : heroes)
          {
            if (hero->getOwner () == active)
              {
                name = hero->getName ();
                desc = hero->getDescription ();
                return;
              }
          }

        if (heroes.empty () == false)
          {
            name = heroes.front ()->getName ();
            desc = heroes.front ()->getDescription ();
          }
      }

    void set (StackTile *stile)
      {
        Glib::ustring hero_name = "", hero_description = "";
        get_hero (stile, hero_name, hero_description);

        auto active = Playerlist::getActiveplayer ();
        std::vector<Stack *> stacks = stile->getStacks ();
        for (auto stack : stacks)
          {
            for (auto army : *stack)
              {
                if (army->getOwner () != active && 
                    GameScenarioOptions::s_see_opponents_stacks == false)
                  continue;

                Gtk::Image *image = new Gtk::Image ();
                image->set_pixel_size (LW_BUTTON_SIZE);
                PixMask *armypic =
                  ImageCache::instance ()->getDialogArmyPic (army)->copy ();
                image->set (armypic->to_pixbuf ());
                delete armypic;
                m_images.push_back (image);
                m_image_hbox->append (*image);
              }
          }
        if (m_images.empty ())
          m_image_hbox->set_visible (false);

        if (hero_name != "")
          {
            m_hero_label->set_text (hero_name);
            m_hero_description_label->set_text (hero_description);
          }
        else
          {
            m_hero_label->set_visible (false);
            m_hero_description_label->set_visible (false);
          }
      }

    void set_justification (int justification)
      {
        switch (justification)
          {
          case MapTipPosition::LEFT:
            set_position (Gtk::PositionType::RIGHT);
            break;
          case MapTipPosition::RIGHT:
            set_position (Gtk::PositionType::LEFT);
            break;
          case MapTipPosition::TOP:
            set_position (Gtk::PositionType::BOTTOM);
            break;
          case MapTipPosition::BOTTOM:
            set_position (Gtk::PositionType::TOP);
            break;
          }
      }

private:
    Gtk::Box *m_vbox = NULL;
    Gtk::Box *m_image_hbox = NULL;
    Gtk::Label *m_hero_label = NULL;
    Gtk::Label *m_hero_description_label = NULL;
    std::list<Gtk::Image*> m_images = {};
};
#endif
