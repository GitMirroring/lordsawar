//  Copyright (C) 2026 Ben Asselstine
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
#ifndef STARTING_ITEMS_DIALOG_H
#define STARTING_ITEMS_DIALOG_H
#include "starting-items-undo-actions.h"
#include "select-item-dialog.h"

class StartingItemRow: public Glib::Object
{
public:
    guint32 m_type_id;

    static Glib::RefPtr<StartingItemRow> create (guint32 id)
      {
        return
          Glib::make_refptr_for_instance<StartingItemRow>
          (new StartingItemRow (id));
      }

protected:
    StartingItemRow (guint32 id)
      : m_type_id (id)
      {
      }
};

class StartingItemsDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "starting-items.ui";
      }

    StartingItemsDialog (BaseObjectType* o,
                         const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_add_button = load <Gtk::Button> ("add_button");
        m_remove_button = load <Gtk::Button> ("remove_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
      }

    ~StartingItemsDialog ()
      {
        disconnect_signals ();
        delete m_umgr;
      }

    std::list<guint32> get_starting_item_ids ()
      {
        std::list<guint32> ids;
        for (guint i = 0; i < m_store->get_n_items (); ++i)
          {
            auto row =
              std::dynamic_pointer_cast<StartingItemRow>(m_store->get_object
                                                         (i));
            ids.push_back (row->m_type_id);
          }
        return ids;
      }

    void setup (std::list<guint32> ids)
      {
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_store = Gio::ListStore<StartingItemRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        m_add_button->set_icon_name ("list-add-symbolic");
        m_remove_button->set_icon_name ("list-remove-symbolic");

        setup_name_column ();

        setup_undo ();

        fill_treeview (ids);

        update ();

      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }

    void update ()
      {
        disconnect_signals ();
        update_buttons ();
        connect_signals ();
      }

private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_add_button;
    Gtk::Button *m_remove_button;
    Gtk::ColumnView *m_treeview;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<StartingItemRow>> m_store;

    std::list<sigc::connection> m_connections;
    UndoMgr *m_umgr;

    void setup_name_column ()
      {
        LwColumn::setup_text_column<StartingItemRow> 
          (m_treeview, "name_label", true, Gtk::Justification::LEFT, _("Items"),
           [] (const auto& row)
           {
             guint32 id = row->m_type_id;
             if (Itemlist::instance ()->find (id) !=
                 Itemlist::instance ()->end ())
               {
                 return (*Itemlist::instance ())[id]->getName ();
               }
             else
               return Glib::ustring ("");
           });
      }

    void fill_treeview (std::list<guint32> ids)
      {
        m_store->remove_all ();
        for (auto id : ids)
          if (Itemlist::instance ()->find (id) != Itemlist::instance ()->end ())
            m_store->append (StartingItemRow::create (id));
      }

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void connect_signals ()
      {
        add_connection
          (m_add_button->signal_clicked ().connect
           ([this] ()
            {
              auto d = LwDialog::build<SelectItemDialog> (this);
              d->setup ();
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                         {
                           int id = d->get_selected_item_id ();
                           if (id >= 0)
                             {
                               m_umgr->add
                                 (new StartingItemsUndoAction_Add
                                  (get_starting_item_ids ()));
                               m_store->append (StartingItemRow::create (id));
                               scroll_treeview_to_bottom ();
                           
                               update ();
                             }
                         }
                       break;

                     default:
                       break;
                     }
                   delete d;
                 });
            }));

        add_connection
          (m_remove_button->signal_clicked ().connect
           ([this] ()
            {
              auto selected = m_selection_model->get_selected_item ();
              if (selected)
                {
                  m_umgr->add
                    (new StartingItemsUndoAction_Remove
                     (get_starting_item_ids ()));
                  m_store->remove (m_selection_model->get_selected ());
                  update ();
                }
            }));
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
      }

    void update_buttons ()
      {
        guint n = m_store->get_n_items ();
        m_remove_button->set_sensitive (n > 0);
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

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &StartingItemsDialog::execute_action));

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

    UndoAction *execute_action (UndoAction *action2)
      {
        StartingItemsUndoAction *action = dynamic_cast<StartingItemsUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case StartingItemsUndoAction::ADD:
              {
                auto a = dynamic_cast<StartingItemsUndoAction_Add*>(action);
                out = new StartingItemsUndoAction_Add
                  (get_starting_item_ids ());
                disconnect_signals ();
                fill_treeview (a->get_ids ());
                connect_signals ();
                update ();
              }
            break;

          case StartingItemsUndoAction::REMOVE:
              {
                auto a = dynamic_cast<StartingItemsUndoAction_Remove*>(action);
                out = new StartingItemsUndoAction_Remove
                  (get_starting_item_ids ());
                disconnect_signals ();
                fill_treeview (a->get_ids ());
                connect_signals ();
                update ();
              }
            break;
          }
        return out;
      }
};
#endif
