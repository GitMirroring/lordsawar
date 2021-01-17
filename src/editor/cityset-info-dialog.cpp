//  Copyright (C) 2007, 2008, 2009, 2010, 2014, 2020, 2021 Ben Asselstine
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

#include "cityset-info-dialog.h"
#include "citysetlist.h"
#include "ucompose.hpp"
#include "defs.h"
#include "File.h"

#define method(x) sigc::mem_fun(*this, &CitySetInfoDialog::x)

CitySetInfoDialog::CitySetInfoDialog(Gtk::Window &parent, Cityset *c)
 : LwEditorDialog(parent, "cityset-info-dialog.ui")
{
  d_cityset = c;
  dialog->set_title(_("City Set Properties"));

  xml->get_widget("close_button", close_button);
  xml->get_widget("status_label", status_label);
  xml->get_widget("location_label", location_label);
  xml->get_widget("name_entry", name_entry);
  xml->get_widget("size_spinbutton", size_spinbutton);
  xml->get_widget("fit_button", fit_button);

  size_spinbutton->set_value ((double)c->getTileSize ());
  size_spinbutton->signal_changed().connect (method(on_size_changed));
  fit_button->signal_clicked().connect (method(on_fit_pressed));

  name_entry->set_text (d_cityset->getName ());
  location_label->property_label () =
    d_cityset->getDirectory ().empty () ? "" :
    d_cityset->getConfigurationFile (true);

  name_entry->signal_changed().connect (method(on_name_changed));

  xml->get_widget("copyright_textview", copyright_textview);
  copyright_textview->get_buffer()->set_text(d_cityset->getCopyright());
  copyright_textview->get_buffer()->signal_changed().connect
    (method(on_copyright_changed));
  xml->get_widget("license_textview", license_textview);
  license_textview->get_buffer()->set_text(d_cityset->getLicense());
  license_textview->get_buffer()->signal_changed().connect
    (method(on_license_changed));
  xml->get_widget("description_textview", description_textview);
  description_textview->get_buffer()->set_text(d_cityset->getInfo());
  description_textview->get_buffer()->signal_changed().connect
    (method(on_description_changed));
  xml->get_widget("notebook", notebook);
  on_name_changed ();
  d_name = d_cityset->getName ();
  d_description = d_cityset->getInfo ();
  d_copyright = d_cityset->getCopyright ();
  d_license = d_cityset->getLicense ();
  d_tilesize = d_cityset->getTileSize ();
  d_changed = false;
}

void CitySetInfoDialog::on_name_changed()
{
  d_changed = true;
  Glib::ustring oldname = d_cityset->getName ();
  d_cityset->setName (String::utrim (name_entry->get_text ()));
  close_button->set_sensitive (File::sanify (d_cityset->getName ()) != "");

  Glib::ustring file =
    Citysetlist::getInstance()->lookupConfigurationFileByName(d_cityset);
  if (file != "" && file != d_cityset->getConfigurationFile (true))
    status_label->set_text (_("That name is already in use."));
  else
    status_label->set_text ("");
  d_cityset->setName (oldname);
  d_name = String::utrim (name_entry->get_text ());
}

bool CitySetInfoDialog::run()
{
  dialog->show_all();
  dialog->run();
  dialog->hide ();
  return d_changed;
}

void CitySetInfoDialog::on_copyright_changed ()
{
  d_changed = true;
  d_copyright = copyright_textview->get_buffer()->get_text();
}

void CitySetInfoDialog::on_license_changed ()
{
  d_changed = true;
  d_license = license_textview->get_buffer()->get_text();
}

void CitySetInfoDialog::on_description_changed ()
{
  d_changed = true;
  d_description = description_textview->get_buffer()->get_text();
}

CitySetInfoDialog::~CitySetInfoDialog()
{
  notebook->property_show_tabs () = false;
}

void CitySetInfoDialog::on_size_changed()
{
  d_changed = true;
  d_tilesize = size_spinbutton->get_value ();
  on_name_changed ();
}

void CitySetInfoDialog::on_fit_pressed()
{
  d_changed = true;
  guint32 ts = 0;
  d_cityset->calculate_preferred_tile_size (ts);
  size_spinbutton->set_value (ts);
  on_name_changed ();
}
