//  Copyright (C) 2017, 2020, 2021, 2026 Ben Asselstine
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
#ifndef BATTLE_CALCULATOR_DIALOG_H
#define BATTLE_CALCULATOR_DIALOG_H
#include "lw-dialog-base.h"
#include "lw-column.h"
#include "hero-editor-dialog.h"
#include "battle-calculator-undo-actions.h"
#include "select-army-dialog.h"

class CombatantRow: public Glib::Object
{
public:
    Army *m_army;

    static Glib::RefPtr<CombatantRow> create (Army *a)
      {
        return
          Glib::make_refptr_for_instance<CombatantRow> (new CombatantRow (a));
      }

    static Glib::RefPtr<CombatantRow> create (Hero *h)
      {
        return
          Glib::make_refptr_for_instance<CombatantRow>
          (new CombatantRow (dynamic_cast<Army*> (h)));
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
    CombatantRow (Army *a)
      : m_army (a)
      {
      }

    sigc::signal<void()> m_signal_changed;

};

class BattleCalculatorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "battle-calculator.ui";
      }

    BattleCalculatorDialog (BaseObjectType* o,
                            const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_fight_button = load <Gtk::Button> ("fight_button");
        m_fight100_button = load <Gtk::Button> ("fight100_button");

        m_attackers_treeview = load <Gtk::ColumnView> ("attackers_treeview");
        m_attackers_scrolledwindow =
          load <Gtk::ScrolledWindow> ("attackers_scrolledwindow");
        m_attacker_copy_button = load <Gtk::Button> ("attacker_copy_button");
        m_attacker_edit_hero_button =
          load <Gtk::Button> ("attacker_edit_hero_button");
        m_attacker_add_button = load <Gtk::Button> ("attacker_add_button");
        m_attacker_remove_button =
          load <Gtk::Button> ("attacker_remove_button");

        m_defenders_treeview = load <Gtk::ColumnView> ("defenders_treeview");
        m_defenders_scrolledwindow =
          load <Gtk::ScrolledWindow> ("defenders_scrolledwindow");
        m_defender_copy_button = load <Gtk::Button> ("defender_copy_button");
        m_defender_edit_hero_button =
          load <Gtk::Button> ("defender_edit_hero_button");
        m_defender_add_button = load <Gtk::Button> ("defender_add_button");
        m_defender_remove_button =
          load <Gtk::Button> ("defender_remove_button");
        m_fortified_switch = load <Gtk::Switch> ("fortified_switch");

        m_city_switch = load <Gtk::Switch> ("city_switch");
        m_terrain_combobox = load <Gtk::Box> ("terrain_combobox");
        m_die_sides_combobox = load <Gtk::Box> ("die_sides_combobox");
      }

    ~BattleCalculatorDialog ()
      {
        disconnect_signals ();
        for (guint i = 0; i < m_astore->get_n_items (); ++i)
          {
            auto item = m_astore->get_item (i);
            if (item)
              delete item->m_army;
          }

        for (guint i = 0; i < m_dstore->get_n_items (); ++i)
          {
            auto item = m_dstore->get_item (i);
            if (item)
              delete item->m_army;
          }
        delete m_umgr;
      }

    void setup (std::list<Army*> attackers, std::list<Army*> defenders)
      {
        load_armies (attackers, defenders);

        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        m_attacker_add_button->set_icon_name ("list-add-symbolic");
        m_attacker_remove_button->set_icon_name ("list-remove-symbolic");

        m_defender_add_button->set_icon_name ("list-add-symbolic");
        m_defender_remove_button->set_icon_name ("list-remove-symbolic");

        fill_terrain_combobox ();
        fill_die_sides_combobox ();

        m_astore = Gio::ListStore<CombatantRow>::create ();
        m_aselection_model = Gtk::SingleSelection::create (m_astore);
        m_attackers_treeview->set_model (m_aselection_model);
        setup_columns (m_attackers_treeview, m_att_spin_conn);
        fill_attackers_treeview ();

        m_dstore = Gio::ListStore<CombatantRow>::create ();
        m_dselection_model = Gtk::SingleSelection::create (m_dstore);
        m_defenders_treeview->set_model (m_dselection_model);
        setup_columns (m_defenders_treeview, m_def_spin_conn);
        fill_defenders_treeview ();

        setup_undo ();

        update ();

      }

private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_fight_button;
    Gtk::Button *m_fight100_button;

    Gtk::ColumnView *m_attackers_treeview;
    Gtk::ScrolledWindow *m_attackers_scrolledwindow;
    Gtk::Button *m_attacker_copy_button;
    Gtk::Button *m_attacker_edit_hero_button;
    Gtk::Button *m_attacker_remove_button;
    Gtk::Button *m_attacker_add_button;

    Gtk::ColumnView *m_defenders_treeview;
    Gtk::ScrolledWindow *m_defenders_scrolledwindow;
    Gtk::Button *m_defender_copy_button;
    Gtk::Button *m_defender_edit_hero_button;
    Gtk::Button *m_defender_remove_button;
    Gtk::Button *m_defender_add_button;
    Gtk::Switch *m_fortified_switch;

    Gtk::Switch *m_city_switch;
    Gtk::Box *m_terrain_combobox;
    LwCombo *m_terrain_combo;
    Gtk::Box *m_die_sides_combobox;
    LwCombo *m_die_sides_combo;

    Glib::RefPtr<Gtk::SingleSelection> m_aselection_model;
    Glib::RefPtr<Gio::ListStore<CombatantRow>> m_astore;
    Glib::RefPtr<Gtk::SingleSelection> m_dselection_model;
    Glib::RefPtr<Gio::ListStore<CombatantRow>> m_dstore;

    std::list<Army *> m_attackers;
    std::list<Army *> m_defenders;

    sigc::connection m_att_spin_conn;
    sigc::connection m_def_spin_conn;
    std::list<sigc::connection> m_connections;
    UndoMgr *m_umgr;
    bool m_fortified_switch_active;
    bool m_city_switch_active;
    guint32 m_terrain_combo_row;
    guint32 m_die_sides_combo_row;

    void fill_terrain_combobox ()
      {
        m_terrain_combo = Gtk::make_managed<LwCombo> ();
        Tileset *tileset = GameMap::getTileset ();
        for (auto t : *tileset)
          m_terrain_combo->append (t->getName ());
        m_terrain_combobox->append (*m_terrain_combo);
      }

    void fill_die_sides_combobox ()
      {
        m_die_sides_combo = Gtk::make_managed<LwCombo> ();
        m_die_sides_combo->append ("20");
        m_die_sides_combo->append ("24");
        m_die_sides_combobox->append (*m_die_sides_combo);
      }

    void fill_attackers_treeview ()
      {
        m_astore->remove_all ();
        for (auto a : m_attackers)
          m_astore->append (CombatantRow::create (a));
      }

    void fill_defenders_treeview ()
      {
        m_dstore->remove_all ();
        for (auto a : m_defenders)
          m_dstore->append (CombatantRow::create (a));
      }

    void setup_columns (Gtk::ColumnView *treeview, sigc::connection &conn)
      {
        LwColumn::setup_picture_column<CombatantRow>
          (treeview, LW_BUTTON_SIZE, LW_BUTTON_SIZE, "army_image", "",
           [] (const auto& row)
           {
             return
               ImageCache::instance ()->getArmyPic (row->m_army)->to_texture ();
           });

        LwColumn::setup_text_column<CombatantRow>
          (treeview, "name_label", true, Gtk::Justification::LEFT,
           _("Name"),
           [] (const auto& row)
           {
             return row->m_army->getName ();
           });

        LwColumn::setup_number_column<CombatantRow>
          (treeview, "str_spin", conn,
           Gtk::Adjustment::create (0, 0, 12, 1, 10, 0), _("Str"),
           [] (const auto& row)
           {
             return row->m_army->getStat (Army::STRENGTH, false);
           },
           [this] (const auto& row, guint32 value)
           {

             bool found_defender = false;
             int defender_row = -1;
             for (guint i = 0; i < m_dstore->get_n_items (); ++i)
               {
                 auto item = m_dstore->get_item (i);
                 if (item == row)
                   {
                     found_defender = true;
                     defender_row = i;
                     break;
                   }
               }

             bool found_attacker = false;
             int attacker_row = -1;
             for (guint i = 0; i < m_astore->get_n_items (); ++i)
               {
                 auto item = m_astore->get_item (i);
                 if (item == row)
                   {
                     found_attacker = true;
                     attacker_row = i;
                     break;
                   }
               }

             if (found_attacker)
               m_umgr->add
                 (new BattleCalculatorUndoAction_AttackerStrength
                  (attacker_row, row->m_army->getStat (Army::STRENGTH, false)));
             else
               m_umgr->add
                 (new BattleCalculatorUndoAction_DefenderStrength
                  (defender_row, row->m_army->getStat (Army::STRENGTH, false)));

             row->m_army->setStat (Army::STRENGTH, value);
           });
      }

    Army *get_selected_attacker ()
      {
        auto item = m_aselection_model->get_selected_item ();
        if (!item)
          return NULL;
        auto row = std::dynamic_pointer_cast<CombatantRow>(item);
        return row->m_army;
      }

    Army *get_selected_defender ()
      {
        auto item = m_dselection_model->get_selected_item ();
        if (!item)
          return NULL;
        auto row = std::dynamic_pointer_cast<CombatantRow>(item);
        return row->m_army;
      }

    void update ()
      {
        disconnect_signals ();
        update_buttons ();
        connect_signals ();
      }

    void update_buttons ()
      {
        bool have_attacker = get_selected_attacker () != NULL;
        m_attacker_copy_button->set_sensitive (have_attacker);
        bool have_defender = get_selected_defender () != NULL;
        m_defender_copy_button->set_sensitive (have_defender);
        bool have_attacker_hero =
          have_attacker && get_selected_attacker ()->isHero ();
        m_attacker_edit_hero_button->set_sensitive (have_attacker_hero);
        bool have_defender_hero =
          have_defender && get_selected_defender ()->isHero ();
        m_defender_edit_hero_button->set_sensitive (have_defender_hero);
        m_fight_button->set_sensitive (have_attacker && have_defender);
        m_fight100_button->set_sensitive (have_attacker && have_defender);
        m_attacker_remove_button->set_sensitive (have_attacker);
        m_defender_remove_button->set_sensitive (have_defender);
        m_attacker_add_button->set_sensitive
          (m_astore->get_n_items () <= MAX_STACK_SIZE);
        int span = GameMap::getCityset ()->getCityTileWidth ();
        m_defender_add_button->set_sensitive
          (m_dstore->get_n_items () <= MAX_STACK_SIZE * (span * span));
      }

    void connect_signals ()
      {
        add_connection
          (m_aselection_model->signal_selection_changed ().connect
           ([this] (const guint &, const guint &)
            {
              update ();
            }));

        add_connection
          (m_dselection_model->signal_selection_changed ().connect
           ([this] (const guint &, const guint &)
            {
              update ();
            }));

        add_connection
          (m_fight_button->signal_clicked ().connect
           ([this] ()
            {
              auto outcome = run_battle ();
              auto msg = outcome == FightResult::ATTACKER_WON ?
                _("Attackers Win") : _("Defenders Win");
              auto d = LwDialog::alert (msg, "");
              d->choose
                (*this,
                 [this, d] (auto result)
                 {
                   d->choose_finish (result);
                   return;
                 });
            }));

        add_connection
          (m_fight100_button->signal_clicked ().connect
           ([this] ()
            {
              int attacker_wins = 0;
              int defender_wins = 0;
              for (int i = 0; i < 100; i++)
                {
                  switch (run_battle ())
                    {
                    case FightResult::ATTACKER_WON:
                      attacker_wins++;
                      break;

                    case FightResult::DEFENDER_WON:
                      defender_wins++;
                      break;

                    case FightResult::DRAW:
                      break;
                    }
                }

              auto msg = _("Battle Outcome");
              auto detail =
                String::ucompose (ngettext
                                  ("The attacker won %1 battle and lost %2.",
                                   "The attacker won %1 battles and lost %2.",
                                   attacker_wins),
                                  attacker_wins, defender_wins);
              auto d = LwDialog::alert (msg, detail);
              d->choose
                (*this,
                 [this, d] (auto result)
                 {
                   d->choose_finish (result);
                   return;
                 });
            }));

        add_connection
          (m_attacker_edit_hero_button->signal_clicked ().connect
           ([this] ()
            {
              auto army = get_selected_attacker ();
              if (army->isHero ())
                {
                  auto action =
                    new BattleCalculatorUndoAction_AttackerHeroDetails
                    (m_attackers);

                  auto d = LwDialog::build<HeroEditorDialog> (this);
                  d->setup (dynamic_cast<Hero*>(army));
                  d->signal_response ().connect
                    ([this, d, action] (Gtk::ResponseType)
                     {
                       if (d->is_changed ())
                         m_umgr->add (action);
                       else
                         delete action;
                       delete d;
                     });
                }
            }));

        add_connection
          (m_defender_edit_hero_button->signal_clicked ().connect
           ([this] ()
            {
              auto army = get_selected_defender ();
              if (army->isHero ())
                {
                  auto action =
                    new BattleCalculatorUndoAction_AttackerHeroDetails
                    (m_defenders);

                  auto d = LwDialog::build<HeroEditorDialog> (this);
                  d->setup (dynamic_cast<Hero*>(army));
                  d->signal_response ().connect
                    ([this, d, action] (Gtk::ResponseType)
                     {
                       if (d->is_changed ())
                         m_umgr->add (action);
                       else
                         delete action;
                       delete d;
                     });
                }
            }));

        add_connection
          (m_attacker_copy_button->signal_clicked ().connect
           ([this] ()
            {
              m_umgr->add
                (new BattleCalculatorUndoAction_AttackerCopy (m_attackers));

              auto army = get_selected_attacker ();
              if (army->isHero ())
                {
                  auto new_hero = new Hero (*dynamic_cast<Hero*>(army));
                  m_astore->append (CombatantRow::create
                                    (new Army (*new_hero)));
                  m_attackers.push_back (new_hero);
                }
              else
                {
                  auto new_army = new Army (*army);
                  m_astore->append (CombatantRow::create
                                    (new Army (*new_army)));
                  m_attackers.push_back (new_army);
                }
            }));

        add_connection
          (m_defender_copy_button->signal_clicked ().connect
           ([this] ()
            {
              m_umgr->add
                (new BattleCalculatorUndoAction_DefenderCopy (m_defenders));

              auto army = get_selected_defender ();
              if (army->isHero ())
                {
                  auto new_hero = new Hero (*dynamic_cast<Hero*>(army));
                  m_dstore->append (CombatantRow::create
                                    (new Army (*new_hero)));
                  m_defenders.push_back (new_hero);
                }
              else
                {
                  auto new_army = new Army (*army);
                  m_dstore->append (CombatantRow::create
                                    (new Army (*new_army)));
                  m_defenders.push_back (new_army);
                }
            }));

        add_connection
          (m_attacker_add_button->signal_clicked ().connect
           ([this] ()
            {
              auto d = LwDialog::build<SelectArmyDialog> (this);
              auto p = Playerlist::getActiveplayer ();
              d->set_allow_choose_player ();
              d->setup (p->get_shield (),
                        SelectArmyDialog::SELECT_NORMAL_WITH_HERO, -1);
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                         {
                           m_umgr->add
                             (new BattleCalculatorUndoAction_AttackerAdd
                              (m_attackers));

                           auto np =
                             Playerlist::instance ()->get
                             (d->get_selected_shield ());

                           auto proto =
                             Armysetlist::instance ()->getArmy
                             (np->getArmyset (), d->get_selected_army ());

                           if (proto->isHero ())
                             {
                               auto heroproto =
                                 HeroTemplates::instance ()->getRandomHero
                                 (np->get_shield ());
                               auto nh = new Hero (*heroproto);
                               m_attackers.push_back (nh);
                               nh->setOwnerId (np->getId ());
                               m_astore->append (CombatantRow::create (nh));
                             }
                           else
                             {
                               auto army = new Army (*proto, np);
                               m_attackers.push_back (army);
                               m_astore->append (CombatantRow::create (army));
                             }
                           scroll_attackers_treeview_to_bottom ();
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
          (m_attacker_remove_button->signal_clicked ().connect
           ([this] ()
            {
              m_umgr->add
                (new BattleCalculatorUndoAction_AttackerRemove (m_attackers));

              auto army = get_selected_attacker ();
              m_astore->remove (m_aselection_model->get_selected ());
              m_attackers.remove (army);
              delete army;
              update ();
            }));

        add_connection
          (m_defender_add_button->signal_clicked ().connect
           ([this] ()
            {
              auto d = LwDialog::build<SelectArmyDialog> (this);
              auto p = Playerlist::getActiveplayer ();
              d->set_allow_choose_player ();
              d->setup (p->get_shield (),
                        SelectArmyDialog::SELECT_NORMAL_WITH_HERO, -1);
              d->signal_response ().connect
                ([this, d] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                         {
                           m_umgr->add
                             (new BattleCalculatorUndoAction_DefenderAdd
                              (m_defenders));

                           auto np =
                             Playerlist::instance ()->get
                             (d->get_selected_shield ());

                           auto proto =
                             Armysetlist::instance ()->getArmy
                             (np->getArmyset (), d->get_selected_army ());

                           if (proto->isHero ())
                             {
                               auto heroproto =
                                 HeroTemplates::instance ()->getRandomHero
                                 (np->get_shield ());
                               auto nh = new Hero (*heroproto);
                               nh->setOwnerId (np->getId ());
                               m_defenders.push_back (nh);
                               m_dstore->append (CombatantRow::create (nh));
                             }
                           else
                             {
                               auto army = new Army (*proto, np);
                               m_defenders.push_back (army);
                               m_dstore->append (CombatantRow::create (army));
                             }
                           scroll_defenders_treeview_to_bottom ();
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
          (m_defender_remove_button->signal_clicked ().connect
           ([this] ()
            {
              m_umgr->add
                (new BattleCalculatorUndoAction_DefenderRemove (m_defenders));

              auto army = get_selected_defender ();
              m_dstore->remove (m_dselection_model->get_selected ());
              m_defenders.remove (army);
              delete army;
              update ();
            }));

        add_connection
          (m_city_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new BattleCalculatorUndoAction_City (m_city_switch_active));

              m_city_switch_active = m_city_switch->get_active ();
            }));

        add_connection
          (m_terrain_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new BattleCalculatorUndoAction_Terrain (m_terrain_combo_row));

              m_terrain_combo_row = m_terrain_combo->get_active_row_number ();
            }));

        add_connection
          (m_die_sides_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new BattleCalculatorUndoAction_Sides (m_die_sides_combo_row));

              m_die_sides_combo_row =
                m_die_sides_combo->get_active_row_number ();
            }));

        add_connection
          (m_fortified_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new BattleCalculatorUndoAction_Fortify
                 (m_fortified_switch_active));

              m_fortified_switch_active = m_fortified_switch->get_active ();
            }));

      }

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    FightResult::Outcome run_battle ()
      {
        auto p = Playerlist::getActiveplayer ();

        //load the attackers into a single stack
        std::list<Stack*> attackers;
        Stack *stack = new Stack (p, Vector<int>(-1,-1));
        attackers.push_back (stack);
        for (guint i = 0; i < m_astore->get_n_items (); ++i)
          {
            auto item = m_astore->get_item (i);
            if (item)
              stack->add (item->m_army);
          }

        //load the defenders into a bunch of stacks
        std::list<Stack*> defenders;
        stack = new Stack (p, Vector<int>(-1,-1));
        defenders.push_back (stack);
        for (guint i = 0; i < m_dstore->get_n_items (); ++i)
          {
            if (stack->size () == MAX_STACK_SIZE)
              {
                stack = new Stack (p, Vector<int>(-1,-1));
                defenders.push_back (stack);
              }
            auto item = m_dstore->get_item (i);
            if (item)
              stack->add (item->m_army);
          }

        Tileset *tileset = GameMap::getTileset ();
        int row = m_terrain_combo->get_active_row_number ();
        bool water = (*tileset)[row]->getType () == Tile::WATER;
        // put them all in the water if we're doing that.
        for (auto s : attackers)
          for (auto a : *s)
            a->setInShip (water);

        for (auto s : defenders)
          for (auto a : *s)
            a->setInShip (water);

        // fortify them if we're doing that
        for (auto s : defenders)
          for (auto a : *s)
            a->setFortified (m_fortified_switch->get_active ());

        Fight f (attackers, defenders, m_city_switch->get_active (),
                 (*tileset)[row]->getType ());

        f.battle (m_die_sides_combo->get_selected () == 1);

        //reset the HP
        std::map<guint32,guint32> initial_hitpoints = f.getInitialHPs ();
        for (auto s : attackers)
          for (auto a : *s)
            a->setHP (initial_hitpoints[a->getId ()]);

        for (auto s : defenders)
          for (auto a : *s)
            a->setHP (initial_hitpoints[a->getId ()]);

        //delete the stacks we made, but keep the armies
        for (auto s: attackers)
          {
            s->clear ();
            delete s;
          }
        for (auto d: defenders)
          {
            d->clear ();
            delete d;
          }

        return f.get_outcome ();
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &BattleCalculatorDialog::execute_action));

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
        auto action = dynamic_cast<BattleCalculatorUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case BattleCalculatorUndoAction::SIDES:
              {
                auto sides_row = m_die_sides_combo->get_active_row_number ();
                auto a =
                  dynamic_cast<BattleCalculatorUndoAction_Sides*>(action);
                out = new BattleCalculatorUndoAction_Sides (sides_row);

                m_die_sides_combo->set_active (a->get_row ());
              }
            break;

          case BattleCalculatorUndoAction::TERRAIN:
              {
                auto terrain_row = m_terrain_combo->get_active_row_number ();
                auto a =
                  dynamic_cast<BattleCalculatorUndoAction_Terrain*>(action);
                out = new BattleCalculatorUndoAction_Terrain (terrain_row);

                m_terrain_combo->set_active (a->get_row ());
              }
            break;

          case BattleCalculatorUndoAction::CITY:
              {
                bool city_active = m_city_switch->get_active ();
                auto a = dynamic_cast<BattleCalculatorUndoAction_City*>(action);
                out = new BattleCalculatorUndoAction_City (city_active);

                m_city_switch->set_active (a->get_active ());
              }
            break;

          case BattleCalculatorUndoAction::FORTIFY:
              {
                auto fortified_active = m_fortified_switch->get_active ();
                auto a =
                  dynamic_cast<BattleCalculatorUndoAction_Fortify*>(action);
                out = new BattleCalculatorUndoAction_Fortify (fortified_active);
                m_fortified_switch->set_active (a->get_active ());
              }
            break;

          case BattleCalculatorUndoAction::DEFENDER_ADD:
              {
                auto a =
                  dynamic_cast<BattleCalculatorUndoAction_DefenderAdd*>(action);
                out = new BattleCalculatorUndoAction_DefenderAdd (m_defenders);
                replace_defenders (a, m_defenders);
              }
            break;

          case BattleCalculatorUndoAction::DEFENDER_REMOVE:
              {
                auto a =
                  dynamic_cast<BattleCalculatorUndoAction_DefenderRemove*>(action);
                out = new BattleCalculatorUndoAction_DefenderRemove (m_defenders);
                replace_defenders (a, m_defenders);
              }
            break;

          case BattleCalculatorUndoAction::DEFENDER_COPY:
              {
                auto a =
                  dynamic_cast<BattleCalculatorUndoAction_DefenderCopy*>(action);
                out = new BattleCalculatorUndoAction_DefenderCopy (m_defenders);
                replace_defenders (a, m_defenders);
              }
            break;

          case BattleCalculatorUndoAction::DEFENDER_HERO_DETAILS:
              {
                auto a =
                  dynamic_cast<BattleCalculatorUndoAction_DefenderHeroDetails*>(action);
                out = new BattleCalculatorUndoAction_DefenderHeroDetails (m_defenders);
                replace_defenders (a, m_defenders);
              }
            break;

          case BattleCalculatorUndoAction::DEFENDER_STRENGTH:
              {
                auto a =
                  dynamic_cast<BattleCalculatorUndoAction_DefenderStrength*>(action);
                out = new BattleCalculatorUndoAction_DefenderStrength
                  (a->get_index (),
                   get_defender_by_index (a)->getStat (Army::STRENGTH, false));
                get_defender_by_index (a)->setStat
                  (Army::STRENGTH, a->get_strength ());
        
                auto item = m_dstore->get_item (a->get_index ());
                if (item)
                  {
                    auto row = std::dynamic_pointer_cast<CombatantRow>(item);
                    row->changed ();
                  }
              }
            break;

          case BattleCalculatorUndoAction::ATTACKER_ADD:
              {
                auto a =
                  dynamic_cast<BattleCalculatorUndoAction_AttackerAdd*>(action);
                out = new BattleCalculatorUndoAction_AttackerAdd (m_attackers);
                replace_attackers (a, m_attackers);
              }
            break;

          case BattleCalculatorUndoAction::ATTACKER_REMOVE:
              {
                auto a =
                  dynamic_cast<BattleCalculatorUndoAction_AttackerRemove*>(action);
                out = new BattleCalculatorUndoAction_AttackerRemove (m_attackers);
                replace_attackers (a, m_attackers);
              }
            break;

          case BattleCalculatorUndoAction::ATTACKER_COPY:
              {
                auto a =
                  dynamic_cast<BattleCalculatorUndoAction_AttackerCopy*>(action);
                out = new BattleCalculatorUndoAction_AttackerCopy (m_attackers);
                replace_attackers (a, m_attackers);
              }
            break;

          case BattleCalculatorUndoAction::ATTACKER_HERO_DETAILS:
              {
                auto a =
                  dynamic_cast<BattleCalculatorUndoAction_AttackerHeroDetails*>(action);
                out =
                  new BattleCalculatorUndoAction_AttackerHeroDetails
                  (m_attackers);
                replace_attackers (a, m_attackers);
              }
            break;

          case BattleCalculatorUndoAction::ATTACKER_STRENGTH:
              {
                auto a =
                  dynamic_cast<BattleCalculatorUndoAction_AttackerStrength*>(action);
                out = new BattleCalculatorUndoAction_AttackerStrength
                  (a->get_index (),
                   get_attacker_by_index (a)->getStat (Army::STRENGTH, false));
                get_attacker_by_index (a)->setStat (Army::STRENGTH,
                                                    a->get_strength ());
                auto item = m_astore->get_item (a->get_index ());
                if (item)
                  {
                    auto row = std::dynamic_pointer_cast<CombatantRow>(item);
                    row->changed ();
                  }
              }
            break;
          }
        return out;
      }

    Army *get_attacker_by_index (BattleCalculatorUndoAction_Index* a)
      {
        auto item = m_astore->get_item (a->get_index ());
        if (!item)
          return NULL;
        auto row = std::dynamic_pointer_cast<CombatantRow>(item);
        return row->m_army;
      }

    Army *get_defender_by_index (BattleCalculatorUndoAction_Index* a)
      {
        auto item = m_dstore->get_item (a->get_index ());
        if (!item)
          return NULL;
        auto row = std::dynamic_pointer_cast<CombatantRow>(item);
        return row->m_army;
      }

    void replace_attackers (BattleCalculatorUndoAction_Armies *action,
                            std::list<Army*> &armies)
      {
        replace_armies (action, armies);
        disconnect_signals ();
        fill_attackers_treeview ();
        connect_signals ();
      }

    void replace_defenders (BattleCalculatorUndoAction_Armies *action,
                            std::list<Army*> &armies)
      {
        replace_armies (action, armies);
        disconnect_signals ();
        fill_defenders_treeview ();
        connect_signals ();
      }

    void replace_armies (BattleCalculatorUndoAction_Armies *action, std::list<Army*> &armies)
      {
        for (auto a : armies)
          delete a;
        armies.clear ();
        for (auto a : action->get_armies ())
          {
            if (a->isHero ())
              {
                Hero *h = dynamic_cast<Hero*>(a);
                armies.push_back (new Hero (*h));
              }
            else
              armies.push_back (new Army (*a));
          }
      }

    void scroll_attackers_treeview_to_bottom ()
      {
        guint n = m_astore->get_n_items ();
        Glib::signal_idle ().connect_once
          ([this, n] ()
           {
             m_attackers_treeview->scroll_to (n - 1);
           });
      }

    void scroll_defenders_treeview_to_bottom ()
      {
        guint n = m_dstore->get_n_items ();
        Glib::signal_idle ().connect_once
          ([this, n] ()
           {
             m_defenders_treeview->scroll_to (n - 1);
           });
      }

    void load_armies (std::list<Army*> attackers, std::list<Army*> defenders)
      {
        //take a copy of attackers and defenders
        for (auto a : attackers)
          {
            if (a->isHero ())
              m_attackers.push_back
                (new Hero (*dynamic_cast<Hero*>(a)));
            else
              m_attackers.push_back
                (new Army (*a, a->getOwner()));
          }

        for (auto a : defenders)
          {
            if (a->isHero ())
              m_defenders.push_back
                (new Hero (*dynamic_cast<Hero*>(a)));
            else
              m_defenders.push_back
                (new Army (*a, a->getOwner()));
          }
      }
};
#endif
