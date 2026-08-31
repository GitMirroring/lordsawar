//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2017 Ben Asselstine
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

#include "quest-assigned-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "playerlist.h"

#define method(x) sigc::mem_fun(*this, &QuestAssignedDialog::x)

QuestAssignedDialog::QuestAssignedDialog(Gtk::Window &parent, Hero *h, Quest *q)
 : LwDialog(parent, "quest-assigned-dialog.ui")
{
  hero = h;
  quest = q;
    
    xml->get_widget("map_image", map_image);

    questmap = new QuestMap(quest);
    questmap->map_changed.connect(method(on_map_changed));

    Gtk::EventBox *map_eventbox;
    xml->get_widget("map_eventbox", map_eventbox);

    dialog->set_title(String::ucompose(_("Quest for %1"), hero->getName()));

    xml->get_widget("label", label);
    Glib::ustring s;
    if (quest)
	s = quest->getDescription();
    else
	s = _("This hero already has a quest.");
    label->set_text(s);
    
}

void QuestAssignedDialog::run()
{
    questmap->resize();
    questmap->draw();

    dialog->show_all();
    dialog->run();
}

void QuestAssignedDialog::on_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
    Gdk::Pixbuf::create(map, 0, 0, 
                        questmap->get_width(), questmap->get_height());
    map_image->property_pixbuf() = pixbuf;
}

