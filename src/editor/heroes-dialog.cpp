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

#include <sigc++/functors/mem_fun.h>

#include "heroes-dialog.h"
#include "herotemplates.h"

#include <gtkmm.h>
#include "player.h"
#include "ucompose.hpp"
#include "heroproto.h"
#include "playerlist.h"

#define method(x) sigc::mem_fun(*this, &HeroesDialog::x)

HeroesDialog::HeroesDialog(Gtk::Window &parent, guint32 player_id, Glib::ustring player_name)
 : LwEditorDialog(parent, "heroes-dialog.ui"),
    name_column(_("Name"), name_renderer)
{
  d_changed = false;
  d_player_id = player_id;
  xml->get_widget("treeview", treeview);
  xml->get_widget("panel_box", panel_box);

  int pw, ph;
  parent.get_size (pw, ph);
  dialog->set_size_request (ph * (16.0/9.0), ph);

  dialog->set_title (String::ucompose (_("Heroes of %1"), player_name));

  // setup the hero settings
  hero_list = Gtk::ListStore::create(hero_columns);
  treeview->set_model(hero_list);

  // name column
  name_renderer.property_editable() = false;
  name_column.set_cell_data_func(name_renderer, method(cell_data_name));
  treeview->append_column(name_column);

  xml->get_widget("name_entry", name_entry);

  Gtk::Box *box;
  xml->get_widget("gender_box", box);
  gender_combobox = manage(new Gtk::ComboBoxText);
  gender_combobox->append (Hero::genderToFriendlyName(Hero::MALE));
  gender_combobox->append (Hero::genderToFriendlyName (Hero::FEMALE));
  box->set_center_widget (*gender_combobox);

  fill_heroes ();

  xml->get_widget("add_button", add_button);
  add_button->signal_clicked().connect (method (on_add_pressed));
  xml->get_widget("remove_button", remove_button);
  remove_button->signal_clicked().connect (method (on_remove_pressed));
  
  treeview->get_selection()->signal_changed().connect(method(on_hero_selected));
  treeview->set_cursor (Gtk::TreePath ("0"));
  update_panel ();
}

bool HeroesDialog::run()
{
  dialog->show_all ();
  dialog->run();
  return d_changed;
}

void HeroesDialog::on_add_pressed ()
{
  Gtk::TreeIter i = hero_list->append();
  (*i)[hero_columns.name] = _("Unnamed Hero");
  HeroProto *hero = new HeroProto;
  hero->setGender (Hero::FEMALE);
  hero->setName ((*i)[hero_columns.name]);
  hero->setOwnerId (d_player_id);
  (*i)[hero_columns.hero] = hero;
  treeview->get_selection ()->select (i);
  update_hero_templates ();
}

void HeroesDialog::on_remove_pressed ()
{
  Glib::RefPtr<Gtk::TreeSelection> selection = treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow)
    {
      Gtk::TreeModel::Row row = *iterrow;
      HeroProto *h = row[hero_columns.hero];
      hero_list->erase(iterrow);
      delete h;
      update_hero_templates ();
    }
}

void HeroesDialog::fill_heroes ()
{
  std::vector<HeroProto *> heroes =
    HeroTemplates::getInstance ()->getHeroes (d_player_id);
  for (auto h : heroes)
    {
      Gtk::TreeIter i = hero_list->append();
      (*i)[hero_columns.name] = h->getName ();
      (*i)[hero_columns.hero] = h;
    }
}

void HeroesDialog::cell_data_name(Gtk::CellRenderer *renderer,
                                  const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text() =
    String::ucompose("%1", (*i)[hero_columns.name]);
}

void HeroesDialog::update_buttons ()
{
  remove_button->set_sensitive (get_selected_hero () != NULL);
}
  
HeroProto* HeroesDialog::get_selected_hero ()
{
  Gtk::TreeIter i = treeview->get_selection()->get_selected();
  if (i)
    {
      HeroProto *h = (*i)[hero_columns.hero];
      return h;
    }
  else
    return NULL;
}

void HeroesDialog::on_hero_selected ()
{
  update_panel ();
  update_buttons ();
}

void HeroesDialog::update_hero_templates ()
{
  d_changed = true;
  std::vector<HeroProto*> heroes;
  for (auto i : hero_list->children ())
    {
      HeroProto *hero = (*i)[hero_columns.hero];
      heroes.push_back (new HeroProto (*hero));
    }
  HeroTemplates::getInstance ()->replaceHeroes (d_player_id, heroes);
}

HeroesDialog::~HeroesDialog ()
{
  for (auto i : hero_list->children ())
    {
      HeroProto *hero = (*i)[hero_columns.hero];
      delete hero;
    }
}

void HeroesDialog::on_name_changed ()
{
  HeroProto *hero = get_selected_hero ();
  if (hero)
    {
      Glib::RefPtr<Gtk::TreeSelection> selection = treeview->get_selection();
      Gtk::TreeModel::iterator iterrow = selection->get_selected();
      if (iterrow) 
        {
          Gtk::TreeModel::Row row = *iterrow;
          row[hero_columns.name] = String::utrim (name_entry->get_text());
        }
      hero->setName (String::utrim (name_entry->get_text ()));
      update_hero_templates ();
    }
}

void HeroesDialog::on_gender_changed ()
{
  HeroProto *hero = get_selected_hero ();
  if (hero)
    {
      if (gender_combobox->get_active_row_number () == 0)
        hero->setGender (Hero::MALE);
      else
        hero->setGender (Hero::FEMALE);
      update_hero_templates ();
    }
}

void HeroesDialog::update_panel ()
{
  disconnect_signals ();
  HeroProto *hero = get_selected_hero ();
  if (hero)
    {
      name_entry->set_text (hero->getName ());
      switch (hero->getGender ())
        {
        case Hero::MALE:
          gender_combobox->set_active (0);
          break;
        case Hero::FEMALE:
          gender_combobox->set_active (1);
          break;
        }
    }
  else
    {
      name_entry->set_text ("");
      gender_combobox->set_active (0);
    }
  panel_box->set_sensitive (hero != NULL);
  connect_signals ();
}

void HeroesDialog::connect_signals ()
{
  connections.push_back
    (gender_combobox->signal_changed().connect (method (on_gender_changed)));
  connections.push_back
    (name_entry->signal_changed().connect (method (on_name_changed)));
}

void HeroesDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}
