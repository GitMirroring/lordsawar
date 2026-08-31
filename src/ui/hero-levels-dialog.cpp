//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2020 Ben Asselstine
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
#include <sigc++/functors/mem_fun.h>

#include "hero-levels-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "player.h"
#include "army.h"
#include "hero.h"
#include "ImageCache.h"
#include "shield.h"
#include "font-size.h"

void HeroLevelsDialog::init(Player *theplayer)
{
  player = theplayer;

  heroes_list = Gtk::ListStore::create(heroes_columns);
  xml->get_widget("treeview", heroes_treeview);
  heroes_treeview->set_model(heroes_list);
  heroes_treeview->append_column("", heroes_columns.image);
  heroes_treeview->append_column(_("Hero"), heroes_columns.name);
  heroes_treeview->append_column(_("Level"), heroes_columns.level);
  heroes_treeview->append_column(_("Exp"), heroes_columns.exp);
  heroes_treeview->append_column(_("Needs"), heroes_columns.needs);
  heroes_treeview->append_column(_("Str"), heroes_columns.str);
  heroes_treeview->append_column(_("Move"), heroes_columns.move);
  heroes_treeview->set_headers_visible(true);
}

HeroLevelsDialog::HeroLevelsDialog(Gtk::Window &parent, std::list<Hero*> heroes)
 : LwDialog(parent, "hero-levels-dialog.ui")
{
  init ((*heroes.front()).getOwner());
  for (std::list<Hero*>::iterator it = heroes.begin(); it != heroes.end(); ++it)
    addHero(*it);
}

HeroLevelsDialog::HeroLevelsDialog(Gtk::Window &parent, Player *theplayer)
 : LwDialog(parent, "hero-levels-dialog.ui")
{
  init (theplayer);
  std::list<Hero*> heroes = theplayer->getHeroes();
  for (std::list<Hero*>::iterator it = heroes.begin(); it != heroes.end(); ++it)
    addHero(*it);
}

void HeroLevelsDialog::addHero(Hero *h)
{
  Gtk::TreeIter i = heroes_list->append();
  (*i)[heroes_columns.name] = h->getName();
  (*i)[heroes_columns.image] = 
    ImageCache::getInstance()->getCircledArmyPic
    (player->getArmyset(), h->getTypeId(), player, NULL, false, Shield::NEUTRAL, 
     true, FontSize::getInstance()->get_height ())->to_pixbuf();
  (*i)[heroes_columns.level] = String::ucompose("%1", h->getLevel());
  (*i)[heroes_columns.exp] = (guint32)h->getXP();
  (*i)[heroes_columns.needs] = (guint32)h->getXpNeededForNextLevel();
  (*i)[heroes_columns.str] = h->getStat(Army::STRENGTH, true);
  (*i)[heroes_columns.move] = h->getStat(Army::MOVES, true);
}
