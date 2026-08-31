//  Copyright (C) 2015, 2020, 2021, 2026 Ben Asselstine
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
#ifndef FIGHT_ORDER_EDITOR_DIALOG_H
#define FIGHT_ORDER_EDITOR_DIALOG_H
#include "army-set-list.h"
#include "player.h"
#include "lw-column.h"
#include "fight-order-undo-actions.h"

class FightOrderEditorRow: public Glib::Object
{
public:
    PixMask *m_image;
    Glib::ustring m_name;
    guint32 m_army_type;

    static Glib::RefPtr<FightOrderEditorRow> create (PixMask *i,
                                                     Glib::ustring n, guint32 a)
      {
        return
          Glib::make_refptr_for_instance<FightOrderEditorRow>
          (new FightOrderEditorRow (i, n, a));
      }

protected:
    FightOrderEditorRow (PixMask *image, Glib::ustring name,
                         guint32 army_type)
      : m_image (image), m_name (name), m_army_type (army_type)
      {
      }
};

class FightOrderEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "fight-order-editor.ui";
      }

    FightOrderEditorDialog (BaseObjectType* o,
                            const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_copy_to_all_button = load <Gtk::Button> ("copy_to_all_button");
        m_up_button = load <Gtk::Button> ("up_button");
        m_down_button = load <Gtk::Button> ("down_button");
        m_owner_combobox = load <Gtk::Box> ("owner_combobox");
        m_treeview = load <Gtk::ColumnView> ("treeview");
      }

    ~FightOrderEditorDialog ()
      {
        delete m_umgr;
      }

    void setup ()
      {
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        fill_owner_combobox ();
        setup_undo ();

        m_store = Gio::ListStore<FightOrderEditorRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        fill_treeview ();
        setup_image_column ();
        setup_name_column ();

        update ();

      }

    std::list<guint32> get_fight_order ()
      {
        std::list<guint32> order;
        for (guint i = 0; i < m_store->get_n_items (); ++i)
          {
            auto item = m_store->get_item (i);
            order.push_back (item->m_army_type);
          }
        return order;
      }

    bool is_changed ()
      {
        //some of the undo events here aren't changes to the model
        guint32 count = 0;
        auto names = m_umgr->get_undo_names ();
        for (auto name : names)
          {
            if (name == "Owner")
              count++;
          }
        return count != names.size () && names.empty () == false;
      }

private:
    Gtk::Button *m_close_button = NULL;
    Gtk::Button *m_copy_to_all_button = NULL;
    Gtk::Button *m_up_button = NULL;
    Gtk::Button *m_down_button = NULL;
    Gtk::Box *m_owner_combobox = NULL;
    LwCombo *m_owner_combo;
    Gtk::ColumnView *m_treeview = NULL;
    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<FightOrderEditorRow>> m_store;
    UndoMgr *m_umgr;
    std::list<sigc::connection> m_connections;
    guint32 m_owner_row;

    void setup_image_column ()
      {
        LwColumn::setup_picture_column<FightOrderEditorRow>
          (m_treeview, LW_BUTTON_SIZE, LW_BUTTON_SIZE, "army_image", "",
           [] (const auto& row)
           {
             return row->m_image->to_texture ();
           });
      }

    void setup_name_column ()
      {
        LwColumn::setup_text_column<FightOrderEditorRow>
          (m_treeview, "name_label", true, Gtk::Justification::LEFT,
           _("Name"),
           [] (const auto& row)
           {
             return row->m_name;
           });
      }

    void reselection(const Glib::RefPtr<FightOrderEditorRow>& target)
      {
        guint n = m_store->get_n_items ();

        for (guint i = 0; i < n; i++)
          {
            if (m_store->get_item (i) == target)
              {
                m_selection_model->select_item (i, true);
                return;
              }
          }
      }

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void connect_signals ()
      {
        add_connection
          (m_owner_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new FightOrderUndoAction_Owner (m_owner_row));
              fill_treeview ();
              m_owner_row = m_owner_combo->get_active_row_number ();
            }));

        add_connection
          (m_copy_to_all_button->signal_clicked ().connect
           ([this] ()
            {
              bool modified = false;
              auto action =
                new FightOrderUndoAction_MakeSame
                (m_owner_combo->get_active_row_number (),
                 get_all_fight_orders ());

              auto *active = get_selected_player ();
              for (auto p : *Playerlist::instance ())
                {
                  if (p != active)
                    {
                      if (active->getFightOrder () != p->getFightOrder ())
                        {
                          set_fight_order (p, active->getFightOrder ());
                          modified = true;
                        }
                    }
                }
              if (modified)
                m_umgr->add (action);
              else
                delete action;
            }));

        add_connection
          (m_up_button->signal_clicked ().connect
           ([this] ()
            {
              int i = m_selection_model->get_selected ();
              if (i <= 0)
                return;

              auto item = m_store->get_item (i);
              if (!item)
                return;

              auto p = get_selected_player ();
              m_umgr->add
                (new FightOrderUndoAction_Order
                 (m_owner_combo->get_active_row_number (),
                  p->getFightOrder ()));
              m_store->remove (i);
              m_store->insert (i - 1, item);

              reselection (item);

              auto index = m_selection_model->get_selected ();
              if (index < m_store->get_n_items ())
                m_treeview->scroll_to (index);
              set_fight_order (p, get_fight_order ());
            }));

        add_connection
          (m_down_button->signal_clicked ().connect
           ([this] ()
            {
              int i = m_selection_model->get_selected ();
              if (i < 0 || i + 1 >= (int)m_store->get_n_items ())
                return;

              auto item = m_store->get_item (i);
              if (!item)
                return;

              auto p = get_selected_player ();
              m_umgr->add
                (new FightOrderUndoAction_Order
                 (m_owner_combo->get_active_row_number (),
                  p->getFightOrder ()));
              m_store->remove (i);
              m_store->insert (i + 1, item);

              reselection (item);

              auto index = m_selection_model->get_selected ();
              if (index + 1 < m_store->get_n_items ())
                m_treeview->scroll_to (index + 1);
              set_fight_order (p, get_fight_order ());
            }));
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    void update ()
      {
        disconnect_signals ();

        update_buttons ();

        connect_signals ();
      }

    void update_buttons ()
      {
        guint n = m_store->get_n_items ();
        int i = m_selection_model->get_selected ();
        m_up_button->set_sensitive (i > 0);
        m_down_button->set_sensitive (i >= 0 && (guint32) i < n - 1);
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &FightOrderEditorDialog::execute_action));

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
        FightOrderUndoAction *action =
          dynamic_cast<FightOrderUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case FightOrderUndoAction::ORDER:
              {
                auto a = dynamic_cast<FightOrderUndoAction_Order*>(action);
                Player *p = row_number_to_player (a->get_row ());
                out = new FightOrderUndoAction_Order
                  (m_owner_combo->get_active_row_number (),
                   p->getFightOrder ());

                set_fight_order (p, a->get_fight_order ());
                fill_treeview ();
              } 
            break;

          case FightOrderUndoAction::MAKE_SAME:
              {
                auto a = dynamic_cast<FightOrderUndoAction_MakeSame*>(action);
                out = new FightOrderUndoAction_MakeSame
                  (m_owner_combo->get_active_row_number (),
                   get_all_fight_orders ());

                auto it = Playerlist::instance ()->begin ();
                for (auto o : a->get_fight_orders ())
                  {
                    set_fight_order ((*it), o);
                    it++;
                  }
                disconnect_signals ();
                m_owner_combo->set_active (a->get_row ());
                fill_treeview ();
                connect_signals ();
              }
            break;

          case FightOrderUndoAction::OWNER:
              {
                auto a = dynamic_cast<FightOrderUndoAction_Owner*>(action);
                out =
                  new FightOrderUndoAction_Owner
                  (m_owner_combo->get_active_row_number ());

                disconnect_signals ();
                m_owner_combo->set_active (a->get_row ());
                fill_treeview ();
                connect_signals ();
              }
            break;
          }
        return out;
      }

    void fill_owner_combobox ()
      {
        m_owner_combo = Gtk::make_managed<LwCombo> ();
        int i = 0;
        int found = -1;
        for (auto p : *Playerlist::instance ())
          {
            m_owner_combo->append (p->getName ());
            if (p == Playerlist::getActiveplayer ())
              found = i;
            i++;
          }
        if (found >= 0)
          {
            m_owner_combo->set_active (found);
            m_owner_row = found;
          }
        else
          m_owner_row = 0;
        m_owner_combobox->append (*m_owner_combo);
        m_owner_combo->set_halign (Gtk::Align::CENTER);
      }

    void fill_treeview ()
      {
        m_store->remove_all ();
        auto p = get_selected_player ();
        std::list<guint32> fight_order = p->getFightOrder ();
        for (auto it = fight_order.begin (); it != fight_order.end (); ++it)
          {
            const ArmyProto *a =
              Armysetlist::instance ()->getArmy (p->getArmyset (), *it);
            auto pix =
              ImageCache::instance ()->getCircledArmyPic
              (p->getArmyset (), *it, p->get_shield (), NULL, false,
               p->getId (), true, Lw::get_dark ());
            m_store->append (FightOrderEditorRow::create (pix, a->getName (), *it));
          }
      }

    Player * get_selected_player ()
      {
        int row = m_owner_combo->get_active_row_number ();
        return row_number_to_player (row);
      }


    std::list<std::list<guint32>> get_all_fight_orders ()
      {
        std::list<std::list<guint32> > orders;
        for (auto p : *Playerlist::instance ())
          orders.push_back (p->getFightOrder ());
        return orders;
      }

    Player * row_number_to_player (int row)
      {
        auto it = Playerlist::instance ()->begin ();
        std::advance (it, row);
        return *it;
      }

    void set_fight_order (Player *p, std::list<guint32> ids)
      {
        p->setFightOrder (ids);
        p->clearActionlist ();
      }
};
#endif
