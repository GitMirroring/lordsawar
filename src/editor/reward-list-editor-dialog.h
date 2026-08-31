//  Copyright (C) 2008, 2009, 2014, 2020, 2021, 2026 Ben Asselstine
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
#ifndef REWARD_LIST_EDITOR_DIALOG_H
#define REWARD_LIST_EDITOR_DIALOG_H
#include "reward-list.h"
#include "reward-list-undo-actions.h"
#include "lw-column.h"
#include "reward-editor-dialog.h"

class ScenarioRewardRow: public Glib::Object
{
public:

    Reward *m_reward;

    static Glib::RefPtr<ScenarioRewardRow> create (Reward *r)
      {
        return
          Glib::make_refptr_for_instance<ScenarioRewardRow>
          (new ScenarioRewardRow (r));
      }

protected:
    ScenarioRewardRow (Reward *r)
      : m_reward (r)
      {
      }

};

class RewardListEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "reward-list.ui";
      }

    RewardListEditorDialog (BaseObjectType* o,
                            const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_add_button = load <Gtk::Button> ("add_button");
        m_remove_button = load <Gtk::Button> ("remove_button");
        m_edit_button = load <Gtk::Button> ("edit_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");

      }

    ~RewardListEditorDialog ()
      {
        disconnect_signals ();
        delete m_rewardlist;
        delete m_umgr;
      }

    void setup (Rewardlist *rewardlist, bool readonly)
      {
        m_rewardlist = rewardlist->copy ();
        m_readonly = readonly;

        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_store = Gio::ListStore<ScenarioRewardRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

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

    Rewardlist *get_reward_list ()
      {
        return m_rewardlist;
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
    Glib::RefPtr<Gio::ListStore<ScenarioRewardRow>> m_store;

    std::list<sigc::connection> m_connections;

    UndoMgr *m_umgr;
    Rewardlist *m_rewardlist;
    bool m_readonly;

    void setup_name_column ()
      {
        LwColumn::setup_text_column<ScenarioRewardRow> 
          (m_treeview, "name_label", true, Gtk::Justification::LEFT,
           _("Rewards"),
           [] (const auto& row)
           {
             return row->m_reward->getName ();
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
              auto d = LwDialog::build<RewardEditorDialog> (this);
              d->setup (NULL, false);
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                       if (d->is_changed ())
                         {
                           Reward *r = d->get_reward ();
                           if (r)
                             {
                               m_umgr->add
                                 (new RewardListUndoAction_Add (m_rewardlist));
                               auto reward = Reward::copy (r);
                               reward->setName (reward->generate_name ());
                               m_rewardlist->push_back (reward);

                               m_store->append
                                 (ScenarioRewardRow::create (reward));

                               guint n = m_selection_model->get_n_items ();
                               if (n > 0)
                                 m_selection_model->set_selected (n - 1);

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
                  m_umgr->add (new RewardListUndoAction_Remove (m_rewardlist));

                  auto reward = get_selected_reward ();
                  m_rewardlist->remove (reward);

                  m_store->remove (m_selection_model->get_selected ());
                  update ();
                }
            }));

        add_connection
          (m_edit_button->signal_clicked ().connect
           ([this] ()
            {
              auto d = LwDialog::build<RewardEditorDialog> (this);
              d->setup (get_selected_reward (), false);
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                       if (d->is_changed ())
                         {
                           Reward *r = d->get_reward ();
                           if (r)
                             {
                               m_umgr->add
                                 (new RewardListUndoAction_Edit 
                                  (m_selection_model->get_selected (),
                                   get_selected_reward ()));

                               Reward *nr = Reward::copy (r);
                               nr->setName (nr->generate_name ());
                               std::replace (m_rewardlist->begin (),
                                             m_rewardlist->end (), 
                                             get_selected_reward (), nr);
        
                               fill_treeview ();
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
        update_buttons ();
        connect_signals ();
      }

    void update_buttons ()
      {
        guint n = m_store->get_n_items ();
        m_remove_button->set_sensitive (n > 0);
        m_edit_button->set_sensitive (n > 0);
        if (m_readonly)
          {
            m_add_button->set_sensitive (false);
            m_remove_button->set_sensitive (false);
            m_edit_button->set_sensitive (false);
          }
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    UndoAction *execute_action (UndoAction *action2)
      {
        RewardListUndoAction *action =
          dynamic_cast<RewardListUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case RewardListUndoAction::ADD:
              {
                auto a = dynamic_cast<RewardListUndoAction_Add*>(action);
                out = new RewardListUndoAction_Add
                  (Rewardlist::instance ()->copy ());
                Rewardlist::reset (a->get_rewards ());
                a->clear_rewards ();
                fill_treeview ();
              }
            break;

          case RewardListUndoAction::REMOVE:
              {
                auto a = dynamic_cast<RewardListUndoAction_Remove*>(action);
                out = new RewardListUndoAction_Remove
                  (Rewardlist::instance ()->copy ());
                Rewardlist::reset (a->get_rewards ());
                a->clear_rewards ();
                fill_treeview ();
              }
            break;

          case RewardListUndoAction::EDIT:
              {
                auto a = dynamic_cast<RewardListUndoAction_Edit*>(action);
                out =
                  new RewardListUndoAction_Edit
                  (m_selection_model->get_selected (), get_selected_reward ());

                Reward *reward = get_reward_by_index (a);
                Rewardlist *rl = Rewardlist::instance ();
                std::replace (rl->begin (), rl->end (), reward,
                              Reward::copy (a->get_reward ()));
                delete reward;
              }
            break;
          }
        return out;
      }

    Reward *get_reward_by_index (RewardListUndoAction_Index *action)
      {
        auto item = m_store->get_item (action->get_index ());
        auto row = std::dynamic_pointer_cast<ScenarioRewardRow>(item);
        if (row)
          return row->m_reward;
        else
          return NULL;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &RewardListEditorDialog::execute_action));

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
        for (auto reward : *m_rewardlist)
          m_store->append (ScenarioRewardRow::create (reward));
      }

    Reward *get_selected_reward ()
      {
        auto single_selection =
          std::dynamic_pointer_cast<Gtk::SingleSelection> (m_selection_model);

        auto item = single_selection->get_selected_item ();

        if (item)
          {
            auto row = std::dynamic_pointer_cast<ScenarioRewardRow> (item);
            return row->m_reward;
          }
        return NULL;
      }

};
#endif
