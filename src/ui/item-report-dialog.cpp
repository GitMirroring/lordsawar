//  Copyright (C) 2010, 2014, 2017 Ben Asselstine
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

#include "item-report-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "stack.h"
#include "MapBackpack.h"
#include "itemmap.h"
#include "playerlist.h"

#define method(x) sigc::mem_fun(*this, &ItemReportDialog::x)

ItemReportDialog::ItemReportDialog(Gtk::Window &parent,
                                   const std::list<Stack*> &item_laden_stacks,
                                   const std::list<MapBackpack*> &bags_of_stuff)
 : LwDialog(parent, "item-report-dialog.ui"), stacks (item_laden_stacks),
    bags (bags_of_stuff), itemmap (new ItemMap(item_laden_stacks, bags))
{
  xml->get_widget("map_image", map_image);

  itemmap->map_changed.connect(method(on_map_changed));

  xml->get_widget("label", label);

  fill_in_item_info();
}

void ItemReportDialog::run()
{
  itemmap->resize();
  itemmap->draw();

  dialog->show_all();
  dialog->run();
}

void ItemReportDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, itemmap->get_width(), itemmap->get_height());
  map_image->property_pixbuf() = pixbuf;
  fill_in_item_info();
}

void ItemReportDialog::fill_in_item_info()
{
  int count = 0;
  for (std::list<Stack*>::iterator it = stacks.begin(); it != stacks.end();
       ++it)
    {
      Stack *stack = (*it);
      count += stack->countItems();
    }

  Glib::ustring s = "";
  if (count > 0)
    s = String::ucompose(ngettext("You have %1 item!",
                                  "You have %1 items!", count), count);
  else
    s += _("You don't have any items!");
  label->set_text(s);
}
