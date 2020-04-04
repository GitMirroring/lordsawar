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
    gender_column(_("Gender"), gender_renderer),
    name_column(_("Name"), name_renderer)
{
  d_changed = false;
  d_player_id = player_id;
  xml->get_widget("treeview", treeview);

  dialog->set_title (String::ucompose (_("Heroes of %1"), player_name));

  // setup the hero settings
  hero_list = Gtk::ListStore::create(hero_columns);
  treeview->set_model(hero_list);

  // the type column
  hero_gender_list = Gtk::ListStore::create(hero_gender_columns);
  Gtk::TreeModel::iterator i;
  i = hero_gender_list->append();
  (*i)[hero_gender_columns.gender] = Hero::genderToFriendlyName(Hero::MALE);
  i = hero_gender_list->append();
  (*i)[hero_gender_columns.gender] = Hero::genderToFriendlyName (Hero::FEMALE);

  // gender column
  gender_renderer.property_model() = hero_gender_list;
  gender_renderer.property_text_column() = 0;
  gender_renderer.property_has_entry() = false;
  gender_renderer.property_editable() = true;

  gender_renderer.signal_edited().connect(method(on_gender_edited));
  gender_column.set_cell_data_func(gender_renderer, method(cell_data_gender));
  treeview->append_column(gender_column);

  // name column
  name_renderer.property_editable() = true;
  name_renderer.signal_edited().connect (method(on_name_edited));
  name_column.set_cell_data_func(name_renderer, method(cell_data_name));
  treeview->append_column(name_column);

  fill_heroes ();

  xml->get_widget("add_button", add_button);
  add_button->signal_clicked().connect (method (on_add_pressed));
  xml->get_widget("remove_button", remove_button);
  remove_button->signal_clicked().connect (method (on_remove_pressed));
  
  treeview->get_selection()->signal_changed().connect(method(on_hero_selected));
  treeview->set_cursor (Gtk::TreePath ("0"));
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
  (*i)[hero_columns.gender] =
    Hero::genderToFriendlyName(Hero::Gender(Hero::FEMALE));
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
      (*i)[hero_columns.gender] =
        Hero::genderToFriendlyName(Hero::Gender(h->getGender ()));
      (*i)[hero_columns.hero] = h;
    }
}

void HeroesDialog::cell_data_name(Gtk::CellRenderer *renderer,
                                  const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text() =
    String::ucompose("%1", (*i)[hero_columns.name]);
}

void HeroesDialog::on_name_edited(const Glib::ustring &path,
                                  const Glib::ustring &new_text)
{
  HeroProto *h = get_selected_hero ();
  h->setName (String::utrim (new_text));
  Gtk::TreeIter i = hero_list->get_iter(Gtk::TreePath(path));
  (*i)[hero_columns.name] = h->getName ();
  update_hero_templates ();
}

void HeroesDialog::cell_data_gender (Gtk::CellRenderer *renderer,
                                     const Gtk::TreeIter& i)
{
  dynamic_cast<Gtk::CellRendererText*>(renderer)->property_text()
    = (*i)[hero_columns.gender];
}

void HeroesDialog::on_gender_edited(const Glib::ustring &path,
                                    const Glib::ustring &new_text)
{
  HeroProto *h = get_selected_hero ();
  (*hero_list->get_iter(Gtk::TreePath(path)))[hero_columns.gender] = new_text;
  Hero::Gender gender = Hero::friendlyNameToGender (new_text);
  h->setGender (gender);
  update_hero_templates ();
}

void HeroesDialog::update_buttons ()
{
  remove_button->set_sensitive (get_selected_hero () != NULL);
}
  
HeroProto * HeroesDialog::get_selected_hero ()
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
