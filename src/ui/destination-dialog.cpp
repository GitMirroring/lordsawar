//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2012, 2014, 2017, 2020 Ben Asselstine
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

#include "destination-dialog.h"

#include "input-helpers.h"
#include "ucompose.hpp"
#include "defs.h"
#include "army.h"
#include "city.h"
#include "armyprodbase.h"
#include "citylist.h"
#include "ImageCache.h"
#include "vectoredunitlist.h"
#include "vectoredunit.h"
#include "shield.h"
#include "playerlist.h"
#include "font-size.h"

#define method(x) sigc::mem_fun(*this, &DestinationDialog::x)

DestinationDialog::DestinationDialog(Gtk::Window &parent, City *c, bool *see_all)
 : LwDialog(parent, "destination-dialog.ui")
{
  d_see_all = see_all;
  city = c;

  xml->get_widget("map_image", map_image);
  xml->get_widget("see_all_togglebutton", see_all_toggle);
  xml->get_widget("vector_togglebutton", vector_toggle);
  xml->get_widget("change_togglebutton", change_toggle);
  xml->get_widget("current_label", current_label);
  xml->get_widget("current_image", current_image);
  xml->get_widget("turns_label", turns_label);
  xml->get_widget("description_label", description_label);
  xml->get_widget("one_turn_away_image", one_turn_away_image);
  xml->get_widget("two_turns_away_image", two_turns_away_image);
  xml->get_widget("next_turn_1_image", next_turn_1_image);
  xml->get_widget("next_turn_2_image", next_turn_2_image);
  xml->get_widget("next_turn_3_image", next_turn_3_image);
  xml->get_widget("next_turn_4_image", next_turn_4_image);
  xml->get_widget("turn_after_1_image", turn_after_1_image);
  xml->get_widget("turn_after_2_image", turn_after_2_image);
  xml->get_widget("turn_after_3_image", turn_after_3_image);
  xml->get_widget("turn_after_4_image", turn_after_4_image);

  see_all_toggle->signal_toggled().connect
    (sigc::bind(method(on_see_all_toggled), see_all_toggle));
  vector_toggle->signal_toggled().connect
    (sigc::bind(method(on_vector_toggled), vector_toggle));
  change_toggle->signal_toggled().connect
    (sigc::bind(method(on_change_toggled), change_toggle));

  vectormap = new VectorMap(c, VectorMap::SHOW_ORIGIN_CITY_VECTORING, false);
  vectormap->map_changed.connect (method(on_map_changed));

  Gtk::EventBox *map_eventbox;
  xml->get_widget("map_eventbox", map_eventbox);
  map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK);
  map_eventbox->signal_button_press_event().connect
    (method(on_map_mouse_button_event));
  fill_in_vectoring_info();
}

void DestinationDialog::run()
{
  vectormap->resize();
  vectormap->draw();
  //see_all_toggle->set_active(*d_see_all);
  dialog->show();
  dialog->run();
}

void DestinationDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  if (vectormap->getClickAction() == VectorMap::CLICK_SELECTS &&
      vector_toggle->get_active() == true)
    vector_toggle->set_active(false);
  else if (vectormap->getClickAction() == VectorMap::CLICK_SELECTS &&
      change_toggle->get_active() == true)
    change_toggle->set_active(false);
  else
    {
      city = vectormap->getCity();
      fill_in_vectoring_info();
    }
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, 
                        vectormap->get_width(), vectormap->get_height());
  map_image->property_pixbuf() = pixbuf;
}

bool DestinationDialog::on_map_mouse_button_event(GdkEventButton *e)
{
    if (e->type != GDK_BUTTON_PRESS)
	return true;	// useless event
    
    vectormap->mouse_button_event(to_input_event(e));
    city = vectormap->getCity();
    fill_in_vectoring_info();
    
    return true;
}

void DestinationDialog::update_toggle_color (Gtk::ToggleButton *toggle)
{
  bool active = toggle->get_active();
  Pango::AttrList attrs;
  Pango::Attribute weight = 
    Pango::Attribute::create_attr_weight(active ?
                                         Pango::WEIGHT_BOLD :
                                         Pango::WEIGHT_NORMAL);
  Pango::Attribute color = active ?
    Pango::Attribute::create_attr_foreground (65535,0,0) :
    Pango::Attribute::create_attr_foreground (65535,65535,65535);
  attrs.insert(color);
  dynamic_cast<Gtk::Label*>(toggle->get_child())->set_attributes(attrs);
}

void DestinationDialog::on_see_all_toggled(Gtk::ToggleButton *toggle)
{
  update_toggle_color(toggle);

  *d_see_all = toggle->get_active();
  if (*d_see_all)
    vectormap->setShowVectoring(VectorMap::SHOW_ALL_VECTORING);
  else
    vectormap->setShowVectoring(VectorMap::SHOW_ORIGIN_CITY_VECTORING);
  vectormap->draw();
}

void DestinationDialog::on_vector_toggled(Gtk::ToggleButton *toggle)
{
  update_toggle_color(toggle);
// the idea here is that we click on the toggle,
// and then after we click on the map, it gets untoggled
// we act when it's untoggled.
  if (toggle->get_active() == false)
    {
      vectormap->setClickAction(VectorMap::CLICK_SELECTS);
      vectormap->draw();
      fill_in_vectoring_info();
    }
  else
    {
      vectormap->setClickAction(VectorMap::CLICK_VECTORS);
      vectormap->draw();
    }
}

void DestinationDialog::on_change_toggled(Gtk::ToggleButton *toggle)
{
  update_toggle_color(toggle);
// the idea here is that we click on the toggle,
// and then after we click on the map, it gets untoggled
// we act when it's untoggled.
  if (toggle->get_active() == false)
    {
      vectormap->setClickAction(VectorMap::CLICK_SELECTS);
      vectormap->draw();
      fill_in_vectoring_info();
    }
  else
    {
      vectormap->setClickAction(VectorMap::CLICK_CHANGES_DESTINATION);
      vectormap->draw();
    }
}

void DestinationDialog::update_description (std::list<VectoredUnit*> vectored)
{
  if (vectored.empty() == true)
    {
      if (city->getVectoring() != Vector<int>(-1,-1))
        {
          City *c = Citylist::getInstance()->getObjectAt(city->getVectoring());
          int turns = VectoredUnit::get_travel_turns (city->getPos(), 
                                                      city->getVectoring());
          /* note to translators:
             "standard" is the hero's flag that can be vectored to.
             it is a flag that can be carried into battle.
             e.g. a battle standard. */
          description_label->set_text
            (String::ucompose(_("+%1t to arrive at %2"),
                              turns, c ? c->getName() : _("standard")));
        }
      else
        description_label->set_text ("");
    }
  else
    {
      Vector<int> t = vectored.front()->getDestination();
      City *c = Citylist::getInstance()->getObjectAt(t);
      int turns = vectored.front()->getDuration();
      description_label->set_text (String::ucompose(_("+%1t to arrive at %2"),
                                                    turns, c ? c->getName() :
                                                    _("standard")));
    }
}

void DestinationDialog::fill_in_vectoring_info()
{
  ImageCache *gc = ImageCache::getInstance();
  std::list<VectoredUnit*> vectored;
  VectoredUnitlist *vul = VectoredUnitlist::getInstance();
  dialog->set_title(city->getName());

  Player *player = city->getOwner();
  unsigned int as = player->getArmyset();
  Glib::RefPtr<Gdk::Pixbuf> pic;
  int slot = city->getActiveProductionSlot();
  Glib::RefPtr<Gdk::Pixbuf> s = 
    gc->getCircledArmyPic(as, 0, player, NULL, false, Shield::NEUTRAL, 
                          true,
                          FontSize::getInstance ()->get_height ())->to_pixbuf();
  Glib::RefPtr<Gdk::Pixbuf> empty_pic =
    gc->getCircledArmyPic(as, 0, player, NULL, false, Shield::NEUTRAL, 
                          false,
                          FontSize::getInstance ()->get_height ())->to_pixbuf();

  vector_toggle->set_sensitive(slot != -1 ? true : false);

  Citylist *cl = Citylist::getInstance();
  bool target = cl->isVectoringTarget(city);
  change_toggle->set_sensitive(target);

  one_turn_away_image->set(empty_pic);
  two_turns_away_image->set(empty_pic);
  next_turn_1_image->set(empty_pic);
  next_turn_2_image->set(empty_pic);
  next_turn_3_image->set(empty_pic);
  next_turn_4_image->set(empty_pic);
  turn_after_1_image->set(empty_pic);
  turn_after_2_image->set(empty_pic);
  turn_after_3_image->set(empty_pic);
  turn_after_4_image->set(empty_pic);

  Glib::ustring s1;
  Glib::ustring s4 = _("Current:");

  vul->getVectoredUnitsComingFrom(city->getPos(), vectored);
  if (slot == -1)
    {
      pic = empty_pic;
      turns_label->set_markup("");
      description_label->set_text("");
    }
  else
    {
      const ArmyProdBase* a = city->getProductionBase(slot);
      pic = gc->getCircledArmyPic(as, a->getTypeId(), player, NULL, false,
                                  Shield::NEUTRAL, true,
                                  FontSize::getInstance ()->get_height ())->to_pixbuf();
      s1 = String::ucompose(_("%1t"), city->getDuration());
      turns_label->set_markup("<i>" + s1 + "</i>");
      update_description (vectored);
    }
    
  current_image->property_pixbuf() = pic;
  current_label->set_markup("<i>" + s4 + "</i>");

  //show the units that have been vectored from this city
  for (std::list<VectoredUnit*>::const_iterator it = vectored.begin();
       it != vectored.end(); ++it)
    {
      int armytype = (*it)->getArmy()->getTypeId();
      if ((*it)->getDuration() == 2)
        {
          pic = gc->getCircledArmyPic(as, armytype, player, NULL, false,
                                      Shield::NEUTRAL, true,
                                      FontSize::getInstance ()->get_height ())->to_pixbuf();
          one_turn_away_image->property_pixbuf() = pic;
        }
      else if ((*it)->getDuration() == 1)
        {
          pic = gc->getCircledArmyPic(as, armytype, player, NULL, false, 
                                      Shield::NEUTRAL, true,
                                      FontSize::getInstance ()->get_height ())->to_pixbuf();
          two_turns_away_image->property_pixbuf() = pic;
        }
    }

  //show the units that are arriving into this city
  vectored.clear();
  vul->getVectoredUnitsGoingTo(city, vectored);
  int count = 0;
  Gtk::Image *image = next_turn_1_image;
  for (std::list<VectoredUnit*>::const_iterator it = vectored.begin();
       it != vectored.end(); ++it)
    {
      if ((*it)->getDuration() != 1)
        continue;

      switch (count)
        {
          case 0: image = next_turn_1_image; break;
          case 1: image = next_turn_2_image; break;
          case 2: image = next_turn_3_image; break;
          case 3: image = next_turn_4_image; break;
        }
      pic = 
        gc->getCircledArmyPic(as, (*it)->getArmy()->getTypeId(), player, NULL, 
                              false, Shield::NEUTRAL, true,
                              FontSize::getInstance ()->get_height ())->to_pixbuf();
      image->property_pixbuf() = pic;
      count++;
    }
  count = 0;
  for (std::list<VectoredUnit*>::const_iterator it = vectored.begin();
       it != vectored.end(); ++it)
    {
      if ((*it)->getDuration() != 2)
        continue;
      switch (count)
        {
          case 0: image = turn_after_1_image; break;
          case 1: image = turn_after_2_image; break;
          case 2: image = turn_after_3_image; break;
          case 3: image = turn_after_4_image; break;
        }
      pic = 
        gc->getCircledArmyPic(as, (*it)->getArmy()->getTypeId(), player, NULL,
                              false, Shield::NEUTRAL, true,
                              FontSize::getInstance ()->get_height ())->to_pixbuf();
      image->property_pixbuf() = pic;
      count++;
    }
}
