//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015, 2026 Ben Asselstine
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
#ifndef HERO_OFFER_DIALOG_H
#define HERO_OFFER_DIALOG_H

#include <gtkmm.h>

#include "player.h"
#include "hero.h"
#include "hero-proto.h"
#include "hero-map.h"
#include "lw-dialog-base.h"
#include "ucompose.hpp"
#include "snd.h"
#include "image-helpers.h"

class HeroOfferDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "hero-offer.ui";
      }

    HeroOfferDialog (BaseObjectType* o,
                    const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_map_drawing_area = load  <Gtk::DrawingArea> ("map_drawing_area");
        m_hero_picture = load  <Gtk::Picture> ("hero_picture");
        m_male_togglebutton = load  <Gtk::ToggleButton> ("male_togglebutton");
        m_female_togglebutton =
          load <Gtk::ToggleButton> ("female_togglebutton");
        m_name_entry = load  <Gtk::Entry> ("name_entry");
        m_accept_button = load  <Gtk::Button> ("accept_button");
        m_close_button = load  <Gtk::Button> ("close_button");
        m_label = load  <Gtk::Label> ("label");
      }

    ~HeroOfferDialog ()
      {
        delete m_hero_map;
      }

    void setup (Player *p, HeroProto *h, City *c, int gold)
      {
        Snd::instance ()->play ("hero", 1);
        set_title (String::ucompose (_("A Hero for %1"), p->getName ()));
        set_response (m_close_button, Gtk::ResponseType::CLOSE);
        set_response (m_accept_button, Gtk::ResponseType::ACCEPT);

        m_close_button->set_sensitive (gold > 0);
        
        m_male_togglebutton->signal_toggled ().connect
          ([this] ()
           {
             update_hero_image ();
           });
        m_female_togglebutton->signal_toggled ().connect
          ([this] ()
           {
             update_hero_image ();
           });

        m_name_entry->set_text (h->getName ());
        m_name_entry->signal_changed ().connect
          ([this] ()
           {
             update_buttons ();
           });

        m_hero_map = new HeroMap (c);
        m_hero_map->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this](double x, double y)
           {
             auto cr = m_hero_map->get_cursor (x, y);
             static ImageCache::CursorType prev_cursor = ImageCache::SHIP;
             if (cr != prev_cursor)
               {
                 auto hotspot = ImageCache::get_hotspot (cr);
                 auto im = ImageCache::instance ()->getCursorPic (cr);
                 auto cursor = Gdk::Cursor::create (im->to_texture (),
                                                    hotspot.x, hotspot.y);
                 m_map_drawing_area->set_cursor (cursor);
               }
             prev_cursor = cr;
           });
        m_map_drawing_area->add_controller (motion);

        m_hero_map->resize ();
        m_hero_map->draw ();

        Glib::ustring s;
        if (gold > 0)
          s = String::ucompose
            (ngettext ("A hero in %2 wants to join you for %1 gold piece!",
                       "A hero in %2 wants to join you for %1 gold pieces!",
                       gold), gold, c->getName ());
        else
          s = String::ucompose (_("A hero in %1 wants to join you!"),
                                c->getName ());
        m_label->set_text (s);

        switch (h->getGender ())
          {
          case Hero::MALE:
            m_male_togglebutton->set_active (true);
            break;
          case Hero::FEMALE:
            m_female_togglebutton->set_active (true);
            break;
          }

        update_buttons ();

        set_default_widget (*m_accept_button);
        m_accept_button->grab_focus ();
        signal_response ().connect
          ([this, h](Gtk::ResponseType response)
           {
             Snd::instance ()->halt ();
             switch (response)
               {
               case Gtk::ResponseType::DELETE_EVENT:
                 //if we close the window and we can't reject the hero
                 if (m_close_button->get_sensitive () == false)
                   accept_hero ();
                 else
                   m_offer_declined.emit ();
                 break;

               case Gtk::ResponseType::ACCEPT:
                 accept_hero ();
                 break;

               default:
                 m_offer_declined.emit ();
                 break;
               }
             hide ();
           });
      }
    
    void update_hero_image ()
      {
        int i = ImageCache::DIALOG_NEW_HERO_FEMALE;
        if (m_male_togglebutton->get_active ())
          i = ImageCache::DIALOG_NEW_HERO_MALE;
        auto im = ImageCache::instance ()->getDialogPic (i);
        m_hero_picture->set_paintable (im->to_texture ());
      }

    sigc::signal<void(Glib::ustring,Hero::Gender)> signal_offer_accepted ()
      {
        return m_offer_accepted;
      }

    sigc::signal<void()> signal_offer_declined ()
      {
        return m_offer_declined;
      }

private:
    HeroMap* m_hero_map = NULL;

    Gtk::DrawingArea *m_map_drawing_area = NULL;
    Gtk::Picture *m_hero_picture = NULL;
    Gtk::ToggleButton *m_male_togglebutton = NULL;
    Gtk::ToggleButton *m_female_togglebutton = NULL;
    Gtk::Entry *m_name_entry = NULL;
    Gtk::Button *m_accept_button = NULL;
    Gtk::Button *m_close_button = NULL;
    Gtk::Label *m_label = NULL;
    
    sigc::signal<void (Glib::ustring, Hero::Gender)> m_offer_accepted;
    sigc::signal<void ()> m_offer_declined;

    void update_buttons ()
      {
        if (String::utrim (m_name_entry->get_text ()) == "")
          m_accept_button->set_sensitive (false);
        else
          m_accept_button->set_sensitive (true);
      }

    void accept_hero ()
      {
        m_offer_accepted.emit (String::utrim (m_name_entry->get_text ()),
                               get_gender ());
      }
        
    Hero::Gender get_gender ()
      {
        if (m_male_togglebutton->get_active ())
          return Hero::MALE;
        return Hero::FEMALE;
      }
};
#endif

