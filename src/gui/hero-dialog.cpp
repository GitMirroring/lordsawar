//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2012, 2014, 2015, 2017 Ben Asselstine
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
#include <iomanip>

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "hero-dialog.h"

#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "hero.h"
#include "Item.h"
#include "GameMap.h"
#include "Backpack.h"
#include "MapBackpack.h"
#include "history.h"
#include "playerlist.h"

#define method(x) sigc::mem_fun(*this, &HeroDialog::x)

HeroDialog::HeroDialog(Gtk::Window &parent, Hero *h, Vector<int> p)
 : LwDialog(parent, "hero-dialog.ui"),
  heroesmap (new HeroesMap(Playerlist::getActiveplayer()->getHeroes())),
  hero (h), pos (p)
{
  inhibit_hero_changed = false;

  xml->get_widget("map_image", map_image);

  std::list<Hero*> heroes = Playerlist::getActiveplayer()->getHeroes();
  if (hero)
    heroesmap->setSelectedHero(hero);
  else
    {
      hero = *heroes.begin();
      pos = Playerlist::getActiveplayer()->getPositionOfArmyById(hero->getId());
      heroesmap->setSelectedHero(hero);
    }
  heroesmap->map_changed.connect (method(on_map_changed));
  Gtk::EventBox *map_eventbox;
  xml->get_widget("map_eventbox", map_eventbox);
  map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK);
  map_eventbox->signal_button_press_event().connect
    (method(on_map_mouse_button_event));

  xml->get_widget("battle_label", battle_label);
  xml->get_widget("strength_label", strength_label);
  xml->get_widget("command_label", command_label);
  xml->get_widget("moves_label", moves_label);
  xml->get_widget("level_label", level_label);
  xml->get_widget("upkeep_label", upkeep_label);
  xml->get_widget("experience_label", experience_label);

  xml->get_widget("drop_button", drop_button);
  xml->get_widget("pickup_button", pickup_button);
  drop_button->signal_clicked().connect(method(on_drop_clicked));
  pickup_button->signal_clicked().connect(method(on_pickup_clicked));

  xml->get_widget("next_button", next_button);
  xml->get_widget("prev_button", prev_button);
  next_button->signal_clicked().connect(method(on_next_clicked));
  prev_button->signal_clicked().connect(method(on_prev_clicked));
  if (heroes.size() <= 1)
    {
      next_button->set_sensitive(false);
      prev_button->set_sensitive(false);
    }

  heroes_list = Gtk::ListStore::create(heroes_columns);
  xml->get_widget("heroes_treeview", heroes_treeview);
  heroes_treeview->set_model(heroes_list);
  heroes_treeview->append_column(_("Hero"), heroes_columns.name);

  heroes_list->clear();
  guint32 count = 0;
  for (std::list<Hero*>::iterator it = heroes.begin(); it != heroes.end(); ++it)
    {
      add_hero (*it);
      if (*it == hero)
        {
          Gtk::TreeModel::Row row = heroes_treeview->get_model()->children()[count];
          heroes_treeview->get_selection()->select(row);
        }
      count++;
    }
  heroes_treeview->get_selection()->signal_changed().connect(method(on_hero_changed));

  item_list = Gtk::ListStore::create(item_columns);
  xml->get_widget("treeview", item_treeview);
  item_treeview->set_model(item_list);
  item_treeview->append_column(_("Item"), item_columns.item_desc);
  item_treeview->get_column (0)->set_expand (true);
  item_treeview->append_column(_("Status"), item_columns.status);


  item_treeview->get_selection()->signal_changed().connect(method(on_item_selection_changed));

  events_list = Gtk::ListStore::create(events_columns);
  xml->get_widget("events_treeview", events_treeview);
  events_treeview->append_column("", events_columns.desc);
  events_treeview->set_model(events_list);
  events_list->clear();

  on_item_selection_changed();
}

HeroDialog::~HeroDialog()
{
  delete heroesmap;
}

void HeroDialog::addHistoryEvent(History *history)
{
  Glib::ustring s = "";
  Gtk::TreeIter i = events_list->append();

  switch (history->getType())
    {
    case History::FOUND_SAGE: 
	{
	  auto *ev = static_cast<History_FoundSage *>(history);
	  s = String::ucompose(_("%1 finds a sage!"), ev->getHeroName());
	  break;
	}
    case History::HERO_EMERGES:
	{
	  auto *ev = static_cast<History_HeroEmerges *>(history);
	  s = String::ucompose(_("%1 emerges in %2!"), ev->getHeroName(),
			       ev->getCityName());
	  break;
	}
    case History::HERO_QUEST_STARTED:
	{
	  auto *ev = static_cast<History_HeroQuestStarted*>(history);
	  s = String::ucompose(_("%1 begins a quest!"), ev->getHeroName());
	  break;
	}
    case History::HERO_QUEST_COMPLETED:
	{
	  auto *ev = static_cast<History_HeroQuestCompleted *>(history);
	  s = String::ucompose(_("%1 finishes a quest!"), ev->getHeroName());
	  break;
	}
    case History::HERO_KILLED_IN_CITY:
	{
	  auto *ev = static_cast<History_HeroKilledInCity *>(history);
	  s = String::ucompose(_("%1 is killed in %2!"), ev->getHeroName(),
			       ev->getCityName());
	  break;
	}
    case History::HERO_KILLED_IN_BATTLE:
	{
	  auto *ev = static_cast<History_HeroKilledInBattle *>(history);
	  s = String::ucompose(_("%1 is killed in battle!"), ev->getHeroName());
	  break;
	}
    case History::HERO_KILLED_SEARCHING:
	{
	  auto *ev = static_cast<History_HeroKilledSearching *>(history);
	  s = String::ucompose(_("%1 is killed while searching!"), 
			       ev->getHeroName());
	  break;
	}
    case History::HERO_CITY_WON:
	{
	  auto *ev = static_cast<History_HeroCityWon *>(history);
	  s = String::ucompose(_("%1 conquers %2!"), ev->getHeroName(), 
			       ev->getCityName());
	  break;
	}
    case History::HERO_FINDS_ALLIES:
	{
	  auto *ev = static_cast<History_HeroFindsAllies*>(history);
	  s = String::ucompose(_("%1 finds allies!"), ev->getHeroName());
	  break;
	}
    default:
      s = _("unknown");
      break;
    }

  (*i)[events_columns.desc] = s;
  (*i)[events_columns.history] = history;
}

void HeroDialog::run()
{
  heroesmap->resize();
  heroesmap->draw();
  GameMap *gm = GameMap::getInstance();
  dialog->show_all();
  show_hero();
  dialog->run();
  if (gm->getTile(pos)->getBackpack()->size() > 0 && 
      gm->getTile(pos)->getType() == Tile::WATER)
    {
      // splash, items lost forever
      while (gm->getTile(pos)->getBackpack()->size())
        {
	  MapBackpack::iterator i = gm->getTile(pos)->getBackpack()->begin();
          gm->getTile(pos)->getBackpack()->removeFromBackpack(*i);
        }
    }
}

void HeroDialog::update_hero_list()
{
  inhibit_hero_changed = true;
  std::list<Hero*> heroes;
  heroes = Playerlist::getActiveplayer()->getHeroes();
  guint32 count = 0;
  for (std::list<Hero*>::iterator it = heroes.begin(); it != heroes.end(); ++it)
    {
      if (*it == hero)
        {
          Gtk::TreeModel::Row row;
          row = heroes_treeview->get_model()->children()[count];
          heroes_treeview->get_selection()->select(row);
        }
      count++;
    }
  inhibit_hero_changed = false;
}

void HeroDialog::on_hero_changed()
{
  if (inhibit_hero_changed == true)
    return;
  Glib::RefPtr<Gtk::TreeSelection> selection = heroes_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow)
    {
      Gtk::TreeModel::Row row = *iterrow;
      hero = row[heroes_columns.hero];
      pos = Playerlist::getActiveplayer()->getPositionOfArmyById(hero->getId());
      heroesmap->setSelectedHero(hero);
      show_hero();
      heroesmap->draw();
    }
}

void HeroDialog::on_item_selection_changed()
{
  Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
  if (i)
    {
      bool droppable = (*i)[item_columns.status] == _("In backpack");
      drop_button->set_sensitive(droppable);
      pickup_button->set_sensitive(!droppable);
    }
  else
    {
      drop_button->set_sensitive(false);
      pickup_button->set_sensitive(false);
    }
}

void HeroDialog::on_drop_clicked()
{
  Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
  if (i)
    {
      bool splash = false;
      Item *item = (*i)[item_columns.item];
      hero->getOwner()->heroDropItem (hero, item, pos, splash);
      if (splash == false)
        (*i)[item_columns.status] = _("On the ground");
      else
        item_list->erase(i); //splash

      on_item_selection_changed();
      fill_in_info_labels();
    }
}

void HeroDialog::on_next_clicked()
{
  std::list<Hero*> heroes;
  heroes = Playerlist::getActiveplayer()->getHeroes();
  std::list<Hero*>::iterator next;
  next = find (heroes.begin(), heroes.end(), hero);
  if (next != heroes.end())
    {
      ++next;
      if (next == heroes.end())
	next = heroes.begin();
      hero = *next;
      heroesmap->setSelectedHero(hero);
      show_hero();
      heroesmap->draw();
    }
  update_hero_list();
}

void HeroDialog::on_prev_clicked()
{
  std::list<Hero*> heroes;
  heroes = Playerlist::getActiveplayer()->getHeroes();
  std::list<Hero*>::reverse_iterator prev;
  prev = find (heroes.rbegin(), heroes.rend(), hero);
  if (prev != heroes.rend())
    {
      ++prev;
      if (prev == heroes.rend())
	prev = heroes.rbegin();
      hero = *prev;
      heroesmap->setSelectedHero(hero);
      show_hero();
      heroesmap->draw();
    }
  update_hero_list();
}

void HeroDialog::on_pickup_clicked()
{
  Gtk::TreeIter i = item_treeview->get_selection()->get_selected();
  if (i)
    {
      Item *item = (*i)[item_columns.item];
      if (item->getPlanted() == true)
        item->setPlanted(false);
      hero->getOwner()->heroPickupItem (hero, item, pos);
      (*i)[item_columns.status] = _("In backpack");
      on_item_selection_changed();
      fill_in_info_labels();
    }
}

void HeroDialog::add_hero(Hero *h)
{
  Gtk::TreeIter i = heroes_list->append();
  (*i)[heroes_columns.name] = h->getName();
  (*i)[heroes_columns.hero] = h;
}

void HeroDialog::add_item(Item *item, bool in_backpack)
{
  std::stringstream ss;
  ss << std::endl;
  Glib::ustring newline = ss.str();

  Gtk::TreeIter i = item_list->append();
  Glib::ustring s = String::indent (item->getBonusDescription (), 2);
  (*i)[item_columns.item_desc] = item->getName() + newline + s;

  if (in_backpack)
    (*i)[item_columns.status] = _("In backpack");
  else
    (*i)[item_columns.status] = _("On the ground");

  (*i)[item_columns.item] = item;
}

void HeroDialog::fill_in_info_labels()
{
  guint32 bonus = 0;
  Backpack *backpack = hero->getBackpack();
  for (Backpack::iterator i = backpack->begin(); i != backpack->end(); ++i)
    {
      if ((*i)->getBonus(Item::ADD1STR))
        bonus += 1;
      if ((*i)->getBonus(Item::ADD2STR))
        bonus += 2;
      if ((*i)->getBonus(Item::ADD3STR))
        bonus += 3;
    }
  battle_label->set_text(String::ucompose("%1", bonus));

  bonus = 0;
  for (Backpack::iterator i = backpack->begin(); i != backpack->end(); ++i)
    {
      if ((*i)->getBonus(Item::ADD1STACK))
        bonus += 1;
      if ((*i)->getBonus(Item::ADD2STACK))
        bonus += 2;
      if ((*i)->getBonus(Item::ADD3STACK))
        bonus += 3;
    }

  //now add natural command
  bonus += hero->calculateNaturalCommand ();

  command_label->set_text (String::ucompose("%1", bonus));
  level_label->set_text(String::ucompose("%1", hero->getLevel()));
  experience_label->set_text(String::ucompose("%1", int(hero->getXP())));

  strength_label->set_text(String::ucompose("%1",
                                            hero->getStat(Army::STRENGTH)));

  // note to translators: %1 is remaining moves, %2 is total moves
  moves_label->set_text(String::ucompose(_("%1/%2"),
                        hero->getMoves(), hero->getStat(Army::MOVES)));
  upkeep_label->set_text(String::ucompose("%1", hero->getUpkeep()));
}

void HeroDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, 
                        heroesmap->get_width(), heroesmap->get_height());
  map_image->property_pixbuf() = pixbuf;
}

bool HeroDialog::on_map_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS)
	return true;	// useless event
    
    heroesmap->mouse_button_event(to_input_event(e));
    
    hero = heroesmap->getSelectedHero();
    pos = Playerlist::getActiveplayer()->getPositionOfArmyById(hero->getId());
    show_hero();
    heroesmap->draw();
    update_hero_list();
    return true;
}

void HeroDialog::show_hero()
{
  dialog->set_title(hero->getName());

  fill_in_info_labels();
  std::list<History* > events;
  events = hero->getOwner()->getHistoryForHeroId(hero->getId());
  events_list->clear();
  for (std::list<History*>::iterator i = events.begin(); i != events.end();
       ++i)
    addHistoryEvent(*i);

  // populate the item list
  item_list->clear();
  Backpack *backpack = hero->getBackpack();
  for (Backpack::iterator i = backpack->begin(); i != backpack->end(); ++i)
    add_item(*i, true);

  MapBackpack *ground = GameMap::getInstance()->getTile(pos)->getBackpack();
  for (MapBackpack::iterator i = ground->begin(); i != ground->end(); ++i)
    add_item(*i, false);

  return;
}
