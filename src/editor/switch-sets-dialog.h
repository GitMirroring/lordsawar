//  Copyright (C) 2009, 2010, 2012, 2014, 2020, 2021, 2026 Ben Asselstine
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
#ifndef SWITCH_SETS_DIALOG_H
#define SWITCH_SETS_DIALOG_H
#include "army-set-list.h"
#include "city-set-list.h"
#include "shield-set-list.h"
#include "tile-set-list.h"
#include "switch-sets-undo-actions.h"

class SwitchSetsDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "switch-sets.ui";
      }

    SwitchSetsDialog (BaseObjectType* o,
                      const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_accept_button = load <Gtk::Button> ("accept_button");
        m_tile_size_combobox = load <Gtk::Box> ("tile_size_combobox");
        m_cityset_combobox = load <Gtk::Box> ("cityset_combobox");
        m_shieldset_combobox = load <Gtk::Box> ("shieldset_combobox");
        m_tileset_combobox = load <Gtk::Box> ("tileset_combobox");
        m_player1_combobox = load <Gtk::Box> ("player1_combobox");
        m_make_same_switch = load <Gtk::Switch> ("make_same_switch");
        m_player2_combobox = load <Gtk::Box> ("player2_combobox");
        m_player3_combobox = load <Gtk::Box> ("player3_combobox");
        m_player4_combobox = load <Gtk::Box> ("player4_combobox");
        m_player5_combobox = load <Gtk::Box> ("player5_combobox");
        m_player6_combobox = load <Gtk::Box> ("player6_combobox");
        m_player7_combobox = load <Gtk::Box> ("player7_combobox");
        m_player8_combobox = load <Gtk::Box> ("player8_combobox");
        m_neutral_combobox = load <Gtk::Box> ("neutral_combobox");
      }

    ~SwitchSetsDialog ()
      {
        disconnect_signals ();
        delete m_umgr;
      }

    void setup ()
      {
        set_response (m_accept_button, Gtk::ResponseType::ACCEPT);

        setup_tile_size_combo ();
        setup_cityset_combo ();
        setup_shieldset_combo ();
        setup_tileset_combo ();
        setup_armyset_combos ();

        fill_tile_size_combo ();
        fill_cityset_combo ();
        fill_shieldset_combo ();
        fill_tileset_combo ();
        fill_armyset_combos ();


        bool make_all_same =
          m_player1_combo->get_text () == m_player2_combo->get_text () &&
          m_player1_combo->get_text () == m_player3_combo->get_text () &&
          m_player1_combo->get_text () == m_player4_combo->get_text () &&
          m_player1_combo->get_text () == m_player5_combo->get_text () &&
          m_player1_combo->get_text () == m_player6_combo->get_text () &&
          m_player1_combo->get_text () == m_player7_combo->get_text () &&
          m_player1_combo->get_text () == m_player8_combo->get_text () &&
          m_player1_combo->get_text () == m_neutral_combo->get_text ();
        m_make_same_switch->set_active (make_all_same);

        m_tile_size = m_tile_size_combo->get_active_row_number ();
        m_cityset_theme = m_cityset_combo->get_active_row_number ();
        m_shieldset_theme = m_shieldset_combo->get_active_row_number ();
        m_tileset_theme = m_tileset_combo->get_active_row_number ();
        m_player1_theme = m_player1_combo->get_active_row_number ();
        m_make_same = make_all_same;
        m_player2_theme = m_player2_combo->get_active_row_number ();
        m_player3_theme = m_player3_combo->get_active_row_number ();
        m_player4_theme = m_player4_combo->get_active_row_number ();
        m_player5_theme = m_player5_combo->get_active_row_number ();
        m_player6_theme = m_player6_combo->get_active_row_number ();
        m_player7_theme = m_player7_combo->get_active_row_number ();
        m_player8_theme = m_player8_combo->get_active_row_number ();
        m_neutral_theme = m_neutral_combo->get_active_row_number ();

        setup_undo ();

        update ();
      }

    std::vector<Armyset*> get_armysets () const
      {
        std::vector<Armyset*> armysets;
        int ts = get_active_tile_size ();

        auto theme = Armysetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_player1_combo->get_text ()), ts);
        armysets.push_back (Armysetlist::instance ()->get (theme));
        theme = Armysetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_player2_combo->get_text ()), ts);
        armysets.push_back (Armysetlist::instance ()->get (theme));
        theme = Armysetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_player3_combo->get_text ()), ts);
        armysets.push_back (Armysetlist::instance ()->get (theme));
        theme = Armysetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_player4_combo->get_text ()), ts);
        armysets.push_back (Armysetlist::instance ()->get (theme));
        theme = Armysetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_player5_combo->get_text ()), ts);
        armysets.push_back (Armysetlist::instance ()->get (theme));
        theme = Armysetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_player6_combo->get_text ()), ts);
        armysets.push_back (Armysetlist::instance ()->get (theme));
        theme = Armysetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_player7_combo->get_text ()), ts);
        armysets.push_back (Armysetlist::instance ()->get (theme));
        theme = Armysetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_player8_combo->get_text ()), ts);
        armysets.push_back (Armysetlist::instance ()->get (theme));
        theme = Armysetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_neutral_combo->get_text ()), ts);
        armysets.push_back (Armysetlist::instance ()->get (theme));

        return armysets;
      }

    Cityset *get_cityset () const
      {
        auto theme = Citysetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_cityset_combo->get_active_text ()),
           get_active_tile_size ());
        return Citysetlist::instance ()->get (theme);
      }

    Shieldset *get_shieldset () const
      {
        auto theme = Shieldsetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_shieldset_combo->get_active_text ()));
        return Shieldsetlist::instance ()->get (theme);
      }

    Tileset *get_tileset () const
      {
        auto theme = Tilesetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_tileset_combo->get_active_text ()),
           get_active_tile_size ());
        return Tilesetlist::instance ()->get (theme);
      }

    bool is_tileset_changed () const
      {
        return get_tileset ()->getId () != GameMap::getTileset ()->getId ();
      }

    bool is_cityset_changed () const
      {
        return get_cityset ()->getId () != GameMap::getCityset ()->getId ();
      }

    bool is_shieldset_changed () const
      {
        return get_shieldset ()->getId () != GameMap::getShieldset ()->getId ();
      }

    bool is_armyset_changed () const
      {
        auto current_armysets = GameMap::getArmysets ();
        auto armysets = get_armysets ();
        for (guint32 i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
          {
            if (!current_armysets[i] || !armysets[i])
              continue;
            if (armysets[i]->getId () != current_armysets[i]->getId ())
              return true;
          }
        return false;
      }

    bool is_changed () const
      {
        return
          is_armyset_changed () || is_cityset_changed () ||
          is_shieldset_changed () || is_tileset_changed ();
      }

private:
    Gtk::Button *m_accept_button;
    Gtk::Box *m_tile_size_combobox;
    LwCombo *m_tile_size_combo;
    Gtk::Box *m_cityset_combobox;
    LwCombo *m_cityset_combo;
    Gtk::Box *m_shieldset_combobox;
    LwCombo *m_shieldset_combo;
    Gtk::Box *m_tileset_combobox;
    LwCombo *m_tileset_combo;
    Gtk::Box *m_player1_combobox;
    LwCombo *m_player1_combo;
    Gtk::Switch *m_make_same_switch;
    Gtk::Box *m_player2_combobox;
    LwCombo *m_player2_combo;
    Gtk::Box *m_player3_combobox;
    LwCombo *m_player3_combo;
    Gtk::Box *m_player4_combobox;
    LwCombo *m_player4_combo;
    Gtk::Box *m_player5_combobox;
    LwCombo *m_player5_combo;
    Gtk::Box *m_player6_combobox;
    LwCombo *m_player6_combo;
    Gtk::Box *m_player7_combobox;
    LwCombo *m_player7_combo;
    Gtk::Box *m_player8_combobox;
    LwCombo *m_player8_combo;
    Gtk::Box *m_neutral_combobox;
    LwCombo *m_neutral_combo;

    guint32 m_tile_size;
    guint32 m_cityset_theme;
    guint32 m_shieldset_theme;
    guint32 m_tileset_theme;
    guint32 m_player1_theme;
    bool m_make_same;
    guint32 m_player2_theme;
    guint32 m_player3_theme;
    guint32 m_player4_theme;
    guint32 m_player5_theme;
    guint32 m_player6_theme;
    guint32 m_player7_theme;
    guint32 m_player8_theme;
    guint32 m_neutral_theme;
    std::list<sigc::connection> m_connections;
    UndoMgr *m_umgr;


    void connect_signals ()
      {
        add_connection
          (m_tile_size_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new SwitchSetsUndoAction_TileSize
                 (m_tile_size, m_tileset_theme, m_player1_theme,
                  m_player2_theme, m_player3_theme, m_player4_theme,
                  m_player5_theme, m_player6_theme, m_player7_theme,
                  m_player8_theme, m_neutral_theme, m_cityset_theme));
              m_tile_size = m_tile_size_combo->get_active_row_number ();
              fill_tileset_combo ();
              fill_cityset_combo ();
              fill_armyset_combos ();
            }));

        add_connection
          (m_cityset_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new SwitchSetsUndoAction_CitySet (m_cityset_theme));
              m_cityset_theme = m_cityset_combo->get_active_row_number ();
            }));

        add_connection
          (m_shieldset_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new SwitchSetsUndoAction_ShieldSet (m_shieldset_theme));
              m_shieldset_theme = m_shieldset_combo->get_active_row_number ();
            }));

        add_connection
          (m_tileset_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new SwitchSetsUndoAction_TileSet (m_tileset_theme));
              m_tileset_theme = m_tileset_combo->get_active_row_number ();
            }));

        add_connection
          (m_player1_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new
                 SwitchSetsUndoAction_ArmySet (m_player1_theme, m_player2_theme,
                                               m_player3_theme, m_player4_theme,
                                               m_player5_theme, m_player6_theme,
                                               m_player7_theme, m_player8_theme,
                                               m_neutral_theme));
              m_player1_theme = m_player1_combo->get_active_row_number ();
              if (m_make_same_switch->get_active ())
                {
                  m_player2_theme = m_player1_theme;
                  m_player3_theme = m_player1_theme;
                  m_player4_theme = m_player1_theme;
                  m_player5_theme = m_player1_theme;
                  m_player6_theme = m_player1_theme;
                  m_player7_theme = m_player1_theme;
                  m_player8_theme = m_player1_theme;
                  m_neutral_theme = m_player1_theme;
                }
              update ();
            }));

        add_connection
          (m_make_same_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new
                 SwitchSetsUndoAction_MakeSame (m_make_same, m_player1_theme,
                                                m_player2_theme, m_player3_theme,
                                                m_player4_theme, m_player5_theme,
                                                m_player6_theme, m_player7_theme,
                                                m_player8_theme, m_neutral_theme));
              m_make_same = m_make_same_switch->get_active ();
              if (m_make_same)
                {
                  m_player2_theme = m_player1_theme;
                  m_player3_theme = m_player1_theme;
                  m_player4_theme = m_player1_theme;
                  m_player5_theme = m_player1_theme;
                  m_player6_theme = m_player1_theme;
                  m_player7_theme = m_player1_theme;
                  m_player8_theme = m_player1_theme;
                  m_neutral_theme = m_player1_theme;
                }
              update ();
            }));

        add_connection
          (m_player2_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new
                 SwitchSetsUndoAction_ArmySet (m_player1_theme, m_player2_theme,
                                               m_player3_theme, m_player4_theme,
                                               m_player5_theme, m_player6_theme,
                                               m_player7_theme, m_player8_theme,
                                               m_neutral_theme));
              m_player2_theme = m_player2_combo->get_active_row_number ();
            }));

        add_connection
          (m_player3_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new
                 SwitchSetsUndoAction_ArmySet (m_player1_theme, m_player2_theme,
                                               m_player3_theme, m_player4_theme,
                                               m_player5_theme, m_player6_theme,
                                               m_player7_theme, m_player8_theme,
                                               m_neutral_theme));
              m_player3_theme = m_player3_combo->get_active_row_number ();
            }));

        add_connection
          (m_player4_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new
                 SwitchSetsUndoAction_ArmySet (m_player1_theme, m_player2_theme,
                                               m_player3_theme, m_player4_theme,
                                               m_player5_theme, m_player6_theme,
                                               m_player7_theme, m_player8_theme,
                                               m_neutral_theme));
              m_player4_theme = m_player4_combo->get_active_row_number ();
            }));

        add_connection
          (m_player5_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new
                 SwitchSetsUndoAction_ArmySet (m_player1_theme, m_player2_theme,
                                               m_player3_theme, m_player4_theme,
                                               m_player5_theme, m_player6_theme,
                                               m_player7_theme, m_player8_theme,
                                               m_neutral_theme));
              m_player5_theme = m_player5_combo->get_active_row_number ();
            }));

        add_connection
          (m_player6_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new
                 SwitchSetsUndoAction_ArmySet (m_player1_theme, m_player2_theme,
                                               m_player3_theme, m_player4_theme,
                                               m_player5_theme, m_player6_theme,
                                               m_player7_theme, m_player8_theme,
                                               m_neutral_theme));
              m_player6_theme = m_player6_combo->get_active_row_number ();
            }));

        add_connection
          (m_player7_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new
                 SwitchSetsUndoAction_ArmySet (m_player1_theme, m_player2_theme,
                                               m_player3_theme, m_player4_theme,
                                               m_player5_theme, m_player6_theme,
                                               m_player7_theme, m_player8_theme,
                                               m_neutral_theme));
              m_player7_theme = m_player7_combo->get_active_row_number ();
            }));

        add_connection
          (m_player8_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new
                 SwitchSetsUndoAction_ArmySet (m_player1_theme, m_player2_theme,
                                               m_player3_theme, m_player4_theme,
                                               m_player5_theme, m_player6_theme,
                                               m_player7_theme, m_player8_theme,
                                               m_neutral_theme));
              m_player8_theme = m_player8_combo->get_active_row_number ();
            }));

        add_connection
          (m_neutral_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new
                 SwitchSetsUndoAction_ArmySet (m_player1_theme, m_player2_theme,
                                               m_player3_theme, m_player4_theme,
                                               m_player5_theme, m_player6_theme,
                                               m_player7_theme, m_player8_theme,
                                               m_neutral_theme));
              m_neutral_theme = m_neutral_combo->get_active_row_number ();
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

    void update ()
      {
        disconnect_signals ();

        m_tile_size_combo->set_active (m_tile_size);
        m_cityset_combo->set_active (m_cityset_theme);
        m_shieldset_combo->set_active (m_shieldset_theme);
        m_tileset_combo->set_active (m_tileset_theme);
        m_player1_combo->set_active (m_player1_theme);
        m_player2_combo->set_active (m_player2_theme);
        m_player3_combo->set_active (m_player3_theme);
        m_player4_combo->set_active (m_player4_theme);
        m_player5_combo->set_active (m_player5_theme);
        m_player6_combo->set_active (m_player6_theme);
        m_player7_combo->set_active (m_player7_theme);
        m_player8_combo->set_active (m_player8_theme);
        m_neutral_combo->set_active (m_neutral_theme);

        m_make_same_switch->set_active (m_make_same);

        m_player2_combo->set_sensitive (!m_make_same);
        m_player3_combo->set_sensitive (!m_make_same);
        m_player4_combo->set_sensitive (!m_make_same);
        m_player5_combo->set_sensitive (!m_make_same);
        m_player6_combo->set_sensitive (!m_make_same);
        m_player7_combo->set_sensitive (!m_make_same);
        m_player8_combo->set_sensitive (!m_make_same);
        m_neutral_combo->set_sensitive (!m_make_same);

        if (!Playerlist::instance ()->get (Shield::WHITE))
          m_player1_combo->set_sensitive (false);
        if (!Playerlist::instance ()->get (Shield::GREEN))
          m_player2_combo->set_sensitive (false);
        if (!Playerlist::instance ()->get (Shield::YELLOW))
          m_player3_combo->set_sensitive (false);
        if (!Playerlist::instance ()->get (Shield::DARK_BLUE))
          m_player4_combo->set_sensitive (false);
        if (!Playerlist::instance ()->get (Shield::ORANGE))
          m_player5_combo->set_sensitive (false);
        if (!Playerlist::instance ()->get (Shield::LIGHT_BLUE))
          m_player6_combo->set_sensitive (false);
        if (!Playerlist::instance ()->get (Shield::RED))
          m_player7_combo->set_sensitive (false);
        if (!Playerlist::instance ()->get (Shield::BLACK))
          m_player8_combo->set_sensitive (false);
        if (!Playerlist::instance ()->get (Shield::NEUTRAL))
          m_neutral_combo->set_sensitive (false);

        bool have_sets =
          m_player1_combo->get_text () != "" &&
          m_player2_combo->get_text () != "" &&
          m_player3_combo->get_text () != "" &&
          m_player4_combo->get_text () != "" &&
          m_player5_combo->get_text () != "" &&
          m_player6_combo->get_text () != "" &&
          m_player7_combo->get_text () != "" &&
          m_player8_combo->get_text () != "" &&
          m_neutral_combo->get_text () != "" &&
          m_tileset_combo->get_text () != "" &&
          m_cityset_combo->get_text () != "" &&
          m_shieldset_combo->get_text () != "";
        m_accept_button->set_sensitive (have_sets);

        connect_signals ();
      }

    void setup_cityset_combo ()
      {
        m_cityset_combo = Gtk::make_managed<LwCombo> ();
        m_cityset_combobox->append (*m_cityset_combo);
      }

    void fill_cityset_combo ()
      {
        m_cityset_combo->remove_all ();
        int counter = 0, default_id = -1;
        for (auto i :
             Citysetlist::instance ()->getValidIds (get_active_tile_size ()))
          {
            auto c = Citysetlist::instance ()->get (i);
            if (c->getId () == GameMap::getCityset ()->getId ())
              default_id = counter;
            m_cityset_combo->append (Glib::filename_to_utf8 (c->getName ()));
            counter++;
          }

        if (default_id >= 0)
          m_cityset_combo->set_active (default_id);
        else if (counter > 0)
          m_cityset_combo->set_active (0);
        m_cityset_combo->set_sensitive (counter > 0);
      }

    void setup_shieldset_combo ()
      {
        m_shieldset_combo = Gtk::make_managed<LwCombo> ();
        m_shieldset_combobox->append (*m_shieldset_combo);
      }

    void fill_shieldset_combo ()
      {
        m_shieldset_combo->remove_all ();
        int counter = 0, default_id = -1;
        for (auto i : Shieldsetlist::instance ()->getValidIds ())
          {
            auto s = Shieldsetlist::instance ()->get (i);
            if (s->getId () == GameMap::getShieldset ()->getId ())
              default_id = counter;
            m_shieldset_combo->append (Glib::filename_to_utf8 (s->getName ()));
            counter++;
          }
        if (default_id >= 0)
          m_shieldset_combo->set_active (default_id);
        m_shieldset_combo->set_sensitive (counter > 0);
      }

    void setup_tileset_combo ()
      {
        m_tileset_combo = Gtk::make_managed<LwCombo> ();
        m_tileset_combobox->append (*m_tileset_combo);
      }

    void fill_tileset_combo ()
      {
        m_tileset_combo->remove_all ();
        int counter = 0, default_id = -1;
        guint32 ts = get_active_tile_size ();
        for (auto i : Tilesetlist::instance ()->getValidIds (ts))
          {
            auto t = Tilesetlist::instance ()->get (i);
            if (t->getId () == GameMap::getTileset ()->getId ())
              default_id = counter;
            m_tileset_combo->append (Glib::filename_to_utf8 (t->getName ()));
            counter++;
          }

        if (default_id >= 0)
          m_tileset_combo->set_active (default_id);
        else if (counter > 0)
          m_tileset_combo->set_active (0);
        m_tileset_combo->set_sensitive (counter > 0);
      }

    void setup_armyset_combos ()
      {
        m_player1_combo = Gtk::make_managed<LwCombo> ();
        m_player1_combobox->append (*m_player1_combo);
        m_player2_combo = Gtk::make_managed<LwCombo> ();
        m_player2_combobox->append (*m_player2_combo);
        m_player3_combo = Gtk::make_managed<LwCombo> ();
        m_player3_combobox->append (*m_player3_combo);
        m_player4_combo = Gtk::make_managed<LwCombo> ();
        m_player4_combobox->append (*m_player4_combo);
        m_player5_combo = Gtk::make_managed<LwCombo> ();
        m_player5_combobox->append (*m_player5_combo);
        m_player6_combo = Gtk::make_managed<LwCombo> ();
        m_player6_combobox->append (*m_player6_combo);
        m_player7_combo = Gtk::make_managed<LwCombo> ();
        m_player7_combobox->append (*m_player7_combo);
        m_player8_combo = Gtk::make_managed<LwCombo> ();
        m_player8_combobox->append (*m_player8_combo);
        m_neutral_combo = Gtk::make_managed<LwCombo> ();
        m_neutral_combobox->append (*m_neutral_combo);
      }

    void fill_armyset_combo (LwCombo *combo, Shield::Color shield)
      {
        combo->remove_all ();
        int counter = 0, default_id = -1;
        Player *p = Playerlist::instance ()->get (shield);
        for (auto i :
             Armysetlist::instance ()->getValidIds (get_active_tile_size ()))
          {
            auto a = Armysetlist::instance ()->get (i);
            if (p && a->getId () == p->getArmyset ())
              default_id = counter;
            combo->append (Glib::filename_to_utf8 (a->getName ()));
            counter++;
          }

        if (default_id >= 0)
          combo->set_active (default_id);
        else if (counter > 0)
          combo->set_active (0);
        combo->set_sensitive (counter > 0);
      }

    void fill_armyset_combos ()
      {
        fill_armyset_combo (m_player1_combo, Shield::WHITE);
        fill_armyset_combo (m_player2_combo, Shield::GREEN);
        fill_armyset_combo (m_player3_combo, Shield::YELLOW);
        fill_armyset_combo (m_player4_combo, Shield::DARK_BLUE);
        fill_armyset_combo (m_player5_combo, Shield::ORANGE);
        fill_armyset_combo (m_player6_combo, Shield::LIGHT_BLUE);
        fill_armyset_combo (m_player7_combo, Shield::RED);
        fill_armyset_combo (m_player8_combo, Shield::BLACK);
        fill_armyset_combo (m_neutral_combo, Shield::NEUTRAL);
      }

    void setup_tile_size_combo ()
      {
        m_tile_size_combo = Gtk::make_managed<LwCombo> ();
        m_tile_size_combobox->append (*m_tile_size_combo);
      }

    void fill_tile_size_combo ()
      {
        std::list<guint32> sizes;
        Tilesetlist::instance ()->getSizes (sizes);
        Citysetlist::instance ()->getSizes (sizes);
        Armysetlist::instance ()->getSizes (sizes);
        int counter = 0, default_id = -1;
        for (auto ts : sizes)
          {
            m_tile_size_combo->append (String::ucompose ("%1x%1", ts));
            if (ts == Tileset::getDefaultTileSize ())
              default_id = counter;
            counter++;
          }
        if (default_id >= 0)
          m_tile_size_combo->set_active (default_id);
        m_tile_size_combo->set_sensitive (counter > 0);
      }

    guint32 get_active_tile_size () const
      {
        return (guint32) std::stoi (m_tile_size_combo->get_active_text ());
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &SwitchSetsDialog::execute_action));

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
        SwitchSetsUndoAction *action = dynamic_cast<SwitchSetsUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case SwitchSetsUndoAction::TILESET:
              {
                auto a = dynamic_cast<SwitchSetsUndoAction_TileSet*>(action);
                out = new SwitchSetsUndoAction_TileSet (m_tileset_theme);
                m_tileset_theme = a->get_index ();
              }
            break;

          case SwitchSetsUndoAction::ARMYSET:
              {
                auto a = dynamic_cast<SwitchSetsUndoAction_ArmySet*>(action);
                out = new
                  SwitchSetsUndoAction_ArmySet (m_player1_theme, m_player2_theme,
                                                m_player3_theme, m_player4_theme,
                                                m_player5_theme, m_player6_theme,
                                                m_player7_theme, m_player8_theme,
                                                m_neutral_theme);
                m_player1_theme = a->get_player1_row ();
                m_player2_theme = a->get_player2_row ();
                m_player3_theme = a->get_player3_row ();
                m_player4_theme = a->get_player4_row ();
                m_player5_theme = a->get_player5_row ();
                m_player6_theme = a->get_player6_row ();
                m_player7_theme = a->get_player7_row ();
                m_player8_theme = a->get_player8_row ();
                m_neutral_theme = a->get_neutral_row ();
              }
            break;

          case SwitchSetsUndoAction::CITYSET:
              {
                auto a = dynamic_cast<SwitchSetsUndoAction_CitySet*>(action);
                out = new SwitchSetsUndoAction_CitySet (m_cityset_theme);
                m_cityset_theme = a->get_index ();
              }
            break;

          case SwitchSetsUndoAction::SHIELDSET:
              {
                auto a = dynamic_cast<SwitchSetsUndoAction_ShieldSet*>(action);
                out = new SwitchSetsUndoAction_ShieldSet (m_shieldset_theme);
                m_shieldset_theme = a->get_index ();
              }
            break;

          case SwitchSetsUndoAction::TILE_SIZE:
              {
                auto a = dynamic_cast<SwitchSetsUndoAction_TileSize*>(action);
                out = new SwitchSetsUndoAction_TileSize (m_tile_size,
                                                         m_tileset_theme,
                                                         m_player1_theme,
                                                         m_player2_theme,
                                                         m_player3_theme,
                                                         m_player4_theme,
                                                         m_player5_theme,
                                                         m_player6_theme,
                                                         m_player7_theme,
                                                         m_player8_theme,
                                                         m_neutral_theme,
                                                         m_cityset_theme);
                m_tile_size = a->get_tile_size_row ();
                fill_tileset_combo ();
                fill_cityset_combo ();
                fill_armyset_combos ();
                m_tileset_theme = a->get_tileset_row ();
                m_player1_theme = a->get_player1_armyset_row ();
                m_player2_theme = a->get_player2_armyset_row ();
                m_player3_theme = a->get_player3_armyset_row ();
                m_player4_theme = a->get_player4_armyset_row ();
                m_player5_theme = a->get_player5_armyset_row ();
                m_player6_theme = a->get_player6_armyset_row ();
                m_player7_theme = a->get_player7_armyset_row ();
                m_player8_theme = a->get_player8_armyset_row ();
                m_neutral_theme = a->get_neutral_armyset_row ();
                m_cityset_theme = a->get_cityset_row ();
              }
            break;

          case SwitchSetsUndoAction::MAKE_SAME:
              {
                auto a = dynamic_cast<SwitchSetsUndoAction_MakeSame*>(action);
                out = new
                  SwitchSetsUndoAction_MakeSame
                  (m_make_same, m_player1_theme, m_player2_theme,
                   m_player3_theme, m_player4_theme, m_player5_theme,
                   m_player6_theme, m_player7_theme, m_player8_theme,
                   m_neutral_theme);

                m_make_same = a->get_value ();
                m_player1_theme = a->get_player1_row ();
                m_player2_theme = a->get_player2_row ();
                m_player3_theme = a->get_player3_row ();
                m_player4_theme = a->get_player4_row ();
                m_player5_theme = a->get_player5_row ();
                m_player6_theme = a->get_player6_row ();
                m_player7_theme = a->get_player7_row ();
                m_player8_theme = a->get_player8_row ();
                m_neutral_theme = a->get_neutral_row ();
              }
            break;
          }
        return out;
      }

};
#endif
