//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2026 Ben Asselstine
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
#ifndef ARMY_INFO_TIP_H
#define ARMY_INFO_TIP_H

#include <gtkmm.h>
#include <memory>
#include "ucompose.hpp"
#include "vector.h"
#include "defs.h"
#include "army.h"
#include "army-prod-base.h"
#include "army-proto.h"
#include "image-cache.h"
#include "player-list.h"
#include "city.h"
#include "file.h"
#include "shield.h"
#include "game-map.h"
#include "tile-set.h"
#include "army-set-list.h"
#include "info-tip.h"

// shows a tooltip like window with information about an army

class ArmyInfoTip : public InfoTip
{
public:
    ArmyInfoTip ()
      :InfoTip ()
      {
        m_hbox = new Gtk::Box ();
        m_hbox->set_orientation (Gtk::Orientation::HORIZONTAL);
        set_child (*m_hbox);

        m_vbox = new Gtk::Box ();
        m_vbox->set_orientation (Gtk::Orientation::VERTICAL);
        m_vbox->set_spacing (3);

        m_army_image = new Gtk::Image ();
        m_vbox->append (*m_army_image);
        m_terrain_image = new Gtk::Image ();
        m_vbox->append (*m_terrain_image);

        m_hbox->append (*m_vbox);

        m_info_label = new Gtk::Label ("");
        m_hbox->append (*m_info_label);

        m_hbox->set_spacing (3);
      }

    ~ArmyInfoTip ()
      {
        delete m_hbox;
        delete m_vbox;
        delete m_army_image;
        delete m_terrain_image;
        delete m_info_label;
      }

    void set (const Army *army)
      {
        Glib::ustring s = army->getName ();
        s += "\n";
        // note to translators: %1 is ranged strength
        s += String::ucompose (_("Strength: %1"),
                               army->getStat (Army::STRENGTH));
        s += "\n";

        // note to translators: %1 is remaining moves, %2 is total moves
        s += String::ucompose (_("Moves: %1/%2"),
                               army->getMoves (), army->getStat (Army::MOVES));
        s += "\n";
        s += String::ucompose (_("Upkeep: %1"), army->getUpkeep ());

        set (ImageCache::instance ()->getCircledArmyPic
             (army->getArmyset (),
              army->getTypeId (),
              army->getOwner ()->get_shield (),
              army->getMedalBonuses (),
              false, Shield::NEUTRAL,
              true, Lw::get_dark ())->to_pixbuf (),
             army->getMoveBonus (), s);
      }

    void set (const ArmyProdBase *army, City *city)
      {
        Glib::ustring s = army->getName ();
        s += "\n";
        // note to translators: %1 is melee strength
        s += String::ucompose (_("Strength: %1"),
                               army->getStrength ());
        s += "\n";
        // note to translators: %1 is total moves
        s += String::ucompose (_("Moves: %1"), army->getMaxMoves ());
        s += "\n";
        s += String::ucompose (_("Time: %1"), army->getProduction ());
        s += "\n";
        s += String::ucompose (_("Cost: %1"), army->getProductionCost ());

        set (ImageCache::instance ()->getCircledArmyPic
             (army->getArmyset (),
              army->getTypeId (),
              city->getOwner ()->get_shield (), NULL,
              false, Shield::NEUTRAL,
              true, Lw::get_dark ())->to_pixbuf (),
             army->getMoveBonus (), s);
      }

    void set (const ArmyProto *army)
      {
        Glib::ustring s = army->getName ();
        s += "\n";
        s += String::ucompose (_("Strength: %1"),
                               army->getStrength ());
        s += "\n";
        s += String::ucompose (_("Movement: %1"), army->getMaxMoves ());
        s += "\n";
        s += String::ucompose (_("Time: %1"), army->getProduction ());
        s += "\n";
        s += String::ucompose (_("Cost: %1"), army->getUpkeep ());

        Player *p = Playerlist::instance ()->getActiveplayer ();
        set (ImageCache::instance ()->getCircledArmyPic
             (army->getArmyset (),
              army->getId (), p->get_shield (), NULL,
              false, Shield::NEUTRAL,
              true, Lw::get_dark ())->to_pixbuf (),
             army->getMoveBonus (), s);
      }

    void show (Gtk::ToggleButton *toggle, double x, double y, sigc::slot<void()> set_army)
      {
        if (get_parent ())
          unparent ();
        set_parent (*toggle);
        Gdk::Rectangle r ((int)x, (int)y, 1, 1);
        r.set_y (r.get_y () - (LW_BUTTON_SIZE / 2));
        set_pointing_to (r);
        set_army ();
        popup ();
      }
private:
    Gtk::Box *m_hbox;
    Gtk::Box *m_vbox;
    Gtk::Image *m_army_image;
    Gtk::Image *m_terrain_image;
    Gtk::Label *m_info_label;

    void set (Glib::RefPtr<Gdk::Pixbuf> pixbuf, guint32 move_bonus, Glib::ustring info)
    {
      m_army_image->set (pixbuf);
      ImageCache *gc = ImageCache::instance ();
      m_terrain_image->set 
        (gc->getMoveBonusPic (GameMap::getTileset ()->getId (),
                              move_bonus)->to_pixbuf ());
      m_info_label->set_text (info);
      m_army_image->set_pixel_size (LW_BUTTON_SIZE);
      m_terrain_image->set_pixel_size (LW_BUTTON_SIZE);

    }

};
#endif
