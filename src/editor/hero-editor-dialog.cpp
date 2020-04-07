//  Copyright (C) 2009, 2014, 2020 Ben Asselstine
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

#include "hero-editor-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "hero.h"
#include "backpack-editor-dialog.h"
#include "Backpack.h"

#define method(x) sigc::mem_fun(*this, &HeroEditorDialog::x)

HeroEditorDialog::HeroEditorDialog(Gtk::Window &parent, Hero *hero)
 : LwEditorDialog(parent, "hero-editor-dialog.ui")
{
  d_hero = hero;

  xml->get_widget("edit_backpack_button", edit_backpack_button);
  edit_backpack_button->signal_clicked().connect (method(on_edit_backpack_clicked));
  xml->get_widget("gender_combobox", gender_combobox);
  xml->get_widget("name_entry", name_entry);
  name_entry->set_text(d_hero->getName());
  gender_combobox->set_active(d_hero->getGender()-1);
  gender_combobox->signal_changed ().connect (method (on_gender_changed));
  name_entry->signal_changed ().connect (method (on_name_changed));
  update_buttons ();
}

int HeroEditorDialog::run()
{
  dialog->show_all();
  return dialog->run ();
}

void HeroEditorDialog::on_edit_backpack_clicked()
{
  BackpackEditorDialog d(*dialog, d_hero->getBackpack());
  d.run();
  update_buttons ();
  return;
}

void HeroEditorDialog::on_name_changed ()
{
  d_hero->setName (String::utrim (name_entry->get_text ()));
}

void HeroEditorDialog::on_gender_changed ()
{
  d_hero->setGender(Hero::Gender(gender_combobox->get_active_row_number()+1));
}

void HeroEditorDialog::update_buttons ()
{
  edit_backpack_button->set_label
    (String::ucompose (ngettext ("Carrying %1 item",
                                 "Carrying %1 items",
                                 d_hero->getBackpack()->size ()),
                       d_hero->getBackpack()->size ()));
}
