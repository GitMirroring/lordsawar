//  Copyright (C) 2008, 2009, 2011, 2014, 2015, 2020, 2021, 2026 Ben Asselstine
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
#ifndef ITEM_LIST_DIALOG_H
#define ITEM_LIST_DIALOG_H
#include "item-proto.h"
#include "item-list.h"
#include "army-type-label.h"
#include "item-list-undo-actions.h"
#include "lw-column.h"

class ItemProtoRow: public Glib::Object
{
public:

    ItemProto *m_item;
    sigc::signal<void()> signal_changed ()
      {
        return m_signal_changed;
      }

    static Glib::RefPtr<ItemProtoRow> create (ItemProto *i)
      {
        return
          Glib::make_refptr_for_instance<ItemProtoRow> (new ItemProtoRow (i));
      }

    void changed ()
      {
        m_signal_changed.emit ();
      }
protected:
    ItemProtoRow (ItemProto *i)
      : m_item (i)
      {
      }

    sigc::signal<void()> m_signal_changed;
};

class ItemListDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "itemlist.ui";
      }

    ItemListDialog (BaseObjectType* o,
                    const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_add_button = load <Gtk::Button> ("add_button");
        m_remove_button = load <Gtk::Button> ("remove_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
        m_scrolled_window = load <Gtk::ScrolledWindow> ("scrolled_window");
        m_name_entry = load <Gtk::Entry> ("name_entry");
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
        m_uses_spinbutton = load <Gtk::SpinButton> ("uses_spinbutton");
        m_steal_percent_spinbutton =
          load <Gtk::SpinButton> ("steal_percent_spinbutton");
        m_add_mp_spinbutton = load <Gtk::SpinButton> ("add_mp_spinbutton");
        m_plantable_switch = load <Gtk::Switch> ("plantable_switch");
      }

    ~ItemListDialog ()
      {
        disconnect_signals ();
        delete m_itemlist;
        delete m_umgr;
      }

    void setup (Itemlist *itemlist)
      {
        m_itemlist = itemlist->copy ();
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_store = Gio::ListStore<ItemProtoRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        m_add_button->set_icon_name ("list-add-symbolic");
        m_remove_button->set_icon_name ("list-remove-symbolic");

        m_selection_model->signal_selection_changed ().connect
          ([this] (const guint &, const guint &)
           {
             scroll_item_to_top ();
             update ();
           });

        auto p = Playerlist::getActiveplayer ();

        m_kill_army_type_label =
          Gtk::make_managed<ArmyTypeLabel>
          (this, m_kill_army_type_button, p->get_shield (),
           SelectArmyDialog::SELECT_RUIN_DEFENDER);

        m_summon_army_type_label =
          Gtk::make_managed<ArmyTypeLabel> (this, m_summon_army_type_button,
                                            p->get_shield (),
                                            SelectArmyDialog::SELECT_NORMAL);
        m_defender_army_type_label =
          Gtk::make_managed<ArmyTypeLabel> (this, m_defender_army_type_button,
                                            p->get_shield (),
                                            SelectArmyDialog::SELECT_NORMAL);

        fill_treeview ();

        setup_name_column ();

        setup_undo ();

        fill_summon_monster_combo ();

        update ();
      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }

    Itemlist *get_item_list ()
      {
        return m_itemlist;
      }
private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_add_button;
    Gtk::Button *m_remove_button;
    Gtk::ColumnView *m_treeview;
    Gtk::ScrolledWindow *m_scrolled_window;
    Gtk::Entry *m_name_entry;
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
    Gtk::SpinButton *m_uses_spinbutton;
    Gtk::SpinButton *m_steal_percent_spinbutton;
    Gtk::SpinButton *m_add_mp_spinbutton;
    Gtk::Switch *m_plantable_switch;

    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<ItemProtoRow>> m_store;

    std::list<sigc::connection> m_connections;

    UndoMgr *m_umgr;
    Itemlist *m_itemlist;

    void setup_name_column ()
      {
        LwColumn::setup_name_column<ItemProtoRow> 
          (m_treeview, true, _("Items"),
           [] (const auto& row)
           {
             return row->m_item->getName ();
           });
      }

    void update_item_panel ()
      {
        disconnect_signals ();
        auto i = get_selected_item ();
        bool sensitive = false;
        if (i)
          {
            sensitive = true;
            m_name_entry->set_text (i->getName ());
            m_uses_spinbutton->set_value (i->getNumberOfUsesLeft ());
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
        else
          {
            sensitive = false;
            m_name_entry->set_text ("");
            m_kill_army_type_label->clear ();
            m_summon_army_type_label->clear ();
            m_defender_army_type_label->clear ();
            m_summon_monster_combo->set_active (0);
            m_disease_city_switch->set_active (false);
            double lo =
              m_disease_army_percent_spinbutton->get_adjustment
              ()->get_lower ();
            m_disease_army_percent_spinbutton->set_value (lo);
            m_raise_defenders_switch->set_active (false);
            lo = m_num_defenders_spinbutton->get_adjustment ()->get_lower ();
            m_num_defenders_spinbutton->set_value (lo);
            m_persuade_neutral_city_switch->set_active (false);
            m_teleport_to_city_switch->set_active (false);
            m_add1str_switch->set_active (false);
            m_add2str_switch->set_active (false);
            m_add3str_switch->set_active (false);
            m_add1stack_switch->set_active (false);
            m_add2stack_switch->set_active (false);
            m_add3stack_switch->set_active (false);
            m_flystack_switch->set_active (false);
            m_doublemovestack_switch->set_active (false);
            m_add2goldpercity_switch->set_active (false);
            m_add3goldpercity_switch->set_active (false);
            m_add4goldpercity_switch->set_active (false);
            m_add5goldpercity_switch->set_active (false);
            m_steals_gold_switch->set_active (false);
            m_pick_up_bags_switch->set_active (false);
            m_add_mp_switch->set_active (false);
            m_sinks_ships_switch->set_active (false);
            m_banish_worms_switch->set_active (false);
            m_burn_bridge_switch->set_active (false);
            m_capture_keeper_switch->set_active (false);
            m_summon_monster_switch->set_active (false);
            lo = m_uses_spinbutton->get_adjustment ()->get_lower ();
            m_uses_spinbutton->set_value (lo);
            lo = m_steal_percent_spinbutton->get_adjustment ()->get_lower ();
            m_steal_percent_spinbutton->set_value (lo);
            lo = m_add_mp_spinbutton->get_adjustment ()->get_lower ();
            m_add_mp_spinbutton->set_value (lo);
            m_plantable_switch->set_active (false);
          }

        m_name_entry->set_sensitive (sensitive);

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
        m_uses_spinbutton->set_sensitive (sensitive);

        b = m_steals_gold_switch->get_active ();
        m_steal_percent_spinbutton->set_sensitive (sensitive && b);

        b = m_add_mp_switch->get_active ();
        m_add_mp_spinbutton->set_sensitive (sensitive && b);

        m_plantable_switch->set_sensitive (sensitive);
        connect_signals ();
      }

    void scroll_item_to_top ()
      {
        m_scrolled_window->get_vadjustment ()->set_value (0.0);
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
          (m_add_button->signal_clicked ().connect
           ([this] ()
            {
              m_umgr->add
                (new ItemListUndoAction_Add
                 (m_itemlist->copy (), m_selection_model->get_selected ()));

              auto i = new ItemProto (_("Untitled"));
              m_itemlist->add (i);

              m_store->append (ItemProtoRow::create (i));

              guint n = m_selection_model->get_n_items ();
              if (n > 0)
                m_selection_model->set_selected (n - 1);

              scroll_item_to_top ();
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
                    (new ItemListUndoAction_Remove
                     (m_itemlist->copy (), m_selection_model->get_selected ()));
                  auto item = get_selected_item ();
                  m_itemlist->remove (item);

                  m_store->remove (m_selection_model->get_selected ());
                  scroll_item_to_top ();
                  update ();
                }
            }));

        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              m_umgr->add
                (new ItemListUndoAction_Name
                 (m_selection_model->get_selected (), item->getName (),
                  m_umgr, m_name_entry));

              item->setName (m_name_entry->get_text ());

              auto i = m_selection_model->get_selected_item ();
              if (i)
                {
                  auto row = std::dynamic_pointer_cast<ItemProtoRow>(i);
                  row->changed ();
                }
            }));

        add_connection
          (m_kill_army_type_label->signal_army_selected ().connect
           ([this] (int type_id)
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_SelectBanishArmy
                     (m_selection_model->get_selected (),
                      item->getArmyTypeToKill (), item->hasArmyTypeToKill ()));
                  if (type_id >= 0)
                    item->setArmyTypeToKill ((guint32) type_id);
                  else
                    item->clearArmyTypeToKill ();
                }
            }));

        add_connection
          (m_summon_army_type_label->signal_army_selected ().connect
           ([this] (int type_id)
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_SelectSummonArmy
                     (m_selection_model->get_selected (),
                      item->getArmyTypeToSummon (),
                      item->hasArmyTypeToSummon ()));
                  if (type_id >= 0)
                    item->setArmyTypeToSummon ((guint32) type_id);
                  else
                    item->clearArmyTypeToSummon ();
                }
            }));

        add_connection
          (m_defender_army_type_label->signal_army_selected ().connect
           ([this] (int type_id)
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_SelectRaiseArmy
                     (m_selection_model->get_selected (),
                      item->getArmyTypeToRaise (),
                      item->hasArmyTypeToRaise ()));

                  if (type_id >= 0)
                    item->setArmyTypeToRaise ((guint32) type_id);
                  else
                    item->clearArmyTypeToRaise ();
                }
            }));

        add_connection
          (m_summon_monster_combo->signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_BuildingTypeToSummonOn
                     (m_selection_model->get_selected  (),
                      item->getBuildingTypeToSummonOn ()));

                  item->setBuildingTypeToSummonOn
                    (m_summon_monster_combo->get_selected ());
                }
            }));

        add_connection
          (m_disease_city_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_DiseaseCity
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::DISEASE_CITY)));

                  m_disease_army_percent_spinbutton->set_sensitive
                    (m_disease_city_switch->get_active ());
                  if (m_disease_city_switch->get_active ())
                    item->addBonus (ItemProto::DISEASE_CITY);
                  else
                    item->removeBonus (ItemProto::DISEASE_CITY);
                }
            }));

        add_connection
          (m_disease_army_percent_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_DiseaseArmies
                     (m_selection_model->get_selected (),
                      item->getPercentArmiesToKill ()));

                  item->setPercentArmiesToKill
                    (m_disease_army_percent_spinbutton->get_value_as_int ());
                }
            }));

        add_connection
          (m_raise_defenders_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_defender_army_type_button->set_sensitive
                (m_raise_defenders_switch->get_active ());
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_RaiseDefenders
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::RAISE_DEFENDERS)));

                  m_num_defenders_spinbutton->set_sensitive
                    (m_raise_defenders_switch->get_active ());
                  if (m_raise_defenders_switch->get_active ())
                    item->addBonus (ItemProto::RAISE_DEFENDERS);
                  else
                    item->removeBonus (ItemProto::RAISE_DEFENDERS);
                }
            }));

        add_connection
          (m_num_defenders_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_NumDefenders
                     (m_selection_model->get_selected (),
                      item->getNumberOfArmiesToRaise ()));

                  item->setNumberOfArmiesToRaise
                    (m_num_defenders_spinbutton->get_value_as_int ());
                }
            }));

        add_connection
          (m_persuade_neutral_city_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_PersuadeNeutralCity
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::PERSUADE_NEUTRALS)));

                  if (m_persuade_neutral_city_switch->get_active ())
                    item->addBonus (ItemProto::PERSUADE_NEUTRALS);
                  else
                    item->removeBonus (ItemProto::PERSUADE_NEUTRALS);
                }
            }));

        add_connection
          (m_teleport_to_city_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_TeleportToCity
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::TELEPORT_TO_CITY)));

                  if (m_teleport_to_city_switch->get_active ())
                    item->addBonus (ItemProto::TELEPORT_TO_CITY);
                  else
                    item->removeBonus (ItemProto::TELEPORT_TO_CITY);
                }
            }));

        add_connection
          (m_add1str_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_AddStr
                     (m_selection_model->get_selected (), 1,
                      item->getBonus (ItemProto::ADD1STR)));

                  if (m_add1str_switch->get_active ())
                    item->addBonus (ItemProto::ADD1STR);
                  else
                    item->removeBonus (ItemProto::ADD1STR);
                }
            }));

        add_connection
          (m_add2str_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_AddStr
                     (m_selection_model->get_selected (), 2,
                      item->getBonus (ItemProto::ADD2STR)));

                  if (m_add2str_switch->get_active ())
                    item->addBonus (ItemProto::ADD2STR);
                  else
                    item->removeBonus (ItemProto::ADD2STR);
                }
            }));

        add_connection
          (m_add3str_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_AddStr
                     (m_selection_model->get_selected (), 3,
                      item->getBonus (ItemProto::ADD3STR)));

                  if (m_add3str_switch->get_active ())
                    item->addBonus (ItemProto::ADD3STR);
                  else
                    item->removeBonus (ItemProto::ADD3STR);
                }
            }));

        add_connection
          (m_add1stack_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_AddStack
                     (m_selection_model->get_selected (), 1,
                      item->getBonus (ItemProto::ADD1STACK)));

                  if (m_add1stack_switch->get_active ())
                    item->addBonus (ItemProto::ADD1STACK);
                  else
                    item->removeBonus (ItemProto::ADD1STACK);
                }
            }));

        add_connection
          (m_add2stack_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_AddStack
                     (m_selection_model->get_selected (), 2,
                      item->getBonus (ItemProto::ADD2STACK)));

                  if (m_add2stack_switch->get_active ())
                    item->addBonus (ItemProto::ADD2STACK);
                  else
                    item->removeBonus (ItemProto::ADD2STACK);
                }
            }));

        add_connection
          (m_add3stack_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_AddStack
                     (m_selection_model->get_selected (), 3,
                      item->getBonus (ItemProto::ADD3STACK)));

                  if (m_add3stack_switch->get_active ())
                    item->addBonus (ItemProto::ADD3STACK);
                  else
                    item->removeBonus (ItemProto::ADD3STACK);
                }
            }));

        add_connection
          (m_flystack_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_FlyStack
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::FLYSTACK)));

                  if (m_flystack_switch->get_active ())
                    item->addBonus (ItemProto::FLYSTACK);
                  else
                    item->removeBonus (ItemProto::FLYSTACK);
                }
            }));

        add_connection
          (m_doublemovestack_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_DoubleMovementStack
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::DOUBLEMOVESTACK)));

                  if (m_doublemovestack_switch->get_active ())
                    item->addBonus (ItemProto::DOUBLEMOVESTACK);
                  else
                    item->removeBonus (ItemProto::DOUBLEMOVESTACK);
                }
            }));

        add_connection
          (m_add2goldpercity_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_AddGoldPerCity
                     (m_selection_model->get_selected (), 2,
                      item->getBonus (ItemProto::ADD2GOLDPERCITY)));

                  if (m_add2goldpercity_switch->get_active ())
                    item->addBonus (ItemProto::ADD2GOLDPERCITY);
                  else
                    item->removeBonus (ItemProto::ADD2GOLDPERCITY);
                }
            }));

        add_connection
          (m_add3goldpercity_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_AddGoldPerCity
                     (m_selection_model->get_selected (), 3,
                      item->getBonus (ItemProto::ADD3GOLDPERCITY)));

                  if (m_add3goldpercity_switch->get_active ())
                    item->addBonus (ItemProto::ADD3GOLDPERCITY);
                  else
                    item->removeBonus (ItemProto::ADD3GOLDPERCITY);
                }
            }));

        add_connection
          (m_add4goldpercity_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_AddGoldPerCity
                     (m_selection_model->get_selected (), 4,
                      item->getBonus (ItemProto::ADD4GOLDPERCITY)));

                  if (m_add4goldpercity_switch->get_active ())
                    item->addBonus (ItemProto::ADD4GOLDPERCITY);
                  else
                    item->removeBonus (ItemProto::ADD4GOLDPERCITY);
                }
            }));

        add_connection
          (m_add5goldpercity_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_AddGoldPerCity
                     (m_selection_model->get_selected (), 5,
                      item->getBonus (ItemProto::ADD5GOLDPERCITY)));

                  if (m_add5goldpercity_switch->get_active ())
                    item->addBonus (ItemProto::ADD5GOLDPERCITY);
                  else
                    item->removeBonus (ItemProto::ADD5GOLDPERCITY);
                }
            }));

        add_connection
          (m_steals_gold_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_steal_percent_spinbutton->set_sensitive
                (m_steals_gold_switch->get_active ());
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_StealsGold
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::STEAL_GOLD)));

                  if (m_steals_gold_switch->get_active ())
                    item->addBonus (ItemProto::STEAL_GOLD);
                  else
                    item->removeBonus (ItemProto::STEAL_GOLD);
                }
            }));

        add_connection
          (m_pick_up_bags_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_PickUpBags
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::PICK_UP_BAGS)));

                  if (m_pick_up_bags_switch->get_active ())
                    item->addBonus (ItemProto::PICK_UP_BAGS);
                  else
                    item->removeBonus (ItemProto::PICK_UP_BAGS);
                }
            }));

        add_connection
          (m_add_mp_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_add_mp_spinbutton->set_sensitive
                (m_add_mp_switch->get_active ());
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_AddMovement
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::ADD_2MP_STACK)));

                  if (m_add_mp_switch->get_active ())
                    item->addBonus (ItemProto::ADD_2MP_STACK);
                  else
                    item->removeBonus (ItemProto::ADD_2MP_STACK);
                }
            }));

        add_connection
          (m_sinks_ships_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_SinkShips
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::SINK_SHIPS)));

                  if (m_sinks_ships_switch->get_active ())
                    item->addBonus (ItemProto::SINK_SHIPS);
                  else
                    item->removeBonus (ItemProto::SINK_SHIPS);
                }
            }));

        add_connection
          (m_banish_worms_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_kill_army_type_button->set_sensitive
                (m_banish_worms_switch->get_active ());
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_BanishWorms
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::BANISH_WORMS)));

                  if (m_banish_worms_switch->get_active ())
                    item->addBonus (ItemProto::BANISH_WORMS);
                  else
                    item->removeBonus (ItemProto::BANISH_WORMS);
                }
            }));

        add_connection
          (m_burn_bridge_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_BurnBridge
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::BURN_BRIDGE)));

                  if (m_burn_bridge_switch->get_active ())
                    item->addBonus (ItemProto::BURN_BRIDGE);
                  else
                    item->removeBonus (ItemProto::SINK_SHIPS);
                }
            }));

        add_connection
          (m_capture_keeper_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_CaptureKeeper
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::CAPTURE_KEEPER)));

                  if (m_capture_keeper_switch->get_active ())
                    item->addBonus (ItemProto::CAPTURE_KEEPER);
                  else
                    item->removeBonus (ItemProto::CAPTURE_KEEPER);
                }
            }));

        add_connection
          (m_summon_monster_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_summon_army_type_button->set_sensitive
                (m_summon_monster_switch->get_active ());
              m_summon_monster_combo->set_sensitive
                (m_summon_monster_switch->get_active ());
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_SummonMonster
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::SUMMON_MONSTER)));

                  if (m_summon_monster_switch->get_active ())
                    item->addBonus (ItemProto::SUMMON_MONSTER);
                  else
                    item->removeBonus (ItemProto::SUMMON_MONSTER);
                }
            }));

        add_connection
          (m_uses_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_Uses
                     (m_selection_model->get_selected (),
                      item->getNumberOfUsesLeft ()));

                  item->setNumberOfUsesLeft
                    (m_uses_spinbutton->get_value_as_int ());
                }
            }));

        add_connection
          (m_steal_percent_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_StealPercent
                     (m_selection_model->get_selected (),
                      item->getPercentGoldToSteal ()));

                  item->setPercentGoldToSteal
                    (m_steal_percent_spinbutton->get_value_as_int ());
                }
            }));

        add_connection
          (m_add_mp_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_AddMp
                     (m_selection_model->get_selected (),
                      item->getMovementPointsToAdd ()));

                  item->setMovementPointsToAdd
                    (m_add_mp_spinbutton->get_value_as_int ());
                }
            }));

        add_connection
          (m_plantable_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              auto item = get_selected_item ();
              if (item)
                {
                  m_umgr->add
                    (new ItemListUndoAction_Plantable
                     (m_selection_model->get_selected (),
                      item->getBonus (ItemProto::PLANT_TO_VECTOR)));

                  if (m_plantable_switch->get_active ())
                    item->addBonus (ItemProto::PLANT_TO_VECTOR);
                  else
                    item->removeBonus (ItemProto::PLANT_TO_VECTOR);
                }
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
        update_item_panel ();
        update_buttons ();
      }

    void update_buttons ()
      {
        guint n = m_store->get_n_items ();
        m_remove_button->set_sensitive (n > 0);
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    UndoAction *execute_action (UndoAction *action2)
      {
        ItemListUndoAction *action = dynamic_cast<ItemListUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case ItemListUndoAction::ADD:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_Add*>(action);
                out = new ItemListUndoAction_Add
                  (Itemlist::instance ()->copy (), m_itemlist->size ());
                Itemlist::reset (a->get_item_list ()->copy ());
                m_itemlist = Itemlist::instance();
                fill_treeview ();
              }
            break;

          case ItemListUndoAction::REMOVE:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_Remove*>(action);
                out = new ItemListUndoAction_Remove
                  (Itemlist::instance ()->copy (), a->get_index ());
                Itemlist::reset (a->get_item_list ()->copy ());
                m_itemlist = Itemlist::instance();
                fill_treeview ();
              }
            break;

          case ItemListUndoAction::BUILDING_TYPE_TO_SUMMON_ON:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_BuildingTypeToSummonOn*>(action);
                int row = 
                  m_summon_monster_combo->get_active_row_number ();
                out = new ItemListUndoAction_BuildingTypeToSummonOn
                  (a->get_index (), row);
                get_item_by_index (a)->setBuildingTypeToSummonOn (a->get_row ());
              } 
            break;

          case ItemListUndoAction::DISEASE_CITY:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_DiseaseCity*>(action);
                out = new ItemListUndoAction_DiseaseCity
                  (a->get_index (), m_disease_city_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::DISEASE_CITY);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::DISEASE_CITY);
              }
            break;

          case ItemListUndoAction::DISEASE_ARMIES_PERCENT:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_DiseaseArmies*>(action);
                out = new ItemListUndoAction_DiseaseArmies
                  (a->get_index (),
                   m_disease_army_percent_spinbutton->get_value ());
                get_item_by_index (a)->setPercentArmiesToKill (a->get_num ());
              }
            break;

          case ItemListUndoAction::RAISE_DEFENDERS:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_RaiseDefenders*>(action);
                out = new ItemListUndoAction_RaiseDefenders
                  (a->get_index (), m_raise_defenders_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::RAISE_DEFENDERS);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::RAISE_DEFENDERS);
              }
            break;

          case ItemListUndoAction::NUM_DEFENDERS:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_NumDefenders*>(action);
                out = new ItemListUndoAction_NumDefenders
                  (a->get_index (), m_num_defenders_spinbutton->get_value ());
                get_item_by_index (a)->setNumberOfArmiesToRaise(a->get_num ());
              }
            break;

          case ItemListUndoAction::PERSUADE_NEUTRAL_CITY:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_PersuadeNeutralCity*>(action);
                out = new ItemListUndoAction_PersuadeNeutralCity
                  (a->get_index (), m_persuade_neutral_city_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::PERSUADE_NEUTRALS);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::PERSUADE_NEUTRALS);
              }
            break;

          case ItemListUndoAction::TELEPORT_TO_CITY:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_TeleportToCity*>(action);
                out = new ItemListUndoAction_TeleportToCity
                  (a->get_index (), m_teleport_to_city_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::TELEPORT_TO_CITY);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::TELEPORT_TO_CITY);
              }
            break;

          case ItemListUndoAction::ADDSTR:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_AddStr*>(action);
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
                out = new ItemListUndoAction_AddStr (a->get_index (), a->get_num (),
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
                  get_item_by_index (a)->addBonus (b);
                else
                  get_item_by_index (a)->removeBonus (b);
              }
            break;

          case ItemListUndoAction::ADDSTACK:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_AddStack*>(action);
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
                out = new ItemListUndoAction_AddStack (a->get_index (),
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
                  get_item_by_index (a)->addBonus (b);
                else
                  get_item_by_index (a)->removeBonus (b);
              }
            break;

          case ItemListUndoAction::FLY_STACK:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_FlyStack*>(action);
                out = new ItemListUndoAction_FlyStack
                  (a->get_index (), m_flystack_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::FLYSTACK);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::FLYSTACK);
              }
            break;

          case ItemListUndoAction::DOUBLE_MOVEMENT_STACK:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_DoubleMovementStack*>(action);
                out = new ItemListUndoAction_DoubleMovementStack
                  (a->get_index (), m_doublemovestack_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::DOUBLEMOVESTACK);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::DOUBLEMOVESTACK);
              }
            break;

          case ItemListUndoAction::ADD_GOLD_PER_CITY:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_AddGoldPerCity*>(action);
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
                out = new ItemListUndoAction_AddGoldPerCity (a->get_index (),
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
                  get_item_by_index (a)->addBonus (b);
                else
                  get_item_by_index (a)->removeBonus (b);
              }
            break;

          case ItemListUndoAction::STEALS_GOLD:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_StealsGold*>(action);
                out = new ItemListUndoAction_StealsGold
                  (a->get_index (), m_steals_gold_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::STEAL_GOLD);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::STEAL_GOLD);
              }
            break;

          case ItemListUndoAction::PICK_UP_BAGS:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_PickUpBags*>(action);
                out = new ItemListUndoAction_PickUpBags
                  (a->get_index (), m_pick_up_bags_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::PICK_UP_BAGS);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::PICK_UP_BAGS);
              }
            break;

          case ItemListUndoAction::ADD_MOVEMENT:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_AddMovement*>(action);
                out = new ItemListUndoAction_AddMovement
                  (a->get_index (), m_add_mp_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::ADD_2MP_STACK);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::ADD_2MP_STACK);
              }
            break;

          case ItemListUndoAction::SINKS_SHIPS:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_SinkShips*>(action);
                out = new ItemListUndoAction_SinkShips
                  (a->get_index (), m_sinks_ships_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::SINK_SHIPS);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::SINK_SHIPS);
              }
            break;

          case ItemListUndoAction::BANISH_WORMS:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_BanishWorms*>(action);
                out = new ItemListUndoAction_BanishWorms
                  (a->get_index (), m_banish_worms_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::BANISH_WORMS);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::BANISH_WORMS);
              }
            break;

          case ItemListUndoAction::BURN_BRIDGE:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_BurnBridge*>(action);
                out = new ItemListUndoAction_BurnBridge
                  (a->get_index (), m_burn_bridge_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::BURN_BRIDGE);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::BURN_BRIDGE);
              }
            break;

          case ItemListUndoAction::CAPTURE_KEEPER:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_CaptureKeeper*>(action);
                out = new ItemListUndoAction_CaptureKeeper
                  (a->get_index (), m_capture_keeper_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::CAPTURE_KEEPER);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::CAPTURE_KEEPER);
              }
            break;

          case ItemListUndoAction::SUMMON_MONSTER:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_SummonMonster*>(action);
                out = new ItemListUndoAction_SummonMonster
                  (a->get_index (), m_summon_monster_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::SUMMON_MONSTER);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::SUMMON_MONSTER);
              }
            break;

          case ItemListUndoAction::USES:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_Uses*>(action);
                out = new ItemListUndoAction_Uses
                  (a->get_index (), m_uses_spinbutton->get_value ());
                get_item_by_index (a)->setNumberOfUsesLeft(a->get_num ());
              }
            break;

          case ItemListUndoAction::STEAL_PERCENT:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_StealPercent*>(action);
                out = new ItemListUndoAction_StealPercent
                  (a->get_index (), m_steal_percent_spinbutton->get_value ());
                get_item_by_index (a)->setPercentGoldToSteal(a->get_num ());
              }
            break;

          case ItemListUndoAction::ADD_MP:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_AddMp*>(action);
                out = new ItemListUndoAction_AddMp
                  (a->get_index (), m_add_mp_spinbutton->get_value ());
                get_item_by_index (a)->setMovementPointsToAdd(a->get_num ());
              }
            break;

          case ItemListUndoAction::PLANTABLE:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_Plantable*>(action);
                out = new ItemListUndoAction_Plantable
                  (a->get_index (), m_plantable_switch->get_active ());
                if (a->get_active ())
                  get_item_by_index (a)->addBonus (ItemProto::PLANT_TO_VECTOR);
                else
                  get_item_by_index (a)->removeBonus (ItemProto::PLANT_TO_VECTOR);
              }
            break;

          case ItemListUndoAction::NAME:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_Name*>(action);
                out = new ItemListUndoAction_Name
                  (a->get_index (), get_item_by_index (a)->getName (), m_umgr,
                   m_name_entry);

                get_item_by_index (a)->setName (a->get_name ());

                auto item = m_store->get_item (a->get_index ());
                if (item)
                  {
                    auto row = std::dynamic_pointer_cast<ItemProtoRow>(item);
                    row->changed ();
                  }
              }
            break;

          case ItemListUndoAction::SELECT_BANISH_ARMY:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_SelectBanishArmy*>(action);
                out = new ItemListUndoAction_SelectBanishArmy
                  (a->get_index (), get_item_by_index (a)->getArmyTypeToKill (),
                   get_item_by_index (a)->hasArmyTypeToKill ());
                if (a->get_present () == false)
                  get_item_by_index (a)->clearArmyTypeToKill ();
                else
                  get_item_by_index (a)->setArmyTypeToKill (a->get_army_type ());
                m_kill_army_type_label->set_army_type (a->get_army_type ());
              }
            break;

          case ItemListUndoAction::SELECT_SUMMON_ARMY:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_SelectSummonArmy*>(action);
                out = new ItemListUndoAction_SelectSummonArmy
                  (a->get_index (), get_item_by_index (a)->getArmyTypeToSummon (),
                   get_item_by_index (a)->hasArmyTypeToSummon ());
                if (a->get_present () == false)
                  get_item_by_index (a)->clearArmyTypeToSummon ();
                else
                  get_item_by_index (a)->setArmyTypeToSummon (a->get_army_type ());
                m_summon_army_type_label->set_army_type (a->get_army_type ());
              }
            break;

          case ItemListUndoAction::SELECT_RAISE_ARMY:
              {
                auto a =
                  dynamic_cast<ItemListUndoAction_SelectRaiseArmy*>(action);
                out = new ItemListUndoAction_SelectRaiseArmy
                  (a->get_index (), get_item_by_index (a)->getArmyTypeToRaise (),
                   get_item_by_index (a)->hasArmyTypeToRaise ());
                if (a->get_present () == false)
                  get_item_by_index (a)->clearArmyTypeToRaise ();
                else
                  get_item_by_index (a)->setArmyTypeToRaise (a->get_army_type ());
                m_defender_army_type_label->set_army_type (a->get_army_type ());
              }
            break;
          }
        return out;
      }

    ItemProto *get_item_by_index (ItemListUndoAction_Index *action)
      {
        auto item = m_store->get_item (action->get_index ());
        auto row = std::dynamic_pointer_cast<ItemProtoRow>(item);
        if (row)
          return row->m_item;
        else
          return NULL;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &ItemListDialog::execute_action));

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
      }

    void fill_treeview ()
      {
        m_store->remove_all ();
        for (auto item : *m_itemlist)
          m_store->append (ItemProtoRow::create (item.second));
      }

    ItemProto *get_selected_item ()
      {
        auto single_selection =
          std::dynamic_pointer_cast<Gtk::SingleSelection> (m_selection_model);

        auto item = single_selection->get_selected_item ();

        if (item)
          {
            auto row = std::dynamic_pointer_cast<ItemProtoRow> (item);
            return row->m_item;
          }
        return NULL;
      }

};
#endif
