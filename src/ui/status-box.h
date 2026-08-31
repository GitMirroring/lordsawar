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
#ifndef STATUS_BOX_H
#define STATUS_BOX_H

#include <memory>
#include <sigc++/trackable.h>
#include <gtkmm.h>
#include <glibmm.h>
#include "configuration.h"
#include "sidebar-stats.h"
#include "stack-tile-box.h"
#include "tartan-progress-bar.h"

class StackTileBox;
class StackTile;
// shows stats/progress/stack listing
class StatusBox: public Gtk::Box
{
 public:
    static StatusBox * create ()
      {
        Glib::RefPtr<Gtk::Builder> xml =
          Gtk::Builder::create_from_resource
          (std::string (RESOURCE) + "status-box.ui");
        StatusBox *box =
          Gtk::Builder::get_widget_derived<StatusBox> (xml, "box");
        return box;
      }

    StatusBox (BaseObjectType* base, const Glib::RefPtr<Gtk::Builder> &xml)
      : Gtk::Box(base), m_builder (xml), m_height_fudge_factor (0)
      {
        m_notebook = xml->get_widget <Gtk::Notebook> ("info_notebook");
        m_stats_box = xml->get_widget <Gtk::Box> ("stats_box");
        m_tartan_box = xml->get_widget <Gtk::Box> ("tartan_box");
        m_stack_info_container = xml->get_widget <Gtk::Box> ("stack_info_container");
        m_progress_status_label = xml->get_widget <Gtk::Label> ("progress_status_label");
        m_cities_stats_image = xml->get_widget <Gtk::Image> ("cities_stats_image");
        m_cities_stats_image->set
          (ImageCache::instance ()->getStatusPic
           (ImageCache::STATUS_CITY)->to_pixbuf ());
        m_cities_stats_image->set_pixel_size (LW_BUTTON_SIZE);

        m_gold_stats_image = xml->get_widget <Gtk::Image>("gold_stats_image");
        m_gold_stats_image->set
          (ImageCache::instance ()->getStatusPic
           (ImageCache::STATUS_TREASURY)->to_pixbuf ());
        m_gold_stats_image->set_pixel_size (LW_BUTTON_SIZE);

        m_income_stats_image = xml->get_widget <Gtk::Image>("income_stats_image");
        m_income_stats_image->set
          (ImageCache::instance ()->getStatusPic
           (ImageCache::STATUS_INCOME)->to_pixbuf ());
        m_income_stats_image->set_pixel_size (LW_BUTTON_SIZE);

        m_upkeep_stats_image = xml->get_widget <Gtk::Image>("upkeep_stats_image");
        m_upkeep_stats_image->set
          (ImageCache::instance ()->getStatusPic
           (ImageCache::STATUS_UPKEEP)->to_pixbuf ());
        m_upkeep_stats_image->set_pixel_size (LW_BUTTON_SIZE);

        m_cities_stats_label = xml->get_widget <Gtk::Label>("cities_stats_label");
        m_gold_stats_label = xml->get_widget <Gtk::Label>("gold_stats_label");
        m_income_stats_label = xml->get_widget <Gtk::Label>("income_stats_label");
        m_upkeep_stats_label = xml->get_widget <Gtk::Label>("upkeep_stats_label");
        m_stack_tile_box_container =
          xml->get_widget <Gtk::Box>("stack_tile_box_container");
        m_stack_tile_box = StackTileBox::create ();
        m_stack_tile_box_container->append (*m_stack_tile_box);
        m_stack_tile_box->signal_stack_composition_modified ().connect
          (sigc::mem_fun (m_stack_composition_modified, 
                          &sigc::signal<void(Stack*)>::emit));
        m_stack_tile_box->signal_stack_tile_group_toggle ().connect
          (sigc::mem_fun (m_stack_tile_group_toggle,
                          &sigc::signal<void(bool)>::emit));
        m_turn_progressbar = Gtk::make_managed<TartanProgressBar> ();
        m_turn_progressbar->set_hexpand (true);
        m_tartan_box->append (*m_turn_progressbar);

        set_vexpand (false);
        set_hexpand (true);
        set_halign (Gtk::Align::FILL);
      }

    ~StatusBox ()
      {
      }


    void show_stats ()
      {
        m_notebook->set_current_page (1);
      }

    void enforce_height ()
      {
        auto asl = Armysetlist::instance ();
        int height =
          asl->getTileSize (Playerlist::getActiveplayer ()->getArmyset ());
        height += m_height_fudge_factor;
        height += 30; //button border pixels + radio button height.

        height += LW_BUTTON_SIZE;

        m_stats_box->get_parent ()->set_size_request (-1, height);
      }

    void show_progress ()
      {
        m_notebook->set_current_page (2);
        m_turn_progressbar->queue_draw ();
        if (Playerlist::getActiveplayer () == Playerlist::getNeutral ())
          m_progress_status_label->set_text ("");
        else
          m_progress_status_label->set_text
            (Playerlist::getActiveplayer ()->getName ());
      }

    void show_stack (StackTile *s)
      {
        m_stack_tile_box->show_stack (s);
        m_notebook->set_current_page (0);
      }

    void on_stack_info_changed (Stack *s)
      {
        m_stack_tile_box->set_selected_stack (s);

        if (!s)
          {
            if (Playerlist::getActiveplayer()->getType () == Player::HUMAN)
              show_stats ();
            else
              show_progress ();
          }
        else
          {
            if (s->getOwner ()->getType () == Player::HUMAN)
              {
                StackTile *stile = GameMap::getStacks (s->getPos ());
                stile->setDefending (s->getOwner (), false);
                stile->setParked (s->getOwner (), false);
                show_stack (stile);
              }
            else
              show_progress ();
          }
        return;
      }

    Stack * get_currently_selected_stack () const
      {
        return m_stack_tile_box->get_currently_selected_stack ();
      }

    void reset_progress (Player *p)
      {
        m_turn_progressbar->init_turn (p);
      }

    void clear_selected_stack ()
      {
        m_stack_tile_box->clear_selected_stack();
      }

    void setHeightFudgeFactor (guint32 n)
      {
        m_height_fudge_factor = n;
      }

    void set_progress_label (Glib::ustring s)
      {
        m_progress_status_label->set_text (s);
      }

    void pulse ()
      {
        //warning: pulsing too quickly and can cause crashing bugs.
        m_turn_progressbar->pulse ();
      }

    void update_sidebar_stats (SidebarStats s)
      {
        m_cities_stats_label->set_text (String::ucompose("%1", s.cities));
        m_gold_stats_label->set_text (String::ucompose("%1", s.gold));
        m_income_stats_label->set_text (String::ucompose("%1", s.income));
        m_upkeep_stats_label->set_text (String::ucompose("%1", s.upkeep));

        Glib::ustring tip =
          String::ucompose (ngettext ("You have %1 city!",
                                      "You have %1 cities!", s.cities),
                            s.cities);
        m_cities_stats_image->set_tooltip_text (tip);
        m_cities_stats_label->set_tooltip_text (tip);
        tip =
          String::ucompose (ngettext ("You have %1 gold piece in your treasury!",
                                      "You have %1 gold pieces in your treasury!", 
                                      s.gold), s.gold);
        m_gold_stats_image->set_tooltip_text (tip);
        m_gold_stats_label->set_tooltip_text (tip);
        tip =
          String::ucompose (ngettext ("You earn %1 gold piece in income!",
                                      "You earn %1 gold pieces in income!",
                                      s.income),
                            s.income);
        m_income_stats_image->set_tooltip_text (tip);
        m_income_stats_label->set_tooltip_text (tip);
        tip =
          String::ucompose (ngettext ("You pay %1 gold piece in upkeep!",
                                      "You pay %1 gold pieces in upkeep!",
                                      s.upkeep),
                            s.upkeep);
        m_upkeep_stats_image->set_tooltip_text (tip);
        m_upkeep_stats_label->set_tooltip_text (tip);
      }

    void toggle_group_ungroup ()
      {
        m_stack_tile_box->toggle_group_ungroup ();
      }

    void setup ()
      {
        m_stack_tile_box->setup ();
      }

    //! Signals
    sigc::signal<void(Stack*)> m_stack_composition_modified;
    sigc::signal<void(bool)> m_stack_tile_group_toggle;
 protected:

 private:
    Glib::RefPtr<Gtk::Builder> m_builder;
    StackTileBox *m_stack_tile_box = NULL;
    guint32 m_height_fudge_factor = 0;
    Gtk::Notebook *m_notebook = NULL;
    Gtk::Image *m_cities_stats_image = NULL;
    Gtk::Label *m_cities_stats_label = NULL;
    Gtk::Image *m_gold_stats_image = NULL;
    Gtk::Label *m_gold_stats_label = NULL;
    Gtk::Image *m_income_stats_image = NULL;
    Gtk::Label *m_income_stats_label = NULL;
    Gtk::Image *m_upkeep_stats_image = NULL;
    Gtk::Label *m_upkeep_stats_label = NULL;
    Gtk::Box *m_stack_info_container = NULL;
    Gtk::Box *m_stack_tile_box_container = NULL;
    Gtk::Box *m_stats_box = NULL;
    Gtk::Box *m_tartan_box = NULL;
    TartanProgressBar *m_turn_progressbar = NULL;
    Gtk::Label *m_progress_status_label = NULL;

    void drop_connections ();
};

#endif
