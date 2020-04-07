//  Copyright (C) 2020 Ben Asselstine
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
#include <ostream>
#include <iostream>
#include <sstream>

#include <sigc++/functors/mem_fun.h>

#include "army-chooser-button.h"

#include "player.h"
#include "armysetlist.h"

#define method(x) sigc::mem_fun(*this, &ArmyChooserButton::x)

ArmyChooserButton::ArmyChooserButton(Gtk::Window &w,
                                     Glib::RefPtr<Gtk::Builder> xml,
                                     Glib::ustring widget,
                                     Player *p, SelectArmyDialog::Mode mode)
:d_window (w), d_player (p), d_mode (mode), d_selected (false),
    d_selected_army_type (0)
{
  xml->get_widget (widget, d_button);
  d_button->signal_clicked ().connect(method (on_button_pressed));
  update_button ();
}

void ArmyChooserButton::on_button_pressed ()
{
  int pre_selected = -1;
  if (d_selected)
    pre_selected = d_selected_army_type;
  SelectArmyDialog d (d_window, d_mode, d_player, pre_selected);
  d.run ();
  const ArmyProto *a = d.get_selected_army ();
  if (a)
    {
      d_selected = true;
      d_selected_army_type = a->getId ();
    }
  update_button ();
  army_selected.emit (a);
}

void ArmyChooserButton::update_button ()
{
  Glib::ustring name = "";
  if (d_selected)
    {
      ArmyProto *a =
        Armysetlist::getInstance ()->getArmy (d_player->getArmyset (),
                                              d_selected_army_type);
      if (a)
        name = a->getName ();
    }

  if (name != "")
    d_button->set_label (name);
  else
    d_button->set_label (_("No army type selected"));
}

void ArmyChooserButton::select (guint32 selected)
{
  d_selected = true;
  d_selected_army_type = selected;
  update_button ();
}

void ArmyChooserButton::clear_selected_army ()
{
  d_selected = false;
  d_selected_army_type = 0;
  update_button ();
}
