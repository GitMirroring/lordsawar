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

#include "characters-dialog.h"

#include "heroproto.h"
#include "PixMask.h"

#define method(x) sigc::mem_fun(*this, &CharactersDialog::x)

CharactersDialog::CharactersDialog(Gtk::Window &parent, std::list<Gtk::Image*> shields, Glib::ustring filename)
 :LwDialog(parent, "characters-dialog.ui")
{
  bool broken = false;
  ScenarioCharacterLoader loader (filename, broken);

  if (loader.characters.empty())
    {
      CharacterLoader defloader (File::getMiscFile("heronames.xml"));
      for (auto c : defloader.characters)
        d_characters.push_back (Character::copy (c));
    }
  else
    {
      for (auto c : loader.characters)
        d_characters.push_back (Character::copy (c));
    }

  heroes_list = Gtk::ListStore::create(heroes_columns);
  xml->get_widget("treeview", heroes_treeview);
  heroes_treeview->set_model(heroes_list);
  heroes_treeview->append_column("", heroes_columns.name);
  heroes_treeview->append_column("", heroes_columns.desc);
  heroes_treeview->get_column (1)->set_expand (true);
  heroes_treeview->set_headers_visible(false);

  xml->get_widget("button_box", button_box);

  Gtk::RadioButtonGroup group;
  Gtk::RadioButton *active = NULL;
  bool first = true;
  guint32 id = 0;
  for (auto sh : shields)
    {
      Gtk::RadioButton *button = Gtk::manage(new Gtk::RadioButton ("ha"));
      button->property_draw_indicator () = false;
      button->remove ();
      PixMask *p = PixMask::create (sh->property_pixbuf ());
      Gtk::Image *image = Gtk::manage (new Gtk::Image ());
      image->property_pixbuf () = p->to_pixbuf ();
      delete p;
      button->add (*image);
      if (first)
        {
          active = button;
          first = false;
        }
      button->signal_toggled ().connect
        (sigc::bind (method (on_button_toggled), id));
      button->set_group (group);
      button_box->add (*button);
      id++;
    }
  active->set_active (true);
  fill_heroes (0);
}

CharactersDialog::~CharactersDialog()
{
  for (auto c : d_characters)
    delete c;
  d_characters.clear ();
}

void CharactersDialog::fill_heroes (guint32 id)
{
  heroes_list->clear ();
  for (auto c : d_characters)
    {
      if (c->owner == id)
        addHero (c);
    }
}

void CharactersDialog::on_button_toggled (guint32 id)
{
  fill_heroes (id);
}

void CharactersDialog::addHero(Character *c)
{
  Gtk::TreeIter i = heroes_list->append();
  (*i)[heroes_columns.name] = c->name;
  if (c->description.empty () == false)
    (*i)[heroes_columns.desc] = " - " + c->description;
}
