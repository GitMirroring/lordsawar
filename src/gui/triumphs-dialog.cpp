//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2015, 2020 Ben Asselstine
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

#include "triumphs-dialog.h"

#include "ucompose.hpp"
#include "ImageCache.h"
#include "armysetlist.h"
#include "playerlist.h"
#include "player.h"
#include "shield.h"
#include "font-size.h"

TriumphsDialog::TriumphsDialog(Gtk::Window &parent, Player *player)
 : LwDialog(parent, "triumphs-dialog.ui")
{
  d_player = player;
  Gtk::Box *contents;
  xml->get_widget("outer_hbox", contents);
  notebook = Gtk::manage(new Gtk::Notebook());
  contents->pack_start(*notebook, true, true, 0);
  fill_in_info();
  //set the notebook to start off on the player's own page
  notebook->set_current_page(d_player->getId());
}

guint32 TriumphsDialog::tally(Player *p, Triumphs::TriumphType type)
{
  Playerlist *pl = Playerlist::getInstance();
  guint32 count = 0;
  if (p == d_player)
    {
      // add up what the other players did to us
      for (Playerlist::iterator it = pl->begin(); it != pl->end(); ++it)
	{
	  if ((*it) == Playerlist::getInstance()->getNeutral())
	    continue;
          count += (*it)->getTriumphs()->getTriumphTally(p, type);
	}
    }
  else
    {
      // add up what we did to that player
      count = d_player->getTriumphs()->getTriumphTally(p, type);
    }
  return count;
}

void TriumphsDialog::fill_in_page(Player *p)
{
  ImageCache *gc = ImageCache::getInstance();
  //here we tally up the stats, make a vbox and append it as a new page
  //tally it up differently when p == d_player
	
  guint32 count;
  Glib::ustring s;
  count = tally(p, Triumphs::TALLY_HERO);
  if (p == d_player)
    s = String::ucompose (ngettext("%1 hero earned fates worthy of legend!",
				   "%1 heroes earned fates worthy of legend!",
				   count), count);
  else
    s = String::ucompose (ngettext
			  ("%1 so-called hero slaughtered without mercy!",
			   "%1 so-called heroes slaughtered without mercy!",
			   count), count);
  Gtk::Label *hero_label = new Gtk::Label(s);

  const ArmyProto *hero = NULL;
  const Armysetlist* al = Armysetlist::getInstance();
  //let's go find the hero army
  Armyset *as = al->get(p->getArmyset());
  for (Armyset::iterator j = as->begin(); j != as->end(); ++j)
    {
      const ArmyProto *a = al->getArmy (p->getArmyset(), (*j)->getId());
      if (a->isHero())
	{
	  hero = a;
	  break;
	}
    }
  Gtk::Image *hero_image = new Gtk::Image();
  hero_image->property_pixbuf() = 
    gc->getCircledArmyPic(p->getArmyset(), hero->getId(), p, NULL, false,
                          Shield::NEUTRAL, true,
                          FontSize::getInstance()->get_height ())->to_pixbuf();
  Gtk::Box *hero_hbox = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
  hero_hbox->pack_start(*manage(hero_image), Gtk::PACK_SHRINK, 10);
  hero_hbox->pack_start(*manage(hero_label), Gtk::PACK_SHRINK, 10);

  count = tally(p, Triumphs::TALLY_SHIP);
  if (p == d_player)
    s = String::ucompose (ngettext("%1 navy not currently in service!",
				   "%1 navies not currently in service!",
				   count), count);
  else
    s = String::ucompose (ngettext("%1 navy rests with the fishes!",
				   "%1 navies rest with the fishes!",
				   count), count);
  Gtk::Label *ship_label = new Gtk::Label(s);
  Gtk::Image *ship_image = new Gtk::Image ();
  ship_image->property_pixbuf() = 
    ImageCache::getInstance()->getCircledShipPic
    (p->getArmyset (), p, false, Shield::NEUTRAL,
     FontSize::getInstance()->get_height ())->to_pixbuf();
  Gtk::Box *ship_hbox = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
  ship_hbox->pack_start(*manage(ship_image), Gtk::PACK_SHRINK, 10);
  ship_hbox->pack_start(*manage(ship_label), Gtk::PACK_SHRINK, 10);

  count = tally(p, Triumphs::TALLY_NORMAL);
  if (p == d_player)
    s = String::ucompose (ngettext("%1 army died to ensure final victory!",
				   "%1 armies died to ensure final victory!",
				   count), count);
  else
    s = String::ucompose (ngettext("%1 army smote like sheep!",
				   "%1 armies smote like sheep!",
				   count), count);
  Gtk::Label *normal_label = new Gtk::Label(s);
  Gtk::Image *normal_image = new Gtk::Image();
  normal_image->property_pixbuf() = 
    gc->getCircledArmyPic(p->getArmyset(), 0, p, NULL, false, Shield::NEUTRAL, 
                          true,
                          FontSize::getInstance()->get_height ())->to_pixbuf();
  Gtk::Box *normal_hbox = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
  normal_hbox->pack_start(*manage(normal_image), Gtk::PACK_SHRINK, 10);
  normal_hbox->pack_start(*manage(normal_label), Gtk::PACK_SHRINK, 10);

  count = tally(p, Triumphs::TALLY_SPECIAL);
  if (p == d_player)
    s = String::ucompose 
      (ngettext ("%1 unnatural creature returned from whence it came!",
		 "%1 unnatural creatures returned from whence they came!",
		 count), count);
  else
    s = String::ucompose (ngettext ("%1 unnatural creature dispatched!",
				    "%1 unnatural creatures dispatched!",
				    count), count);
  Gtk::Label *special_label = new Gtk::Label(s);
  //let's go find a special army
  const ArmyProto *special = NULL;
  for (Armyset::iterator j = as->begin(); j != as->end(); ++j)
    {
      const ArmyProto *a = al->getArmy (p->getArmyset(), (*j)->getId());
      if (a->getAwardable())
	{
	  special = a;
	  break;
	}
    }
  Gtk::Image *special_image = new Gtk::Image();
  special_image->property_pixbuf() = 
    gc->getCircledArmyPic(p->getArmyset(), special->getId(), p, NULL, false,
                          Shield::NEUTRAL, true,
                          FontSize::getInstance()->get_height ())->to_pixbuf();
  Gtk::Box *special_hbox = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
  special_hbox->pack_start(*manage(special_image), Gtk::PACK_SHRINK, 10);
  special_hbox->pack_start(*manage(special_label), Gtk::PACK_SHRINK, 10);

  count = tally(p, Triumphs::TALLY_FLAG);
  if (p == d_player)
    s = String::ucompose (ngettext ("%1 standard betrayed by its guardian!",
				    "%1 standards betrayed by its guardian!",
				    count), count);
  else
    s = String::ucompose (ngettext 
			  ("%1 standard wrested from a vanquished foe!",
			   "%1 standards wrested from a vanquished foe!",
			   count), count);
  Gtk::Label *flag_label = new Gtk::Label(s);
  Gtk::Image *flag_image = new Gtk::Image ();
  flag_image->property_pixbuf() = 
    ImageCache::getInstance()->getCircledStandardPic
    (p->getArmyset (), p, false, Shield::NEUTRAL,
     FontSize::getInstance()->get_height ())->to_pixbuf();
  Gtk::Box *flag_hbox = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
  flag_hbox->pack_start(*manage(flag_image), Gtk::PACK_SHRINK, 10);
  flag_hbox->pack_start(*manage(flag_label), Gtk::PACK_SHRINK, 10);

  Gtk::Box *contents = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
  contents->add(*manage(normal_hbox));
  contents->add(*manage(special_hbox));
  contents->add(*manage(hero_hbox));
  contents->add(*manage(ship_hbox));
  contents->add(*manage(flag_hbox));
  Gtk::Image *shield_image = new Gtk::Image();
  shield_image->property_pixbuf() =
    gc->getShieldPic(2, p, false,
                     FontSize::getInstance ()->get_height ())->to_pixbuf();
  notebook->append_page (*manage(contents), *manage(shield_image));
}

void TriumphsDialog::fill_in_info()
{
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      Player *p = Playerlist::getInstance()->getPlayer(i);
      if (p == NULL)
	continue;
      if (p == Playerlist::getInstance()->getNeutral())
	continue;
      fill_in_page(p);
    }
}

TriumphsDialog::~TriumphsDialog()
{
  notebook->property_show_tabs () = false;
}
