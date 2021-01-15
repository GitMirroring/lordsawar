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

#include "shieldset-info-dialog.h"
#include "shieldsetlist.h"

#include "ucompose.hpp"
#include "defs.h"
#include "File.h"

#define method(x) sigc::mem_fun(*this, &ShieldSetInfoDialog::x)

ShieldSetInfoDialog::ShieldSetInfoDialog(Gtk::Window &parent, Shieldset *s)
 : LwEditorDialog(parent, "shieldset-info-dialog.ui")
{
  d_shieldset = s;
  dialog->set_title(_("Shield Set Properties"));

  xml->get_widget("close_button", close_button);
  xml->get_widget("status_label", status_label);
  xml->get_widget("location_label", location_label);
  xml->get_widget("name_entry", name_entry);
  xml->get_widget("small_width_spinbutton", small_width_spinbutton);
  xml->get_widget("small_height_spinbutton", small_height_spinbutton);
  xml->get_widget("medium_width_spinbutton", medium_width_spinbutton);
  xml->get_widget("medium_height_spinbutton", medium_height_spinbutton);
  xml->get_widget("large_width_spinbutton", large_width_spinbutton);
  xml->get_widget("large_height_spinbutton", large_height_spinbutton);
  xml->get_widget("fit_button", fit_button);
  fit_button->signal_clicked().connect (method(on_fit_pressed));

  small_width_spinbutton->set_value (s->getSmallWidth ());
  small_width_spinbutton->signal_changed().connect
    (method(on_small_width_changed));
  small_height_spinbutton->set_value (s->getSmallHeight ());
  small_height_spinbutton->signal_changed().connect
    (method(on_small_height_changed));
  medium_width_spinbutton->set_value (s->getMediumWidth ());
  medium_width_spinbutton->signal_changed().connect
    (method(on_medium_width_changed));
  medium_height_spinbutton->set_value (s->getMediumHeight ());
  medium_height_spinbutton->signal_changed().connect
    (method(on_medium_height_changed));
  large_width_spinbutton->set_value (s->getLargeWidth ());
  large_width_spinbutton->signal_changed().connect
    (method(on_large_width_changed));
  large_height_spinbutton->set_value (s->getLargeHeight ());
  large_height_spinbutton->signal_changed().connect
    (method(on_large_height_changed));

  name_entry->set_text (d_shieldset->getName ());
  location_label->property_label () =
    d_shieldset->getDirectory ().empty () ? "" :
    d_shieldset->getConfigurationFile (true);

  name_entry->signal_changed().connect (method(on_name_changed));

  xml->get_widget("copyright_textview", copyright_textview);
  copyright_textview->get_buffer()->set_text(d_shieldset->getCopyright());
  copyright_textview->get_buffer()->signal_changed().connect
    (method(on_copyright_changed));
  xml->get_widget("license_textview", license_textview);
  license_textview->get_buffer()->set_text(d_shieldset->getLicense());
  license_textview->get_buffer()->signal_changed().connect
    (method(on_license_changed));
  xml->get_widget("description_textview", description_textview);
  description_textview->get_buffer()->set_text(d_shieldset->getInfo());
  description_textview->get_buffer()->signal_changed().connect
    (method(on_description_changed));
  xml->get_widget("notebook", notebook);
  on_name_changed ();
  d_name = d_shieldset->getName ();
  d_description = d_shieldset->getInfo ();
  d_copyright = d_shieldset->getCopyright ();
  d_license = d_shieldset->getLicense ();
  d_small_width = d_shieldset->getSmallWidth ();
  d_small_height = d_shieldset->getSmallHeight ();
  d_medium_width = d_shieldset->getMediumWidth ();
  d_medium_height = d_shieldset->getMediumHeight ();
  d_large_width = d_shieldset->getLargeWidth ();
  d_large_height = d_shieldset->getLargeHeight ();
  d_changed = false;
}

void ShieldSetInfoDialog::on_name_changed()
{
  d_changed = true;
  Glib::ustring oldname = d_shieldset->getName ();
  d_shieldset->setName (String::utrim (name_entry->get_text ()));
  close_button->set_sensitive (File::sanify (d_shieldset->getName ()) != "");

  Glib::ustring file =
    Shieldsetlist::getInstance()->lookupConfigurationFileByName(d_shieldset);
  if (file != "" && file != d_shieldset->getConfigurationFile (true))
    status_label->set_text (_("That name is already in use."));
  else
    status_label->set_text ("");
  d_shieldset->setName (oldname);
  d_name = String::utrim (name_entry->get_text ());
}

bool ShieldSetInfoDialog::run()
{
  dialog->show_all();
  dialog->run();
  dialog->hide ();
  return d_changed;
}

void ShieldSetInfoDialog::on_copyright_changed ()
{
  d_changed = true;
  d_copyright = copyright_textview->get_buffer()->get_text();
}

void ShieldSetInfoDialog::on_license_changed ()
{
  d_changed = true;
  d_license = license_textview->get_buffer()->get_text();
}

void ShieldSetInfoDialog::on_description_changed ()
{
  d_changed = true;
  d_description = description_textview->get_buffer()->get_text();
}

ShieldSetInfoDialog::~ShieldSetInfoDialog()
{
  notebook->property_show_tabs () = false;
}

void ShieldSetInfoDialog::on_small_width_changed ()
{
  d_changed = true;
  d_small_width = small_width_spinbutton->get_value ();
}

void ShieldSetInfoDialog::on_small_height_changed ()
{
  d_changed = true;
  d_small_height = small_height_spinbutton->get_value ();
}

void ShieldSetInfoDialog::on_medium_width_changed ()
{
  d_changed = true;
  d_medium_width = medium_width_spinbutton->get_value ();
}

void ShieldSetInfoDialog::on_medium_height_changed ()
{
  d_changed = true;
  d_medium_height = medium_height_spinbutton->get_value ();
}

void ShieldSetInfoDialog::on_large_width_changed ()
{
  d_changed = true;
  d_large_width = large_width_spinbutton->get_value ();
}

void ShieldSetInfoDialog::on_large_height_changed ()
{
  d_changed = true;
  d_large_height = large_height_spinbutton->get_value ();
}

void ShieldSetInfoDialog::on_fit_pressed ()
{
  d_changed = true;
  d_shieldset->setHeightsAndWidthsFromImages();
  small_width_spinbutton->set_value ((double)d_shieldset->getSmallWidth());
  small_height_spinbutton->set_value ((double)d_shieldset->getSmallHeight());
  medium_width_spinbutton->set_value ((double)d_shieldset->getMediumWidth());
  medium_height_spinbutton->set_value ((double)d_shieldset->getMediumHeight());
  large_width_spinbutton->set_value ((double)d_shieldset->getLargeWidth());
  large_height_spinbutton->set_value ((double)d_shieldset->getLargeHeight());
}
