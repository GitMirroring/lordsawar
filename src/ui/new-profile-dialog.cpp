//  Copyright (C) 2011, 2014, 2015 Ben Asselstine
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

#include "new-profile-dialog.h"

#define method(x) sigc::mem_fun(*this, &NewProfileDialog::x)

NewProfileDialog::NewProfileDialog(Gtk::Window &parent)
 : LwDialog(parent, "new-profile-dialog.ui")
{
  xml->get_widget("accept_button", accept_button);
  xml->get_widget("nick_entry", nick_entry);
  nick_entry->set_activates_default(true);
  nick_entry->signal_changed().connect(method(on_nickname_changed));
  update_buttons();
}
	    
void NewProfileDialog::on_nickname_changed()
{
  update_buttons();
}

void NewProfileDialog::update_buttons()
{
  if (String::utrim(nick_entry->get_text()) == "")
    accept_button->set_sensitive(false);
  else
    {
      accept_button->set_sensitive(true);
      accept_button->property_can_focus() = true;
      accept_button->property_can_default() = true;
      accept_button->property_has_default() = true;
      nick_entry->property_activates_default() = true;
      accept_button->property_receives_default() = true;
    }
}
