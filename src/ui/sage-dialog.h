//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2017, 2026 Ben Asselstine
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
#pragma once
#ifndef SAGE_DIALOG_H
#define SAGE_DIALOG_H
#include "player.h"
#include "hero.h"
#include "reward.h"
#include "ruin.h"
#include "item.h"
#include "sage.h"
#include "stack.h"
#include "sage-map.h"
#include "lw-dialog-base.h"
#include "ucompose.hpp"
#include "image-helpers.h"
#include "lw-column.h"

class RewardRow: public Glib::Object
{
public:
    Reward *reward;

    static Glib::RefPtr<RewardRow> create (Reward *r)
      {
        return
          Glib::make_refptr_for_instance<RewardRow> (new RewardRow (r));
      }

protected:
    RewardRow (Reward *r)
      : reward (r)
      {
      }
};

class SageDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "sage.ui";
      }

    SageDialog (BaseObjectType* o,
                    const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_map_drawing_area = load  <Gtk::DrawingArea> ("map_drawing_area");
        m_continue_button = load  <Gtk::Button> ("continue_button");
        m_label = load  <Gtk::Label> ("label");
        m_treeview = load  <Gtk::ColumnView> ("reward_treeview");
      }

    ~SageDialog ()
      {
        delete m_sage_map;
      }

    void setup (Sage *sage, Hero *hero, Ruin *ruin)
      {
        m_sage = sage;
        m_hero = hero;
        m_ruin = ruin;
        set_title (_("A Sage!"));
        set_response (m_continue_button, Gtk::ResponseType::ACCEPT);

        m_sage_map = new SageMap (m_ruin, NULL);
        m_sage_map->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });

        m_sage_map->resize ();
        m_sage_map->draw ();

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this](double x, double y)
           {
             auto c = m_sage_map->get_cursor (x, y);
             static ImageCache::CursorType prev_cursor = ImageCache::SHIP;
             if (c != prev_cursor)
               {
                 auto hotspot = ImageCache::get_hotspot (c);
                 auto im = ImageCache::instance ()->getCursorPic (c);
                 auto cursor = Gdk::Cursor::create (im->to_texture (),
                                                    hotspot.x, hotspot.y);
                 m_map_drawing_area->set_cursor (cursor);
               }
             prev_cursor = c;
           });
        m_map_drawing_area->add_controller (motion);

        m_store = Gio::ListStore<RewardRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        setup_column ();

        m_selection_model->signal_selection_changed ().connect
          ([this] (const guint &, const guint &)
           {
             update_target ();
           });

        for (auto r : *sage)
          m_store->append (RewardRow::create (r));

        set_default_widget (*m_continue_button);

        update_target ();

        signal_response ().connect
          ([this](Gtk::ResponseType resp)
           {
             switch (resp)
               {
               case Gtk::ResponseType::ACCEPT:
                 m_sage->selectReward (get_selected_reward ());
                 break;

               default:
                 break;
               }
             hide ();
           });
      }

    Reward * get_selected_reward ()
      {
        auto single_selection =
          std::dynamic_pointer_cast<Gtk::SingleSelection> (m_selection_model);

        auto item = single_selection->get_selected_item ();

        if (item)
          {
            auto row = std::dynamic_pointer_cast<RewardRow> (item);
            return row->reward;
          }
        return NULL;
      }
    
private:
    Gtk::DrawingArea *m_map_drawing_area = NULL;
    Gtk::Button *m_continue_button = NULL;
    Gtk::Label *m_label = NULL;
    Gtk::ColumnView *m_treeview = NULL;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<RewardRow>> m_store;

    SageMap* m_sage_map = NULL;
    Sage *m_sage = NULL;
    Hero *m_hero = NULL;
    Ruin *m_ruin = NULL;

    void setup_column ()
      {
        LwColumn::setup_text_column<RewardRow>
          (m_treeview, "reward_label", true, Gtk::Justification::LEFT,
           _("Rewards"),
           [] (const auto& row)
           {
             Reward *r = row->reward;

             switch (r->getType ())
               {
               case Reward::GOLD:
                 return _("Gold");
                 break;

               case Reward::ITEM:
                   {
                     Reward_Item *item = static_cast<Reward_Item*> (r);
                     return item->getItem ()->getName ();
                   }
                 break;

               case Reward::ALLIES:
                 return _("Allies");
                 break;

               case Reward::MAP:
                   {
                     Reward_Map *m = static_cast<Reward_Map*> (r);
                     return String::capitalize (m->getName ());
                   }
                 break;

               case Reward::RUIN:
                   {
                     Reward_Ruin *rr = static_cast<Reward_Ruin*> (r);
                     Ruin *ruin = rr->getRuin ();
                     Reward *rur = ruin->getReward ();
                     Glib::ustring name = "";
                     if (rur->getType () == Reward::ITEM)
                       {
                         Item *item =
                           static_cast<Reward_Item*>(rur)->getItem ();
                         name = item->getName ();
                       }
                     else if (rur->getType () == Reward::ALLIES)
                       name = _("Allies");
                     else if (rur->getType () == Reward::MAP)
                       name = rur->getName ();
                     return name;
                   }
                 break;
               }

             return Glib::ustring ("");
           });
      }
             
    void update_target ()
      {
        auto r = get_selected_reward ();
        if (r->getType () == Reward::RUIN)
          {
            Reward_Ruin *rr = dynamic_cast<Reward_Ruin*> (r);
            m_sage_map->set_target (rr->getRuin ());
          }
        else
          m_sage_map->set_target (NULL);
        m_sage_map->draw ();
      }
};
#endif

