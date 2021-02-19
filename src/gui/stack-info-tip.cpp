//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2012, 2014, 2015, 2016, 2020,
//  2021 Ben Asselstine
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

#include "stack-info-tip.h"

#include "vector.h"
#include "stack.h"
#include "ImageCache.h"
#include "map-tip-position.h"
#include "builder-cache.h"
#include "File.h"
#include "stacktile.h"
#include "GameScenarioOptions.h"
#include "playerlist.h"
#include "GameMap.h"
#include "font-size.h"

StackInfoTip::StackInfoTip(Gtk::Widget *target, MapTipPosition mpos, StackTile *stile)
{
    ImageCache *gc = ImageCache::getInstance();
    Glib::RefPtr<Gtk::Builder> xml = BuilderCache::get("stack-info-window.ui");

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
    xml->get_widget("image_hbox", image_hbox);
    xml->get_widget("hero_name_box", hero_name_box);
    xml->get_widget("hero_description_box", hero_description_box);

    //fill up the hbox with images of the armies in the stack

    Player *active = Playerlist::getActiveplayer();
    std::vector<Stack *> stks;
    stks = stile->getFriendlyStacks(active);
    bool enemies = false;
    if (stks.empty() == true)
      {
        stks = stile->getEnemyStacks(active);
        enemies = true;
      }

    Glib::ustring hero_name;
    Glib::ustring hero_desc;
    for (auto s : stks)
      {
        if (s->getStrongestHero ())
          {
            hero_name = s->getStrongestHero ()->getName ();
            hero_desc =
              dynamic_cast<Hero*>(s->getStrongestHero ())->getDescription ();
            break;
          }
      }
        
    if (GameScenarioOptions::s_see_opponents_stacks == false &&
        enemies)
      stks.clear ();

    if (enemies && hero_name.empty () == true)
      return;
    guint32 fs = FontSize::getInstance ()->get_height ();
    for (std::vector<Stack *>::iterator i = stks.begin(); i != stks.end(); ++i)
      for (Stack::iterator it = (*i)->begin(); it != (*i)->end(); ++it)
	{
	  Gtk::Image *image = new Gtk::Image();
          PixMask *armypic = gc->getDialogArmyPic(*it, fs)->copy();
	  image->property_pixbuf() = armypic->to_pixbuf();
          delete armypic;
	  image_hbox->add(*manage(image));
	}

    if (stks.empty () == false)
      image_hbox->show_all();

    if (hero_name.empty () == false)
      {
        Gtk::Label *hero_label = new Gtk::Label (hero_name);
        hero_label->set_hexpand (true);
        hero_label->property_justify () = Gtk::JUSTIFY_CENTER;
        hero_name_box->add (*manage (hero_label));
        hero_name_box->show_all ();
      }

    if (hero_desc.empty () == false)
      {
        Gtk::Label *desc_label = new Gtk::Label (hero_desc);
        desc_label->set_hexpand (true);
        desc_label->property_justify () = Gtk::JUSTIFY_CENTER;
        hero_description_box->add (*manage (desc_label));
        hero_description_box->show_all ();
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

StackInfoTip::~StackInfoTip()
{
  delete window;
}
