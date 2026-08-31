//  Copyright (C) 2011, 2015, 2020, 2026 Ben Asselstine
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
#ifndef STACK_ARMY_BUTTON_H
#define STACK_ARMY_BUTTON_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include <glibmm.h>
#include "configuration.h"
#include "army-info-tip.h"
#include "image-cache.h"
#include "file.h"
#include "player-list.h"
#include "player.h"
#include "shield.h"
#include "stack.h"
#include "army.h"

// a button-pair.  shows an army button, and maybe another button for the stack.
// the other button is a stack selected/unselected icon.
class StackArmyButton: public Gtk::Box
{
public:

    static StackArmyButton* create ()
      {
        Glib::RefPtr<Gtk::Builder> xml =
          Gtk::Builder::create_from_resource
          (std::string (RESOURCE) + "stack-army-button.ui");
        StackArmyButton *box =
          Gtk::Builder::get_widget_derived<StackArmyButton> (xml, "box");
        return box;
      }

    StackArmyButton (BaseObjectType* base,
                     const Glib::RefPtr<Gtk::Builder> &xml)
      : Gtk::Box (base), m_builder (xml)
      {
        m_army_button = xml->get_widget<Gtk::ToggleButton> ("army_button");
        m_army_image = xml->get_widget<Gtk::Image> ("army_image");
        m_army_label = xml->get_widget<Gtk::Label> ("army_label");
        m_stack_button = xml->get_widget<Gtk::Button> ("stack_button");
        m_stack_image = xml->get_widget<Gtk::Image> ("stack_image");
        m_stack_button_container =
          xml->get_widget<Gtk::Box> ("stack_button_container");
      }

    void setup ()
      {
        auto click = Gtk::GestureClick::create ();
        click->set_button (3);
        click->signal_pressed ().connect
          ([this](int, double x, double y)
           {

             m_army_info_tip->show
               (m_army_button, x, y,
                [this] ()
                {
                  m_army_info_tip->set (m_army);
                });
           });
        m_army_button->add_controller (click);

        auto release = Gtk::GestureClick::create ();
        release->set_button (3);
        release->signal_released ().connect
          ([this](int, double, double)
           {
             m_army_info_tip->popdown ();
           });
        m_army_button->add_controller (release);

        m_army_info_tip = new ArmyInfoTip ();
        m_army_info_tip->set_parent (*m_army_button);
      }


    ~StackArmyButton ()
      {
        if (m_army_info_tip)
          delete m_army_info_tip;
      }

    bool get_active () const
      {
        return m_army_button->get_active ();
      }

    //go back to an empty disabled, untoggled button with circle
    void reset ()
      {
        clear_signals ();
        m_army_button->set_sensitive (true);
        m_stack_button->set_sensitive (true);
        set (NULL, NULL, 0, false);
        m_army_button->set_sensitive (false);
        m_stack_button->set_sensitive (false);
      }

    void set (Stack *s, Army *a, guint32 circle_color_id, bool toggled)
      {
        m_stack = s;
        m_army = a;
        m_circle_color_id = circle_color_id;
        m_army_button->set_sensitive (true);
        m_stack_button->set_sensitive (true);
        if (m_army_button->get_active () != toggled)
          m_army_button->set_active (toggled);

        Player *p = Playerlist::getActiveplayer ();
        ImageCache *gc = ImageCache::instance ();
        if (m_army)
          {
            bool greyed_out = false;
            Stack *active_stack = p->getActivestack ();
            if (active_stack->getArmyById (m_army->getId ()) == NULL)
              greyed_out = true;
            m_army_image->set
              (gc->getCircledArmyPic
               (p->getArmyset (), m_army->getTypeId (), p->get_shield (),
                m_army->getMedalBonuses (), greyed_out, 
                !greyed_out ? p->getId () : m_circle_color_id,
                true, Lw::get_dark ())->to_pixbuf ());

            m_army_label->set_label
              (String::ucompose ("%1", m_army->getMoves ()));
          }
        else
          {
            m_army_image->set
              (gc->getEmptyCircledArmyPic (Lw::get_dark ())->to_pixbuf ());
               //(p->getArmyset (), 0, p, NULL, false, Shield::NEUTRAL,
                //false, Lw::get_dark ())->to_pixbuf ());

            m_stack_image->clear ();
            m_army_label->set_text ("  ");
          }
        m_army_image->set_pixel_size (LW_BUTTON_SIZE);

        update_stack_button
          (m_stack == Playerlist::getActiveplayer ()->getActivestack ());
        setup_signals ();
      }

    void update_stack_button (bool selected)
      {
        if (m_stack)
          {
            m_stack_image->clear ();

            std::string file;
            if (selected)
              file = File::getVariousFile ("army-unit-selected.svg");
            else
              file = File::getVariousFile ("army-unit-unselected.svg");
            m_stack_image->set (file);
            m_stack_button->show ();
          }
        else
          m_stack_button->hide ();
      }

    //Signals
    sigc::signal<void()> signal_stack_clicked ()
      {
        return m_stack_clicked;
      }

    sigc::signal<void()> signal_army_toggled ()
      {
        return m_army_toggled;
      }

private:
    Glib::RefPtr<Gtk::Builder> m_builder;
    Stack *m_stack = NULL;
    Army *m_army = NULL;
    guint32 m_circle_color_id = 0;

    Gtk::ToggleButton *m_army_button = NULL;
    Gtk::Image *m_army_image = NULL;
    Gtk::Label *m_army_label = NULL;
    Gtk::Button *m_stack_button = NULL;
    Gtk::Image *m_stack_image = NULL;
    Gtk::Box *m_stack_button_container = NULL;
    ArmyInfoTip *m_army_info_tip = NULL;

    sigc::connection m_stack_conn;
    sigc::connection m_army_conn;

    sigc::signal<void()> m_stack_clicked;
    sigc::signal<void()> m_army_toggled;

    void setup_signals ()
      {
        clear_signals ();
        m_stack_conn = m_stack_button->signal_clicked ().connect
          (sigc::mem_fun (m_stack_clicked, &sigc::signal<void()>::emit));
        m_army_conn = m_army_button->signal_toggled ().connect
          (sigc::mem_fun (m_army_toggled, &sigc::signal<void()>::emit));
      }

    void clear_signals ()
      {
        m_stack_conn.disconnect ();
        m_army_conn.disconnect ();
      }

};

#endif
