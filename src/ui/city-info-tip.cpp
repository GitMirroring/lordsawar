//  Copyright (C) 2009, 2011, 2012, 2014, 2015, 2016, 2020 Ben Asselstine
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

#include "city-info-tip.h"
#include "builder-cache.h"
#include "ucompose.hpp"
#include "vector.h"
#include "defs.h"
#include "ImageCache.h"
#include "playerlist.h"
#include "city.h"
#include "File.h"
#include "font-size.h"


CityInfoTip::CityInfoTip(Gtk::Widget *target, MapTipPosition mpos, const City *city)
{
    ImageCache *gc = ImageCache::getInstance();
    Glib::RefPtr<Gtk::Builder> xml = BuilderCache::get("city-info-window.ui");

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
    Gtk::Image *left_shield_image;
    xml->get_widget("left_shield_image", left_shield_image);
    Gtk::Image *right_shield_image;
    xml->get_widget("right_shield_image", right_shield_image);
    Gtk::Image *income_image;
    xml->get_widget("income_image", income_image);
    Gtk::Image *defense_image;
    xml->get_widget("defense_image", defense_image);
    Gtk::Label *name_label;
    xml->get_widget("name_label", name_label);
    Gtk::Label *income_label;
    xml->get_widget("income_label", income_label);
    Gtk::Label *defense_label;
    xml->get_widget("defense_label", defense_label);
    Gtk::Label *capital_label;
    xml->get_widget("capital_label", capital_label);
    Gtk::Image *capital_image;
    xml->get_widget("capital_image", capital_image);
    Gtk::Label *razed_label;
    xml->get_widget("razed_label", razed_label);

    if (city->isBurnt() == true)
      razed_label->set_markup("<b>" + Glib::ustring(_("Razed!")) + "</b>");
      
    name_label->set_markup("<b>" + city->getName() + "</b>");
    left_shield_image->property_pixbuf() = 
      gc->getShieldPic(1, city->getOwner(), false,
                       FontSize::getInstance ()->get_height ())->to_pixbuf();
    right_shield_image->property_pixbuf() = 
      gc->getShieldPic(1, city->getOwner(), false,
                       FontSize::getInstance ()->get_height ())->to_pixbuf();
    income_image->property_pixbuf () =
      ImageCache::getInstance ()->getStatusPic
      (ImageCache::STATUS_INCOME,
       FontSize::getInstance ()->get_height ())->to_pixbuf ();
    defense_image->property_pixbuf () =
      ImageCache::getInstance ()->getStatusPic
      (ImageCache::STATUS_DEFENSE,
       FontSize::getInstance ()->get_height ())->to_pixbuf ();
    income_label->set_markup(String::ucompose("<b>%1</b>",
					      city->getGold()));
    defense_label->set_markup(String::ucompose("<b>%1</b>",
					       city->getDefenseLevel()));
			  
    if (city->isCapital())
      {
	Glib::ustring s = _("Capital of\n");
	s += city->getCapitalOwner()->getName();
	capital_label->set_text (s);
	capital_image->property_pixbuf() = 
	  gc->getShieldPic(1, city->getCapitalOwner(), false,
                           FontSize::getInstance ()->get_height ())->to_pixbuf();
      }
    else
      {
	Glib::RefPtr<Gdk::Pixbuf> empty_pic
	  = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, 1, 1);
	empty_pic->fill(0x00000000);
	capital_image->property_pixbuf() = empty_pic;
      }

    // move into correct position
    window->get_child()->show();
    Vector<int> p(0, 0);
    target->get_window()->get_origin(p.x, p.y);
    if (target->get_has_window() == false)
    {
	Gtk::Allocation a = target->get_allocation();
	p.x += a.get_x();
	p.y += a.get_y();
    }
    Vector<int> size(0, 0);
    window->get_size(size.x, size.y);
    switch (mpos.justification)
    {
    case MapTipPosition::LEFT:
	window->set_gravity(Gdk::GRAVITY_NORTH_WEST);
	break;
    case MapTipPosition::RIGHT:
	window->set_gravity(Gdk::GRAVITY_NORTH_EAST);
	p.x -= size.x;
	break;
    case MapTipPosition::TOP:
	window->set_gravity(Gdk::GRAVITY_NORTH_WEST);
	break;
    case MapTipPosition::BOTTOM:
	window->set_gravity(Gdk::GRAVITY_SOUTH_WEST);
	p.y -= size.y;
	break;
    }

    p += mpos.pos;
	
    window->move(p.x, p.y);
    window->show();
}
