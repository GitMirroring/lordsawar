//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2012, 2014, 2015, 2017, 2020, 2021,
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
#ifndef STACK_EDITOR_DIALOG_H
#define STACK_EDITOR_DIALOG_H
#include "stack.h"
#include "stack-undo-actions.h"
#include "lw-column.h"
#include "hero-editor-dialog.h"

class StackArmyRow: public Glib::Object
{
public:

    Army *m_army;

    static Glib::RefPtr<StackArmyRow> create (Army *a)
      {
        return
          Glib::make_refptr_for_instance<StackArmyRow> (new StackArmyRow (a));
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
    StackArmyRow (Army *a)
      : m_army (a)
      {
      }

    sigc::signal<void()> m_signal_changed;
};

class StackEditorDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "stack-editor.ui";
      }

    StackEditorDialog (BaseObjectType* o,
                       const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_add_button = load <Gtk::Button> ("add_button");
        m_remove_button = load <Gtk::Button> ("remove_button");
        m_treeview = load <Gtk::ColumnView> ("treeview");
        m_fortified_switch = load <Gtk::Switch> ("fortified_switch");
        m_owner_combobox = load <Gtk::Box> ("owner_combobox");

        m_copy_button = load <Gtk::Button> ("copy_button");
        m_hero_button = load <Gtk::Button> ("hero_button");
      }

    ~StackEditorDialog ()
      {
        disconnect_signals ();
        m_strength_conn.disconnect ();
        m_moves_conn.disconnect ();
        m_upkeep_conn.disconnect ();
        delete m_umgr;
        delete m_stack;
      }

    void setup (Stack *s, Vector<int> pos)
      {
        if (s == NULL)
          m_stack = new Stack (Playerlist::getActiveplayer (), pos);
        else
          m_stack = new Stack (*s);

        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        fill_owner_combobox ();

        m_store = Gio::ListStore<StackArmyRow>::create ();
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

    Stack *get_stack ()
      {
        return m_stack;
      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }

private:
    Gtk::Button *m_close_button;
    Gtk::Box *m_owner_combobox;
    Gtk::Switch *m_fortified_switch;
    LwCombo *m_owner_combo;
    Gtk::ColumnView *m_treeview;
    Gtk::Button *m_add_button;
    Gtk::Button *m_remove_button;
    Gtk::Button *m_copy_button;
    Gtk::Button *m_hero_button;

    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<StackArmyRow>> m_store;

    std::list<sigc::connection> m_connections;
    sigc::connection m_strength_conn;
    sigc::connection m_moves_conn;
    sigc::connection m_upkeep_conn;

    UndoMgr *m_umgr;
    Stack *m_stack;

    void setup_name_column ()
      {
        LwColumn::setup_text_column<StackArmyRow> 
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
          (m_fortified_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              if (m_stack->getOwner () != Playerlist::getNeutral ())
                {
                  m_umgr->add
                    (new StackUndoAction_Fortify (m_stack->getFortified ()));
                  m_stack->setFortified (m_fortified_switch->get_active ());
                  update ();
                }
            }));

        add_connection
          (m_owner_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new StackUndoAction_Owner (m_stack->getOwner ()));
  
              m_stack->setPlayer (get_selected_player ());
              update ();
            }));

        add_connection
          (m_add_button->signal_clicked ().connect
           ([this] ()
            {
              auto d = LwDialog::build<SelectArmyDialog> (this);
              auto p = m_stack->getOwner ();
              d->setup (p->get_shield (),
                        SelectArmyDialog::SELECT_NORMAL_WITH_HERO, -1);
              d->signal_response ().connect
                ([this, d, p] (Gtk::ResponseType resp)
                 {
                   switch (resp)
                     {
                     case Gtk::ResponseType::ACCEPT:
                         {
                           m_umgr->add (new StackUndoAction_Add (m_stack));

                           auto proto =
                             Armysetlist::instance ()->getArmy
                             (p->getArmyset (), d->get_selected_army ());

                           if (proto->isHero ())
                             {
                               auto heroproto =
                                 HeroTemplates::instance ()->getRandomHero
                                 (p->get_shield ());
                               auto nh = new Hero (*heroproto);
                               nh->setOwnerId (p->getId ());
                               m_stack->add (nh);
                               m_store->append (StackArmyRow::create (nh));
                             }
                           else
                             {
                               auto army = new Army (*proto, p);
                               m_stack->add (army);
                               m_store->append (StackArmyRow::create (army));
                             }
                           scroll_treeview_to_bottom ();
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
              m_umgr->add
                (new StackUndoAction_Remove (m_stack));

              auto army = get_selected_army ();
              m_store->remove (m_selection_model->get_selected ());
              auto it = std::find (m_stack->begin (), m_stack->end(), army);
              if (it != m_stack->end ())
                m_stack->flErase (it);
              update ();
            }));

        add_connection
          (m_copy_button->signal_clicked ().connect
           ([this] ()
            {
              m_umgr->add (new StackUndoAction_Copy (m_stack));
              auto army = get_selected_army ();
              Army *new_army = new Army (*army, army->getOwner ());
              new_army->assignNewId ();
              m_stack->add (new_army);
              m_store->append (StackArmyRow::create (new_army));
              scroll_treeview_to_bottom ();
              update ();
            }));

        add_connection
          (m_hero_button->signal_clicked ().connect
           ([this] ()
            {
              auto army = get_selected_army ();
              auto hero = dynamic_cast<Hero*> (army);
              auto action =
                new StackUndoAction_HeroDetails
                (m_selection_model->get_selected (), hero);

              auto d = LwDialog::build<HeroEditorDialog> (this);
              d->setup (hero);
              d->signal_response ().connect
                ([this, d, action] (Gtk::ResponseType)
                 {
                   if (d->is_changed ())
                     m_umgr->add (action);
                   else
                     delete action;
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

        m_fortified_switch->set_active (m_stack->getFortified ());

        Player *p = get_selected_player ();
        m_fortified_switch->set_sensitive (p != Playerlist::getNeutral ());

        int i = 0;
        for (auto pl : *Playerlist::instance ())
          {
            if (pl == m_stack->getOwner ())
              break;
            i++;
          }
        m_owner_combo->set_active (i);

        connect_signals ();
        update_buttons ();
      }

    void update_buttons ()
      {
        guint n = m_store->get_n_items ();
        m_remove_button->set_sensitive (n > 0);
        m_add_button->set_sensitive (n < MAX_STACK_SIZE);
        Army *a = get_selected_army ();
        if (a)
          m_hero_button->set_sensitive (a->isHero ());
        else
          m_hero_button->set_sensitive (false);
        m_copy_button->set_sensitive (a != NULL && n < MAX_STACK_SIZE);
        m_close_button->set_sensitive (n > 0);
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    UndoAction *execute_action (UndoAction *action2)
      {
        StackUndoAction *action = dynamic_cast<StackUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case StackUndoAction::OWNER:
              {
                auto a = dynamic_cast<StackUndoAction_Owner*>(action);
                out = new StackUndoAction_Owner (m_stack->getOwner ());
                disconnect_signals ();
                m_stack->setPlayer (a->get_owner ());
                connect_signals ();
              }
            break;

          case StackUndoAction::FORTIFY:
              {
                auto a = dynamic_cast<StackUndoAction_Fortify*>(action);
                out = new StackUndoAction_Fortify (m_stack->getFortified ());
                m_stack->setFortified (a->get_fortify ());
              }
            break;

          case StackUndoAction::ADD:
              {
                auto a = dynamic_cast<StackUndoAction_Add*>(action);
                out = new StackUndoAction_Add (m_stack);
                populate_stack_with_armies (a->get_stack ());
                disconnect_signals ();
                fill_treeview ();
                connect_signals ();
              }
            break;

          case StackUndoAction::REMOVE:
              {
                auto a = dynamic_cast<StackUndoAction_Remove*>(action);
                out = new StackUndoAction_Remove (m_stack);
                populate_stack_with_armies (a->get_stack ());
                disconnect_signals ();
                fill_treeview ();
                connect_signals ();
              }
            break;

          case StackUndoAction::COPY:
              {
                auto a = dynamic_cast<StackUndoAction_Copy*>(action);
                out = new StackUndoAction_Copy (m_stack);
                populate_stack_with_armies (a->get_stack ());
                disconnect_signals ();
                fill_treeview ();
                connect_signals ();
              }
            break;

          case StackUndoAction::STRENGTH:
              {
                auto a = dynamic_cast<StackUndoAction_Strength*>(action);
                out = new StackUndoAction_Strength
                  (a->get_index (), get_army_by_index (a)->getStrength ());

                get_army_by_index (a)->setStrength (a->get_strength ());
        
                auto item = m_store->get_item (a->get_index ());
                auto row = std::dynamic_pointer_cast<StackArmyRow>(item);
                row->changed ();
              }
            break;

          case StackUndoAction::MOVES:
              {
                auto a = dynamic_cast<StackUndoAction_Moves*>(action);
                out = new StackUndoAction_Moves
                  (a->get_index (), get_army_by_index (a)->getMaxMoves ());

                get_army_by_index (a)->setMaxMoves (a->get_moves ());

                auto item = m_store->get_item (a->get_index ());
                auto row = std::dynamic_pointer_cast<StackArmyRow>(item);
                row->changed ();
              }
            break;

          case StackUndoAction::UPKEEP:
              {
                auto a = dynamic_cast<StackUndoAction_Upkeep*>(action);
                out = new StackUndoAction_Upkeep
                  (a->get_index (), get_army_by_index (a)->getUpkeep ());

                get_army_by_index (a)->setUpkeep (a->get_upkeep ());

                auto item = m_store->get_item (a->get_index ());
                auto row = std::dynamic_pointer_cast<StackArmyRow>(item);
                row->changed ();
              }
            break;

          case StackUndoAction::HERO_DETAILS:
              {
                auto a = dynamic_cast<StackUndoAction_HeroDetails*>(action);
                out = new StackUndoAction_HeroDetails
                  (a->get_index (), dynamic_cast<Hero*>(get_army_by_index (a)));

                guint32 row = 0;
                for (auto it = m_stack->begin (); it != m_stack->end (); ++it,
                     row++)
                  {
                    if (row == a->get_index ())
                      {
                        delete *it;
                        *it = new Hero (*a->get_hero ());
                        break;
                      }
                  }
              }
            break;
          }
        return out;
      }

    Army *get_army_by_index (StackUndoAction_Index *action)
      {
        auto item = m_store->get_item (action->get_index ());
        auto row = std::dynamic_pointer_cast<StackArmyRow>(item);
        if (row)
          return row->m_army;
        else
          return NULL;
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &StackEditorDialog::execute_action));

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
        for (auto a : *m_stack)
          m_store->append (StackArmyRow::create (a));
      }

    Army *get_selected_army ()
      {
        auto single_selection =
          std::dynamic_pointer_cast<Gtk::SingleSelection> (m_selection_model);

        auto item = single_selection->get_selected_item ();

        if (item)
          {
            auto row = std::dynamic_pointer_cast<StackArmyRow> (item);
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
            if (p == m_stack->getOwner ())
              found = i;
            i++;
          }
        if (found >= 0)
          m_owner_combo->set_active (found);
        m_owner_combobox->append (*m_owner_combo);
      }

    void populate_stack_with_armies (Stack *s)
      {
        for (auto it = m_stack->begin (); it != m_stack->end (); ++it)
          delete *it;
        m_stack->clear ();
        for (auto it = s->begin (); it != s->end (); ++it)
          {
            if ((*it)->isHero ())
              m_stack->add (new Hero (*dynamic_cast<Hero*>(*it)));
            else
              m_stack->add (new Army (*(*it)));
          }
      }

    void setup_image_column ()
      {
        LwColumn::setup_picture_column<StackArmyRow>
          (m_treeview, LW_BUTTON_SIZE, LW_BUTTON_SIZE, "army_image", "",
           [] (const auto& row)
           {
             return
               ImageCache::instance ()->getArmyPic (row->m_army)->to_texture ();
           });
      }

    void setup_strength_column ()
      {
        int min = ArmyProto::min_strength;
        int max = ArmyProto::max_strength;
        LwColumn::setup_number_column<StackArmyRow>
          (m_treeview, "str_spin", m_strength_conn,
           Gtk::Adjustment::create (min, min, max, 1, 10, 0), _("Str"),
           [] (const auto& row)
           {
             return row->m_army->getStat (Army::STRENGTH, false);
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
                   (new StackUndoAction_Strength
                    (irow, row->m_army->getStat (Army::STRENGTH, false)));

                 row->m_army->setStat (Army::STRENGTH, value);
               }
           });
      }

    void setup_moves_column ()
      {
        int min = ArmyProto::min_moves;
        int max = ArmyProto::max_moves;
        LwColumn::setup_number_column<StackArmyRow>
          (m_treeview, "moves_spin", m_moves_conn,
           Gtk::Adjustment::create (min, min, max, 1, 10, 0), _("Max Moves"),
           [] (const auto& row)
           {
             return row->m_army->getStat (Army::MOVES, false);
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
                   (new StackUndoAction_Moves
                    (irow, row->m_army->getStat (Army::MOVES, false)));

                 row->m_army->setStat (Army::MOVES, value);
               }
           });
      }

    void setup_upkeep_column ()
      {
        int min = ArmyProto::min_upkeep;
        int max = ArmyProto::max_upkeep;
        LwColumn::setup_number_column<StackArmyRow>
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
                   (new StackUndoAction_Upkeep
                    (irow, row->m_army->getUpkeep ()));

                 row->m_army->setUpkeep (value);
               }
           });
      }

    void setup_columns ()
      {
        setup_image_column ();
        setup_strength_column ();
        setup_moves_column ();
        setup_upkeep_column ();
        setup_name_column ();
      }

    Player *get_selected_player ()
      {
        int row = m_owner_combo->get_active_row_number ();
        auto it = Playerlist::instance ()->begin ();
        std::advance (it, row);
        return *it;
      }
};
#endif
