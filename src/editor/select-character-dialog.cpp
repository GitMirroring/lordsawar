//  Copyright (C) 2021 Ben Asselstine
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
#include <assert.h>

#include "select-character-dialog.h"

#include "ucompose.hpp"
#include "defs.h"
#include "herotemplates.h"
#include "player.h"

SelectCharacterDialog::SelectCharacterDialog(Gtk::Window &parent, Player *p, guint32 id)
 : LwEditorDialog(parent, "select-character-dialog.ui")
{
  d_player = p;
  selected_character = id;

  xml->get_widget("select_button", select_button);

  xml->get_widget("characters_treeview", characters_treeview);
  characters_list = Gtk::ListStore::create(characters_columns);
  characters_treeview->set_model(characters_list);
  characters_treeview->append_column("", characters_columns.name);
  characters_treeview->set_headers_visible(false);

  std::vector<Character*> chr =
    HeroTemplates::getInstance ()->getHeroes (p->getId ());
  for (auto c : chr)
    addCharacter (c);
  if (chr.empty () == true)
    select_button->set_sensitive(false);
  for (auto c : chr)
    delete c;

}

void SelectCharacterDialog::addCharacter(Character *c)
{
  Gtk::TreeIter i = characters_list->append();
  (*i)[characters_columns.name] = c->name;
  (*i)[characters_columns.id] = c->id;
  if (c->id == (guint32)selected_character)
    characters_treeview->get_selection()->select(i);
}

bool SelectCharacterDialog::run()
{
  dialog->show_all();
  int response = dialog->run();

  if (response != Gtk::RESPONSE_ACCEPT)
    selected_character = -1;
  else
    {
      Glib::RefPtr<Gtk::TreeSelection> selection = 
        characters_treeview->get_selection();
      Gtk::TreeModel::iterator iterrow = selection->get_selected();

      if (iterrow) 
        {
          Gtk::TreeModel::Row row = *iterrow;
          selected_character = row[characters_columns.id];
        }
    }
  return response == Gtk::RESPONSE_ACCEPT;
}
