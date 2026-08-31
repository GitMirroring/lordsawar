//  Copyright (C) 2008, 2009, 2011, 2014, 2017, 2020, 2021, 2026 Ben Asselstine
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
#ifndef REWARD_EDITOR_DIALOG_H
#define REWARD_EDITOR_DIALOG_H
#include "reward.h"
#include "reward-undo-actions.h"
#include "sight-map.h"
#include "select-hidden-ruin-dialog.h"
#include "button-label.h"
class RewardEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "reward-editor.ui";
      }

    RewardEditorDialog (BaseObjectType* o,
                        const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_clear_button = load <Gtk::Button> ("clear_button");
        m_reward_combobox = load <Gtk::Box> ("reward_combobox");
        m_notebook = load <Gtk::Notebook> ("notebook");
        m_gold_spinbutton = load <Gtk::SpinButton> ("gold_spinbutton");
        m_item_button = load <Gtk::Button> ("item_button");
        m_num_allies_spinbutton =
          load <Gtk::SpinButton> ("num_allies_spinbutton");
        m_ally_button = load <Gtk::Button> ("ally_button");
        m_map_x_spinbutton = load <Gtk::SpinButton> ("map_x_spinbutton");
        m_map_y_spinbutton = load <Gtk::SpinButton> ("map_y_spinbutton");
        m_map_width_spinbutton =
          load <Gtk::SpinButton> ("map_width_spinbutton");
        m_map_height_spinbutton =
          load <Gtk::SpinButton> ("map_height_spinbutton");
        m_map_name_entry = load <Gtk::Entry> ("map_name_entry");
        m_hidden_ruin_button = load <Gtk::Button> ("hidden_ruin_button");
        m_random_button = load <Gtk::Button> ("random_button");
        m_show_clear = false;
      }

    ~RewardEditorDialog ()
      {
        disconnect_signals ();
        delete m_umgr;
        delete m_gold_reward;
        delete m_allies_reward;
        delete m_item_reward;
        delete m_map_reward;
        delete m_ruin_reward;
      }

    void setup (Reward *r, bool allow_renewables)
      {
        m_allow_renewables = allow_renewables;
        //don't allow renewable reward when we say no renewables
        if (r && !allow_renewables && r->is_renewable ())
          r = NULL;

        m_empty_reward = r == NULL;

        set_response (m_close_button, Gtk::ResponseType::ACCEPT);
        set_response (m_clear_button, Gtk::ResponseType::REJECT);

        m_clear_button->set_visible (!m_empty_reward && m_show_clear);

        setup_rewards (r);

        m_hidden_ruin_button_label =
          Gtk::make_managed <ButtonLabel> (m_hidden_ruin_button);
        m_item_button_label =
          Gtk::make_managed <ButtonLabel> (m_item_button);

        fill_type_combo ();
        m_ally_army_type_label =
          Gtk::make_managed<ArmyTypeLabel>
          (this, m_ally_button, Shield::NEUTRAL,
           SelectArmyDialog::SELECT_REWARDABLE_ARMY);
        setup_undo ();
        m_map_x_spinbutton->set_adjustment
          (Gtk::Adjustment::create (0, 0, GameMap::getWidth () - 1, 1, 10, 0));
        m_map_y_spinbutton->set_adjustment
          (Gtk::Adjustment::create (0, 0, GameMap::getHeight () - 1, 1, 10, 0));
        m_map_width_spinbutton->set_adjustment
          (Gtk::Adjustment::create (20, 1, GameMap::getWidth (), 1, 10, 0));
        m_map_height_spinbutton->set_adjustment
          (Gtk::Adjustment::create (20, 1, GameMap::getHeight (), 1, 10, 0));
        update ();
      }

    bool is_changed ()
      {
        if (m_empty_reward)
          return true;
        return m_umgr->undo_empty () == false;
      }

    Reward* get_reward ()
      {
        return m_reward;
      }

    void set_show_clear ()
      {
        m_show_clear = true;
      }
private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_clear_button;
    Gtk::Box *m_reward_combobox;
    LwCombo *m_reward_combo;
    Gtk::Notebook *m_notebook;
    Gtk::SpinButton *m_gold_spinbutton;
    Gtk::Button *m_item_button;
    ButtonLabel *m_item_button_label;
    Gtk::SpinButton *m_num_allies_spinbutton;
    Gtk::Button *m_ally_button;
    ArmyTypeLabel *m_ally_army_type_label;
    Gtk::SpinButton *m_map_x_spinbutton;
    Gtk::SpinButton *m_map_y_spinbutton;
    Gtk::SpinButton *m_map_width_spinbutton;
    Gtk::SpinButton *m_map_height_spinbutton;
    Gtk::Button *m_hidden_ruin_button;
    ButtonLabel *m_hidden_ruin_button_label;
    Gtk::Button *m_random_button;
    Gtk::Entry *m_map_name_entry;

    UndoMgr *m_umgr;
    std::list<sigc::connection> m_connections;

    Reward *m_reward;
    Reward_Gold *m_gold_reward;
    Reward_Allies *m_allies_reward;
    Reward_Item *m_item_reward;
    Reward_Map *m_map_reward;
    Reward_Ruin *m_ruin_reward;
    bool m_empty_reward;
    bool m_show_clear;
    bool m_allow_renewables;

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void update ()
      {
        disconnect_signals ();
        switch (m_reward->getType ())
          {
          case Reward::GOLD:
            m_reward_combo->set_active (_("Gold"));
            m_notebook->set_current_page (0);
            break;

          case Reward::ITEM:
            m_reward_combo->set_active (_("Item"));
            m_notebook->set_current_page (1);
            break;

          case Reward::ALLIES:
            m_reward_combo->set_active (_("Allies"));
            m_notebook->set_current_page (2);
            break;

          case Reward::MAP:
            m_reward_combo->set_active (_("Map"));
            m_notebook->set_current_page (3);
            break;

          case Reward::RUIN:
            m_reward_combo->set_active (_("Hidden Ruin"));
            m_notebook->set_current_page (4);
            break;
          }

        m_gold_spinbutton->set_value (m_gold_reward->getGold ());

        Glib::ustring item = _("No item set");
        if (m_item_reward->getItem ())
          item = m_item_reward->getItem ()->getName ();
        m_item_button_label->set_label (item);

        m_num_allies_spinbutton->set_value (m_allies_reward->getNoOfAllies ());
        int id = -1;
        if (m_allies_reward->getArmy ())
          id = m_allies_reward->getArmy ()->getId ();
        m_ally_army_type_label->set_army_type (id);

        m_map_x_spinbutton->set_value (m_map_reward->getSightMap ()->x);
        m_map_y_spinbutton->set_value (m_map_reward->getSightMap ()->y);
        m_map_width_spinbutton->set_value (m_map_reward->getSightMap ()->w);
        m_map_height_spinbutton->set_value (m_map_reward->getSightMap ()->h);
        m_map_name_entry->set_text (m_map_reward->getMapName ());

        Glib::ustring ruin = _("No ruin set");
        if (m_ruin_reward->getRuin ())
          ruin = m_ruin_reward->getRuin ()->getName ();
        m_hidden_ruin_button_label->set_label (ruin);

        update_buttons ();

        connect_signals ();
      }

    void connect_signals ()
      {
        add_connection
          (m_reward_combo->signal_changed ().connect
           ([this] ()
            {
              Reward::Type new_type =
                row_to_reward_type (m_reward_combo->get_active_text ());

              auto action = new RewardUndoAction_Type (m_reward);
              if (switch_reward_type (new_type))
                {
                  m_umgr->add (action);
                  update ();
                }
              else
                delete action;
            }));

        add_connection
          (m_gold_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto r = m_gold_reward;
              guint32 value = m_gold_spinbutton->get_value_as_int ();
              if (r->getGold () != value)
                {
                  m_umgr->add (new RewardUndoAction_Gold (m_reward));
                  r->setGold (value);
                }
            }));

        add_connection
          (m_item_button->signal_clicked ().connect
           ([this] ()
            {
              auto d = LwDialog::build<SelectItemDialog> (this);
              if (m_item_reward->getItem ())
                d->set_selected_item (m_item_reward->getItem ()->getId ());
              else
                d->set_selected_item (-1);
              d->set_show_clear ();

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
                                 (new RewardUndoAction_Item (m_reward));
                               auto itemproto = (*Itemlist::instance ())[id];
                               auto item = new Item (*itemproto, id);
                               m_item_reward->setItem (item);
                               update ();
                             }
                         }
                       break;

                     case Gtk::ResponseType::REJECT:
                       m_umgr->add (new RewardUndoAction_Item (m_reward));
                         {
                           auto item = m_item_reward->getItem ();
                           if (item)
                             delete item;
                           m_item_reward->setItem (NULL);
                         }
                       update ();
                       break;

                     default:
                       break;
                     }
                   delete d;
                 });
            }));

        add_connection
          (m_num_allies_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto r = m_allies_reward;
              guint32 value = m_num_allies_spinbutton->get_value_as_int ();
              if (r->getNoOfAllies () != value)
                {
                  m_umgr->add (new RewardUndoAction_AllyCount (m_reward));
                  r->setNoOfAllies (value);
                }
            }));

        add_connection
          (m_ally_army_type_label->signal_army_selected ().connect
           ([this] (int id)
            {
              auto r = m_allies_reward;
              if (id >= 0)
                {
                  int old_id = -1;
                  if (r->getArmy ())
                    old_id = r->getArmy ()->getId ();

                  if (old_id != id)
                    {
                      m_umgr->add (new RewardUndoAction_AllyType (m_reward));
                      auto as = Playerlist::getNeutral ()->getArmyset ();
                      auto a = Armysetlist::instance ()->getArmy (as, id);

                      r->clearAllies ();
                      r->setArmy (a);
                      r->setNoOfAllies
                        (m_num_allies_spinbutton->get_value_as_int ());
                      update ();
                    }
                }
              else
                {
                  m_umgr->add (new RewardUndoAction_AllyType (m_reward));
                  r->clearAllies ();
                  update ();
                }
            }));

        add_connection
          (m_map_x_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto r = m_map_reward;
              auto value = m_map_x_spinbutton->get_value_as_int ();
              if (r->getSightMap ()->x != value)
                {
                  m_umgr->add (new RewardUndoAction_XCoord (m_reward));
                  r->getSightMap ()->x = value;
                }
            }));

        add_connection
          (m_map_y_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto r = m_map_reward;
              auto value = m_map_y_spinbutton->get_value_as_int ();
              if (r->getSightMap ()->y != value)
                {
                  m_umgr->add (new RewardUndoAction_YCoord (m_reward));
                  r->getSightMap ()->y = value;
                }
            }));

        add_connection
          (m_map_width_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto r = m_map_reward;
              auto value = m_map_width_spinbutton->get_value_as_int ();
              if (r->getSightMap ()->w != value)
                {
                  m_umgr->add (new RewardUndoAction_Width (m_reward));
                  r->getSightMap ()->w = value;
                }
            }));

        add_connection
          (m_map_height_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto r = m_map_reward;
              auto value = m_map_height_spinbutton->get_value_as_int ();
              if (r->getSightMap ()->h != value)
                {
                  m_umgr->add (new RewardUndoAction_Height (m_reward));
                  r->getSightMap ()->h = value;
                }
            }));

        add_connection
          (m_map_name_entry->signal_changed ().connect
           ([this] ()
            {
              auto r = m_map_reward;
              auto value = m_map_name_entry->get_text ();
              if (r->getSightMap ()->getName () != value)
                {
                  m_umgr->add
                    (new RewardUndoAction_MapName (m_reward, m_umgr,
                                                   m_map_name_entry));
                  r->getSightMap ()->setName (value);

                  update_buttons ();
                }
            }));

        add_connection
          (m_hidden_ruin_button->signal_clicked ().connect
           ([this] ()
            {
              auto d = LwDialog::build<SelectHiddenRuinDialog> (this);
              if (m_ruin_reward->getRuin ())
                d->set_selected_ruin (m_ruin_reward->getRuin ());
              d->set_show_clear ();

              d->setup ();
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                         {
                           auto ruin = d->get_selected_ruin ();
                           if (ruin)
                             {
                               m_ruin_reward->setRuinPos (ruin->getPos ());
                               update ();
                             }
                         }
                       break;

                     case Gtk::ResponseType::REJECT:
                       m_umgr->add (new RewardUndoAction_HiddenRuin (m_reward));
                       m_ruin_reward->clearRuin ();
                       update ();
                       break;

                     default:
                       break;
                     }
                   delete d;
                 });
            }));

        add_connection
          (m_random_button->signal_clicked ().connect
           ([this] ()
            {
              switch (m_reward->getType ())
                {
                case Reward::GOLD:
                  random_gold ();
                  break;

                case Reward::ALLIES:
                  random_allies ();
                  break;

                case Reward::ITEM:
                  random_item ();
                  break;

                case Reward::MAP:
                  random_map ();
                  break;

                case Reward::RUIN:
                  random_ruin ();
                  break;
                }
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
        RewardUndoAction *action = dynamic_cast<RewardUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case RewardUndoAction::TYPE:
              {
                auto a = dynamic_cast<RewardUndoAction_Type*>(action);
                out = new RewardUndoAction_Type (m_reward);
                switch_reward_type (a->get_reward ()->getType ());
              }
            break;

          case RewardUndoAction::GOLD_PIECES:
              {
                auto a = dynamic_cast<RewardUndoAction_Gold*>(action);
                out = new RewardUndoAction_Gold (m_reward);
                m_gold_reward->setGold
                  (dynamic_cast<Reward_Gold*>(a->get_reward ())->getGold ());
              }
            break;

          case RewardUndoAction::RANDOMIZE_GOLD:
              {
                auto a = dynamic_cast<RewardUndoAction_RandomizeGold*>(action);
                out = new RewardUndoAction_RandomizeGold (m_reward);
                m_gold_reward->setGold
                  (dynamic_cast<Reward_Gold*>(a->get_reward ())->getGold ());
              }
            break;

          case RewardUndoAction::ITEM:
              {
                auto a = dynamic_cast<RewardUndoAction_Item*>(action);
                out = new RewardUndoAction_Item (m_reward);
                auto r = m_item_reward;
                if (r->getItem ())
                  delete r->getItem ();
                Reward_Item *newr = dynamic_cast<Reward_Item*>(a->get_reward ());
                if (newr->getItem ())
                  r->setItem (new Item (*newr->getItem ()));
                else
                  r->setItem (NULL);
              }
            break;

          case RewardUndoAction::RANDOMIZE_ITEM:
              {
                auto a = dynamic_cast<RewardUndoAction_RandomizeItem*>(action);
                out = new RewardUndoAction_RandomizeItem (m_reward);
                auto r = m_item_reward;
                if (r->getItem ())
                  delete r->getItem ();
                Reward_Item *newr = dynamic_cast<Reward_Item*>(a->get_reward ());
                if (newr->getItem ())
                  r->setItem (new Item (*newr->getItem ()));
                else
                  r->setItem (NULL);
              }
            break;

          case RewardUndoAction::ALLY_TYPE:
              {
                auto a = dynamic_cast<RewardUndoAction_AllyType*>(action);
                out = new RewardUndoAction_AllyType (m_reward);
                auto r = m_allies_reward;
                r->setArmy
                  (dynamic_cast<Reward_Allies*>(a->get_reward ())->getArmy ());
              }
            break;

          case RewardUndoAction::RANDOMIZE_ALLY:
              {
                auto a = dynamic_cast<RewardUndoAction_RandomizeAlly*>(action);
                out = new RewardUndoAction_RandomizeAlly (m_reward);
                auto r = m_allies_reward;
                r->setArmy
                  (dynamic_cast<Reward_Allies*>(a->get_reward ())->getArmy ());
                r->setNoOfAllies
                  (dynamic_cast<Reward_Allies*>(a->get_reward ())->getNoOfAllies ());
              }
            break;

          case RewardUndoAction::ALLY_COUNT:
              {
                auto a = dynamic_cast<RewardUndoAction_AllyCount*>(action);
                out = new RewardUndoAction_AllyCount (m_reward);
                auto r = m_allies_reward;
                Reward_Allies *ar =
                  dynamic_cast<Reward_Allies*> (a->get_reward ());
                r->setNoOfAllies (ar->getNoOfAllies ());
              }
            break;

          case RewardUndoAction::XCOORD:
              {
                auto a = dynamic_cast<RewardUndoAction_XCoord*>(action);
                out = new RewardUndoAction_XCoord (m_reward);
                auto r = m_map_reward;
                Reward_Map *ar = dynamic_cast<Reward_Map*> (a->get_reward ());
                r->getSightMap ()->x = ar->getSightMap ()->x;
              }
            break;

          case RewardUndoAction::YCOORD:
              {
                auto a = dynamic_cast<RewardUndoAction_YCoord*>(action);
                out = new RewardUndoAction_YCoord (m_reward);
                auto r = m_map_reward;
                Reward_Map *ar = dynamic_cast<Reward_Map*> (a->get_reward ());
                r->getSightMap ()->y = ar->getSightMap ()->y;
              }
            break;

          case RewardUndoAction::WIDTH:
              {
                auto a = dynamic_cast<RewardUndoAction_Width*>(action);
                out = new RewardUndoAction_Width (m_reward);
                auto r = m_map_reward;
                Reward_Map *ar = dynamic_cast<Reward_Map*> (a->get_reward ());
                r->getSightMap ()->w = ar->getSightMap ()->w;
              }
            break;

          case RewardUndoAction::HEIGHT:
              {
                auto a = dynamic_cast<RewardUndoAction_Height*>(action);
                out = new RewardUndoAction_Height (m_reward);
                auto r = m_map_reward;
                Reward_Map *ar = dynamic_cast<Reward_Map*> (a->get_reward ());
                r->getSightMap ()->h = ar->getSightMap ()->h;
              }
            break;

          case RewardUndoAction::RANDOMIZE_MAP:
              {
                auto a = dynamic_cast<RewardUndoAction_RandomizeMap*>(action);
                out = new RewardUndoAction_RandomizeMap (m_reward);
                auto r = m_map_reward;
                Reward_Map *ar = dynamic_cast<Reward_Map*> (a->get_reward ());
                r->getSightMap ()->x = ar->getSightMap ()->x;
                r->getSightMap ()->y = ar->getSightMap ()->y;
                r->getSightMap ()->w = ar->getSightMap ()->w;
                r->getSightMap ()->h = ar->getSightMap ()->h;
              }
            break;

          case RewardUndoAction::MAP_NAME:
              {
                auto a = dynamic_cast<RewardUndoAction_MapName*>(action);
                out = new RewardUndoAction_MapName (m_reward, m_umgr,
                                                    m_map_name_entry);
                auto r = m_map_reward;
                Reward_Map *ar = dynamic_cast<Reward_Map*> (a->get_reward ());
                r->getSightMap ()->setName (ar->getSightMap ()->getName ());
              }
            break;

          case RewardUndoAction::HIDDEN_RUIN:
              {
                auto a = dynamic_cast<RewardUndoAction_HiddenRuin*>(action);
                out = new RewardUndoAction_HiddenRuin (m_reward);
                Reward_Ruin *r = dynamic_cast<Reward_Ruin*>(a->get_reward ());
                if (r->getRuin ())
                  m_ruin_reward->setRuinPos
                    (r->getRuin ()->getPos ());
                else
                  m_ruin_reward->clearRuin ();
              }
            break;

          case RewardUndoAction::RANDOM_RUIN:
              {
                auto a = dynamic_cast<RewardUndoAction_RandomRuin*>(action);
                out = new RewardUndoAction_RandomRuin (m_reward);
                Reward_Ruin *r = dynamic_cast<Reward_Ruin*>(a->get_reward ());
                if (r->getRuin ())
                  m_ruin_reward->setRuinPos (r->getRuin ()->getPos ());
                else
                  m_ruin_reward->clearRuin ();
              }
            break;
          }
        return out;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &RewardEditorDialog::execute_action));

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
        
        m_umgr->add_cursor (m_map_name_entry);
      }

    void fill_type_combo ()
      {
        m_reward_combo = Gtk::make_managed<LwCombo> ();
        if (m_allow_renewables)
          {
            m_reward_combo->append (_("Gold"));
            m_reward_combo->append (_("Item"));
            m_reward_combo->append (_("Allies"));
            m_reward_combo->append (_("Map"));
            m_reward_combo->append (_("Hidden Ruin"));
          }
        else
          {
            m_reward_combo->append (_("Item"));
            m_reward_combo->append (_("Map"));
            m_reward_combo->append (_("Hidden Ruin"));
          }
        m_reward_combo->set_hexpand (false);
        m_reward_combobox->append (*m_reward_combo);
      }

    bool switch_reward_type (Reward::Type t)
      {
        if (!m_reward || t != m_reward->getType ())
          {
            switch (t)
              {
              case Reward::GOLD:
                m_reward = m_gold_reward;
                break;

              case Reward::ALLIES:
                m_reward = m_allies_reward;
                break;

              case Reward::ITEM:
                m_reward = m_item_reward;
                break;

              case Reward::MAP:
                m_reward = m_map_reward;
                break;

              case Reward::RUIN:
                m_reward = m_ruin_reward;
                break;
              }
            return true;
          }
        return false;
      }

    void setup_rewards (Reward *r)
      {
        m_reward = NULL;
        if (r && r->getType () == Reward::GOLD)
          m_gold_reward = new Reward_Gold (*dynamic_cast<Reward_Gold*>(r));
        else
          m_gold_reward = new Reward_Gold (100);

        if (r && r->getType () == Reward::ALLIES)
          m_allies_reward =
            new Reward_Allies (*dynamic_cast<Reward_Allies*>(r));
        else
          m_allies_reward = new Reward_Allies ();

        if (r && r->getType () == Reward::ITEM)
          m_item_reward = new Reward_Item (*dynamic_cast<Reward_Item*>(r));
        else
          m_item_reward = new Reward_Item ();

        if (r && r->getType () == Reward::MAP)
          m_map_reward = new Reward_Map (*dynamic_cast<Reward_Map*>(r));
        else
          m_map_reward = new Reward_Map (Vector<int>(0, 0),
                                         _("dusty map"), 1, 1);

        if (r && r->getType () == Reward::RUIN)
          m_ruin_reward = new Reward_Ruin (*dynamic_cast<Reward_Ruin*>(r));
        else
          m_ruin_reward = new Reward_Ruin ();

        if (!r)
          {
            if (m_allow_renewables)
              m_reward = m_gold_reward;
            else
              m_reward = m_item_reward;
          }
        else
          switch_reward_type (r->getType ());
      }

    Reward::Type row_to_reward_type (Glib::ustring text)
      {
        if (text == _("Gold"))
          return Reward::GOLD;
        else if (text == _("Item"))
          return Reward::ITEM;
        else if (text == _("Allies"))
          return Reward::ALLIES;
        else if (text == _("Map"))
          return Reward::MAP;
        else if (text == _("Hidden Ruin"))
          return Reward::RUIN;
            
        return Reward::GOLD;
      }

    void random_gold ()
      {
        m_umgr->add (new RewardUndoAction_RandomizeGold (m_reward));
        auto r = m_gold_reward;
        r->setGold (Reward_Gold::getRandomGoldPieces ());
        update ();
      }

    void random_allies ()
      {
        auto r = m_allies_reward;
        const ArmyProto *a = Reward_Allies::randomArmyAlly();
        if (!a)
          return;
        m_umgr->add (new RewardUndoAction_RandomizeAlly (m_reward));
        r->clearAllies ();
        r->setArmy (a);
        r->setNoOfAllies (Reward_Allies::getRandomAmountOfAllies ());
        update ();
      }

    void random_item ()
      {
        m_umgr->add (new RewardUndoAction_RandomizeItem (m_reward));
        auto r = m_item_reward;

        Item *item = r->getItem ();
        if (item)
          delete item;
        r->setItem (NULL);
        r->setItem (Reward_Item::getRandomItem ());

        update ();
      }

    void random_map ()
      {
        m_umgr->add (new RewardUndoAction_RandomizeMap (m_reward));
        auto r = m_map_reward;
        int x, y, width, height;
        Reward_Map::getRandomMap (&x, &y, &width, &height);
        r->getSightMap ()->x = x;
        r->getSightMap ()->y = y;
        r->getSightMap ()->w = width;
        r->getSightMap ()->h = height;
        r->getSightMap ()->setName (Reward_Map::getRandomName ());
        update ();
      }

    void random_ruin ()
      {
        Ruin *ruin = Reward_Ruin::getRandomHiddenRuin ();
        if (ruin)
          {
            m_umgr->add (new RewardUndoAction_RandomRuin (m_reward));
            auto r = m_ruin_reward;
            r->clearRuin ();
            r->setRuinPos (ruin->getPos ());
            update ();
          }
      }
                  
    void update_buttons ()
      {
        m_close_button->set_sensitive (Reward::is_valid (m_reward));
      }
};
#endif
