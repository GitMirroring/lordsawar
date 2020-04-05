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

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "keeper-editor-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "keeper.h"
#include "playerlist.h"
#include "select-army-dialog.h"
#include "armyproto.h"
#include "CreateScenarioRandomize.h"

#define method(x) sigc::mem_fun(*this, &KeeperEditorDialog::x)

KeeperEditorDialog::KeeperEditorDialog(Gtk::Window &parent, Keeper *k, Vector<int> pos, CreateScenarioRandomize *r)
 : LwEditorDialog(parent, "keeper-dialog.ui")
{
  d_pos = pos;
  d_changed = false;
  d_randomizer = r;
  xml->get_widget("keeper_button", keeper_button);
  xml->get_widget ("name_entry", name_entry);
  xml->get_widget("randomize_button", randomize_button);

  if (k)
    d_keeper = new Keeper (*k);
  else
    d_keeper = new Keeper (NULL, pos);

  fill_in_keeper_info();
  name_entry->signal_changed().connect (method(on_name_changed));
  keeper_button->signal_clicked().connect (method(on_keeper_clicked));
  randomize_button->signal_clicked().connect (method(on_randomize_clicked));
}

void KeeperEditorDialog::fill_in_keeper_info()
{
  name_entry->set_text (d_keeper->getName ());
  set_keeper_button_name();
}

bool KeeperEditorDialog::run()
{
  dialog->show_all();
  int response = dialog->run();

  if (response != Gtk::RESPONSE_ACCEPT)
    {
      if (d_keeper)
	delete d_keeper;
      d_keeper = NULL;
      d_changed = true;
    }
  return d_changed;
}

void KeeperEditorDialog::on_keeper_clicked()
{
  Player *neutral = Playerlist::getInstance ()->getNeutral ();
  SelectArmyDialog d(*dialog, true, neutral, false, true, false);
  d.run();
  Glib::ustring oldname =  "";
  Glib::ustring newname =  "";
  if (d_keeper->getStack ())
    oldname = d_keeper->getStack()->front()->getName ();
  if (d.get_selected_army())
    {
      d_keeper->clearStack ();
      ArmyProto *defender = new ArmyProto(*(d.get_selected_army()));
      d_keeper->add (defender, d_pos);
      fill_in_keeper_info ();
      newname = defender->getName ();
    }
  else
    {
      newname = "";
      d_keeper->clearStack ();
      d_keeper->setName ("");
      fill_in_keeper_info ();
    }
  if (oldname != newname)
    d_changed = true;
}

void KeeperEditorDialog::on_randomize_clicked()
{
  Keeper *k = d_randomizer->getRandomRuinKeeper (d_pos);
  if (!k)
    return;
  d_changed = true;

  if (d_keeper)
    delete d_keeper;
  d_keeper = k;

  fill_in_keeper_info ();
}

void KeeperEditorDialog::set_keeper_button_name()
{
  Glib::ustring name;
  if (d_keeper->getStack () && d_keeper->getStack()->empty () == false)
    name = d_keeper->getStack()->front()->getName();
  else
    name = _("No keeper");

  keeper_button->set_label(name);
}

void KeeperEditorDialog::on_name_changed ()
{
  d_changed = true;
  d_keeper->setName (String::utrim (name_entry->get_text ()));
}
