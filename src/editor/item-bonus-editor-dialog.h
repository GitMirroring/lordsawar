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
#ifndef ITEM_BONUS_EDITOR_DIALOG_H
#define ITEM_BONUS_EDITOR_DIALOG_H
#include "item-proto.h"
#include "item-list.h"
#include "army-type-label.h"
#include "item-bonus-editor-undo-actions.h"

class ItemBonusEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "item-bonus-editor.ui";
      }

    ItemBonusEditorDialog (BaseObjectType* o,
                           const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_kill_army_type_button = load <Gtk::Button> ("kill_army_type_button");
        m_summon_army_type_button =
          load <Gtk::Button> ("summon_army_type_button");
        m_defender_army_type_button =
          load <Gtk::Button> ("defender_army_type_button");
        m_summon_monster_combobox = load <Gtk::Box> ("summon_monster_combobox");
        m_disease_city_switch = load <Gtk::Switch> ("disease_city_switch");
        m_disease_army_percent_spinbutton =
          load <Gtk::SpinButton> ("disease_armies_percent_spinbutton");
        m_raise_defenders_switch =
          load <Gtk::Switch> ("raise_defenders_switch");
        m_num_defenders_spinbutton =
          load <Gtk::SpinButton> ("num_defenders_spinbutton");
        m_persuade_neutral_city_switch =
          load <Gtk::Switch> ("persuade_neutral_city_switch");
        m_teleport_to_city_switch =
          load <Gtk::Switch> ("teleport_to_city_switch");
        m_add1str_switch = load <Gtk::Switch> ("add1str_switch");
        m_add2str_switch = load <Gtk::Switch> ("add2str_switch");
        m_add3str_switch = load <Gtk::Switch> ("add3str_switch");
        m_add1stack_switch = load <Gtk::Switch> ("add1stack_switch");
        m_add2stack_switch = load <Gtk::Switch> ("add2stack_switch");
        m_add3stack_switch = load <Gtk::Switch> ("add3stack_switch");
        m_flystack_switch = load <Gtk::Switch> ("flystack_switch");
        m_doublemovestack_switch =
          load <Gtk::Switch> ("doublemovestack_switch");
        m_add2goldpercity_switch =
          load <Gtk::Switch> ("add2goldpercity_switch");
        m_add3goldpercity_switch =
          load <Gtk::Switch> ("add3goldpercity_switch");
        m_add4goldpercity_switch =
          load <Gtk::Switch> ("add4goldpercity_switch");
        m_add5goldpercity_switch =
          load <Gtk::Switch> ("add5goldpercity_switch");
        m_steals_gold_switch = load <Gtk::Switch> ("steals_gold_switch");
        m_pick_up_bags_switch = load <Gtk::Switch> ("pick_up_bags_switch");
        m_add_mp_switch = load <Gtk::Switch> ("add_mp_switch");
        m_sinks_ships_switch = load <Gtk::Switch> ("sinks_ships_switch");
        m_banish_worms_switch = load <Gtk::Switch> ("banish_worms_switch");
        m_burn_bridge_switch = load <Gtk::Switch> ("burn_bridge_switch");
        m_capture_keeper_switch = load <Gtk::Switch> ("capture_keeper_switch");
        m_summon_monster_switch = load <Gtk::Switch> ("summon_monster_switch");
        m_steal_percent_spinbutton =
          load <Gtk::SpinButton> ("steal_percent_spinbutton");
        m_add_mp_spinbutton = load <Gtk::SpinButton> ("add_mp_spinbutton");
        m_plantable_switch = load <Gtk::Switch> ("plantable_switch");
      }

    ~ItemBonusEditorDialog ()
      {
        disconnect_signals ();
        delete m_item;
        delete m_umgr;
      }

    void setup (Item *item, Shield::Color shield)
      {
        m_item = new Item (*item, true);
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_kill_army_type_label =
          Gtk::make_managed<ArmyTypeLabel>
          (this, m_kill_army_type_button, shield,
           SelectArmyDialog::SELECT_RUIN_DEFENDER);

        m_summon_army_type_label =
          Gtk::make_managed<ArmyTypeLabel> (this, m_summon_army_type_button,
                                            shield,
                                            SelectArmyDialog::SELECT_NORMAL);
        m_defender_army_type_label =
          Gtk::make_managed<ArmyTypeLabel> (this, m_defender_army_type_button,
                                            shield,
                                            SelectArmyDialog::SELECT_NORMAL);
        setup_undo ();

        fill_summon_monster_combo ();

        update ();
      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }

    Item *get_item ()
      {
        return m_item;
      }
private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_kill_army_type_button;
    ArmyTypeLabel *m_kill_army_type_label;
    Gtk::Button *m_summon_army_type_button;
    ArmyTypeLabel *m_summon_army_type_label;
    Gtk::Button *m_defender_army_type_button;
    ArmyTypeLabel *m_defender_army_type_label;
    Gtk::Box *m_summon_monster_combobox;
    LwCombo *m_summon_monster_combo;
    Gtk::Switch *m_disease_city_switch;
    Gtk::SpinButton *m_disease_army_percent_spinbutton;
    Gtk::Switch *m_raise_defenders_switch;
    Gtk::SpinButton *m_num_defenders_spinbutton;
    Gtk::Switch *m_persuade_neutral_city_switch;
    Gtk::Switch *m_teleport_to_city_switch;
    Gtk::Switch *m_add1str_switch;
    Gtk::Switch *m_add2str_switch;
    Gtk::Switch *m_add3str_switch;
    Gtk::Switch *m_add1stack_switch;
    Gtk::Switch *m_add2stack_switch;
    Gtk::Switch *m_add3stack_switch;
    Gtk::Switch *m_flystack_switch;
    Gtk::Switch *m_doublemovestack_switch;
    Gtk::Switch *m_add2goldpercity_switch;
    Gtk::Switch *m_add3goldpercity_switch;
    Gtk::Switch *m_add4goldpercity_switch;
    Gtk::Switch *m_add5goldpercity_switch;
    Gtk::Switch *m_steals_gold_switch;
    Gtk::Switch *m_pick_up_bags_switch;
    Gtk::Switch *m_add_mp_switch;
    Gtk::Switch *m_sinks_ships_switch;
    Gtk::Switch *m_banish_worms_switch;
    Gtk::Switch *m_burn_bridge_switch;
    Gtk::Switch *m_capture_keeper_switch;
    Gtk::Switch *m_summon_monster_switch;
    Gtk::SpinButton *m_steal_percent_spinbutton;
    Gtk::SpinButton *m_add_mp_spinbutton;
    Gtk::Switch *m_plantable_switch;

    std::list<sigc::connection> m_connections;

    UndoMgr *m_umgr;
    Item *m_item;

    void update_item_panel ()
      {
        disconnect_signals ();
        auto i = m_item;
        if (i)
          {
            m_steals_gold_switch->set_active
              (i->getBonus (ItemProto::STEAL_GOLD));
            m_steal_percent_spinbutton->set_value (i->getPercentGoldToSteal ());
            m_sinks_ships_switch->set_active
              (i->getBonus (ItemProto::SINK_SHIPS));
            m_persuade_neutral_city_switch->set_active
              (i->getBonus (ItemProto::PERSUADE_NEUTRALS));
            m_burn_bridge_switch->set_active
              (i->getBonus (ItemProto::BURN_BRIDGE));
            m_capture_keeper_switch->set_active
              (i->getBonus (ItemProto::CAPTURE_KEEPER));
            m_teleport_to_city_switch->set_active
              (i->getBonus (ItemProto::TELEPORT_TO_CITY));
            m_pick_up_bags_switch->set_active
              (i->getBonus (ItemProto::PICK_UP_BAGS));
            m_disease_city_switch->set_active
              (i->getBonus (ItemProto::DISEASE_CITY));
            m_disease_army_percent_spinbutton->set_value
              (i->getPercentArmiesToKill ());
            m_add_mp_switch->set_active
              (i->getBonus (ItemProto::ADD_2MP_STACK));
            m_add_mp_spinbutton->set_value (i->getMovementPointsToAdd ());
            m_banish_worms_switch->set_active
              (i->getBonus (ItemProto::BANISH_WORMS));
            if (i->hasArmyTypeToKill ())
              m_kill_army_type_label->set_army_type (i->getArmyTypeToKill ());
            else
              m_kill_army_type_label->clear ();
            m_summon_monster_switch->set_active
              (i->getBonus (ItemProto::SUMMON_MONSTER));
            if (i->hasArmyTypeToSummon ())
              m_summon_army_type_label->set_army_type (i->getArmyTypeToSummon ());
            else
              m_summon_army_type_label->clear ();
            m_summon_monster_combo->set_active
              (i->getBuildingTypeToSummonOn ());
            m_raise_defenders_switch->set_active
              (i->getBonus (ItemProto::RAISE_DEFENDERS));
            m_num_defenders_spinbutton->set_value
              (i->getNumberOfArmiesToRaise ());
            if (i->hasArmyTypeToRaise ())
              m_defender_army_type_label->set_army_type
                (i->getArmyTypeToRaise ());
            else
              m_defender_army_type_label->clear ();
            m_add1str_switch->set_active (i->getBonus (ItemProto::ADD1STR));
            m_add2str_switch->set_active (i->getBonus (ItemProto::ADD2STR));
            m_add3str_switch->set_active (i->getBonus (ItemProto::ADD3STR));
            m_add1stack_switch->set_active (i->getBonus (ItemProto::ADD1STACK));
            m_add2stack_switch->set_active (i->getBonus (ItemProto::ADD2STACK));
            m_add3stack_switch->set_active (i->getBonus (ItemProto::ADD3STACK));
            m_flystack_switch->set_active (i->getBonus (ItemProto::FLYSTACK));
            m_doublemovestack_switch->set_active
              (i->getBonus (ItemProto::DOUBLEMOVESTACK));
            m_add2goldpercity_switch->set_active
              (i->getBonus (ItemProto::ADD2GOLDPERCITY));
            m_add3goldpercity_switch->set_active
              (i->getBonus (ItemProto::ADD3GOLDPERCITY));
            m_add4goldpercity_switch->set_active
              (i->getBonus (ItemProto::ADD4GOLDPERCITY));
            m_add5goldpercity_switch->set_active
              (i->getBonus (ItemProto::ADD5GOLDPERCITY));
            m_plantable_switch->set_active
              (i->getBonus (ItemProto::PLANT_TO_VECTOR));
          }

        bool sensitive = true;
        bool b = m_banish_worms_switch->get_active ();
        m_kill_army_type_button->set_sensitive (sensitive && b);

        b = m_summon_monster_switch->get_active ();
        m_summon_army_type_button->set_sensitive (sensitive && b);
        m_summon_monster_combo->set_sensitive (sensitive && b);

        m_disease_city_switch->set_sensitive (sensitive);
        b = m_disease_city_switch->get_active ();
        m_disease_army_percent_spinbutton->set_sensitive (sensitive && b);

        m_raise_defenders_switch->set_sensitive (sensitive);
        b = m_raise_defenders_switch->get_active ();
        m_num_defenders_spinbutton->set_sensitive (sensitive && b);
        m_defender_army_type_button->set_sensitive (sensitive && b);

        m_persuade_neutral_city_switch->set_sensitive (sensitive);
        m_teleport_to_city_switch->set_sensitive (sensitive);
        m_add1str_switch->set_sensitive (sensitive);
        m_add2str_switch->set_sensitive (sensitive);
        m_add3str_switch->set_sensitive (sensitive);
        m_add1stack_switch->set_sensitive (sensitive);
        m_add2stack_switch->set_sensitive (sensitive);
        m_add3stack_switch->set_sensitive (sensitive);
        m_flystack_switch->set_sensitive (sensitive);
        m_doublemovestack_switch->set_sensitive (sensitive);
        m_add2goldpercity_switch->set_sensitive (sensitive);
        m_add3goldpercity_switch->set_sensitive (sensitive);
        m_add4goldpercity_switch->set_sensitive (sensitive);
        m_add5goldpercity_switch->set_sensitive (sensitive);
        m_steals_gold_switch->set_sensitive (sensitive);
        m_pick_up_bags_switch->set_sensitive (sensitive);
        m_add_mp_switch->set_sensitive (sensitive);
        m_sinks_ships_switch->set_sensitive (sensitive);
        m_banish_worms_switch->set_sensitive (sensitive);
        m_burn_bridge_switch->set_sensitive (sensitive);
        m_capture_keeper_switch->set_sensitive (sensitive);
        m_summon_monster_switch->set_sensitive (sensitive);

        b = m_steals_gold_switch->get_active ();
        m_steal_percent_spinbutton->set_sensitive (sensitive && b);

        b = m_add_mp_switch->get_active ();
        m_add_mp_spinbutton->set_sensitive (sensitive && b);

        m_plantable_switch->set_sensitive (sensitive);
        connect_signals ();
      }

    void fill_summon_monster_combo ()
      {
        m_summon_monster_combo = Gtk::make_managed<LwCombo> ();
        m_summon_monster_combo->append (_("Anywhere"));
        m_summon_monster_combo->append (_("On a City"));
        m_summon_monster_combo->append (_("On a Ruin"));
        m_summon_monster_combo->append (_("On a Temple"));
        m_summon_monster_combo->append (_("On a Signpost"));
        m_summon_monster_combo->append (_("On a Road"));
        m_summon_monster_combo->append (_("On a Port"));
        m_summon_monster_combo->append (_("On a Bridge"));
        m_summon_monster_combo->append (_("On a Standing Stone"));
        m_summon_monster_combo->set_hexpand (false);
        m_summon_monster_combobox->append (*m_summon_monster_combo);
      }

    void
    add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void connect_signals ()
      {
        add_connection
          (m_kill_army_type_label->signal_army_selected ().connect
           ([this] (int type_id)
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_SelectBanishArmy
                 (m_item, m_item->getArmyTypeToKill (),
                  m_item->hasArmyTypeToKill ()));
              if (type_id >= 0)
                m_item->setArmyTypeToKill ((guint32) type_id);
              else
                m_item->clearArmyTypeToKill ();
            }));

        add_connection
          (m_summon_army_type_label->signal_army_selected ().connect
           ([this] (int type_id)
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_SelectSummonArmy
                 (m_item, m_item->getArmyTypeToSummon (),
                  m_item->hasArmyTypeToSummon ()));
              if (type_id >= 0)
                m_item->setArmyTypeToSummon ((guint32) type_id);
              else
                m_item->clearArmyTypeToSummon ();
            }));

        add_connection
          (m_defender_army_type_label->signal_army_selected ().connect
           ([this] (int type_id)
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_SelectRaiseArmy
                 (m_item, m_item->getArmyTypeToRaise (),
                  m_item->hasArmyTypeToRaise ()));

              if (type_id >= 0)
                m_item->setArmyTypeToRaise ((guint32) type_id);
              else
                m_item->clearArmyTypeToRaise ();
            }));

        add_connection
          (m_summon_monster_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_BuildingTypeToSummonOn
                 (m_item, m_item->getBuildingTypeToSummonOn ()));

              m_item->setBuildingTypeToSummonOn
                (m_summon_monster_combo->get_selected ());
            }));

        add_connection
          (m_disease_city_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_DiseaseCity
                 (m_item, m_item->getBonus (ItemProto::DISEASE_CITY)));

              m_disease_army_percent_spinbutton->set_sensitive
                (m_disease_city_switch->get_active ());
              if (m_disease_city_switch->get_active ())
                m_item->addBonus (ItemProto::DISEASE_CITY);
              else
                m_item->removeBonus (ItemProto::DISEASE_CITY);
            }));

        add_connection
          (m_disease_army_percent_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_DiseaseArmies
                 (m_item, m_item->getPercentArmiesToKill ()));

              m_item->setPercentArmiesToKill
                (m_disease_army_percent_spinbutton->get_value_as_int ());
            }));

        add_connection
          (m_raise_defenders_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_defender_army_type_button->set_sensitive
                (m_raise_defenders_switch->get_active ());
              m_umgr->add
                (new ItemBonusEditorUndoAction_RaiseDefenders
                 (m_item, m_item->getBonus (ItemProto::RAISE_DEFENDERS)));

              m_num_defenders_spinbutton->set_sensitive
                (m_raise_defenders_switch->get_active ());
              if (m_raise_defenders_switch->get_active ())
                m_item->addBonus (ItemProto::RAISE_DEFENDERS);
              else
                m_item->removeBonus (ItemProto::RAISE_DEFENDERS);
            }));

        add_connection
          (m_num_defenders_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_NumDefenders
                 (m_item, m_item->getNumberOfArmiesToRaise ()));

              m_item->setNumberOfArmiesToRaise
                (m_num_defenders_spinbutton->get_value_as_int ());
            }));

        add_connection
          (m_persuade_neutral_city_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_PersuadeNeutralCity
                 (m_item, m_item->getBonus (ItemProto::PERSUADE_NEUTRALS)));

              if (m_persuade_neutral_city_switch->get_active ())
                m_item->addBonus (ItemProto::PERSUADE_NEUTRALS);
              else
                m_item->removeBonus (ItemProto::PERSUADE_NEUTRALS);
            }));

        add_connection
          (m_teleport_to_city_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_TeleportToCity
                 (m_item, m_item->getBonus (ItemProto::TELEPORT_TO_CITY)));

              if (m_teleport_to_city_switch->get_active ())
                m_item->addBonus (ItemProto::TELEPORT_TO_CITY);
              else
                m_item->removeBonus (ItemProto::TELEPORT_TO_CITY);
            }));

        add_connection
          (m_add1str_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_AddStr
                 (m_item, 1, m_item->getBonus (ItemProto::ADD1STR)));

              if (m_add1str_switch->get_active ())
                m_item->addBonus (ItemProto::ADD1STR);
              else
                m_item->removeBonus (ItemProto::ADD1STR);
            }));

        add_connection
          (m_add2str_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_AddStr
                 (m_item, 2, m_item->getBonus (ItemProto::ADD2STR)));

              if (m_add2str_switch->get_active ())
                m_item->addBonus (ItemProto::ADD2STR);
              else
                m_item->removeBonus (ItemProto::ADD2STR);
            }));

        add_connection
          (m_add3str_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_AddStr
                 (m_item, 3, m_item->getBonus (ItemProto::ADD3STR)));

              if (m_add3str_switch->get_active ())
                m_item->addBonus (ItemProto::ADD3STR);
              else
                m_item->removeBonus (ItemProto::ADD3STR);
            }));

        add_connection
          (m_add1stack_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_AddStack
                 (m_item, 1, m_item->getBonus (ItemProto::ADD1STACK)));

              if (m_add1stack_switch->get_active ())
                m_item->addBonus (ItemProto::ADD1STACK);
              else
                m_item->removeBonus (ItemProto::ADD1STACK);
            }));

        add_connection
          (m_add2stack_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_AddStack
                 (m_item, 2, m_item->getBonus (ItemProto::ADD2STACK)));

              if (m_add2stack_switch->get_active ())
                m_item->addBonus (ItemProto::ADD2STACK);
              else
                m_item->removeBonus (ItemProto::ADD2STACK);
            }));

        add_connection
          (m_add3stack_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_AddStack
                 (m_item, 3, m_item->getBonus (ItemProto::ADD3STACK)));

              if (m_add3stack_switch->get_active ())
                m_item->addBonus (ItemProto::ADD3STACK);
              else
                m_item->removeBonus (ItemProto::ADD3STACK);
            }));

        add_connection
          (m_flystack_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_FlyStack
                 (m_item, m_item->getBonus (ItemProto::FLYSTACK)));

              if (m_flystack_switch->get_active ())
                m_item->addBonus (ItemProto::FLYSTACK);
              else
                m_item->removeBonus (ItemProto::FLYSTACK);
            }));

        add_connection
          (m_doublemovestack_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_DoubleMovementStack
                 (m_item, m_item->getBonus (ItemProto::DOUBLEMOVESTACK)));

              if (m_doublemovestack_switch->get_active ())
                m_item->addBonus (ItemProto::DOUBLEMOVESTACK);
              else
                m_item->removeBonus (ItemProto::DOUBLEMOVESTACK);
            }));

        add_connection
          (m_add2goldpercity_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_AddGoldPerCity
                 (m_item, 2, m_item->getBonus (ItemProto::ADD2GOLDPERCITY)));

              if (m_add2goldpercity_switch->get_active ())
                m_item->addBonus (ItemProto::ADD2GOLDPERCITY);
              else
                m_item->removeBonus (ItemProto::ADD2GOLDPERCITY);
            }));

        add_connection
          (m_add3goldpercity_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_AddGoldPerCity
                 (m_item, 3, m_item->getBonus (ItemProto::ADD3GOLDPERCITY)));

              if (m_add3goldpercity_switch->get_active ())
                m_item->addBonus (ItemProto::ADD3GOLDPERCITY);
              else
                m_item->removeBonus (ItemProto::ADD3GOLDPERCITY);
            }));

        add_connection
          (m_add4goldpercity_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_AddGoldPerCity
                 (m_item, 4, m_item->getBonus (ItemProto::ADD4GOLDPERCITY)));

              if (m_add4goldpercity_switch->get_active ())
                m_item->addBonus (ItemProto::ADD4GOLDPERCITY);
              else
                m_item->removeBonus (ItemProto::ADD4GOLDPERCITY);
            }));

        add_connection
          (m_add5goldpercity_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_AddGoldPerCity
                 (m_item, 5, m_item->getBonus (ItemProto::ADD5GOLDPERCITY)));

              if (m_add5goldpercity_switch->get_active ())
                m_item->addBonus (ItemProto::ADD5GOLDPERCITY);
              else
                m_item->removeBonus (ItemProto::ADD5GOLDPERCITY);
            }));

        add_connection
          (m_steals_gold_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_steal_percent_spinbutton->set_sensitive
                (m_steals_gold_switch->get_active ());
              m_umgr->add
                (new ItemBonusEditorUndoAction_StealsGold
                 (m_item, m_item->getBonus (ItemProto::STEAL_GOLD)));

              if (m_steals_gold_switch->get_active ())
                m_item->addBonus (ItemProto::STEAL_GOLD);
              else
                m_item->removeBonus (ItemProto::STEAL_GOLD);
            }));

        add_connection
          (m_pick_up_bags_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_PickUpBags
                 (m_item, m_item->getBonus (ItemProto::PICK_UP_BAGS)));

              if (m_pick_up_bags_switch->get_active ())
                m_item->addBonus (ItemProto::PICK_UP_BAGS);
              else
                m_item->removeBonus (ItemProto::PICK_UP_BAGS);
            }));

        add_connection
          (m_add_mp_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_add_mp_spinbutton->set_sensitive
                (m_add_mp_switch->get_active ());
              m_umgr->add
                (new ItemBonusEditorUndoAction_AddMovement
                 (m_item, m_item->getBonus (ItemProto::ADD_2MP_STACK)));

              if (m_add_mp_switch->get_active ())
                m_item->addBonus (ItemProto::ADD_2MP_STACK);
              else
                m_item->removeBonus (ItemProto::ADD_2MP_STACK);
            }));

        add_connection
          (m_sinks_ships_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_SinkShips
                 (m_item, m_item->getBonus (ItemProto::SINK_SHIPS)));

              if (m_sinks_ships_switch->get_active ())
                m_item->addBonus (ItemProto::SINK_SHIPS);
              else
                m_item->removeBonus (ItemProto::SINK_SHIPS);
            }));

        add_connection
          (m_banish_worms_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_kill_army_type_button->set_sensitive
                (m_banish_worms_switch->get_active ());
              m_umgr->add
                (new ItemBonusEditorUndoAction_BanishWorms
                 (m_item, m_item->getBonus (ItemProto::BANISH_WORMS)));

              if (m_banish_worms_switch->get_active ())
                m_item->addBonus (ItemProto::BANISH_WORMS);
              else
                m_item->removeBonus (ItemProto::BANISH_WORMS);
            }));

        add_connection
          (m_burn_bridge_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_BurnBridge
                 (m_item, m_item->getBonus (ItemProto::BURN_BRIDGE)));

              if (m_burn_bridge_switch->get_active ())
                m_item->addBonus (ItemProto::BURN_BRIDGE);
              else
                m_item->removeBonus (ItemProto::SINK_SHIPS);
            }));

        add_connection
          (m_capture_keeper_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_CaptureKeeper
                 (m_item, m_item->getBonus (ItemProto::CAPTURE_KEEPER)));

              if (m_capture_keeper_switch->get_active ())
                m_item->addBonus (ItemProto::CAPTURE_KEEPER);
              else
                m_item->removeBonus (ItemProto::CAPTURE_KEEPER);
            }));

        add_connection
          (m_summon_monster_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_summon_army_type_button->set_sensitive
                (m_summon_monster_switch->get_active ());
              m_summon_monster_combo->set_sensitive
                (m_summon_monster_switch->get_active ());
              m_umgr->add
                (new ItemBonusEditorUndoAction_SummonMonster
                 (m_item, m_item->getBonus (ItemProto::SUMMON_MONSTER)));

              if (m_summon_monster_switch->get_active ())
                m_item->addBonus (ItemProto::SUMMON_MONSTER);
              else
                m_item->removeBonus (ItemProto::SUMMON_MONSTER);
            }));

        add_connection
          (m_steal_percent_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_StealPercent
                 (m_item, m_item->getPercentGoldToSteal ()));

              m_item->setPercentGoldToSteal
                (m_steal_percent_spinbutton->get_value_as_int ());
            }));

        add_connection
          (m_add_mp_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_AddMp
                 (m_item, m_item->getMovementPointsToAdd ()));

              m_item->setMovementPointsToAdd
                (m_add_mp_spinbutton->get_value_as_int ());
            }));

        add_connection
          (m_plantable_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemBonusEditorUndoAction_Plantable
                 (m_item, m_item->getBonus (ItemProto::PLANT_TO_VECTOR)));

              if (m_plantable_switch->get_active ())
                m_item->addBonus (ItemProto::PLANT_TO_VECTOR);
              else
                m_item->removeBonus (ItemProto::PLANT_TO_VECTOR);
            }));

      }

    void update ()
      {
        update_item_panel ();
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    UndoAction *execute_action (UndoAction *action2)
      {
        ItemBonusEditorUndoAction *action = dynamic_cast<ItemBonusEditorUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case ItemBonusEditorUndoAction::BUILDING_TYPE_TO_SUMMON_ON:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_BuildingTypeToSummonOn*>(action);
                int row = 
                  m_summon_monster_combo->get_active_row_number ();
                out = new ItemBonusEditorUndoAction_BuildingTypeToSummonOn
                  (m_item, row);
                m_item->setBuildingTypeToSummonOn (a->get_row ());
              } 
            break;

          case ItemBonusEditorUndoAction::DISEASE_CITY:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_DiseaseCity*>(action);
                out = new ItemBonusEditorUndoAction_DiseaseCity
                  (m_item, m_disease_city_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::DISEASE_CITY);
                else
                  m_item->removeBonus (ItemProto::DISEASE_CITY);
              }
            break;

          case ItemBonusEditorUndoAction::DISEASE_ARMIES_PERCENT:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_DiseaseArmies*>(action);
                out = new ItemBonusEditorUndoAction_DiseaseArmies
                  (m_item, m_disease_army_percent_spinbutton->get_value ());
                m_item->setPercentArmiesToKill (a->get_num ());
              }
            break;

          case ItemBonusEditorUndoAction::RAISE_DEFENDERS:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_RaiseDefenders*>(action);
                out = new ItemBonusEditorUndoAction_RaiseDefenders
                  (m_item, m_raise_defenders_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::RAISE_DEFENDERS);
                else
                  m_item->removeBonus (ItemProto::RAISE_DEFENDERS);
              }
            break;

          case ItemBonusEditorUndoAction::NUM_DEFENDERS:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_NumDefenders*>(action);
                out = new ItemBonusEditorUndoAction_NumDefenders
                  (m_item, m_num_defenders_spinbutton->get_value ());
                m_item->setNumberOfArmiesToRaise (a->get_num ());
              }
            break;

          case ItemBonusEditorUndoAction::PERSUADE_NEUTRAL_CITY:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_PersuadeNeutralCity*>(action);
                out = new ItemBonusEditorUndoAction_PersuadeNeutralCity
                  (m_item, m_persuade_neutral_city_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::PERSUADE_NEUTRALS);
                else
                  m_item->removeBonus (ItemProto::PERSUADE_NEUTRALS);
              }
            break;

          case ItemBonusEditorUndoAction::TELEPORT_TO_CITY:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_TeleportToCity*>(action);
                out = new ItemBonusEditorUndoAction_TeleportToCity
                  (m_item, m_teleport_to_city_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::TELEPORT_TO_CITY);
                else
                  m_item->removeBonus (ItemProto::TELEPORT_TO_CITY);
              }
            break;

          case ItemBonusEditorUndoAction::ADDSTR:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_AddStr*>(action);
                bool state = false;
                switch (a->get_num ())
                  {
                  default:
                  case 1:
                    state = m_add1str_switch->get_active ();
                    break;

                  case 2:
                    state = m_add2str_switch->get_active ();
                    break;

                  case 3:
                    state = m_add3str_switch->get_active ();
                    break;
                  }
                out = new ItemBonusEditorUndoAction_AddStr (m_item, a->get_num (),
                                                     state);
                ItemProto::Bonus b = ItemProto::ADD1STR;
                switch (a->get_num ())
                  {
                  default:
                  case 1:
                    b = ItemProto::ADD1STR;
                    break;

                  case 2:
                    b = ItemProto::ADD2STR;
                    break;

                  case 3:
                    b = ItemProto::ADD3STR;
                    break;
                  }
                if (a->get_active ())
                  m_item->addBonus (b);
                else
                  m_item->removeBonus (b);
              }
            break;

          case ItemBonusEditorUndoAction::ADDSTACK:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_AddStack*>(action);
                bool state = false;
                switch (a->get_num ())
                  {
                  default:
                  case 1:
                    state = m_add1stack_switch->get_active ();
                    break;

                  case 2:
                    state = m_add2stack_switch->get_active ();
                    break;

                  case 3:
                    state = m_add3stack_switch->get_active ();
                    break;
                  }
                out = new ItemBonusEditorUndoAction_AddStack (m_item,
                                                       a->get_num (), state);
                ItemProto::Bonus b = ItemProto::ADD1STACK;
                switch (a->get_num ())
                  {
                  default:
                  case 1:
                    b = ItemProto::ADD1STACK;
                    break;

                  case 2:
                    b = ItemProto::ADD2STACK;
                    break;

                  case 3:
                    b = ItemProto::ADD3STACK;
                    break;
                  }
                if (a->get_active ())
                  m_item->addBonus (b);
                else
                  m_item->removeBonus (b);
              }
            break;

          case ItemBonusEditorUndoAction::FLY_STACK:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_FlyStack*>(action);
                out = new ItemBonusEditorUndoAction_FlyStack
                  (m_item, m_flystack_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::FLYSTACK);
                else
                  m_item->removeBonus (ItemProto::FLYSTACK);
              }
            break;

          case ItemBonusEditorUndoAction::DOUBLE_MOVEMENT_STACK:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_DoubleMovementStack*>(action);
                out = new ItemBonusEditorUndoAction_DoubleMovementStack
                  (m_item, m_doublemovestack_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::DOUBLEMOVESTACK);
                else
                  m_item->removeBonus (ItemProto::DOUBLEMOVESTACK);
              }
            break;

          case ItemBonusEditorUndoAction::ADD_GOLD_PER_CITY:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_AddGoldPerCity*>(action);
                bool state = false;
                switch (a->get_num ())
                  {
                  default:
                  case 2:
                    state = m_add2goldpercity_switch->get_active ();
                    break;

                  case 3:
                    state = m_add3goldpercity_switch->get_active ();
                    break;

                  case 4:
                    state = m_add4goldpercity_switch->get_active ();
                    break;
                  case 5:
                    state = m_add5goldpercity_switch->get_active ();
                    break;
                  }
                out = new ItemBonusEditorUndoAction_AddGoldPerCity (m_item,
                                                             a->get_num (), state);
                ItemProto::Bonus b = ItemProto::ADD2GOLDPERCITY;
                switch (a->get_num ())
                  {
                  default:
                  case 2:
                    b = ItemProto::ADD2GOLDPERCITY;
                    break;

                  case 3:
                    b = ItemProto::ADD3GOLDPERCITY;
                    break;

                  case 4:
                    b = ItemProto::ADD4GOLDPERCITY;
                    break;

                  case 5:
                    b = ItemProto::ADD5GOLDPERCITY;
                    break;
                  }
                if (a->get_active ())
                  m_item->addBonus (b);
                else
                  m_item->removeBonus (b);
              }
            break;

          case ItemBonusEditorUndoAction::STEALS_GOLD:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_StealsGold*>(action);
                out = new ItemBonusEditorUndoAction_StealsGold
                  (m_item, m_steals_gold_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::STEAL_GOLD);
                else
                  m_item->removeBonus (ItemProto::STEAL_GOLD);
              }
            break;

          case ItemBonusEditorUndoAction::PICK_UP_BAGS:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_PickUpBags*>(action);
                out = new ItemBonusEditorUndoAction_PickUpBags
                  (m_item, m_pick_up_bags_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::PICK_UP_BAGS);
                else
                  m_item->removeBonus (ItemProto::PICK_UP_BAGS);
              }
            break;

          case ItemBonusEditorUndoAction::ADD_MOVEMENT:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_AddMovement*>(action);
                out = new ItemBonusEditorUndoAction_AddMovement
                  (m_item, m_add_mp_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::ADD_2MP_STACK);
                else
                  m_item->removeBonus (ItemProto::ADD_2MP_STACK);
              }
            break;

          case ItemBonusEditorUndoAction::SINKS_SHIPS:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_SinkShips*>(action);
                out = new ItemBonusEditorUndoAction_SinkShips
                  (m_item, m_sinks_ships_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::SINK_SHIPS);
                else
                  m_item->removeBonus (ItemProto::SINK_SHIPS);
              }
            break;

          case ItemBonusEditorUndoAction::BANISH_WORMS:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_BanishWorms*>(action);
                out = new ItemBonusEditorUndoAction_BanishWorms
                  (m_item, m_banish_worms_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::BANISH_WORMS);
                else
                  m_item->removeBonus (ItemProto::BANISH_WORMS);
              }
            break;

          case ItemBonusEditorUndoAction::BURN_BRIDGE:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_BurnBridge*>(action);
                out = new ItemBonusEditorUndoAction_BurnBridge
                  (m_item, m_burn_bridge_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::BURN_BRIDGE);
                else
                  m_item->removeBonus (ItemProto::BURN_BRIDGE);
              }
            break;

          case ItemBonusEditorUndoAction::CAPTURE_KEEPER:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_CaptureKeeper*>(action);
                out = new ItemBonusEditorUndoAction_CaptureKeeper
                  (m_item, m_capture_keeper_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::CAPTURE_KEEPER);
                else
                  m_item->removeBonus (ItemProto::CAPTURE_KEEPER);
              }
            break;

          case ItemBonusEditorUndoAction::SUMMON_MONSTER:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_SummonMonster*>(action);
                out = new ItemBonusEditorUndoAction_SummonMonster
                  (m_item, m_summon_monster_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::SUMMON_MONSTER);
                else
                  m_item->removeBonus (ItemProto::SUMMON_MONSTER);
              }
            break;

          case ItemBonusEditorUndoAction::STEAL_PERCENT:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_StealPercent*>(action);
                out = new ItemBonusEditorUndoAction_StealPercent
                  (m_item, m_steal_percent_spinbutton->get_value ());
                m_item->setPercentGoldToSteal(a->get_num ());
              }
            break;

          case ItemBonusEditorUndoAction::ADD_MP:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_AddMp*>(action);
                out = new ItemBonusEditorUndoAction_AddMp
                  (m_item, m_add_mp_spinbutton->get_value ());
                m_item->setMovementPointsToAdd(a->get_num ());
              }
            break;

          case ItemBonusEditorUndoAction::PLANTABLE:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_Plantable*>(action);
                out = new ItemBonusEditorUndoAction_Plantable
                  (m_item, m_plantable_switch->get_active ());
                if (a->get_active ())
                  m_item->addBonus (ItemProto::PLANT_TO_VECTOR);
                else
                  m_item->removeBonus (ItemProto::PLANT_TO_VECTOR);
              }
            break;

          case ItemBonusEditorUndoAction::SELECT_BANISH_ARMY:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_SelectBanishArmy*>(action);
                out = new ItemBonusEditorUndoAction_SelectBanishArmy
                  (m_item, m_item->getArmyTypeToKill (),
                   m_item->hasArmyTypeToKill ());
                if (a->get_present () == false)
                  m_item->clearArmyTypeToKill ();
                else
                  m_item->setArmyTypeToKill (a->get_army_type ());
                m_kill_army_type_label->set_army_type (a->get_army_type ());
              }
            break;

          case ItemBonusEditorUndoAction::SELECT_SUMMON_ARMY:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_SelectSummonArmy*>(action);
                out = new ItemBonusEditorUndoAction_SelectSummonArmy
                  (m_item, m_item->getArmyTypeToSummon (),
                   m_item->hasArmyTypeToSummon ());
                if (a->get_present () == false)
                  m_item->clearArmyTypeToSummon ();
                else
                  m_item->setArmyTypeToSummon (a->get_army_type ());
                m_summon_army_type_label->set_army_type (a->get_army_type ());
              }
            break;

          case ItemBonusEditorUndoAction::SELECT_RAISE_ARMY:
              {
                auto a =
                  dynamic_cast<ItemBonusEditorUndoAction_SelectRaiseArmy*>(action);
                out = new ItemBonusEditorUndoAction_SelectRaiseArmy
                  (m_item, m_item->getArmyTypeToRaise (),
                   m_item->hasArmyTypeToRaise ());
                if (a->get_present () == false)
                  m_item->clearArmyTypeToRaise ();
                else
                  m_item->setArmyTypeToRaise (a->get_army_type ());
                m_defender_army_type_label->set_army_type (a->get_army_type ());
              }
            break;
          }
        return out;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &ItemBonusEditorDialog::execute_action));

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
};
#endif
