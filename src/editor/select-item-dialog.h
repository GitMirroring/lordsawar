//  Copyright (C) 2008, 2009, 2011, 2014, 2020, 2026 Ben Asselstine
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
#ifndef SELECT_ITEM_DIALOG_H
#define SELECT_ITEM_DIALOG_H
class SelectItemRow: public Glib::Object
{
public:
    guint32 m_item_id;

    static Glib::RefPtr<SelectItemRow> create (guint32 id)
      {
        return
          Glib::make_refptr_for_instance<SelectItemRow>
          (new SelectItemRow (id));
      }

protected:
    SelectItemRow (guint32 id)
      : m_item_id (id)
      {
      }
};

class SelectItemDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "select-item.ui";
      }

    enum
      {
        NO_ITEM_SELECTED = -1
      };

    SelectItemDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_selected_item_id = NO_ITEM_SELECTED;
        m_show_clear = false;
        m_select_button = load <Gtk::Button> ("select_button");
        m_clear_button = load <Gtk::Button> ("clear_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
      }

    void set_show_clear ()
      {
        m_show_clear = true;
      }

    void set_selected_item (guint32 id)
      {
        m_selected_item_id = id; 
      }

    void setup ()
      {
        m_clear_button->set_visible (m_show_clear);
        set_response (m_select_button, Gtk::ResponseType::ACCEPT);
        set_response (m_clear_button, Gtk::ResponseType::REJECT);

        m_store = Gio::ListStore<SelectItemRow>::create ();
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
                 auto row = std::dynamic_pointer_cast<SelectItemRow>(item);
                 m_selected_item_id = row->m_item_id;
               }
           });

        auto item = m_selection_model->get_selected_item ();
        if (item)
          {
            auto row = std::dynamic_pointer_cast<SelectItemRow>(item);
            m_selected_item_id = row->m_item_id;
          }
      }

    ItemProto* get_selected_item ()
      {
        if (m_selected_item_id == NO_ITEM_SELECTED)
          return NULL;
        return (*Itemlist::instance ())[m_selected_item_id];
      }

    int get_selected_item_id ()
      {
        return m_selected_item_id;
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
    Glib::RefPtr<Gio::ListStore<SelectItemRow>> m_store;

    bool m_show_clear;
    int m_selected_item_id;

    void setup_name_column ()
      {
        LwColumn::setup_text_column<SelectItemRow> 
          (m_treeview, "name_label", true, Gtk::Justification::LEFT,
           _("Items"),
           [] (const auto& row)
           {
             return (*Itemlist::instance ())[row->m_item_id]->getName ();
           });
      }

    void fill_treeview ()
      {
        m_store->remove_all ();
        int i = 0;
        int found = -1;
        for (auto j : *Itemlist::instance ())
          {
            int id = j.first;
            m_store->append (SelectItemRow::create (id));
            if (id == m_selected_item_id)
              found = i;
            i++;
          }

        if (found >= 0)
          m_selection_model->set_selected (found);
      }
};
#endif
