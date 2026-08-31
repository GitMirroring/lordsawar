//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2020, 2021, 2026 Ben Asselstine
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
#ifndef ARMY_BONUS_DIALOG_H
#define ARMY_BONUS_DIALOG_H
#include "army-set-list.h"
#include "player.h"
#include "game-map.h"
#include "tile-set.h"
#include "army-set.h"
#include "army-set-list.h"
#include "image-cache.h"
#include "player-list.h"
#include "lw-column.h"
class ArmyBonusRow: public Glib::Object
{
public:
    Player *m_player;
    guint32 m_army_type;
    ArmyProto *m_army;

    static Glib::RefPtr<ArmyBonusRow> create (Player *p, guint32 a)
      {
        return
          Glib::make_refptr_for_instance<ArmyBonusRow>
          (new ArmyBonusRow (p, a));
      }

protected:
    ArmyBonusRow (Player *p, guint32 army_type)
      :m_player (p), m_army_type (army_type)
      {
        m_army =
          Armysetlist::instance ()->getArmy (p->getArmyset (), army_type);
      }
};

class ArmyBonusDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "army-bonus.ui";
      }

    ArmyBonusDialog (BaseObjectType* o,
                     const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
        m_button_box = load <Gtk::Box> ("button_box");
      }

    void setup ()
      {
        set_response (m_close_button, Gtk::ResponseType::CLOSE);

        m_store = Gio::ListStore<ArmyBonusRow>::create ();
        m_selection_model = Gtk::NoSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        setup_image_column ();
        setup_name_column ();
        setup_strength_column ();
        setup_moves_column ();
        setup_move_bonus_column ();
        setup_army_bonus_column ();

        fill_player_buttons ();

        signal_response ().connect
          ([this](Gtk::ResponseType)
           {
             hide ();
           });
      }

private:
    Gtk::Button *m_close_button = NULL;
    Gtk::Box *m_button_box = NULL;
    Gtk::ColumnView *m_treeview = NULL;
    Glib::RefPtr<Gtk::NoSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<ArmyBonusRow>> m_store;

    void fill_player_buttons ()
      {
        Gtk::ToggleButton *active_player_toggle = NULL;
        Gtk::ToggleButton *first = NULL;
        for (auto p : *Playerlist::instance ())
          {
            auto toggle = Gtk::make_managed<Gtk::ToggleButton> ();
            auto picture = Gtk::make_managed<Gtk::Picture> ();
            auto im = ImageCache::instance ()->getShieldPic (2, p, false);
            picture->set_paintable (im->to_texture ());

            if (first)
              toggle->set_group (*first);
            if (!first)
              first = toggle;
            toggle->set_child (*picture);
            toggle->signal_toggled ().connect
              ([this, p] ()
               {
                 fill_armies (p);
               });
            if (p == Playerlist::getActiveplayer ())
              active_player_toggle = toggle;
            m_button_box->append (*toggle);
          }
        active_player_toggle->set_active (true);
      }

    void fill_armies (Player *p)
      {
        m_store->remove_all ();
        Armyset *as = Armysetlist::instance ()->get (p->getArmyset ());
        for (auto army_proto : *as)
          if (army_proto->isHero () == false)
            m_store->append (ArmyBonusRow::create (p, army_proto->getId ()));
      }

    void setup_image_column ()
      {
        LwColumn::setup_picture_column<ArmyBonusRow>
          (m_treeview, LW_BUTTON_SIZE, LW_BUTTON_SIZE, "army_image", "",
           [] (const auto& row)
           {
             Player *p = row->m_player;
             auto im = 
               ImageCache::instance ()->getCircledArmyPic
               (p->getArmyset (), row->m_army_type, p->get_shield (), NULL,
                false, p->getId (), true, Lw::get_dark ())->to_texture ();
             return im;
           });
      }

    void setup_name_column ()
      {
        LwColumn::setup_text_column<ArmyBonusRow>
          (m_treeview, "name_label",  false, Gtk::Justification::CENTER,
           _("Name"),
           [] (const auto& row)
           {
             return row->m_army->getName ();
           });
      }

    void setup_strength_column ()
      {
        LwColumn::setup_text_column<ArmyBonusRow>
          (m_treeview, "strength_label", false, Gtk::Justification::CENTER,
           _("Str"),
           [] (const auto& row)
           {
             return String::ucompose ("%1", row->m_army->getStrength ());
           });
      }

    void setup_moves_column ()
      {
        LwColumn::setup_text_column<ArmyBonusRow>
          (m_treeview, "moves_label", false, Gtk::Justification::CENTER,
           _("Move"),
           [] (const auto& row)
           {
             return String::ucompose ("%1", row->m_army->getMaxMoves ());
           });
      }

    void setup_move_bonus_column ()
      {
        LwColumn::setup_picture_column<ArmyBonusRow>
          (m_treeview, LW_BUTTON_SIZE, LW_BUTTON_SIZE, "move_bonus_image", "",
           [] (const auto& row)
           {
             guint32 b = row->m_army->getMoveBonus ();
             auto im =
               ImageCache::instance ()->getMoveBonusPic
               (GameMap::getTileset ()->getId (), b)->to_texture ();
             return im;
           });
      }

    void setup_army_bonus_column ()
      {
        LwColumn::setup_text_column<ArmyBonusRow>
          (m_treeview, "army_bonus_label", false, Gtk::Justification::LEFT,
           _("Bonus"),
           [] (const auto& row)
           {
             return row->m_army->getArmyBonusDescription (true);
           });
      }

};
#endif
