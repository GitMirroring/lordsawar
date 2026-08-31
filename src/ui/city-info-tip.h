//  Copyright (C) 2009, 2011, 2012, 2014, 2015, 2016, 2020, 2026 Ben Asselstine
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
#ifndef CITY_INFO_TIP_H
#define CITY_INFO_TIP_H

#include <gtkmm.h>
#include <memory>
#include "ucompose.hpp"
#include "vector.h"
#include "defs.h"
#include "image-cache.h"
#include "map-tip-position.h"
#include "city.h"
#include "info-tip.h"

// shows a tooltip like window with information about a city

class CityInfoTip : public InfoTip
{
public:
    CityInfoTip ()
      :InfoTip ()
      {
        m_vbox = new Gtk::Box (Gtk::Orientation::VERTICAL);
        m_vbox->set_spacing (3);
        set_child (*m_vbox);

        //row 1
        m_title_hbox = new Gtk::Box (Gtk::Orientation::HORIZONTAL);
        m_title_hbox->set_spacing (3);
        m_vbox->append (*m_title_hbox);

        m_left_image = new Gtk::Image ();
        m_left_image->set_pixel_size (LW_BUTTON_SIZE / 2);
        m_title_hbox->append (*m_left_image);

        m_name_label = new Gtk::Label ();
        m_title_hbox->append (*m_name_label);

        m_right_image = new Gtk::Image ();
        m_right_image->set_pixel_size (LW_BUTTON_SIZE / 2);
        m_title_hbox->append (*m_right_image);

        // row 2
        m_razed_label = new Gtk::Label ("");
        m_vbox->append (*m_razed_label);

        // row 3
        m_incdef_hbox = new Gtk::Box (Gtk::Orientation::HORIZONTAL);
        m_incdef_hbox->set_spacing (12);
        m_incdef_hbox->set_halign (Gtk::Align::CENTER);
        m_vbox->append (*m_incdef_hbox);

        m_inc_hbox = new Gtk::Box (Gtk::Orientation::HORIZONTAL);
        m_incdef_hbox->append (*m_inc_hbox);

        m_def_hbox = new Gtk::Box (Gtk::Orientation::HORIZONTAL);
        m_incdef_hbox->append (*m_def_hbox);

        m_income_image = new Gtk::Image ();
        m_income_image->set_pixel_size (LW_BUTTON_SIZE / 2);
        m_inc_hbox->append (*m_income_image);

        m_income_label = new Gtk::Label ("");
        m_inc_hbox->append (*m_income_label);

        m_defense_image = new Gtk::Image ();
        m_defense_image->set_pixel_size (LW_BUTTON_SIZE / 2);
        m_def_hbox->append (*m_defense_image);

        m_defense_label = new Gtk::Label ("");
        m_def_hbox->append (*m_defense_label);

        // row 4
        m_capital_label = new Gtk::Label ("");
        m_capital_label->set_justify (Gtk::Justification::CENTER);
        m_vbox->append (*m_capital_label);

        // row 5
        m_capital_image = new Gtk::Image ();
        m_capital_image->set_pixel_size (LW_BUTTON_SIZE / 2);
        m_vbox->append (*m_capital_image);
      }

    void set (City *city)
      {
        // row 1
        m_left_image->set
          (ImageCache::instance ()->getShieldPic
           (1, city->getOwner (), false)->to_pixbuf ());

        m_name_label->set_text (city->getName ());

        m_right_image->set
          (ImageCache::instance ()->getShieldPic
           (1, city->getOwner (), false)->to_pixbuf ());

        // row 2
        if (city->isBurnt () == true)
          m_razed_label->set_text (_("Razed!"));

        //row 3
        m_income_image->set
          (ImageCache::instance ()->getStatusPic
           (ImageCache::STATUS_INCOME)->to_pixbuf ());
        m_defense_image->set
          (ImageCache::instance ()->getStatusPic
           (ImageCache::STATUS_DEFENSE)->to_pixbuf ());
        m_income_label->set_text
          (String::ucompose ("%1", city->getGold ()));
        m_defense_label->set_text
          (String::ucompose ("%1", city->getDefenseLevel ()));

        //rows 4 and 5
        if (city->isCapital ())
          {
            m_capital_label->set_text
              (String::ucompose (_("Capital of\n%1"), 
                                 city->getCapitalOwner ()->getName ()));
            m_capital_image->set
              (ImageCache::instance ()->getShieldPic
               (1, city->getCapitalOwner (), false)->to_pixbuf ());
          }
        else
          {
            Glib::RefPtr<Gdk::Pixbuf> empty_pic =
              Gdk::Pixbuf::create (Gdk::Colorspace::RGB, true, 8, 1, 1);
            empty_pic->fill (0x00000000);
            m_capital_image->set (empty_pic);
          }
      }

    ~CityInfoTip ()
      {
        delete m_vbox;
        delete m_title_hbox;
        delete m_razed_label;
        delete m_incdef_hbox;
        delete m_inc_hbox;
        delete m_def_hbox;
        delete m_capital_label;
        delete m_capital_image;
        delete m_left_image;
        delete m_name_label;
        delete m_right_image;
        delete m_income_image;
        delete m_income_label;
        delete m_defense_image;
        delete m_defense_label;
      }
                  
private:
    Gtk::Box *m_vbox = NULL;
    Gtk::Box *m_title_hbox = NULL;
    Gtk::Label *m_razed_label = NULL;
    Gtk::Box *m_incdef_hbox = NULL;
    Gtk::Box *m_inc_hbox = NULL;
    Gtk::Box *m_def_hbox = NULL;
    Gtk::Label *m_capital_label = NULL;
    Gtk::Image *m_capital_image = NULL;
    Gtk::Image *m_left_image = NULL;
    Gtk::Label *m_name_label = NULL;
    Gtk::Image *m_right_image = NULL;
    Gtk::Image *m_income_image = NULL;
    Gtk::Label *m_income_label = NULL;
    Gtk::Image *m_defense_image = NULL;
    Gtk::Label *m_defense_label = NULL;
};
#endif
