//  Copyright (C) 2021, 2026 Ben Asselstine
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
#ifndef SELECT_CHARACTER_DIALOG_H
#define SELECT_CHARACTER_DIALOG_H
class SelectCharacterRow: public Glib::Object
{
public:
    Character *m_character;

    static Glib::RefPtr<SelectCharacterRow> create (Character *c)
      {
        return
          Glib::make_refptr_for_instance<SelectCharacterRow>
          (new SelectCharacterRow (c));
      }

protected:
    SelectCharacterRow (Character *c)
      : m_character (c)
      {
      }
};

class SelectCharacterDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "select-character.ui";
      }

    SelectCharacterDialog (BaseObjectType* o,
                           const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_select_button = load <Gtk::Button> ("select_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
      }

    void setup (guint32 character_id, Shield::Color shield)
      {
        m_shield = shield;
        m_selected_character_id = character_id;
        set_response (m_select_button, Gtk::ResponseType::ACCEPT);

        m_store = Gio::ListStore<SelectCharacterRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        setup_name_column ();

        update ();

        m_selection_model->signal_selection_changed ().connect
          ([this] (const guint &, const guint &)
           {
             auto item = m_selection_model->get_selected_item ();
             if (item)
               {
                 auto row = std::dynamic_pointer_cast<SelectCharacterRow>(item);
                 m_selected_character_id = row->m_character->get_id ();
               }
           });

        auto item = m_selection_model->get_selected_item ();
        if (item)
          {
            auto row = std::dynamic_pointer_cast<SelectCharacterRow>(item);
            m_selected_character_id = row->m_character->get_id ();
          }
      }

    guint32 get_selected_character ()
      {
        return m_selected_character_id;
      }

    void update ()
      {
        fill_treeview ();
      }
private:
    Gtk::Button *m_select_button;
    Gtk::ColumnView *m_treeview;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<SelectCharacterRow>> m_store;

    Shield::Color m_shield;
    guint32 m_selected_character_id;

    void setup_name_column ()
      {
        LwColumn::setup_text_column<SelectCharacterRow> 
          (m_treeview, "name_label", true, Gtk::Justification::LEFT,
           _("Characters"),
           [] (const auto& row)
           {
             return row->m_character->get_name ();
           });
      }

    void fill_treeview ()
      {
        m_store->remove_all ();
        int i = 0;
        int found = -1;
        for (auto c : HeroTemplates::instance ()->getHeroes (m_shield))
          {
            m_store->append (SelectCharacterRow::create (c));
            if (c->get_id () == m_selected_character_id)
              found = i;
            i++;
          }

        if (found >= 0)
          m_selection_model->set_selected (found);
      }
};
#endif
