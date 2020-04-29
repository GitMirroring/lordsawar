//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2012, 2014, 2015, 2016,
//  2020 Ben Asselstine
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <config.h>

#include <gtkmm.h>

#include "army-info-tip.h"
#include "builder-cache.h"

#include "ucompose.hpp"
#include "vector.h"
#include "defs.h"
#include "army.h"
#include "armyprodbase.h"
#include "armyproto.h"
#include "ImageCache.h"
#include "playerlist.h"
#include "city.h"
#include "File.h"
#include "shield.h"
#include "font-size.h"
#include "GameMap.h"
#include "tileset.h"

void ArmyInfoTip::init (Gtk::Widget *target, Glib::RefPtr<Gdk::Pixbuf> image, guint32 move_bonus, Glib::ustring info)
{
  Glib::RefPtr<Gtk::Builder> xml = BuilderCache::get("army-info-window.ui");

  xml->get_widget("window", window);
  Gtk::Widget *w = target->get_ancestor (GTK_TYPE_WINDOW);
  if (w)
    window->set_transient_for (*dynamic_cast<Gtk::Window*>(w));
  else
    {
      w = target->get_ancestor (GTK_TYPE_DIALOG);
      if (w)
        window->set_transient_for (*dynamic_cast<Gtk::Dialog*>(w));
    }
  Gtk::Image *army_image;
  xml->get_widget("army_image", army_image);
  army_image->property_pixbuf() = image;
  ImageCache *gc = ImageCache::getInstance();
  Gtk::Image *terrain_image;
  xml->get_widget("terrain_image", terrain_image);
  terrain_image->property_pixbuf() =
    gc->getMoveBonusPic(GameMap::getTileset()->getId(), move_bonus,
                        FontSize::getInstance ()->get_height ())->to_pixbuf();
  Gtk::Label *info_label;
  xml->get_widget("info_label", info_label);
  info_label->set_text(info);

  // move into correct position
  window->get_child()->show();
  Vector<int> pos(0, 0);
  target->get_window()->get_origin(pos.x, pos.y);
  if (target->get_has_window() == false)
    {
      Gtk::Allocation a = target->get_allocation();
      pos.x += a.get_x();
      pos.y += a.get_y();
    }
  Vector<int> size(0, 0);
  window->get_size(size.x, size.y);
  window->set_gravity(Gdk::GRAVITY_SOUTH);
  pos.y -= size.y + 2;

  window->move(pos.x, pos.y);
  window->show();
}

ArmyInfoTip::ArmyInfoTip(Gtk::Widget *target, const Army *army)
{
  Glib::ustring s = army->getName();
  s += "\n";
  // note to translators: %1 is ranged strength
  s += String::ucompose(_("Strength: %1"),
                        army->getStat(Army::STRENGTH));
  s += "\n";

  // note to translators: %1 is remaining moves, %2 is total moves
  s += String::ucompose(_("Moves: %1/%2"),
                        army->getMoves(), army->getStat(Army::MOVES));
  s += "\n";
  s += String::ucompose(_("Upkeep: %1"), army->getUpkeep());

  init (target, 
        ImageCache::getInstance()->getCircledArmyPic
        (army->getArmyset (),
         army->getTypeId(),
         army->getOwner(),
         army->getMedalBonuses(),
         false, Shield::NEUTRAL,
         true,
         FontSize::getInstance ()->get_height ())->to_pixbuf(),
        army->getMoveBonus(), s);
}

ArmyInfoTip::ArmyInfoTip(Gtk::Widget *target, const ArmyProdBase *army, 
			 City *city)
{
  Glib::ustring s = army->getName();
  s += "\n";
  // note to translators: %1 is melee strength
  s += String::ucompose(_("Strength: %1"),
                        army->getStrength());
  s += "\n";
  // note to translators: %1 is total moves
  s += String::ucompose(_("Moves: %1"), army->getMaxMoves());
  s += "\n";
  s += String::ucompose(_("Time: %1"), army->getProduction());
  s += "\n";
  s += String::ucompose(_("Cost: %1"), army->getProductionCost());

  init (target,
        ImageCache::getInstance()->getCircledArmyPic
        (army->getArmyset (),
         army->getTypeId(),
         city->getOwner (), NULL,
         false, Shield::NEUTRAL,
         true,
         FontSize::getInstance ()->get_height ())->to_pixbuf(),
        army->getMoveBonus(), s);
}

ArmyInfoTip::ArmyInfoTip(Gtk::Widget *target, const ArmyProto *army)
{
  Glib::ustring s = army->getName();
  s += "\n";
  // note to translators: %1 is melee strength, %2 is ranged strength
  s += String::ucompose(_("Strength: %1"),
                        army->getStrength());
  s += "\n";
  // note to translators: %1 is remaining moves, %2 is total moves
  s += String::ucompose(_("Movement: %1"), army->getMaxMoves());
  s += "\n";
  s += String::ucompose(_("Time: %1"), army->getProduction());
  s += "\n";
  s += String::ucompose(_("Cost: %1"), army->getUpkeep());

  Player *p = Playerlist::getInstance()->getActiveplayer();
  init (target,
        ImageCache::getInstance()->getCircledArmyPic
        (army->getArmyset(),
         army->getId(), p, NULL,
         false, Shield::NEUTRAL,
         true,
         FontSize::getInstance ()->get_height ())->to_pixbuf(),
        army->getMoveBonus(), s);
}
