//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2020 Ben Asselstine
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

#include "temple-editor-dialog.h"

#include "ucompose.hpp"
#include "CreateScenarioRandomize.h"
#include "defs.h"
#include "temple.h"
#include "RenamableLocation.h"

#define method(x) sigc::mem_fun(*this, &TempleEditorDialog::x)

TempleEditorDialog::TempleEditorDialog(Gtk::Window &parent, Temple *t, CreateScenarioRandomize *randomizer)
 : LwEditorDialog(parent, "temple-editor-dialog.ui")
{
    d_randomizer = randomizer;
    temple = t;
    
    xml->get_widget("name_entry", name_entry);
    name_entry->signal_changed().connect (method(on_name_changed));
    name_entry->set_text(temple->getName());

    xml->get_widget("description_entry", description_entry);
    description_entry->signal_changed().connect
      (method(on_description_changed));
    description_entry->set_text(temple->getDescription());

    xml->get_widget("type_spinbutton", type_spinbutton);
    type_spinbutton->set_value(temple->getType());
    type_spinbutton->signal_changed().connect (method(on_type_changed));
    type_spinbutton->signal_insert_text().connect
      (sigc::hide(sigc::hide(method(on_type_text_changed))));

    xml->get_widget("randomize_name_button", randomize_name_button);
    randomize_name_button->signal_clicked().connect(
	sigc::mem_fun(this, &TempleEditorDialog::on_randomize_name_clicked));
}

void TempleEditorDialog::on_type_changed ()
{
  if (type_spinbutton->get_value() >= TEMPLE_TYPES)
    type_spinbutton->set_value(TEMPLE_TYPES - 1);
  else
    temple->setType(int(type_spinbutton->get_value()));
}

void TempleEditorDialog::on_type_text_changed ()
{
  type_spinbutton->set_value(atoi(type_spinbutton->get_text().c_str()));
  on_type_changed();
}

int TempleEditorDialog::run()
{
  dialog->show_all();
  return dialog->run();
}

void TempleEditorDialog::on_randomize_name_clicked()
{
  Glib::ustring existing_name = name_entry->get_text();
  if (existing_name == Temple::getDefaultName())
    name_entry->set_text(d_randomizer->popRandomTempleName());
  else
    {
      name_entry->set_text(d_randomizer->popRandomTempleName());
      d_randomizer->pushRandomTempleName(existing_name);
    }
}

void TempleEditorDialog::on_description_changed ()
{
  temple->setDescription(description_entry->get_text());
}

void TempleEditorDialog::on_name_changed ()
{
  Location *l = temple;
  RenamableLocation *renamable_temple = static_cast<RenamableLocation*>(l);
  renamable_temple->setName(name_entry->get_text());
}
