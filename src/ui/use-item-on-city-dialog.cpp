//  Copyright (C) 2011, 2012, 2014, 2015, 2017 Ben Asselstine
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

#include "use-item-on-city-dialog.h"

#include "input-helpers.h"
#include "ucompose.hpp"
#include "ImageCache.h"
#include "city.h"
#include "playerlist.h"

#define method(x) sigc::mem_fun(*this, &UseItemOnCityDialog::x)

UseItemOnCityDialog::UseItemOnCityDialog(Gtk::Window &parent, SelectCityMap::Type type)
 : LwDialog(parent, "use-item-on-city-dialog.ui")
{
  xml->get_widget("map_image", map_image);
  xml->get_widget("continue_button", continue_button);

  citymap = new SelectCityMap(type);
  citymap->map_changed.connect (method(on_map_changed));
  citymap->city_selected.connect(sigc::hide(method(on_city_selected)));

  Gtk::EventBox *map_eventbox;
  xml->get_widget("map_eventbox", map_eventbox);
  map_eventbox->add_events(Gdk::BUTTON_PRESS_MASK);
  map_eventbox->signal_button_press_event().connect
    (method(on_map_mouse_button_event));

  continue_button->set_sensitive(false);

  xml->get_widget("label", label);
  switch (type)
    {
    case SelectCityMap::ANY_CITY:
      label->set_text(_("Select a city to target."));
      break;
    case SelectCityMap::FRIENDLY_CITY:
      label->set_text(_("Select one of your cities to target."));
      break;
    case SelectCityMap::ENEMY_CITY:
      label->set_text(_("Select an enemy city to target."));
      break;
    case SelectCityMap::NEUTRAL_CITY:
      label->set_text(_("Select a neutral city to target."));
      break;
    }
}

City* UseItemOnCityDialog::run()
{
  citymap->resize();
  citymap->draw();
  dialog->show_all();
  dialog->run();
  return citymap->get_selected_city();
}

void UseItemOnCityDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  map_image->property_pixbuf() =
    Gdk::Pixbuf::create(map, 0, 0, citymap->get_width(), citymap->get_height());
}

void UseItemOnCityDialog::on_city_selected()
{
  continue_button->set_sensitive(true);
}

bool UseItemOnCityDialog::on_map_mouse_button_event(GdkEventButton *e)
{
  if (e->type != GDK_BUTTON_PRESS)
    return true;	// useless event
  citymap->mouse_button_event(to_input_event(e));
  return true;
}

