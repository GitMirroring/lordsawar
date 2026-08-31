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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#include <gtkmm.h>
#include "lw-dialog-base.h"
#include "character.h"
#include "image-helpers.h"
#include "lw-column.h"
#ifndef CHARACTERS_DIALOG_H
#define CHARACTERS_DIALOG_H
class CharacterRow: public Glib::Object
{
public:
    Glib::ustring m_name;
    Glib::ustring m_description;

    static Glib::RefPtr<CharacterRow> create (Glib::ustring n, Glib::ustring d)
      {
        return
          Glib::make_refptr_for_instance<CharacterRow>
          (new CharacterRow (n, d));
      }

protected:
    CharacterRow (Glib::ustring n, Glib::ustring d)
      : m_name (n), m_description (d)
      {
      }
};

class CharactersDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "characters.ui";
      }

    CharactersDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_button_box = load <Gtk::Box> ("button_box");
        m_treeview = load <Gtk::ColumnView> ("treeview");
      }

    void setup (std::list<Gtk::Image*> shields, std::string filename)
      {
        m_store = Gio::ListStore<CharacterRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);
        bool broken = false;
        ScenarioCharacterLoader loader (filename, broken);

        if (loader.characters.empty ())
          {
            CharacterLoader defloader (File::getMiscFile ("heronames.xml"));
            for (auto c : defloader.characters)
              m_characters.push_back (Character::copy (c));
          }
        else
          {
            for (auto c : loader.characters)
              m_characters.push_back (Character::copy (c));
          }

        Gtk::ToggleButton *active = NULL;
        bool first = true;
        int i = 0;
        for (auto sh : shields)
          {
            auto button = Gtk::make_managed<Gtk::ToggleButton> ("");
            button->set_child (*clone_image (sh));
            if (first)
              {
                active = button;
                first = false;
              }
            else
              button->set_group (*active);
            button->signal_toggled ().connect
              ([this, i] ()
               {
                 fill_heroes (Shield::Color (i));
               });
            m_button_box->append (*button);
            i++;
          }
        active->set_active (true);

        set_response (m_close_button, Gtk::ResponseType::CLOSE);

        signal_response ().connect
          ([this](Gtk::ResponseType  response)
           {
             switch (response)
               {
               default:
                 break;
               }
             hide ();
           });
        fill_heroes (Shield::WHITE);
        setup_name_column ();
        setup_desc_column ();
      }
private:
    Gtk::Box *m_button_box;
    Gtk::ColumnView *m_treeview;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<CharacterRow>> m_store;
    Gtk::Button *m_close_button;
    std::list<Character*> m_characters;

    void setup_name_column ()
      {
        LwColumn::setup_text_column<CharacterRow>
          (m_treeview, "name_label", false, Gtk::Justification::LEFT,
           _("Name"),
           [] (const auto& row)
           {
             return row->m_name;
           });
      }

    void setup_desc_column ()
      {
        LwColumn::setup_text_column<CharacterRow>
          (m_treeview, "desc_label", true, Gtk::Justification::LEFT,
           _("Description"),
           [] (const auto& row)
           {
             return row->m_description;
           });
      }

    void fill_heroes (Shield::Color shield)
      {
        m_store->remove_all ();
        for (auto j : m_characters)
          {
            if (j->get_shield () == shield)
              m_store->append (CharacterRow::create (j->get_name (),
                                                     j->get_description ()));
          }
      }
};
#endif
