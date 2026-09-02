//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2017, 2020, 2021,
//  2026 Ben Asselstine
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
#ifndef RUIN_EDITOR_DIALOG_H
#define RUIN_EDITOR_DIALOG_H
#include "item.h"
#include "keeper.h"
#include "ruin-undo-actions.h"
#include "army-type-label.h"
#include "keeper-editor-dialog.h"
#include "reward-editor-dialog.h"
#include "button-label.h"
class RuinEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "ruin-editor.ui";
      }

    RuinEditorDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_name_entry = load <Gtk::Entry> ("name_entry");
        m_type_spinbutton = load <Gtk::SpinButton> ("type_spinbutton");
        m_description_entry = load <Gtk::Entry> ("description_entry");
        m_keeper_switch = load <Gtk::Switch> ("keeper_switch");
        m_keeper_button = load <Gtk::Button> ("keeper_button");
        m_reward_switch = load <Gtk::Switch> ("reward_switch");
        m_reward_button = load <Gtk::Button> ("reward_button");
        m_hidden_switch = load <Gtk::Switch> ("hidden_switch");
        m_hidden_combobox = load <Gtk::Box> ("hidden_combobox");
        m_random_name_button = load <Gtk::Button> ("random_name_button");
        m_ruin_picture = load <Gtk::Picture> ("ruin_picture");
      }

    ~RuinEditorDialog ()
      {
        disconnect_signals ();
        delete m_umgr;
        delete m_ruin;
      }

    void setup (Ruin *ruin, CreateScenarioRandomize *randomizer)
      {
        m_randomize = randomizer;
        m_ruin = new Ruin (*ruin, true);
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);
        fill_owner_combobox ();
        m_random_reward_active = m_ruin->getReward () == NULL;
        m_random_keeper_active = m_ruin->getOccupant () == NULL;
        m_hidden_active = m_ruin->isHidden ();
    
        m_keeper_button_label =
          Gtk::make_managed <ButtonLabel> (m_keeper_button);
        m_reward_button_label =
          Gtk::make_managed <ButtonLabel> (m_reward_button);
        setup_undo ();
        update ();
      }

    Ruin *get_ruin ()
      {
        return m_ruin;
      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }
private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_random_name_button;
    Gtk::Entry *m_name_entry;
    Gtk::SpinButton *m_type_spinbutton;
    Gtk::Entry *m_description_entry;
    Gtk::Switch *m_reward_switch;
    Gtk::Button *m_reward_button;
    ButtonLabel *m_reward_button_label;
    Gtk::Switch *m_keeper_switch;
    Gtk::Button *m_keeper_button;
    ButtonLabel *m_keeper_button_label;
    Gtk::Switch *m_hidden_switch;
    Gtk::Box *m_hidden_combobox;
    LwCombo *m_hidden_combo;
    ArmyTypeLabel *m_keeper_armytype_label;
    Gtk::Picture *m_ruin_picture;

    Ruin *m_ruin;
    CreateScenarioRandomize *m_randomize;
    bool m_random_reward_active;
    bool m_random_keeper_active;
    bool m_hidden_active;
    int m_hidden_row;
    UndoMgr *m_umgr;
    std::list<sigc::connection> m_connections;

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void update ()
      {
        disconnect_signals ();
        m_name_entry->set_text (m_ruin->getName ());
        m_type_spinbutton->set_value (m_ruin->getType ());
        m_description_entry->set_text (m_ruin->getDescription ());

        Keeper *keeper = m_ruin->getOccupant ();
        if (keeper)
          {
            if (keeper->getName () == "")
              m_keeper_button_label->set_label (_("Unnamed keeper"));
            else
              m_keeper_button_label->set_label (keeper->getName ());
          }
        else
          m_keeper_button_label->set_label (_("No keeper set"));

        m_keeper_switch->set_active (m_random_keeper_active);
        m_keeper_button->set_sensitive (!m_random_keeper_active);

        Reward *reward = m_ruin->getReward ();
        if (reward)
          m_reward_button_label->set_label (reward->getName ());
        else
          m_reward_button_label->set_label (_("No reward set"));

        m_reward_switch->set_active (m_random_reward_active);
        m_reward_button->set_sensitive (!m_random_reward_active);

        m_hidden_switch->set_active (m_hidden_active);
        m_hidden_combo->set_sensitive (m_hidden_active);
        m_hidden_combo->set_active (m_hidden_row);

        m_ruin_picture->set_paintable
          (ImageCache::instance ()->getRuinPic
           (m_ruin->getType ())->to_texture ());

        connect_signals ();
      }

    void connect_signals ()
      {
        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new RuinUndoAction_Name (m_ruin, m_umgr, m_name_entry));
              m_ruin->setName (m_name_entry->get_text ());
            }));

        add_connection
          (m_type_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add (new RuinUndoAction_Type (m_ruin));
              m_ruin->setType (Ruin::Type (m_type_spinbutton->get_value_as_int ()));
              update ();
            }));

        add_connection
          (m_description_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new RuinUndoAction_Description
                 (m_ruin, m_umgr, m_description_entry));
              m_ruin->setDescription (m_description_entry->get_text ());
            }));

        add_connection
          (m_keeper_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new RuinUndoAction_RandomKeeper (m_ruin,
                                                  m_random_keeper_active));
              m_random_keeper_active = m_keeper_switch->get_active ();
              if (m_random_keeper_active)
                m_ruin->setOccupant (NULL);
              update ();
            }));

        add_connection
          (m_keeper_button->signal_clicked ().connect
           ([this] ()
            {
              auto d = LwDialog::build<KeeperEditorDialog> (this);
              auto pos = m_ruin->getPos ();
              d->setup (m_ruin->getOccupant (), pos, m_randomize);
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                       if (d->is_changed ())
                         {
                           m_umgr->add
                             (new RuinUndoAction_Keeper (m_ruin));
                           if (d->get_keeper ())
                             {
                               auto keeper = new Keeper (*d->get_keeper ());
                               m_ruin->setOccupant (keeper);
                             }
                           else
                             m_ruin->setOccupant (NULL);
                           update ();
                         }
                       break;

                     case Gtk::ResponseType::REJECT:
                       if (m_ruin->getOccupant () != NULL)
                         {
                           m_umgr->add (new RuinUndoAction_Keeper (m_ruin));
                           m_ruin->setOccupant (NULL);
                           update ();
                         }
                       break;

                     default:
                       break;
                     }
                   delete d;
                 });
            }));

        add_connection
          (m_reward_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new RuinUndoAction_RandomReward (m_ruin,
                                                  m_random_reward_active));
              m_random_reward_active = m_reward_switch->get_active ();
              if (m_random_reward_active)
                m_ruin->setReward (NULL);
              update ();
            }));

        add_connection
          (m_reward_button->signal_clicked ().connect
           ([this] ()
            {
              auto d = LwDialog::build<RewardEditorDialog> (this);
              d->set_show_clear ();
              d->setup (m_ruin->getReward (), true);
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                       if (d->is_changed ())
                         {
                           m_umgr->add
                             (new RuinUndoAction_Reward (m_ruin));
                           Reward *r = d->get_reward ();
                           if (r)
                             {
                               auto reward = Reward::copy (r);
                               reward->setName (reward->generate_name ());
                               m_ruin->setReward (reward);
                             }
                           else
                             m_ruin->setReward (NULL);
                           update ();
                         }
                       break;

                     case Gtk::ResponseType::REJECT:
                       m_umgr->add (new RuinUndoAction_Reward (m_ruin));
                       m_ruin->setReward (NULL);
                       update ();
                       break;

                     default:
                       break;
                     }
                   delete d;
                 });
            }));

        add_connection
          (m_hidden_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new RuinUndoAction_OnlySeenBy (m_ruin, m_hidden_active));
              m_hidden_active = m_hidden_switch->get_active ();
              m_ruin->setHidden (m_hidden_active);
              update ();
            }));

        add_connection
          (m_hidden_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new RuinUndoAction_OnlySeenPlayer (m_ruin, m_hidden_row));
              m_hidden_row = m_hidden_combo->get_active_row_number ();
              auto it = Playerlist::instance ()->begin ();
              std::advance (it, m_hidden_row);
              m_ruin->setOwner (*it);
            }));

        add_connection
          (m_random_name_button->signal_clicked ().connect
           ([this] ()
            {
              m_umgr->add (new RuinUndoAction_RandomizeName (m_ruin));
              Glib::ustring existing_name = m_name_entry->get_text ();
              if (existing_name == Ruin::getDefaultName ())
                m_ruin->setName (m_randomize->popRandomRuinName ());
              else
                {
                  m_ruin->setName (m_randomize->popRandomRuinName ());
                  m_randomize->pushRandomRuinName (existing_name);
                }
              update ();
            }));
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    UndoAction *execute_action (UndoAction *action2)
      {
        RuinUndoAction *action = dynamic_cast<RuinUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case RuinUndoAction::NAME:
              {
                auto a = dynamic_cast<RuinUndoAction_Name*>(action);
                out = new RuinUndoAction_Name (m_ruin, m_umgr, m_name_entry);
                m_ruin->setName (a->get_ruin ()->getName ());
              }
            break;

          case RuinUndoAction::RANDOMIZE_NAME:
              {
                auto a = dynamic_cast<RuinUndoAction_RandomizeName*>(action);
                out = new RuinUndoAction_RandomizeName (m_ruin);
                m_ruin->setName (a->get_ruin ()->getName ());
              }
            break;

          case RuinUndoAction::DESCRIPTION:
              {
                auto a = dynamic_cast<RuinUndoAction_Description*>(action);
                out = new RuinUndoAction_Description (m_ruin, m_umgr,
                                                      m_description_entry);
                m_ruin->setDescription (a->get_ruin ()->getDescription ());
              }
            break;

          case RuinUndoAction::RANDOM_KEEPER:
              {
                auto a = dynamic_cast<RuinUndoAction_RandomKeeper*>(action);
                out =
                  new RuinUndoAction_RandomKeeper (m_ruin,
                                                   m_random_keeper_active);
                Keeper *keeper = a->get_ruin ()->getOccupant ();
                if (keeper)
                  m_ruin->setOccupant (new Keeper (*keeper));
                else
                  m_ruin->setOccupant (NULL);
                m_random_keeper_active = a->get_active ();
              }
            break;

          case RuinUndoAction::KEEPER:
              {
                auto a = dynamic_cast<RuinUndoAction_Keeper*>(action);
                out = new RuinUndoAction_Keeper (m_ruin);
                Keeper *keeper = a->get_ruin ()->getOccupant ();
                if (keeper)
                  m_ruin->setOccupant (new Keeper (*keeper));
                else
                  m_ruin->setOccupant (NULL);
              }
            break;

          case RuinUndoAction::ONLY_SEEN_BY:
              {
                auto a = dynamic_cast<RuinUndoAction_OnlySeenBy*>(action);
                out = new RuinUndoAction_OnlySeenBy (m_ruin, m_hidden_active);
                m_ruin->setHidden (a->get_ruin ()->isHidden ());
                m_ruin->setOwner (a->get_ruin ()->getOwner ());
                m_hidden_active = a->get_active ();
              }
            break;

          case RuinUndoAction::ONLY_SEEN_PLAYER:
              {
                auto a = dynamic_cast<RuinUndoAction_OnlySeenPlayer*>(action);
                out = new RuinUndoAction_OnlySeenPlayer (m_ruin, 
                                                         m_hidden_row);
                m_ruin->setOwner (a->get_ruin ()->getOwner ());
                m_hidden_row = a->get_row ();
              }
            break;

          case RuinUndoAction::TYPE:
              {
                auto a = dynamic_cast<RuinUndoAction_Type*>(action);
                out = new RuinUndoAction_Type (m_ruin);
                m_ruin->setType (Ruin::Type (a->get_ruin ()->getType ()));
              }
            break;

          case RuinUndoAction::RANDOM_REWARD:
              {
                auto a = dynamic_cast<RuinUndoAction_RandomReward*>(action);
                out = new RuinUndoAction_RandomReward (m_ruin,
                                                       m_random_reward_active);
                disconnect_signals ();
                m_reward_switch->set_active (a->get_active ());
                m_random_reward_active = a->get_active ();
                Reward *reward = a->get_ruin ()->getReward ();
                if (!reward)
                  m_ruin->setReward (reward);
                else
                  m_ruin->setReward (Reward::copy (reward));
                connect_signals ();
              }
            break;

          case RuinUndoAction::REWARD:
              {
                auto a = dynamic_cast<RuinUndoAction_Reward*>(action);
                out = new RuinUndoAction_Reward (m_ruin);
                Reward *reward = a->get_ruin ()->getReward ();
                if (!reward)
                  m_ruin->setReward (reward);
                else
                  m_ruin->setReward (Reward::copy (reward));
              }
            break;
          }
        return out;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &RuinEditorDialog::execute_action));

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

        m_umgr->add_cursor (m_name_entry);
        m_umgr->add_cursor (m_description_entry);
      }

    void fill_owner_combobox ()
      {
        m_hidden_combo = Gtk::make_managed<LwCombo> ();
        for (auto p : *Playerlist::instance ())
          m_hidden_combo->append (p->getName ());
        m_hidden_combobox->append (*m_hidden_combo);
        update_owner_row ();
      }

    void update_owner_row ()
      {
        m_hidden_row = -1;
        if (m_ruin->getOwner ())
          {
            int i = 0;
            int found = -1;
            for (auto p : *Playerlist::instance ())
              {
                if (m_ruin->getOwner () == p)
                  found = i;
                i++;
              }
            if (found >= 0)
              {
                m_hidden_row = found;
              }
          }
        if (m_hidden_row == -1)
          {
            int row = Playerlist::instance ()->size () - 1;
            m_hidden_row = row;
          }
      }
};
#endif
