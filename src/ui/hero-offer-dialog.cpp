//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2012, 2014, 2015, 2017,
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
#include <sigc++/functors/mem_fun.h>

#include "hero-offer-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "GameMap.h"
#include "File.h"
#include "snd.h"
#include "city.h"
#include "playerlist.h"
#include "ImageCache.h"
#include "font-size.h"

#define method(x) sigc::mem_fun(*this, &HeroOfferDialog::x)

HeroOfferDialog::HeroOfferDialog(Gtk::Window &parent, Player *player, HeroProto *h, City *c, int gold)
 : LwDialog(parent, "hero-offer-dialog.ui")
{
    city = c;
    hero = h;
    
    xml->get_widget("map_image", map_image);

    heromap = new HeroMap(city);
    heromap->map_changed.connect(method(on_map_changed));

    dialog->set_title(String::ucompose(_("A Hero for %1"), player->getName()));

    xml->get_widget("hero_image", hero_image);
    xml->get_widget("hero_male", male_radiobutton);
    xml->get_widget("hero_female", female_radiobutton);
    male_radiobutton->set_active(hero->getGender() == Hero::MALE);
    female_radiobutton->set_active(hero->getGender() == Hero::FEMALE);
    male_radiobutton->signal_clicked().connect(method(on_toggled));
    
    on_toggled();
    
    xml->get_widget("name", name_entry);
    name_entry->set_text(hero->getName());
    name_entry->signal_changed().connect (method(on_name_changed));

    xml->get_widget("close_button", close_button);
    close_button->set_sensitive (gold > 0);

    xml->get_widget("accept_button", accept_button);
    Gtk::Label *label;
    xml->get_widget("label", label);
    
    Glib::ustring s;
    if (gold > 0)
	s = String::ucompose(
	    ngettext("A hero in %2 wants to join you for %1 gold piece!",
		     "A hero in %2 wants to join you for %1 gold pieces!",
		     gold), gold, city->getName());
    else
	s = String::ucompose(_("A hero in %1 wants to join you!"), city->getName());
    label->set_text(s);
    update_buttons();
}

void HeroOfferDialog::update_buttons()
{
  if (String::utrim(name_entry->get_text()) == "")
    accept_button->set_sensitive(false);
  else
    {
      accept_button->set_sensitive(true);
      accept_button->property_can_focus() = true;
      accept_button->property_can_default() = true;
      accept_button->property_has_default() = true;
      name_entry->property_activates_default() = true;
      accept_button->property_receives_default() = true;
    }
}

void HeroOfferDialog::on_name_changed()
{
  update_buttons();
}

void HeroOfferDialog::on_toggled()
{
  if (male_radiobutton->get_active())
    hero_image->property_pixbuf() =
      ImageCache::getInstance()->getDialogPic
      (ImageCache::DIALOG_NEW_HERO_MALE,
       FontSize::getInstance ()->get_height ())->to_pixbuf();
  else
    hero_image->property_pixbuf() =
      ImageCache::getInstance()->getDialogPic
      (ImageCache::DIALOG_NEW_HERO_FEMALE,
       FontSize::getInstance ()->get_height ())->to_pixbuf();
}

bool HeroOfferDialog::run()
{
    heromap->resize();
    heromap->draw();

    Snd::getInstance()->play("hero", 1);
    dialog->show_all();
    int response = dialog->run();
    Snd::getInstance()->halt();

    if (response == Gtk::RESPONSE_ACCEPT)	// accepted
      {
        hero->setName(String::utrim(name_entry->get_text()));
        if (male_radiobutton->get_active() == true && 
	    hero->getGender() == Hero::FEMALE)
          hero->setGender(Hero::MALE);
        else if (male_radiobutton->get_active() == false &&
		 hero->getGender() == Hero::MALE)
	  hero->setGender(Hero::FEMALE);
	return true;
      }
    else
	return false;
}

void HeroOfferDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  map_image->property_pixbuf() = 
    Gdk::Pixbuf::create(map, 0, 0, heromap->get_width(), heromap->get_height());
}

