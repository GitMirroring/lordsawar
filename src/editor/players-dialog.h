//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2020, 2021, 2026 Ben Asselstine
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
#ifndef PLAYERS_DIALOG_H
#define PLAYERS_DIALOG_H
#include "players-undo-actions.h"
#include "character-editor-dialog.h"

class PlayersDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "players.ui";
      }

    PlayersDialog (BaseObjectType* o,
                   const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_randomize_gold_button = load <Gtk::Button> ("randomize_gold_button");
        m_grid = load <Gtk::Grid> ("players_grid");
      }

    ~PlayersDialog ()
      {
        disconnect_signals ();
        delete m_hero_templates;
        delete m_umgr;
      }

    HeroTemplates *get_hero_templates ()
      {
        return m_hero_templates;
      }

    void setup (CreateScenarioRandomize *random, Playerlist *pl,
                HeroTemplates *hero_templates)
      {
        m_hero_templates = hero_templates->copy ();
        m_random = random;
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        setup_undo ();

        for (guint32 i = Shield::WHITE; i <= Shield::BLACK; i++)
          {
            auto p = pl->get (Shield::Color (i));
            if (!p)
              {
                m_type.push_back (0);
                m_name.push_back (random->getPlayerName (Shield::Color (i)));
                m_gold.push_back (0);
                add_player (i);
                continue;
              }

            m_gold.push_back (p->getGold ());
            m_name.push_back (p->getName ());
            switch (p->getType ())
              {
              case Player::AI_DUMMY:
                break;

              case Player::NETWORKED:
              case Player::HUMAN:
                m_type.push_back (1);
                break;

              case Player::AI_FAST:
                m_type.push_back (2);
                break;

              case Player::AI_SMART:
                m_type.push_back (3);
                break;
              }
            add_player (i);
          }

        update ();
      }

    bool is_changed ()
      {
        return m_umgr->undo_empty () == false;
      }

    GameParameters get_params ()
      {
        GameParameters g;
        for (guint32 i = Shield::WHITE; i <= Shield::BLACK; i++)
          g.players.push_back (to_player (i));
        return g;
      }

    guint32 get_gold (Shield::Color shield)
      {
        return m_gold[(guint32) shield];
      }

private:
    Gtk::Button *m_close_button;
    Gtk::Button *m_randomize_gold_button;
    Gtk::Grid *m_grid;
    UndoMgr *m_umgr;
    std::list<sigc::connection> m_connections;
    CreateScenarioRandomize *m_random;
    std::vector<guint32> m_gold;
    std::vector<guint32> m_type;
    std::vector<Glib::ustring> m_name;

    typedef std::vector<Glib::ustring> player_name_seq;

    std::vector<LwCombo*> m_type_comboboxes;
    std::vector<Gtk::Entry*> m_name_entries;
    std::vector<Gtk::SpinButton*> m_gold_spinbuttons;
    std::vector<Gtk::Button*> m_heroes_buttons;
    HeroTemplates *m_hero_templates;

    void update ()
      {
        disconnect_signals ();
        auto nit = m_name.begin ();
        auto tit = m_type.begin ();
        auto git = m_gold.begin ();

        auto wnit = m_name_entries.begin ();
        auto wtit = m_type_comboboxes.begin ();
        auto wgit = m_gold_spinbuttons.begin ();
        auto whit = m_heroes_buttons.begin ();

        for (; nit != m_name.end ();)
          {
            (*wtit)->set_active (*tit);
            (*wnit)->set_text (*nit);
            (*wgit)->set_value (*git);

            bool sens = (*wtit)->get_active_row_number () != 0;
            (*wnit)->set_sensitive (sens);
            (*whit)->set_sensitive (sens);
            (*wgit)->set_sensitive (sens);
            nit++;
            tit++;
            git++;
            wnit++;
            wtit++;
            wgit++;
            whit++;
          }
        m_umgr->set_cursors ();

        connect_signals ();
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &PlayersDialog::execute_action));

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
        PlayersUndoAction *action =
          dynamic_cast<PlayersUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case PlayersUndoAction::TYPE:
              {
                auto *a = dynamic_cast<PlayersUndoAction_Type*>(action);
                out = new PlayersUndoAction_Type (a->get_index (),
                                                  m_type[a->get_index ()]);
                m_type[a->get_index ()] = a->get_player_type ();
              }
            break;

          case PlayersUndoAction::NAME:
              {
                auto *a = dynamic_cast<PlayersUndoAction_Name*>(action);
                int i = a->get_index ();
                out = new PlayersUndoAction_Name (i, m_name[i], m_umgr,
                                                  m_name_entries[i]);
                m_name[a->get_index ()] = a->get_name ();
              } 
            break;

          case PlayersUndoAction::GOLD:
              {
                auto *a = dynamic_cast<PlayersUndoAction_Gold*>(action);
                out = new PlayersUndoAction_Gold (a->get_index (),
                                                  m_gold[a->get_index ()]);
                m_gold[a->get_index ()] = a->get_gold ();
              }
            break;

          case PlayersUndoAction::RANDOMIZE_GOLD:
              {
                auto a = dynamic_cast<PlayersUndoAction_RandomizeGold*>(action);
                out = new PlayersUndoAction_RandomizeGold (m_gold);
                int row = 0;
                for (auto g : a->get_players_gold ())
                  {
                    m_gold[row] = g;
                    row++;
                  }
              }
            break;

          case PlayersUndoAction::HEROES:
              {
                auto a = dynamic_cast<PlayersUndoAction_Heroes*>(action);
                out = new PlayersUndoAction_Heroes
                  (a->get_index (),
                   HeroTemplates::instance ()->getHeroes
                   (Shield::Color (a->get_index ())));
                HeroTemplates::instance ()->replaceHeroes
                  (Shield::Color (a->get_index ()), a->get_heroes ());
                a->clear_heroes ();
              }
            break;
          }
        return out;
      }

    void disconnect_signals ()
      {
        for (auto c : m_connections)
          c.disconnect ();
        m_connections.clear ();
      }

    void add_connection (sigc::connection c)
      {
        m_connections.push_back (c);
      }

    void connect_signals ()
      {
        add_connection
          (m_randomize_gold_button->signal_clicked ().connect
           ([this] ()
            {
              m_umgr->add (new PlayersUndoAction_RandomizeGold (m_gold));
              for (guint32 i = 0; i < m_type_comboboxes.size (); i++)
                {
                  if (m_type_comboboxes[i]->get_active_row_number () == 0)
                    continue;
                  int gold = 0;
                  m_random->getBaseGold (100, &gold);
                  gold = m_random->adjustBaseGold (gold);
                  m_gold[i] = gold;
                  update ();
                }
            }));

        int row = 0;
        for (auto c : m_type_comboboxes)
          {
            add_connection
              (c->signal_changed ().connect
               ([this, row, c] ()
                {
                  m_umgr->add (new PlayersUndoAction_Type (row, m_type[row]));
                  m_type[row] = c->get_active_row_number ();
                  update ();
                }));
            row++;
          }

        row = 0;
        for (auto e : m_name_entries)
          {
            add_connection
              (e->signal_changed ().connect
               ([this, row, e] ()
                {
                  m_umgr->add
                    (new PlayersUndoAction_Name (row, m_name[row], m_umgr,
                                                 m_name_entries[row]));
                  m_name[row] = e->get_text ();
                  update ();
                }));
            row++;
          }

        row = 0;
        for (auto g : m_gold_spinbuttons)
          {
            add_connection
              (g->signal_value_changed ().connect
               ([this, row, g] ()
                {
                  m_umgr->add (new PlayersUndoAction_Gold (row, m_gold[row]));
                  m_gold[row] = g->get_value_as_int ();
                  update ();
                }));
            row++;
          }

        row = 0;
        for (auto h : m_heroes_buttons)
          {
            add_connection
              (h->signal_clicked ().connect
               ([this, row] ()
                {
                  auto d = LwDialog::build<CharacterEditorDialog> (this);
                  d->setup (Shield::Color (row), m_hero_templates);
                  d->signal_response ().connect
                    ([d, row, this] (Gtk::ResponseType resp)
                     {
                       switch (resp)
                         {
                         case Gtk::ResponseType::ACCEPT:
                           if (d->is_changed ())
                             {
                               m_umgr->add
                                 (new PlayersUndoAction_Heroes
                                  (row, m_hero_templates->getHeroes
                                   (Shield::Color (row))));


                               delete m_hero_templates;
                               m_hero_templates = d->get_hero_templates ()->copy ();
                             }
                           break;

                         default:
                           break;
                         }
                       delete d;
                     });
                }));
            row++;
          }
      }
  
    Gtk::Entry *add_name_entry (Glib::ustring name)
      {
        Gtk::Entry *e = Gtk::make_managed<Gtk::Entry> ();
        e->set_text (name);
        e->property_hexpand () = true;
        m_umgr->add_cursor (e);
        m_name_entries.push_back (e);
        return e;
      }

    Gtk::Button* add_heroes_button ()
      {
        Gtk::Button *b = Gtk::make_managed<Gtk::Button> ();
        b->set_label (_("Heroes"));
        b->set_focus_on_click (false);
        b->property_margin_end () = 6;
        m_heroes_buttons.push_back (b);
        return b;
      }

    Gtk::SpinButton* add_gold_spinbutton (int gold)
      {
        Gtk::SpinButton *b = Gtk::make_managed<Gtk::SpinButton> ();
        b->set_adjustment (Gtk::Adjustment::create (gold, 0, 10000));
        m_gold_spinbuttons.push_back (b);
        return b;
      }

    LwCombo* add_type_combo (guint32 row)
      {
        LwCombo *c = Gtk::make_managed<LwCombo> ();
        c->set_halign (Gtk::Align::START);
        c->set_hexpand (false);

        c->append (NO_PLAYER_TYPE);
        c->append (HUMAN_PLAYER_TYPE);
        c->append (EASY_PLAYER_TYPE);
        c->append (HARD_PLAYER_TYPE);
        c->set_active (row);

        c->property_margin_start () = 6;
        m_type_comboboxes.push_back (c);
        return c;
      }

    void add_player (int i)
      {
        m_grid->attach (*add_type_combo (m_type[i]), 0, i + 1);
        m_grid->attach (*add_name_entry (m_name[i]), 1, i + 1);
        m_grid->attach (*add_gold_spinbutton (m_gold[i]), 2, i + 1);
        m_grid->attach (*add_heroes_button (), 3, i + 1);
      }

    GameParameters::Player to_player (int row)
      {
        GameParameters::Player player;
        switch (m_type[row])
          {
          case 0:
            player.type = GameParameters::Player::OFF;
            break;

          case 1:
            player.type = GameParameters::Player::HUMAN;
            break;

          case 2:
            player.type = GameParameters::Player::EASY;
            break;

          case 3:
            player.type = GameParameters::Player::HARD;
            break;
          }
        player.name = m_name[row];
        player.id = row;
        return player;
      }
};
#endif
