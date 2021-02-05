//  Copyright (C) 2008, 2009, 2014, 2020, 2021 Ben Asselstine
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
#include <iostream>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include <gtkmm.h>

#include "rewardlist-dialog.h"
#include "defs.h"
#include "Configuration.h"
#include "rewardlist.h"
#include "ucompose.hpp"
#include "playerlist.h"
#include "reward-editor-dialog.h"
#include "rewardlist-editor-actions.h"

#define method(x) sigc::mem_fun(*this, &RewardlistDialog::x)

RewardlistDialog::RewardlistDialog(Gtk::Window &parent, bool select, bool clear)
 : LwEditorDialog(parent, "reward-list-dialog.ui")
{
  umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
  umgr->execute ().connect (method (executeAction));
  d_changed = false;
  d_select = select;
  d_clear = clear;
  d_reward = NULL;
  xml->get_widget("close_button", close_button);
  if (select)
    {
      dialog->set_title (_("Select a reward"));
      close_button->set_label (_("Select"));
    }
  xml->get_widget("clear_button", clear_button);
  xml->get_widget("rewards_treeview", rewards_treeview);
  xml->get_widget("add_button", add_button);
  add_button->signal_clicked().connect (method(on_add_clicked));
  xml->get_widget("remove_button", remove_button);
  remove_button->signal_clicked().connect (method(on_remove_clicked));
  xml->get_widget("edit_button", edit_button);
  edit_button->signal_clicked().connect (method(on_edit_clicked));
  xml->get_widget("undo_button", undo_button);
  undo_button->signal_activate ().connect (method (on_undo_activated));
  xml->get_widget("redo_button", redo_button);
  redo_button->signal_activate ().connect (method (on_redo_activated));

  rewards_list = Gtk::ListStore::create(rewards_columns);
  rewards_treeview->set_model(rewards_list);
  rewards_treeview->append_column("", rewards_columns.name);
  rewards_treeview->set_headers_visible(false);

  guint32 max = Rewardlist::getInstance ()->size();
  if (max)
    {
      Gtk::TreeModel::Row row;
      row = rewards_treeview->get_model()->children()[0];
      if(row)
        rewards_treeview->get_selection()->select(row);
    }

  fill_rewards ();
  connect_signals ();
  update ();
}

void RewardlistDialog::fill_rewards()
{
  rewards_list->clear ();
  Rewardlist *rewardlist = Rewardlist::getInstance();
  for (Rewardlist::iterator iter = rewardlist->begin();
       iter != rewardlist->end(); ++iter)
    addReward(*iter);
}
void
RewardlistDialog::update_rewardlist_buttons()
{
  if (!rewards_treeview->get_selection()->get_selected())
    {
      remove_button->set_sensitive(false);
      edit_button->set_sensitive(false);
      if (d_select)
        close_button->set_sensitive (false);
    }
  else
    {
      remove_button->set_sensitive(true);
      edit_button->set_sensitive(true);
      close_button->set_sensitive (true);
    }
  if (d_select && d_clear)
    {
      Glib::RefPtr<Gtk::TreeSelection> selection =
        rewards_treeview->get_selection();
      Gtk::TreeModel::iterator i = selection->get_selected();
      Gtk::TreeModel::Path path = rewards_treeview->get_model()->get_path (i);
      edit_button->set_sensitive (path.to_string () == "0");
    }
}

void RewardlistDialog::addReward(Reward *reward)
{
  Gtk::TreeIter i = rewards_list->append();
  (*i)[rewards_columns.name] = reward->getName();
  (*i)[rewards_columns.reward] = reward;
}

void RewardlistDialog::on_reward_selected()
{
  Glib::RefPtr<Gtk::TreeSelection> selection =
    rewards_treeview->get_selection();
  Gtk::TreeModel::iterator i = selection->get_selected();
  if (i)
    d_reward = (*i)[rewards_columns.reward];
  update_rewardlist_buttons();
}

void RewardlistDialog::on_add_clicked()
{
  Player *neutral = Playerlist::getInstance()->getNeutral();
  RewardEditorDialog d(*dialog, neutral, true, NULL);
  d.run();
  if (d.get_reward())
    {
      umgr->add (new RewardlistEditorAction_Add (Rewardlist::getInstance ()));
      d_changed = true;
      Reward *reward = Reward::copy (d.get_reward());
      Gtk::TreeIter i = rewards_list->append();
      (*i)[rewards_columns.name] = reward->getName();
      (*i)[rewards_columns.reward] = reward;
      Rewardlist::getInstance()->push_back(reward);
      rewards_treeview->get_selection()->select(i);
    }
}

void RewardlistDialog::on_remove_clicked()
{
  Glib::RefPtr<Gtk::TreeSelection> selection =
    rewards_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow)
    {
      umgr->add (new RewardlistEditorAction_Remove
                 (Rewardlist::getInstance ()));
      d_changed = true;
      Gtk::TreeModel::Row row = *iterrow;
      Reward *a = row[rewards_columns.reward];
      rewards_list->erase(iterrow);
      Rewardlist::getInstance()->flRemove(a);
    }
}

int RewardlistDialog::getCurIndex ()
{
  int idx = -1;
  Gtk::TreeIter i = rewards_treeview->get_selection()->get_selected();
  if (i)
    {
      auto path = rewards_treeview->get_model ()->get_path (i);
      idx = atoi (path.to_string ().c_str ());
    }
  return idx;
}

Reward* RewardlistDialog::getCurReward ()
{
  Glib::RefPtr<Gtk::TreeSelection> selection =
    rewards_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow) 
    {
      // Row selected
      Gtk::TreeModel::Row row = *iterrow;

      return row[rewards_columns.reward];
    }
  return NULL;
}

void RewardlistDialog::on_edit_clicked()
{
  Glib::RefPtr<Gtk::TreeSelection> selection =
    rewards_treeview->get_selection();
  Gtk::TreeModel::iterator iterrow = selection->get_selected();

  if (iterrow)
    {
      Gtk::TreeModel::Row row = *iterrow;
      Reward *reward = row[rewards_columns.reward];
      Player *neutral = Playerlist::getInstance()->getNeutral();
      RewardlistEditorAction_Edit *action =
        new RewardlistEditorAction_Edit (getCurIndex (), getCurReward ());
      RewardEditorDialog d(*dialog, neutral, true, reward);
      if (d.run())
        {
          umgr->add (action);
          if (d.get_reward())
            {
              d_changed = true;
              Rewardlist::iterator i =
                std::find (Rewardlist::getInstance ()->begin (),
                           Rewardlist::getInstance ()->end (), reward);
              delete reward;
              reward = Reward::copy (d.get_reward());
              *i = reward;
              (*iterrow)[rewards_columns.name] = reward->getName();
              (*iterrow)[rewards_columns.reward] = reward;
              d_reward = reward;
            }
          else
            {
              d_changed = true;
              rewards_list->erase(iterrow);
              Rewardlist::getInstance()->flRemove(reward);
              d_reward = NULL;
            }
        }
      else
        delete action;
    }
}

bool RewardlistDialog::run ()
{
  dialog->show_all ();
  if (!d_clear)
    {
      clear_button->set_visible (false);
      if (d_select)
        edit_button->set_visible (false);
    }
  if (d_select)
    remove_button->set_visible (false);
  int response = dialog->run ();
  if (response != Gtk::RESPONSE_ACCEPT)
    d_reward = NULL;

  return d_changed;
}

void RewardlistDialog::connect_signals ()
{
  umgr->connect_signals ();
  connections.push_back
    (rewards_treeview->get_selection()->signal_changed().connect
     (method(on_reward_selected)));
}

void RewardlistDialog::disconnect_signals ()
{
  umgr->disconnect_signals ();
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

void RewardlistDialog::on_undo_activated ()
{
  umgr->undo ();
  if (umgr->undoEmpty ())
    d_changed = false;
  update ();
  return;
}

void RewardlistDialog::on_redo_activated ()
{
  umgr->redo ();
  d_changed = true;
  update ();
}

void RewardlistDialog::update ()
{
  disconnect_signals ();
  update_rewardlist_buttons();
  auto i = Rewardlist::getInstance ()->begin ();
  for (Gtk::TreeIter j = rewards_list->children().begin(),
       jend = rewards_list->children().end(); j != jend; ++j, ++i)
    {
      (*j)[rewards_columns.reward] = *i;
      (*j)[rewards_columns.name] = (*i)->getName ();
    }
  umgr->setCursors ();
  connect_signals ();
}

Reward* RewardlistDialog::getRewardByIndex (RewardlistEditorAction_Index *a)
{
  auto path = Gtk::TreePath (String::ucompose ("%1", a->getIndex ()));
  auto iterrow = rewards_treeview->get_model ()->get_iter (path);
  Gtk::TreeModel::Row row = *iterrow;
  Reward *reward = row[rewards_columns.reward];
  return reward;
}

UndoAction *RewardlistDialog::executeAction (UndoAction *action2)
{
  RewardlistEditorAction *action =
    dynamic_cast<RewardlistEditorAction*>(action2);
  UndoAction *out = NULL;

  switch (action->getType ())
    {
      case RewardlistEditorAction::ADD:
          {
            RewardlistEditorAction_Add *a =
              dynamic_cast<RewardlistEditorAction_Add*>(action);
            out = new RewardlistEditorAction_Add
              (Rewardlist::getInstance ()->copy ());
            Rewardlist::reset (a->getRewards ());
            a->clearRewards ();
            fill_rewards ();
          }
        break;
      case RewardlistEditorAction::REMOVE:
          {
            RewardlistEditorAction_Remove *a =
              dynamic_cast<RewardlistEditorAction_Remove*>(action);
            out = new RewardlistEditorAction_Remove
              (Rewardlist::getInstance ()->copy ());
            Rewardlist::reset (a->getRewards ());
            a->clearRewards ();
            fill_rewards ();
          }
        break;
      case RewardlistEditorAction::EDIT:
          {
            RewardlistEditorAction_Edit *a =
              dynamic_cast<RewardlistEditorAction_Edit*>(action);
            out = new RewardlistEditorAction_Edit (getCurIndex (),
                                                   getCurReward ());
            Reward *reward = getRewardByIndex (a);
            Rewardlist *rl = Rewardlist::getInstance ();
            std::replace (rl->begin (), rl->end (), reward,
                          Reward::copy (a->getReward ()));
            delete reward;
          }
        break;
    }
  return out;
}
