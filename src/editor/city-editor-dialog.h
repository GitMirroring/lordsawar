//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015, 2020, 2021,
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
#ifndef CITY_EDITOR_DIALOG_H
#define CITY_EDITOR_DIALOG_H
#include "city.h"
#include "city-undo-actions.h"
#include "lw-column.h"
#include "select-army-dialog.h"

class CityArmyRow: public Glib::Object
{
public:

    ArmyProdBase *m_army;

    static Glib::RefPtr<CityArmyRow> create (ArmyProdBase *a)
      {
        return
          Glib::make_refptr_for_instance<CityArmyRow> (new CityArmyRow (a));
      }

    sigc::signal<void()> signal_changed ()
      {
        return m_signal_changed;
      }

    void changed ()
      {
        m_signal_changed.emit ();
      }

protected:
    CityArmyRow (ArmyProdBase *a)
      : m_army (a)
      {
      }

    sigc::signal<void()> m_signal_changed;
};

class CityEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "city-editor.ui";
      }

    CityEditorDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_add_button = load <Gtk::Button> ("add_button");
        m_remove_button = load <Gtk::Button> ("remove_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
        m_owner_combobox = load <Gtk::Box> ("owner_combobox");
        m_name_entry = load <Gtk::Entry> ("name_entry");
        m_description_entry = load <Gtk::Entry> ("description_entry");
        m_capital_switch = load <Gtk::Switch> ("capital_switch");
        m_burned_switch = load <Gtk::Switch> ("burned_switch");
        m_income_spinbutton = load <Gtk::SpinButton> ("income_spinbutton");
        m_production_switch = load <Gtk::Switch> ("production_switch");
        m_random_button = load <Gtk::Button> ("random_button");
        m_status_label = load <Gtk::Label> ("status_label");
        m_scrolled_window = load <Gtk::ScrolledWindow> ("scrolled_window");
        m_up_button = load <Gtk::Button> ("up_button");
        m_down_button = load <Gtk::Button> ("down_button");
      }

    ~CityEditorDialog ()
      {
        disconnect_signals ();
        m_strength_conn.disconnect ();
        m_moves_conn.disconnect ();
        m_upkeep_conn.disconnect ();
        m_turns_conn.disconnect ();
        delete m_umgr;
        delete m_city;
      }

    void setup (City *c, CreateScenarioRandomize *r)
      {
        m_city = new City (*c);
        m_randomize = r;

        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        fill_owner_combobox ();

        m_store = Gio::ListStore<CityArmyRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        m_selection_model->signal_selection_changed ().connect
          ([this] (const guint &, const guint &)
           {
             update ();
           });

        fill_treeview ();
        setup_columns ();
        setup_undo ();
        update ();
      }

    City *get_city ()
      {
        return m_city;
      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }

private:
    Gtk::Button *m_close_button;
    Gtk::Box *m_owner_combobox;
    LwCombo *m_owner_combo;
    Gtk::Entry *m_name_entry;
    Gtk::Entry *m_description_entry;
    Gtk::ColumnView *m_treeview;
    Gtk::Button *m_add_button;
    Gtk::Button *m_remove_button;
    Gtk::Switch *m_burned_switch;
    Gtk::Switch *m_capital_switch;
    Gtk::SpinButton *m_income_spinbutton;
    Gtk::Switch *m_production_switch;
    Gtk::Button *m_random_button;
    Gtk::Label *m_status_label;
    Gtk::ScrolledWindow *m_scrolled_window;
    Gtk::Button *m_up_button;
    Gtk::Button *m_down_button;

    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<CityArmyRow>> m_store;

    std::list<sigc::connection> m_connections;
    sigc::connection m_strength_conn;
    sigc::connection m_moves_conn;
    sigc::connection m_upkeep_conn;
    sigc::connection m_turns_conn;

    UndoMgr *m_umgr;
    City *m_city;
    CreateScenarioRandomize *m_randomize;

    void setup_name_column ()
      {
        LwColumn::setup_text_column<CityArmyRow>
          (m_treeview, "name_label", true, Gtk::Justification::LEFT, _("Name"),
           [] (const auto& row)
           {
             return row->m_army->getName ();
           });
      }

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void connect_signals ()
      {
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

              m_umgr->add (new CityUndoAction_Order (m_city));
              m_store->remove (i);
              m_store->insert (i - 1, item);

              reselection (item);

              sync_treeview_to_production_slots ();
              fill_treeview ();
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

              m_umgr->add (new CityUndoAction_Order (m_city));
              m_store->remove (i);
              m_store->insert (i + 1, item);

              reselection (item);

              sync_treeview_to_production_slots ();
              fill_treeview ();
            }));

        add_connection
          (m_owner_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new CityUndoAction_Owner (m_city));
              m_city->setOwner (get_selected_player ());
              fill_treeview ();
            }));

        add_connection
          (m_add_button->signal_clicked ().connect
           ([this] ()
            {
              auto d = LwDialog::build<SelectArmyDialog> (this);
              auto p = m_city->getOwner ();
              d->setup (p->get_shield (),
                        SelectArmyDialog::SELECT_NORMAL, -1);
              d->signal_response ().connect
                ([this, d, p] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                         {
                           m_umgr->add (new CityUndoAction_Add (m_city));

                           auto proto =
                             Armysetlist::instance ()->getArmy
                             (p->getArmyset (), d->get_selected_army ());

                           auto army = new ArmyProdBase (*proto);
                           m_store->append (CityArmyRow::create (army));
        
                           m_city->addProductionBase
                             (m_store->get_n_items () - 1, army);

                           scroll_to_bottom ();
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
          (m_remove_button->signal_clicked ().connect
           ([this] ()
            {
              m_umgr->add (new CityUndoAction_Remove (m_city));
              m_city->removeProductionBase (m_selection_model->get_selected ());
              fill_treeview ();
              update ();
            }));

        add_connection
          (m_name_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new CityUndoAction_Name (m_city, m_umgr, m_name_entry));
              m_city->setName (m_name_entry->get_text ());
            }));

        add_connection
          (m_description_entry->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new CityUndoAction_Description (m_city, m_umgr,
                                                 m_description_entry));
              m_city->setDescription (m_description_entry->get_text ());
            }));

        add_connection
          (m_burned_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new CityUndoAction_Razed (m_city));
              m_city->setBurnt (m_burned_switch->get_active ());
            }));

        add_connection
          (m_capital_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new CityUndoAction_Capital (m_city));
              Player *p = get_selected_player ();
              // turn off an existing capital and then set it to this one
              for (auto c : *Citylist::instance ())
                {
                  if (c->isCapital () && c->getOwner () == p)
                    {
                      c->setCapital (false);
                      c->setCapitalOwner (NULL);
                    }
                }
              if (m_capital_switch->get_active ())
                {
                  m_city->setCapital (true);
                  m_city->setCapitalOwner (p);
                }
              else
                {
                  m_city->setCapital (false);
                  m_city->setCapitalOwner (NULL);
                }
            }));

        add_connection
          (m_production_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new CityUndoAction_NewProd (m_city));
              m_city->setBuildProduction (m_production_switch->get_active ());
            }));

        add_connection
          (m_income_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add (new CityUndoAction_Income (m_city));
              m_city->setGold (m_income_spinbutton->get_value_as_int ());
            }));

        add_connection
          (m_random_button->signal_clicked ().connect
           ([this] ()
            {
              m_umgr->add (new CityUndoAction_Randomize (m_city));
              guint32 c = 0;
              for (; c < m_city->getMaxNoOfProductionBases (); ++c)
                m_city->removeProductionBase (c);
              m_city->setRandomArmytypes (true, 1);
              fill_treeview ();

              m_income_spinbutton->set_value
                (m_randomize->getRandomCityIncome
                 (m_capital_switch->get_active ()));

              Glib::ustring existing_name = m_name_entry->get_text ();
              if (existing_name == City::getDefaultName ())
                m_name_entry->set_text (m_randomize->popRandomCityName ());
              else
                {
                  m_name_entry->set_text (m_randomize->popRandomCityName ());
                  m_randomize->pushRandomCityName (existing_name);
                }
              update ();
            }));
      }

    void scroll_to_bottom ()
      {
        Glib::signal_idle ().connect_once
          ([this] ()
           {
             auto vadj = m_scrolled_window->get_vadjustment ();
             if (vadj)
               vadj->set_value (vadj->get_upper () - vadj->get_page_size ());
           });
      }

    void update ()
      {
        disconnect_signals ();

        m_name_entry->set_text (m_city->getName ());
        m_description_entry->set_text (m_city->getDescription ());
        m_burned_switch->set_active (m_city->isBurnt ());
        m_capital_switch->set_active
          (m_city->getCapitalOwner () == m_city->getOwner ());
        m_income_spinbutton->set_value (m_city->getGold ());
        m_production_switch->set_active (m_city->getBuildProduction ());
        int i = 0;
        for (auto pl : *Playerlist::instance ())
          {
            if (pl == m_city->getOwner ())
              break;
            i++;
          }
        m_owner_combo->set_active (i);

        int n = m_store->get_n_items ();
        Glib::ustring s = "";
        if (n == 0)
          s = _("There are none.");
        else
          s = String::ucompose
            (ngettext ("There is %1 production unit.",
                       "There are %1 production units.",
                            n), n);
        m_status_label->set_text (s);
        m_treeview->set_visible (n > 0);

        connect_signals ();
        update_buttons ();
      }

    void update_buttons ()
      {
        guint n = m_store->get_n_items ();
        m_remove_button->set_sensitive (n > 0);
        m_add_button->set_sensitive (n < MAX_PRODUCTION_SLOTS_IN_A_CITY);
        int i = m_selection_model->get_selected ();
        m_up_button->set_sensitive (i > 0);
        m_down_button->set_sensitive (i >= 0 && (guint32) i < n - 1);
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    UndoAction *execute_action (UndoAction *action2)
      {
        auto action = dynamic_cast<CityUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case CityUndoAction::OWNER:
              {
                auto a = dynamic_cast<CityUndoAction_Owner*>(action);
                out = new CityUndoAction_Owner (m_city);
                m_city->setOwner (a->get_city ()->getOwner ());
              }
            break;

          case CityUndoAction::CAPITAL:
              {
                auto a = dynamic_cast<CityUndoAction_Capital*>(action);
                out = new CityUndoAction_Capital (m_city);
                m_city->setCapital (a->get_city ()->isCapital ());
                m_city->setCapitalOwner (a->get_city ()->getCapitalOwner ());
              }
            break;

          case CityUndoAction::RAZED:
              {
                auto a = dynamic_cast<CityUndoAction_Razed*>(action);
                out = new CityUndoAction_Razed (m_city);
                m_city->setBurnt (a->get_city ()->isBurnt ());
                replace_prod_bases (a);
                fill_treeview ();
              }
            break;

          case CityUndoAction::NAME:
              {
                auto a = dynamic_cast<CityUndoAction_Name*>(action);
                out = new CityUndoAction_Name (m_city, m_umgr, m_name_entry);
                m_city->setName (a->get_city ()->getName ());
              }
            break;

          case CityUndoAction::INCOME:
              {
                auto a = dynamic_cast<CityUndoAction_Income*>(action);
                out = new CityUndoAction_Income (m_city);
                m_city->setGold (a->get_city ()->getGold ());
              }
            break;

          case CityUndoAction::NEWPROD:
              {
                auto a = dynamic_cast<CityUndoAction_NewProd*>(action);
                out = new CityUndoAction_NewProd (m_city);
                m_city->setBuildProduction
                  (a->get_city ()->getBuildProduction ());
              }
            break;

          case CityUndoAction::ADD:
              {
                auto a = dynamic_cast<CityUndoAction_Add*>(action);
                out = new CityUndoAction_Add (m_city);
                replace_prod_bases (a);
                fill_treeview ();
              }
            break;

          case CityUndoAction::REMOVE:
              {
                auto a = dynamic_cast<CityUndoAction_Remove*>(action);
                out = new CityUndoAction_Remove (m_city);
                replace_prod_bases (a);
                fill_treeview ();
              }
            break;

          case CityUndoAction::RANDOMIZE:
              {
                auto a = dynamic_cast<CityUndoAction_Randomize*>(action);
                out = new CityUndoAction_Randomize (m_city);
                replace_prod_bases (a);
                fill_treeview ();
              }
            break;

          case CityUndoAction::DESCRIPTION:
              {
                auto a = dynamic_cast<CityUndoAction_Description*>(action);
                out = new CityUndoAction_Description (m_city, m_umgr,
                                                      m_description_entry);
                m_city->setDescription (a->get_city ()->getDescription ());
              }
            break;

          case CityUndoAction::STRENGTH:
              {
                auto a = dynamic_cast<CityUndoAction_Strength*>(action);
                auto army = get_army_by_index (a);
                out = new CityUndoAction_Strength (a->get_index (),
                                                   army->getStrength ());
                army->setStrength (a->get_strength ());

                auto item = m_store->get_item (a->get_index ());
                auto row = std::dynamic_pointer_cast<CityArmyRow>(item);
                if (row)
                  row->changed ();
              }
            break;

          case CityUndoAction::TURNS:
              {
                auto a = dynamic_cast<CityUndoAction_Turns*>(action);
                auto army = get_army_by_index (a);
                out = new CityUndoAction_Turns (a->get_index (),
                                                army->getProduction ());
                army->setProduction (a->get_turns ());

                auto item = m_store->get_item (a->get_index ());
                auto row = std::dynamic_pointer_cast<CityArmyRow>(item);
                if (row)
                  row->changed ();
              }
            break;

          case CityUndoAction::MOVES:
              {
                auto a = dynamic_cast<CityUndoAction_Moves*>(action);
                auto army = get_army_by_index (a);
                out = new CityUndoAction_Moves (a->get_index (),
                                                army->getMaxMoves ());
                army->setMaxMoves (a->get_moves ());

                auto item = m_store->get_item (a->get_index ());
                auto row = std::dynamic_pointer_cast<CityArmyRow>(item);
                if (row)
                  row->changed ();
              }
            break;

          case CityUndoAction::UPKEEP:
              {
                auto a = dynamic_cast<CityUndoAction_Upkeep*>(action);
                auto army = get_army_by_index (a);
                out = new CityUndoAction_Upkeep (a->get_index (),
                                                 army->getUpkeep ());
                army->setUpkeep (a->get_upkeep ());

                auto item = m_store->get_item (a->get_index ());
                auto row = std::dynamic_pointer_cast<CityArmyRow>(item);
                if (row)
                  row->changed ();
              }
            break;

          case CityUndoAction::ORDER:
              {
                auto a = dynamic_cast<CityUndoAction_Order*>(action);
                out = new CityUndoAction_Order (m_city);
                replace_prod_bases (a);
                fill_treeview ();
              }
            break;
          }
        return out;
      }

    ArmyProdBase *get_army_by_index (CityUndoAction_Index *action)
      {
        auto item = m_store->get_item (action->get_index ());
        auto row = std::dynamic_pointer_cast<CityArmyRow>(item);
        if (row)
          return row->m_army;
        else
          return NULL;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &CityEditorDialog::execute_action));

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

    void fill_treeview ()
      {
        m_store->remove_all ();
        for (auto slot : *m_city)
          {
            if (slot->getArmyProdBase ())
              m_store->append (CityArmyRow::create (slot->getArmyProdBase ()));
          }
      }

    ArmyProdBase *get_selected_army ()
      {
        auto single_selection =
          std::dynamic_pointer_cast<Gtk::SingleSelection> (m_selection_model);

        auto item = single_selection->get_selected_item ();

        if (item)
          {
            auto row = std::dynamic_pointer_cast<CityArmyRow> (item);
            return row->m_army;
          }
        return NULL;
      }

    void fill_owner_combobox ()
      {
        m_owner_combo = Gtk::make_managed<LwCombo> ();
        int i = 0;
        int found = -1;
        for (auto p : *Playerlist::instance ())
          {
            m_owner_combo->append (p->getName ());
            if (p == m_city->getOwner ())
              found = i;
            i++;
          }
        if (found >= 0)
          m_owner_combo->set_active (found);
        m_owner_combobox->append (*m_owner_combo);
      }

    void setup_image_column ()
      {
        LwColumn::setup_picture_column<CityArmyRow>
          (m_treeview, LW_BUTTON_SIZE, LW_BUTTON_SIZE, "army_image", "",
           [this] (const auto& row)
           {
             auto p = m_city->getOwner ();
             auto pixmask =
               ImageCache::instance ()->getArmyPic
               (p->getArmyset (), row->m_army->getTypeId (), p->get_shield (),
                NULL, false);
             return pixmask->to_texture ();
           });
      }

    void setup_strength_column ()
      {
        int min = ArmyProto::min_strength;
        int max = ArmyProto::max_strength;
        LwColumn::setup_number_column<CityArmyRow>
          (m_treeview, "str_spin", m_strength_conn,
           Gtk::Adjustment::create (min, min, max, 1, 10, 0), _("Str"),
           [] (const auto& row)
           {
             return row->m_army->getStrength ();
           },
           [this] (const auto& row, guint32 value)
           {
             bool found = false;
             int irow = -1;
             for (guint i = 0; i < m_store->get_n_items (); ++i)
               {
                 auto item = m_store->get_item (i);
                 if (item == row)
                   {
                     found = true;
                     irow = i;
                     break;
                   }
               }

             if (found)
               {
                 m_umgr->add
                   (new CityUndoAction_Strength
                    (irow, row->m_army->getStrength ()));

                 row->m_army->setStrength (value);
               }
           });
      }

    void setup_moves_column ()
      {
        int min = ArmyProto::min_moves;
        int max = ArmyProto::max_moves;
        LwColumn::setup_number_column<CityArmyRow>
          (m_treeview, "moves_spin", m_moves_conn,
           Gtk::Adjustment::create (min, min, max, 1, 10, 0), _("Max Moves"),
           [] (const auto& row)
           {
             return row->m_army->getMaxMoves ();
           },
           [this] (const auto& row, guint32 value)
           {
             bool found = false;
             int irow = -1;
             for (guint i = 0; i < m_store->get_n_items (); ++i)
               {
                 auto item = m_store->get_item (i);
                 if (item == row)
                   {
                     found = true;
                     irow = i;
                     break;
                   }
               }

             if (found)
               {
                 m_umgr->add
                   (new CityUndoAction_Moves
                    (irow, row->m_army->getMaxMoves ()));

                 row->m_army->setMaxMoves (value);
               }
           });
      }

    void setup_upkeep_column ()
      {
        int min = ArmyProto::min_upkeep;
        int max = ArmyProto::max_upkeep;
        LwColumn::setup_number_column<CityArmyRow>
          (m_treeview, "upkeep_spin", m_upkeep_conn,
           Gtk::Adjustment::create (min, min, max, 1, 10, 0), _("Upkeep"),
           [] (const auto& row)
           {
             return row->m_army->getUpkeep ();
           },
           [this] (const auto& row, guint32 value)
           {
             bool found = false;
             int irow = -1;
             for (guint i = 0; i < m_store->get_n_items (); ++i)
               {
                 auto item = m_store->get_item (i);
                 if (item == row)
                   {
                     found = true;
                     irow = i;
                     break;
                   }
               }

             if (found)
               {
                 m_umgr->add
                   (new CityUndoAction_Upkeep
                    (irow, row->m_army->getUpkeep ()));

                 row->m_army->setUpkeep (value);
               }
           });
      }

    void setup_turns_column ()
      {
        int min = ArmyProto::min_production_turns;
        int max = ArmyProto::max_production_turns;
        LwColumn::setup_number_column<CityArmyRow>
          (m_treeview, "turns_spin", m_turns_conn,
           Gtk::Adjustment::create (min, min, max, 1, 10, 0), _("Turns"),
           [] (const auto& row)
           {
             return row->m_army->getProduction ();
           },
           [this] (const auto& row, guint32 value)
           {
             bool found = false;
             int irow = -1;
             for (guint i = 0; i < m_store->get_n_items (); ++i)
               {
                 auto item = m_store->get_item (i);
                 if (item == row)
                   {
                     found = true;
                     irow = i;
                     break;
                   }
               }

             if (found)
               {
                 m_umgr->add
                   (new CityUndoAction_Turns
                    (irow, row->m_army->getProduction ()));

                 row->m_army->setProduction (value);
               }
           });
      }

    void setup_columns ()
      {
        setup_image_column ();
        setup_strength_column ();
        setup_moves_column ();
        setup_upkeep_column ();
        setup_turns_column ();
        setup_name_column ();
      }

    Player *get_selected_player ()
      {
        int row = m_owner_combo->get_active_row_number ();
        auto it = Playerlist::instance ()->begin ();
        std::advance (it, row);
        return *it;
      }

    void replace_prod_bases (CityUndoAction_City *a)
      {
        guint32 c = 0;
        for (; c < m_city->getMaxNoOfProductionBases (); ++c)
          m_city->removeProductionBase (c);

        c = 0;
        for (; c < m_city->getMaxNoOfProductionBases (); ++c)
          {
            const ArmyProdBase *army = a->get_city ()->getProductionBase (c);
            if (army)
              m_city->addProductionBase (c, new ArmyProdBase (*army));
          }
      }

    void reselection (const Glib::RefPtr<CityArmyRow>& target)
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

    void sync_treeview_to_production_slots ()
      {
        std::vector<ArmyProdBase*> armies;

        for (guint i = 0; i < m_store->get_n_items (); ++i)
          {
            auto item = m_store->get_item (i);
            auto row = std::dynamic_pointer_cast<CityArmyRow>(item);
            armies.push_back (new ArmyProdBase (*row->m_army));
          }
        guint32 c = 0;
        for (; c < m_city->getMaxNoOfProductionBases (); ++c)
          m_city->removeProductionBase (c);

        c = 0;
        for (auto army : armies)
          {
            m_city->addProductionBase
              (c, army);
            c++;
          }
      }
};
#endif
