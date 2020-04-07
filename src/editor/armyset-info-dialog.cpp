//  Copyright (C) 2007, 2008, 2009, 2010, 2014, 2020 Ben Asselstine
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

#include "armyset-info-dialog.h"
#include "armysetlist.h"

#include "ucompose.hpp"
#include "defs.h"
#include "File.h"


#define method(x) sigc::mem_fun(*this, &ArmySetInfoDialog::x)

ArmySetInfoDialog::ArmySetInfoDialog(Gtk::Window &parent, Armyset *armyset)
 : LwEditorDialog(parent, "armyset-info-dialog.ui")
{
  d_armyset = armyset;
  dialog->set_title(_("Army Set Properties"));

  xml->get_widget("close_button", close_button);
  xml->get_widget("status_label", status_label);
  xml->get_widget("location_label", location_label);
  location_label->property_label () =
    d_armyset->getDirectory ().empty () ? "" :
    d_armyset->getConfigurationFile (true);

  xml->get_widget("name_entry", name_entry);
  name_entry->set_text(armyset->getName());
  name_entry->signal_changed().connect (method (on_name_changed));

  xml->get_widget("copyright_textview", copyright_textview);
  copyright_textview->get_buffer()->set_text(d_armyset->getCopyright());
  copyright_textview->get_buffer()->signal_changed().connect
    (method(on_copyright_changed));
  xml->get_widget("license_textview", license_textview);
  license_textview->get_buffer()->set_text(d_armyset->getLicense());
  license_textview->get_buffer()->signal_changed().connect
    (method(on_license_changed));
  xml->get_widget("description_textview", description_textview);
  description_textview->get_buffer()->set_text(d_armyset->getInfo());
  description_textview->get_buffer()->signal_changed().connect
    (method(on_description_changed));
  xml->get_widget("notebook", notebook);
  xml->get_widget("size_spinbutton", size_spinbutton);
  xml->get_widget("fit_button", fit_button);

  size_spinbutton->set_value ((double)armyset->getTileSize ());
  size_spinbutton->signal_changed().connect (method(on_size_changed));
  fit_button->signal_clicked().connect (method(on_fit_pressed));
  on_name_changed ();
  d_changed = false;
}

void ArmySetInfoDialog::on_name_changed()
{
  d_changed = true;
  d_armyset->setName (String::utrim (name_entry->get_text ()));
  close_button->set_sensitive (File::sanify (d_armyset->getName ()) != "");

  Glib::ustring file =
    Armysetlist::getInstance()->lookupConfigurationFileByName(d_armyset);
  if (file != "" && file != d_armyset->getConfigurationFile (true))
    status_label->set_text (_("That name is already in use."));
  else
    status_label->set_text ("");
}

bool ArmySetInfoDialog::run()
{
  dialog->show_all();
  dialog->run();
  dialog->hide ();
  return d_changed;
}

void ArmySetInfoDialog::on_copyright_changed ()
{
  d_changed = true;
  d_armyset->setCopyright(copyright_textview->get_buffer()->get_text());
}

void ArmySetInfoDialog::on_license_changed ()
{
  d_changed = true;
  d_armyset->setLicense(license_textview->get_buffer()->get_text());
}

void ArmySetInfoDialog::on_description_changed ()
{
  d_changed = true;
  d_armyset->setInfo(description_textview->get_buffer()->get_text());
}

ArmySetInfoDialog::~ArmySetInfoDialog()
{
  notebook->property_show_tabs () = false;
}

void ArmySetInfoDialog::on_size_changed()
{
  d_changed = true;
  d_armyset->setTileSize (size_spinbutton->get_value ());
  on_name_changed ();
}

void ArmySetInfoDialog::on_fit_pressed()
{
  d_changed = true;
  guint32 ts = 0;
  d_armyset->calculate_preferred_tile_size (ts);
  size_spinbutton->set_value (ts);
  on_name_changed ();
}
