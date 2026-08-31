//  Copyright (C) 2009, 2011, 2014, 2020, 2021, 2026 Ben Asselstine
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
#ifndef BACKPACK_EDITOR_DIALOG_H
#define BACKPACK_EDITOR_DIALOG_H
#include "item.h"
#include "backpack.h"
#include "backpack-editor-undo-actions.h"
#include "lw-column.h"
#include "item-editor-dialog.h"

class PackItemRow: public Glib::Object
{
public:

    Item *m_item;

    static Glib::RefPtr<PackItemRow> create (Item *i)
      {
        return
          Glib::make_refptr_for_instance<PackItemRow> (new PackItemRow (i));
      }

protected:
    PackItemRow (Item *i)
      : m_item (i)
      {
      }

};

class BackpackEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "backpack-editor.ui";
      }

    BackpackEditorDialog (BaseObjectType* o,
                    const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_add_button = load <Gtk::Button> ("add_button");
        m_remove_button = load <Gtk::Button> ("remove_button");
        m_edit_button = load <Gtk::Button> ("edit_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");

      }

    ~BackpackEditorDialog ()
      {
        disconnect_signals ();
        delete m_backpack;
        delete m_umgr;
      }

    void setup (Backpack *backpack, Shield::Color shield)
      {
        m_backpack = new Backpack (*backpack);
        m_shield = shield;

        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_store = Gio::ListStore<PackItemRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        m_add_button->set_icon_name ("list-add-symbolic");
        m_remove_button->set_icon_name ("list-remove-symbolic");

        m_selection_model->signal_selection_changed ().connect
          ([this] (const guint &, const guint &)
           {
             update ();
           });

        fill_treeview ();

        setup_name_column ();

        setup_undo ();

        update ();
      }

    Backpack *get_backpack ()
      {
        return m_backpack;
      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }

private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_add_button;
    Gtk::Button *m_remove_button;
    Gtk::Button *m_edit_button;
    Gtk::ColumnView *m_treeview;

    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<PackItemRow>> m_store;

    std::list<sigc::connection> m_connections;

    UndoMgr *m_umgr;
    Backpack *m_backpack;
    Shield::Color m_shield;

    void setup_name_column ()
      {
        LwColumn::setup_text_column<PackItemRow> 
          (m_treeview, "name_label", true, Gtk::Justification::LEFT, _("Items"),
           [] (const auto& row)
           {
             return row->m_item->getName ();
           });
      }

    void
    add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void connect_signals ()
      {
        add_connection
          (m_add_button->signal_clicked ().connect
           ([this] ()
            {
              m_umgr->add (new BackpackEditorUndoAction_Add (m_backpack));

              auto i = new Item (_("Untitled"), false, NULL);
              m_backpack->addToBackpack (i);

              m_store->append (PackItemRow::create (i));

              guint n = m_selection_model->get_n_items ();
              if (n > 0)
                m_selection_model->set_selected (n - 1);

              scroll_treeview_to_bottom ();

              update ();
            }));

        add_connection
          (m_remove_button->signal_clicked ().connect
           ([this] ()
            {
              auto selected = m_selection_model->get_selected_item ();
              if (selected)
                {
                  m_umgr->add
                    (new BackpackEditorUndoAction_Remove (m_backpack));

                  auto item = get_selected_item ();
                  m_backpack->remove (item);

                  m_store->remove (m_selection_model->get_selected ());
                  update ();
                }
            }));

        add_connection
          (m_edit_button->signal_clicked ().connect
           ([this] ()
            {
              auto item = get_selected_item ();

              auto *action =
                new BackpackEditorUndoAction_Edit
                (m_selection_model->get_selected (), item);

              auto d = LwDialog::build<ItemEditorDialog> (this);
              d->setup (item, m_shield);
              d->signal_response ().connect
                ([this, d, action] (Gtk::ResponseType)
                 {
                   if (d->is_changed ())
                     m_umgr->add (action);
                   else
                     delete action;
                   fill_treeview ();
                   delete d;
                 });
            }));
      }

    void scroll_treeview_to_bottom ()
      {
        guint n = m_store->get_n_items ();
        Glib::signal_idle ().connect_once
          ([this, n] ()
           {
             m_treeview->scroll_to (n - 1);
           });
      }

    void update ()
      {
        disconnect_signals ();
        connect_signals ();
        update_buttons ();
      }

    void update_buttons ()
      {
        guint n = m_store->get_n_items ();
        m_remove_button->set_sensitive (n > 0);
        m_edit_button->set_sensitive (n > 0);
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    UndoAction *execute_action (UndoAction *action2)
      {
        auto action = dynamic_cast<BackpackEditorUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case BackpackEditorUndoAction::ADD:
              {
                auto a = dynamic_cast<BackpackEditorUndoAction_Add*>(action);
                out = new BackpackEditorUndoAction_Add (m_backpack);
                m_backpack->removeAllFromBackpack ();
                m_backpack->add (a->get_backpack ());
                fill_treeview ();
              }
            break;

          case BackpackEditorUndoAction::REMOVE:
              {
                auto a = dynamic_cast<BackpackEditorUndoAction_Remove*>(action);
                out = new BackpackEditorUndoAction_Remove (m_backpack);
                m_backpack->removeAllFromBackpack ();
                m_backpack->add (a->get_backpack ());
                fill_treeview ();
              }
            break;

          case BackpackEditorUndoAction::EDIT:
              {
                auto a = dynamic_cast<BackpackEditorUndoAction_Edit*>(action);
                out =
                  new BackpackEditorUndoAction_Edit
                  (m_selection_model->get_selected (), get_selected_item ());
                Item *item = get_item_by_index (a);
                std::replace (m_backpack->begin (), m_backpack->end (), item,
                              new Item (*a->get_item ()));
                delete item;
              }
            break;
          }
        return out;
      }

    Item *get_item_by_index (BackpackEditorUndoAction_Index *action)
      {
        auto item = m_store->get_item (action->get_index ());
        auto row = std::dynamic_pointer_cast<PackItemRow>(item);
        if (row)
          return row->m_item;
        else
          return NULL;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &BackpackEditorDialog::execute_action));

        setup_undo_and_redo ();

        signal_undo ().connect
           ([this] ()
            {
              m_umgr->undo ();
              update ();
            });

        signal_redo ().connect
           ([this] ()
            {
              m_umgr->redo ();
              update ();
            });
      }

    void fill_treeview ()
      {
        m_store->remove_all ();
        for (auto item : *m_backpack)
          m_store->append (PackItemRow::create (item));
      }

    Item *get_selected_item ()
      {
        auto single_selection =
          std::dynamic_pointer_cast<Gtk::SingleSelection> (m_selection_model);

        auto item = single_selection->get_selected_item ();

        if (item)
          {
            auto row = std::dynamic_pointer_cast<PackItemRow> (item);
            return row->m_item;
          }
        return NULL;
      }

};
#endif
