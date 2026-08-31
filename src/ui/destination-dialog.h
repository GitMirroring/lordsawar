//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2012, 2014, 2017, 2020,
//  2026 Ben Asselstine
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
#include "lw-dialog-base.h"
#ifndef DESTINATION_DIALOG_H
#define DESTINATION_DIALOG_H
#include "input-events.h"
#include "ucompose.hpp"
#include "defs.h"
#include "army.h"
#include "city.h"
#include "army-prod-base.h"
#include "city-list.h"
#include "image-cache.h"
#include "vectored-unit-list.h"
#include "vectored-unit.h"
#include "shield.h"
#include "player-list.h"

class DestinationDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "destination.ui";
      }

    DestinationDialog (BaseObjectType* o,
                       const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_map_drawing_area = load < Gtk::DrawingArea> ("map_drawing_area");
        m_see_all_toggle = load <Gtk::ToggleButton> ("see_all_togglebutton");
        m_vector_toggle = load <Gtk::ToggleButton> ("vector_togglebutton");
        m_change_toggle = load <Gtk::ToggleButton> ("change_togglebutton");
        m_current_label = load <Gtk::Label> ("current_label");
        m_current_image = load <Gtk::Image> ("current_image");
        m_turns_label = load <Gtk::Label> ("turns_label");
        m_description_label = load <Gtk::Label> ("description_label");
        m_one_turn_away_image = load <Gtk::Image> ("one_turn_away_image");
        m_two_turns_away_image = load <Gtk::Image> ("two_turns_away_image");
        m_next_turn_1_image = load <Gtk::Image> ("next_turn_1_image");
        m_next_turn_2_image = load <Gtk::Image> ("next_turn_2_image");
        m_next_turn_3_image = load <Gtk::Image> ("next_turn_3_image");
        m_next_turn_4_image = load <Gtk::Image> ("next_turn_4_image");
        m_turn_after_1_image = load <Gtk::Image> ("turn_after_1_image");
        m_turn_after_2_image = load <Gtk::Image> ("turn_after_2_image");
        m_turn_after_3_image = load <Gtk::Image> ("turn_after_3_image");
        m_turn_after_4_image = load <Gtk::Image> ("turn_after_4_image");
      }

    void setup (City *c, bool *see_all)
      {
        m_city = c;
        m_see_all = see_all;

        set_default_widget (*m_close_button);

        m_current_image->set_pixel_size (LW_BUTTON_SIZE);
        m_one_turn_away_image->set_pixel_size (LW_BUTTON_SIZE);
        m_two_turns_away_image->set_pixel_size (LW_BUTTON_SIZE);
        m_next_turn_1_image->set_pixel_size (LW_BUTTON_SIZE);
        m_next_turn_2_image->set_pixel_size (LW_BUTTON_SIZE);
        m_next_turn_3_image->set_pixel_size (LW_BUTTON_SIZE);
        m_next_turn_4_image->set_pixel_size (LW_BUTTON_SIZE);
        m_turn_after_1_image->set_pixel_size (LW_BUTTON_SIZE);
        m_turn_after_2_image->set_pixel_size (LW_BUTTON_SIZE);
        m_turn_after_3_image->set_pixel_size (LW_BUTTON_SIZE);
        m_turn_after_4_image->set_pixel_size (LW_BUTTON_SIZE);

        set_response (m_close_button, Gtk::ResponseType::CLOSE);

        m_vectormap =
          new VectorMap (m_city, VectorMap::SHOW_ORIGIN_CITY_VECTORING, false);

        m_vectormap->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             if (m_vectormap->getClickAction () == VectorMap::CLICK_SELECTS &&
                 m_vector_toggle->get_active () == true)
               m_vector_toggle->set_active (false);
             else if (m_vectormap->getClickAction () ==
                      VectorMap::CLICK_SELECTS &&
                      m_change_toggle->get_active () == true)
               m_change_toggle->set_active (false);
             else
               {
                 m_city = m_vectormap->getCity ();
                 fill_in_vectoring_info ();
               }
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });

        m_vectormap->resize ();
        m_vectormap->draw ();

        m_see_all_toggle->signal_toggled ().connect
          ([this] ()
           {
             update_toggle_color (m_see_all_toggle);

             *m_see_all = m_see_all_toggle->get_active ();
             if (*m_see_all)
               m_vectormap->setShowVectoring (VectorMap::SHOW_ALL_VECTORING);
             else
               m_vectormap->setShowVectoring
                 (VectorMap::SHOW_ORIGIN_CITY_VECTORING);
             m_vectormap->draw ();
           });

        m_vector_toggle->signal_toggled ().connect
          ([this] ()
           {
             update_toggle_color (m_vector_toggle);
             // the idea here is that we click on the toggle,
             // and then after we click on the map, it gets untoggled
             // we act when it's untoggled.
             if (m_vector_toggle->get_active () == false)
               {
                 m_vectormap->setClickAction (VectorMap::CLICK_SELECTS);
                 m_vectormap->draw ();
                 fill_in_vectoring_info ();
               }
             else
               {
                 m_vectormap->setClickAction (VectorMap::CLICK_VECTORS);
                 m_vectormap->draw ();
               }
           });

        m_change_toggle->signal_toggled ().connect
          ([this] ()
           {
             update_toggle_color (m_change_toggle);
             // the idea here is that we click on the toggle,
             // and then after we click on the map, it gets untoggled
             // we act when it's untoggled.
             if (m_change_toggle->get_active () == false)
               {
                 m_vectormap->setClickAction (VectorMap::CLICK_SELECTS);
                 m_vectormap->draw ();
                 fill_in_vectoring_info ();
               }
             else
               {
                 m_vectormap->setClickAction (VectorMap::CLICK_CHANGES_DESTINATION);
                 m_vectormap->draw ();
               }
           });

        auto click = Gtk::GestureClick::create ();
        click->set_button (1);
        click->signal_pressed ().connect
          ([this] (int n, double x, double y)
           {
             struct MouseButtonEvent ev = to_input_event (n, x, y, true);
             m_vectormap->mouse_button_event (ev);
             m_city = m_vectormap->getCity ();
             fill_in_vectoring_info ();
           });

        click->signal_released ().connect
          ([this] (int n, double x, double y)
           {
             struct MouseButtonEvent ev = to_input_event (n, x, y, false);
             m_vectormap->mouse_button_event (ev);
             m_city = m_vectormap->getCity ();
             fill_in_vectoring_info ();
           });
        m_map_drawing_area->add_controller (click);

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this](double x, double y)
           {
             auto cr = m_vectormap->get_cursor (x, y);
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

        fill_in_vectoring_info();

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }
private:
    Gtk::Button *m_close_button;
    Gtk::DrawingArea *m_map_drawing_area;
    Gtk::ToggleButton *m_see_all_toggle;
    Gtk::ToggleButton *m_vector_toggle;
    Gtk::ToggleButton *m_change_toggle;
    Gtk::Label *m_current_label;
    Gtk::Image *m_current_image;
    Gtk::Label *m_turns_label;
    Gtk::Label *m_description_label;
    Gtk::Image *m_one_turn_away_image;
    Gtk::Image *m_two_turns_away_image;
    Gtk::Image *m_next_turn_1_image;
    Gtk::Image *m_next_turn_2_image;
    Gtk::Image *m_next_turn_3_image;
    Gtk::Image *m_next_turn_4_image;
    Gtk::Image *m_turn_after_1_image;
    Gtk::Image *m_turn_after_2_image;
    Gtk::Image *m_turn_after_3_image;
    Gtk::Image *m_turn_after_4_image;
    
    VectorMap* m_vectormap;
    City *m_city;
    bool *m_see_all;

    void update_toggle_color (Gtk::ToggleButton *toggle)
      {
        if (toggle->get_active ())
          toggle->add_css_class ("red-text");
        else
          toggle->remove_css_class ("red-text");
      }

    void update_description (std::list<VectoredUnit*> vectored)
      {
        if (vectored.empty () == true)
          {
            if (m_city->getVectoring () != Vector<int>(-1,-1))
              {
                City *c =
                  Citylist::instance ()->getObjectAt (m_city->getVectoring ());
                int turns =
                  VectoredUnit::get_travel_turns (m_city->getPos (),
                                                  m_city->getVectoring ());
                /* note to translators:
                   "standard" is the hero's flag that can be vectored to.
                   it is a flag that can be carried into battle.
                   e.g. a battle standard. */
                m_description_label->set_text
                  (String::ucompose (_("+%1t to arrive at %2"),
                                     turns, c ? c->getName () : _("standard")));
              }
            else
              m_description_label->set_text ("");
          }
        else
          {
            Vector<int> t = vectored.front ()->getDestination ();
            City *c = Citylist::instance ()->getObjectAt (t);
            int turns = vectored.front ()->getDuration ();
            m_description_label->set_text
              (String::ucompose (_("+%1t to arrive at %2"), turns,
                                 c ? c->getName() : _("standard")));
          }
      }
    void fill_in_vectoring_info()
      {
        ImageCache *gc = ImageCache::instance ();
        std::list<VectoredUnit*> vectored;
        VectoredUnitlist *vul = VectoredUnitlist::instance ();
        set_title (m_city->getName ());

        Player *player = m_city->getOwner ();
        unsigned int as = player->getArmyset ();
        Glib::RefPtr<Gdk::Pixbuf> pic;
        int slot = m_city->getActiveProductionSlot ();
        Glib::RefPtr<Gdk::Pixbuf> s = 
          gc->getCircledArmyPic (as, 0, player->get_shield (), NULL, false,
                                 Shield::NEUTRAL, true,
                                 Lw::get_dark ())->to_pixbuf();
        Glib::RefPtr<Gdk::Pixbuf> empty_pic =
          gc->getCircledArmyPic (as, 0, player->get_shield (), NULL, false,
                                 Shield::NEUTRAL, false,
                                 Lw::get_dark ())->to_pixbuf();

        m_vector_toggle->set_sensitive (slot != -1 ? true : false);

        Citylist *cl = Citylist::instance ();
        bool target = cl->isVectoringTarget (m_city);
        m_change_toggle->set_sensitive (target);

        m_one_turn_away_image->set (empty_pic);
        m_two_turns_away_image->set (empty_pic);
        m_next_turn_1_image->set (empty_pic);
        m_next_turn_2_image->set (empty_pic);
        m_next_turn_3_image->set (empty_pic);
        m_next_turn_4_image->set (empty_pic);
        m_turn_after_1_image->set (empty_pic);
        m_turn_after_2_image->set (empty_pic);
        m_turn_after_3_image->set (empty_pic);
        m_turn_after_4_image->set (empty_pic);

        Glib::ustring s1;
        Glib::ustring s4 = _("Current:");

        vul->getVectoredUnitsComingFrom (m_city->getPos (), vectored);
        if (slot == -1)
          {
            pic = empty_pic;
            m_turns_label->set_text ("");
            m_description_label->set_text ("");
          }
        else
          {
            const ArmyProdBase* a = m_city->getProductionBase (slot);
            pic = gc->getCircledArmyPic (as, a->getTypeId (),
                                         player->get_shield (), NULL,
                                         false, Shield::NEUTRAL,
                                         true, Lw::get_dark ())->to_pixbuf ();
            s1 = String::ucompose (_("%1t"), m_city->getDuration ());
            m_turns_label->set_text (s1);
            update_description (vectored);
          }

        m_current_image->set (pic);
        m_current_label->set_text (s4);

        //show the units that have been vectored from this city
        for (auto it = vectored.begin(); it != vectored.end(); ++it)
          {
            int armytype = (*it)->getArmy ()->getTypeId ();
            if ((*it)->getDuration () == 2)
              {
                pic = gc->getCircledArmyPic (as, armytype,
                                             player->get_shield (), NULL, false,
                                             Shield::NEUTRAL,
                                             true,
                                             Lw::get_dark ())->to_pixbuf();
                m_one_turn_away_image->set (pic);
              }
            else if ((*it)->getDuration () == 1)
              {
                pic = gc->getCircledArmyPic (as, armytype,
                                             player->get_shield (), NULL, false,
                                             Shield::NEUTRAL,
                                             true,
                                             Lw::get_dark ())->to_pixbuf ();
                m_two_turns_away_image->set (pic);
              }
          }

        //show the units that are arriving into this city
        vectored.clear ();
        vul->getVectoredUnitsGoingTo (m_city, vectored);
        int count = 0;
        Gtk::Image *image = m_next_turn_1_image;
        for (auto it = vectored.begin (); it != vectored.end (); ++it)
          {
            if ((*it)->getDuration () != 1)
              continue;

            switch (count)
              {
              case 0: image = m_next_turn_1_image; break;
              case 1: image = m_next_turn_2_image; break;
              case 2: image = m_next_turn_3_image; break;
              case 3: image = m_next_turn_4_image; break;
              }
            pic = 
              gc->getCircledArmyPic (as, (*it)->getArmy ()->getTypeId (),
                                     player->get_shield (), NULL, false,
                                     Shield::NEUTRAL, true,
                                     Lw::get_dark ())->to_pixbuf ();
            image->set (pic);
            count++;
          }
        count = 0;
        for (auto it = vectored.begin (); it != vectored.end (); ++it)
          {
            if ((*it)->getDuration () != 2)
              continue;
            switch (count)
              {
              case 0: image = m_turn_after_1_image; break;
              case 1: image = m_turn_after_2_image; break;
              case 2: image = m_turn_after_3_image; break;
              case 3: image = m_turn_after_4_image; break;
              }
            pic = 
              gc->getCircledArmyPic (as, (*it)->getArmy ()->getTypeId (),
                                     player->get_shield (), NULL, false,
                                     Shield::NEUTRAL, true,
                                     Lw::get_dark ())->to_pixbuf ();
            image->set (pic);
            count++;
          }
      }
};
#endif
