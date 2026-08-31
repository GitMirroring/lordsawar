//  Copyright (C) 2008, 2009, 2014, 2026 Ben Asselstine
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
#ifndef SELECT_HIDDEN_RUIN_DIALOG_H
#define SELECT_HIDDEN_RUIN_DIALOG_H
class SelectHiddenRuinRow: public Glib::Object
{
public:
    Ruin *m_ruin;

    static Glib::RefPtr<SelectHiddenRuinRow> create (Ruin *r)
      {
        return
          Glib::make_refptr_for_instance<SelectHiddenRuinRow>
          (new SelectHiddenRuinRow (r));
      }

protected:
    SelectHiddenRuinRow (Ruin *r)
      : m_ruin (r)
      {
      }
};

class SelectHiddenRuinDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "select-hidden-ruin.ui";
      }

    SelectHiddenRuinDialog (BaseObjectType* o,
                            const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_show_clear = false;
        m_select_button = load <Gtk::Button> ("select_button");
        m_clear_button = load <Gtk::Button> ("clear_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
        m_selected_ruin = NULL;
      }

    void set_show_clear ()
      {
        m_show_clear = true;
      }

    Ruin *get_selected_ruin ()
      {
        auto item = m_selection_model->get_selected_item ();
        if (item)
          {
            auto row = std::dynamic_pointer_cast<SelectHiddenRuinRow>(item);
            return row->m_ruin;
          }
        return NULL;
      }

    void set_selected_ruin (Ruin *r)
      {
        m_selected_ruin = r;
      }

    void setup ()
      {
        m_clear_button->set_visible (m_show_clear);
        set_response (m_select_button, Gtk::ResponseType::ACCEPT);
        set_response (m_clear_button, Gtk::ResponseType::REJECT);

        m_store = Gio::ListStore<SelectHiddenRuinRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        setup_name_column ();

        update ();
      }

    void update ()
      {
        fill_treeview ();
      }
private:
    Gtk::Button *m_select_button;
    Gtk::Button *m_clear_button;
    Gtk::ColumnView *m_treeview;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<SelectHiddenRuinRow>> m_store;

    bool m_show_clear;
    Ruin *m_selected_ruin;

    void setup_name_column ()
      {
        LwColumn::setup_text_column<SelectHiddenRuinRow> 
          (m_treeview, "name_label", true, Gtk::Justification::LEFT,
           _("Ruins"),
           [] (const auto& row)
           {
             return
               String::ucompose
               ("%1 (%2, %3)", row->m_ruin->getName (),
                row->m_ruin->getPos ().x, row->m_ruin->getPos ().y);
           });
      }

    void fill_treeview ()
      {
        m_store->remove_all ();
        int i = 0;
        int found = -1;

        for (auto ruin : *Ruinlist::instance ())
          {
            if (ruin->isHidden ())
              {
                m_store->append (SelectHiddenRuinRow::create (ruin));
                if (ruin == m_selected_ruin)
                  found = i;
                i++;
              }
          }

        if (found >= 0)
          m_selection_model->set_selected (found);
      }
};
#endif
