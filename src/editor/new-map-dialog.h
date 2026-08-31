//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2012, 2014, 2015, 2017, 2020, 2021,
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
#ifndef NEW_MAP_DIALOG_H
#define NEW_MAP_DIALOG_H
#include "army-set-list.h"
#include "city-set-list.h"
#include "shield-set-list.h"
#include "tile-set-list.h"
#include "new-map-undo-actions.h"

class NewMapDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "new-map.ui";
      }

    enum
      {
        MAP_SIZE_NORMAL = 0,
        MAP_SIZE_SMALL,
        MAP_SIZE_TINY,
        MAP_SIZE_CUSTOM
      };

    NewMapDialog (BaseObjectType* o,
                  const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_inhibit_scales = false;
        m_percentages.resize (6);
        m_accept_button = load <Gtk::Button> ("accept_button");
        m_grass_scale = load <Gtk::Scale> ("grass_scale");
        m_water_scale = load <Gtk::Scale> ("water_scale");
        m_forest_scale = load <Gtk::Scale> ("forest_scale");
        m_swamp_scale = load <Gtk::Scale> ("swamp_scale");
        m_hills_scale = load <Gtk::Scale> ("hills_scale");
        m_mountains_scale = load <Gtk::Scale> ("mountains_scale");
        m_cities_scale = load <Gtk::Scale> ("cities_scale");
        m_ruins_scale = load <Gtk::Scale> ("ruins_scale");
        m_temples_scale = load <Gtk::Scale> ("temples_scale");
        m_signposts_scale = load <Gtk::Scale> ("signposts_scale");
        m_stones_scale = load <Gtk::Scale> ("stones_scale");
        m_size_combobox = load <Gtk::Box> ("size_combobox");
        m_width_spinbutton = load <Gtk::SpinButton> ("width_spinbutton");
        m_height_spinbutton = load <Gtk::SpinButton> ("height_spinbutton");
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
        m_roads_switch = load <Gtk::Switch> ("roads_switch");
        m_names_switch = load <Gtk::Switch> ("names_switch");
        m_stone_road_spinbutton =
          load <Gtk::SpinButton> ("stone_road_spinbutton");
        m_fill_style_combobox = load <Gtk::Box> ("fill_style_combobox");
        m_player1_switch = load <Gtk::Switch> ("player1_switch");
        m_player2_switch = load <Gtk::Switch> ("player2_switch");
        m_player3_switch = load <Gtk::Switch> ("player3_switch");
        m_player4_switch = load <Gtk::Switch> ("player4_switch");
        m_player5_switch = load <Gtk::Switch> ("player5_switch");
        m_player6_switch = load <Gtk::Switch> ("player6_switch");
        m_player7_switch = load <Gtk::Switch> ("player7_switch");
        m_player8_switch = load <Gtk::Switch> ("player8_switch");
      }
    
    ~NewMapDialog ()
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
        setup_size_combo ();
        setup_fill_style_combo ();

        fill_tile_size_combo ();
        fill_cityset_combo ();
        fill_shieldset_combo ();
        fill_tileset_combo ();
        fill_armyset_combos ();

        fill_size_combo ();
        fill_fill_style_combo ();

        m_names_switch->set_active (true);
        m_roads_switch->set_active (true);
        m_player1_switch->set_active (true);
        m_player2_switch->set_active (true);
        m_player3_switch->set_active (true);
        m_player4_switch->set_active (true);
        m_player5_switch->set_active (true);
        m_player6_switch->set_active (true);
        m_player7_switch->set_active (true);
        m_player8_switch->set_active (true);

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

        m_grass = (guint32) m_grass_scale->get_value ();
        m_water = (guint32) m_water_scale->get_value ();
        m_forest = (guint32) m_forest_scale->get_value ();
        m_swamp = (guint32) m_swamp_scale->get_value ();
        m_hills = (guint32) m_hills_scale->get_value ();
        m_mountains = (guint32) m_mountains_scale->get_value ();
        m_cities = (guint32) m_cities_scale->get_value ();
        m_ruins = (guint32) m_ruins_scale->get_value ();
        m_temples = (guint32) m_temples_scale->get_value ();
        m_signposts = (guint32) m_signposts_scale->get_value ();
        m_stones = (guint32) m_stones_scale->get_value ();
        m_size = m_size_combo->get_active_row_number ();
        m_width = m_width_spinbutton->get_value_as_int ();
        m_height = m_height_spinbutton->get_value_as_int ();
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
        m_roads = m_roads_switch->get_active ();
        m_names = m_names_switch->get_active ();
        m_stone_road = m_stone_road_spinbutton->get_value_as_int ();
        m_fill_style = m_fill_style_combo->get_active_row_number ();
        m_player1 = m_player1_switch->get_active ();
        m_player2 = m_player2_switch->get_active ();
        m_player3 = m_player3_switch->get_active ();
        m_player4 = m_player4_switch->get_active ();
        m_player5 = m_player5_switch->get_active ();
        m_player6 = m_player6_switch->get_active ();
        m_player7 = m_player7_switch->get_active ();
        m_player8 = m_player8_switch->get_active ();

        setup_undo ();

        update_map_size_buildings ();

        update ();
      }

    guint32 get_width () const
      {
        return m_width;
      }

    guint32 get_height () const
      {
        return m_height;
      }

    std::vector<bool> get_players () const
      {
        std::vector<bool> players;
        players.push_back (m_player1);
        players.push_back (m_player2);
        players.push_back (m_player3);
        players.push_back (m_player4);
        players.push_back (m_player5);
        players.push_back (m_player6);
        players.push_back (m_player7);
        players.push_back (m_player8);
        return players;
      }

    std::vector<Glib::ustring> get_armyset_themes ()
      {
        std::vector<Glib::ustring> armysets;
        int ts = get_active_tile_size ();
        armysets.push_back
          (Armysetlist::instance ()->getSetDir 
           (Glib::filename_from_utf8 (m_player1_combo->get_text ()), ts));
        armysets.push_back
          (Armysetlist::instance ()->getSetDir 
           (Glib::filename_from_utf8 (m_player2_combo->get_text ()), ts));
        armysets.push_back
          (Armysetlist::instance ()->getSetDir 
           (Glib::filename_from_utf8 (m_player3_combo->get_text ()), ts));
        armysets.push_back
          (Armysetlist::instance ()->getSetDir 
           (Glib::filename_from_utf8 (m_player4_combo->get_text ()), ts));
        armysets.push_back
          (Armysetlist::instance ()->getSetDir 
           (Glib::filename_from_utf8 (m_player5_combo->get_text ()), ts));
        armysets.push_back
          (Armysetlist::instance ()->getSetDir 
           (Glib::filename_from_utf8 (m_player6_combo->get_text ()), ts));
        armysets.push_back
          (Armysetlist::instance ()->getSetDir 
           (Glib::filename_from_utf8 (m_player7_combo->get_text ()), ts));
        armysets.push_back
          (Armysetlist::instance ()->getSetDir 
           (Glib::filename_from_utf8 (m_player8_combo->get_text ()), ts));
        armysets.push_back
          (Armysetlist::instance ()->getSetDir 
           (Glib::filename_from_utf8 (m_neutral_combo->get_text ()), ts));

        return armysets;
      }

    std::string get_cityset_theme () const
      {
        return Citysetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_cityset_combo->get_active_text ()),
           get_active_tile_size ());
      }

    std::string get_shieldset_theme () const
      {
        return Shieldsetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_shieldset_combo->get_active_text ()));
      }

    std::string get_tileset_theme () const
      {
        return Tilesetlist::instance ()->getSetDir 
          (Glib::filename_from_utf8 (m_tileset_combo->get_active_text ()),
           get_active_tile_size ());
      }

    bool get_random_names () const
      {
        return m_names;
      }

    bool get_random_roads () const
      {
        return m_roads;
      }

    guint32 get_number_of_standing_stones () const
      {
        return m_stones;
      }

    guint32 get_standing_stone_on_road_chance () const
      {
        return m_stone_road;
      }

    bool get_filled_map () 
      {
        auto tileset = get_active_tileset ();
        bool generated = tileset && m_fill_style == (guint32) tileset->size ();
        return !generated;
      }

    Tile::Type get_filled_terrain_tile ()
      {
        auto tileset = get_active_tileset ();

        if (m_fill_style >= tileset->size ())
          return Tile::GRASS;

        auto tile = (*tileset)[m_fill_style];
        return tile->getType ();
      }

    GameParameters get_map ()
      {
        CreateScenarioRandomize random;
        GameParameters g;
        GameParameters::Player p;
        p.type = GameParameters::Player::HUMAN;
        g.players.clear ();
        add_player (g, 0, Shield::WHITE, random);
        add_player (g, 1, Shield::GREEN, random);
        add_player (g, 2, Shield::YELLOW, random);
        add_player (g, 3, Shield::DARK_BLUE, random);
        add_player (g, 4, Shield::ORANGE, random);
        add_player (g, 5, Shield::LIGHT_BLUE, random);
        add_player (g, 6, Shield::RED, random);
        add_player (g, 7, Shield::BLACK, random);

        g.map_path = "";
        g.map.width = m_width;
        g.map.height = m_height;
        g.map.ruins = m_ruins;
        g.map.temples = m_temples;
        g.map.signposts = m_signposts;

        g.map.grass = m_grass;
        g.map.water = m_water;
        g.map.forest = m_forest;
        g.map.hills = m_hills;
        g.map.mountains = m_mountains;
        g.map.swamp = m_swamp;

        g.map.cities = m_cities;

        auto army_themes = get_armyset_themes ();
        for (guint32  i = 0; i < MAX_PLAYERS + 1; i++)
          g.army_theme[i] = army_themes[i];

        g.city_theme = get_cityset_theme ();
        g.shield_theme = get_shieldset_theme ();
        g.tile_theme = get_tileset_theme ();

        g.see_opponents_stacks = GameScenarioOptions::s_see_opponents_stacks;
        g.see_opponents_production = GameScenarioOptions::s_see_opponents_production;
        g.play_with_quests = GameScenarioOptions::s_play_with_quests;
        g.hidden_map = GameScenarioOptions::s_hidden_map;
        g.neutral_cities = GameScenarioOptions::s_neutral_cities;
        g.razing_cities = GameScenarioOptions::s_razing_cities;
        g.diplomacy = GameScenarioOptions::s_diplomacy;
        g.random_turns = GameScenarioOptions::s_random_turns;
        g.quick_start = Configuration::s_quick_start;
        g.intense_combat = GameScenarioOptions::s_intense_combat;
        g.military_advisor = GameScenarioOptions::s_military_advisor;
        g.cities_can_produce_allies =  GameScenarioOptions::s_cities_can_produce_allies;
        g.cusp_of_war = GameScenarioOptions::s_cusp_of_war;
        g.vectoring_mode = GameScenarioOptions::s_vectoring_mode;
        g.build_production_mode = GameScenarioOptions::s_build_production_mode;
        g.sacking_mode = GameScenarioOptions::s_sacking_mode;

        g.name = _("Autogenerated");
        random.cleanup ();
        return g;
      }

private:
    Gtk::Button *m_accept_button;
    Gtk::Scale *m_grass_scale;
    Gtk::Scale *m_water_scale;
    Gtk::Scale *m_forest_scale;
    Gtk::Scale *m_swamp_scale;
    Gtk::Scale *m_hills_scale;
    Gtk::Scale *m_mountains_scale;
    Gtk::Scale *m_cities_scale;
    Gtk::Scale *m_ruins_scale;
    Gtk::Scale *m_temples_scale;
    Gtk::Scale *m_signposts_scale;
    Gtk::Scale *m_stones_scale;
    Gtk::Box *m_size_combobox;
    LwCombo *m_size_combo;
    Gtk::SpinButton *m_width_spinbutton;
    Gtk::SpinButton *m_height_spinbutton;
    Gtk::Switch *m_player1_switch;
    Gtk::Switch *m_player2_switch;
    Gtk::Switch *m_player3_switch;
    Gtk::Switch *m_player4_switch;
    Gtk::Switch *m_player5_switch;
    Gtk::Switch *m_player6_switch;
    Gtk::Switch *m_player7_switch;
    Gtk::Switch *m_player8_switch;
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
    Gtk::Switch *m_roads_switch;
    Gtk::Switch *m_names_switch;
    Gtk::SpinButton *m_stone_road_spinbutton;
    Gtk::Box *m_fill_style_combobox;
    LwCombo *m_fill_style_combo;

    guint32 m_grass;
    guint32 m_water;
    guint32 m_forest;
    guint32 m_swamp;
    guint32 m_hills;
    guint32 m_mountains;
    guint32 m_cities;
    guint32 m_ruins;
    guint32 m_temples;
    guint32 m_signposts;
    guint32 m_stones;
    guint32 m_size;
    guint32 m_width;
    guint32 m_height;
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
    bool m_roads;
    bool m_names;
    guint32 m_stone_road;
    guint32 m_fill_style;
    bool m_player1;
    bool m_player2;
    bool m_player3;
    bool m_player4;
    bool m_player5;
    bool m_player6;
    bool m_player7;
    bool m_player8;
    std::list<sigc::connection> m_connections;
    UndoMgr *m_umgr;
    std::vector<double> m_percentages;
    bool m_inhibit_scales;

    void update_terrain ()
      {
        m_grass = (guint32) m_grass_scale->get_value ();
        m_water = (guint32) m_water_scale->get_value ();
        m_swamp = (guint32) m_swamp_scale->get_value ();
        m_forest = (guint32) m_forest_scale->get_value ();
        m_hills = (guint32) m_hills_scale->get_value ();
        m_mountains = (guint32) m_mountains_scale->get_value ();
      }

    void connect_signals ()
      {
        add_connection
          (m_grass_scale->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new NewMapUndoAction_Terrain (m_grass, m_water, m_swamp,
                                               m_forest, m_hills, m_mountains));
              on_scale_changed (Tile::GRASS);
              update_terrain ();
            })); 
        {
          auto gesture = Gtk::GestureClick::create ();
          gesture->set_button (0);
          add_connection
            (gesture->signal_pressed ().connect
             ([this] (int, double, double)
              {
                take_percentages ();
              }));
          m_grass_scale->add_controller (gesture);
        }

        add_connection
          (m_water_scale->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new NewMapUndoAction_Terrain (m_grass, m_water, m_swamp,
                                               m_forest, m_hills, m_mountains));
              on_scale_changed (Tile::WATER);
              update_terrain ();
            })); 
        {
          auto gesture = Gtk::GestureClick::create ();
          gesture->set_button (0);
          add_connection
            (gesture->signal_pressed ().connect
             ([this] (int, double, double)
              {
                take_percentages ();
             }));
          m_water_scale->add_controller (gesture);
        }

        add_connection
          (m_forest_scale->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new NewMapUndoAction_Terrain (m_grass, m_water, m_swamp,
                                               m_forest, m_hills, m_mountains));
              on_scale_changed (Tile::FOREST);
              update_terrain ();
            })); 
        {
          auto gesture = Gtk::GestureClick::create ();
          gesture->set_button (0);
          add_connection
            (gesture->signal_pressed ().connect
             ([this] (int, double, double)
              {
                take_percentages ();
              }));
          m_forest_scale->add_controller (gesture);
        }

        add_connection
          (m_swamp_scale->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new NewMapUndoAction_Terrain (m_grass, m_water, m_swamp,
                                               m_forest, m_hills, m_mountains));
              on_scale_changed (Tile::SWAMP);
              update_terrain ();
            })); 
        {
          auto gesture = Gtk::GestureClick::create ();
          gesture->set_button (0);
          add_connection
            (gesture->signal_pressed ().connect
             ([this] (int, double, double)
              {
                take_percentages ();
              }));
          m_swamp_scale->add_controller (gesture);
        }

        add_connection
          (m_hills_scale->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new NewMapUndoAction_Terrain (m_grass, m_water, m_swamp,
                                               m_forest, m_hills, m_mountains));
              on_scale_changed (Tile::HILLS);
              update_terrain ();
            })); 
        {
          auto gesture = Gtk::GestureClick::create ();
          gesture->set_button (0);
          add_connection
            (gesture->signal_pressed ().connect
             ([this] (int, double, double)
              {
                take_percentages ();
              }));
          m_hills_scale->add_controller (gesture);
        }

        add_connection
          (m_mountains_scale->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new NewMapUndoAction_Terrain (m_grass, m_water, m_swamp,
                                               m_forest, m_hills, m_mountains));
              on_scale_changed (Tile::MOUNTAIN);
              update_terrain ();
            })); 
        {
          auto gesture = Gtk::GestureClick::create ();
          gesture->set_button (0);
          add_connection
            (gesture->signal_pressed ().connect
             ([this] (int, double, double)
              {
                take_percentages ();
              }));
          m_mountains_scale->add_controller (gesture);
        }

        add_connection
          (m_cities_scale->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Cities (m_cities));
              m_cities = (guint32) m_cities_scale->get_value ();
            }));

        add_connection
          (m_ruins_scale->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Ruins (m_ruins));
              m_ruins = (guint32) m_ruins_scale->get_value ();
            }));

        add_connection
          (m_temples_scale->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Temples (m_temples));
              m_temples = (guint32) m_temples_scale->get_value ();
            }));

        add_connection
          (m_signposts_scale->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Signposts (m_signposts));
              m_signposts = (guint32) m_signposts_scale->get_value ();
            }));

        add_connection
          (m_stones_scale->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Stones (m_stones));
              m_stones = (guint32) m_stones_scale->get_value ();
            }));

        add_connection
          (m_size_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new NewMapUndoAction_MapSize (m_size, m_cities, m_ruins,
                                               m_temples, m_signposts,
                                               m_stones, m_width, m_height));
              m_size = m_size_combo->get_active_row_number ();

              update_map_size_buildings ();
            }));

        add_connection
          (m_width_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Width (m_width));
              m_width = m_width_spinbutton->get_value_as_int ();
            }));

        add_connection
          (m_height_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Height (m_height));
              m_height = m_width_spinbutton->get_value_as_int ();
            }));

        add_connection
          (m_player1_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Player (0, m_player1));
              m_player1 = m_player1_switch->get_active ();
              if (!m_player1)
                m_make_same_switch->set_active (true);
              update ();
            }));

        add_connection
          (m_player2_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Player (1, m_player2));
              m_player2 = m_player2_switch->get_active ();
              update ();
            }));

        add_connection
          (m_player3_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Player (2, m_player3));
              m_player3 = m_player3_switch->get_active ();
              update ();
            }));

        add_connection
          (m_player4_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Player (3, m_player4));
              m_player4 = m_player4_switch->get_active ();
              update ();
            }));

        add_connection
          (m_player5_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Player (4, m_player5));
              m_player5 = m_player5_switch->get_active ();
              update ();
            }));

        add_connection
          (m_player6_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Player (5, m_player6));
              m_player6 = m_player6_switch->get_active ();
              update ();
            }));

        add_connection
          (m_player7_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Player (6, m_player7));
              m_player7 = m_player7_switch->get_active ();
              update ();
            }));

        add_connection
          (m_player8_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_Player (7, m_player8));
              m_player8 = m_player8_switch->get_active ();
              update ();
            }));

        add_connection
          (m_tile_size_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new NewMapUndoAction_TileSize
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
              m_umgr->add (new NewMapUndoAction_CitySet (m_cityset_theme));
              m_cityset_theme = m_cityset_combo->get_active_row_number ();
            }));

        add_connection
          (m_shieldset_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_ShieldSet (m_shieldset_theme));
              m_shieldset_theme = m_shieldset_combo->get_active_row_number ();
            }));

        add_connection
          (m_tileset_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_TileSet (m_tileset_theme));
              m_tileset_theme = m_tileset_combo->get_active_row_number ();
            }));

        add_connection
          (m_player1_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new
                 NewMapUndoAction_ArmySet (m_player1_theme, m_player2_theme,
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
                 NewMapUndoAction_MakeSame (m_make_same, m_player1_theme,
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
                 NewMapUndoAction_ArmySet (m_player1_theme, m_player2_theme,
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
                 NewMapUndoAction_ArmySet (m_player1_theme, m_player2_theme,
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
                 NewMapUndoAction_ArmySet (m_player1_theme, m_player2_theme,
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
                 NewMapUndoAction_ArmySet (m_player1_theme, m_player2_theme,
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
                 NewMapUndoAction_ArmySet (m_player1_theme, m_player2_theme,
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
                 NewMapUndoAction_ArmySet (m_player1_theme, m_player2_theme,
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
                 NewMapUndoAction_ArmySet (m_player1_theme, m_player2_theme,
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
                 NewMapUndoAction_ArmySet (m_player1_theme, m_player2_theme,
                                           m_player3_theme, m_player4_theme,
                                           m_player5_theme, m_player6_theme,
                                           m_player7_theme, m_player8_theme,
                                           m_neutral_theme));
              m_neutral_theme = m_neutral_combo->get_active_row_number ();
            }));

        add_connection
          (m_roads_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_RandomRoads (m_roads));
              m_roads = m_roads_switch->get_active ();
            }));

        add_connection
          (m_names_switch->property_active ().signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_RandomNames (m_names));
              m_names = m_names_switch->get_active ();
            }));

        add_connection
          (m_stone_road_spinbutton->signal_value_changed ().connect
           ([this] ()
            {
              m_umgr->add
                (new NewMapUndoAction_StoneRoadChance (m_stone_road));
              m_stone_road = m_stone_road_spinbutton->get_value_as_int ();
            }));

        add_connection
          (m_fill_style_combo->signal_changed ().connect
           ([this] ()
            {
              m_umgr->add (new NewMapUndoAction_FillStyle (m_fill_style));
              m_fill_style = m_fill_style_combo->get_active_row_number ();
              update ();
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

        m_grass_scale->set_value (m_grass);
        m_water_scale->set_value (m_water);
        m_forest_scale->set_value (m_forest);
        m_swamp_scale->set_value (m_swamp);
        m_hills_scale->set_value (m_hills);
        m_mountains_scale->set_value (m_mountains);
        m_cities_scale->set_value (m_cities);
        m_ruins_scale->set_value (m_ruins);
        m_temples_scale->set_value (m_temples);
        m_signposts_scale->set_value (m_signposts);
        m_stones_scale->set_value (m_stones);
        m_size_combo->set_active (m_size);
        m_width_spinbutton->set_value (m_width);
        m_height_spinbutton->set_value (m_height);
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
        m_roads_switch->set_active (m_roads);
        m_names_switch->set_active (m_names);
        m_stone_road_spinbutton->set_value (m_stone_road);
        m_fill_style_combo->set_active (m_fill_style);
        m_player1_switch->set_active (m_player1);
        m_player2_switch->set_active (m_player2);
        m_player3_switch->set_active (m_player3);
        m_player4_switch->set_active (m_player4);
        m_player5_switch->set_active (m_player5);
        m_player6_switch->set_active (m_player6);
        m_player7_switch->set_active (m_player7);
        m_player8_switch->set_active (m_player8);

        auto tileset = get_active_tileset ();
        bool generated = tileset && m_fill_style == (guint32) tileset->size ();
        m_roads_switch->set_sensitive (generated);
        m_names_switch->set_sensitive (generated);
        m_stone_road_spinbutton->set_sensitive (generated);

        m_make_same_switch->set_active (m_make_same);

        m_player1_combo->set_sensitive (m_player1);
        m_player2_combo->set_sensitive (!m_make_same && m_player2);
        m_player3_combo->set_sensitive (!m_make_same && m_player3);
        m_player4_combo->set_sensitive (!m_make_same && m_player4);
        m_player5_combo->set_sensitive (!m_make_same && m_player5);
        m_player6_combo->set_sensitive (!m_make_same && m_player6);
        m_player7_combo->set_sensitive (!m_make_same && m_player7);
        m_player8_combo->set_sensitive (!m_make_same && m_player8);
        m_neutral_combo->set_sensitive (!m_make_same);

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

        int player_count = 0;
        if (m_player1_switch->get_active ())
          player_count++;
        if (m_player2_switch->get_active ())
          player_count++;
        if (m_player3_switch->get_active ())
          player_count++;
        if (m_player4_switch->get_active ())
          player_count++;
        if (m_player5_switch->get_active ())
          player_count++;
        if (m_player6_switch->get_active ())
          player_count++;
        if (m_player7_switch->get_active ())
          player_count++;
        if (m_player8_switch->get_active ())
          player_count++;
        bool have_players = player_count >= 2;
        m_accept_button->set_sensitive (have_sets && have_players);

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
            if (c->getId () == 1)
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
            if (s->getId () == 1)
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
        for (auto i :
             Tilesetlist::instance ()->getValidIds (get_active_tile_size ()))
          {
            auto t = Tilesetlist::instance ()->get (i);
            if (t->getId () == 1)
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

    void fill_armyset_combo (LwCombo *combo)
      {
        combo->remove_all ();
        int counter = 0, default_id = -1;
        for (auto i :
             Armysetlist::instance ()->getValidIds (get_active_tile_size ()))
          {
            auto a = Armysetlist::instance ()->get (i);
            if (a->getId () == 1)
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
        fill_armyset_combo (m_player1_combo);
        fill_armyset_combo (m_player2_combo);
        fill_armyset_combo (m_player3_combo);
        fill_armyset_combo (m_player4_combo);
        fill_armyset_combo (m_player5_combo);
        fill_armyset_combo (m_player6_combo);
        fill_armyset_combo (m_player7_combo);
        fill_armyset_combo (m_player8_combo);
        fill_armyset_combo (m_neutral_combo);
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

    void setup_size_combo ()
      {
        m_size_combo = Gtk::make_managed<LwCombo> ();
        m_size_combobox->append (*m_size_combo);
      }

    void fill_size_combo ()
      {
        m_size_combo->remove_all ();
        m_size_combo->append (_("Normal"));
        m_size_combo->append (_("Small"));
        m_size_combo->append (_("Tiny"));
        m_size_combo->append (_("Custom"));
        m_size_combo->set_active (0);
      }

    void setup_fill_style_combo ()
      {
        m_fill_style_combo = Gtk::make_managed<LwCombo> ();
        m_fill_style_combobox->append (*m_fill_style_combo);
      }

    Tileset *get_active_tileset ()
      {
        auto theme = Tilesetlist::instance ()->getSetDir
          (Glib::filename_from_utf8 (m_tileset_combo->get_active_text ()),
           get_active_tile_size ());
        return Tilesetlist::instance ()->get (theme);
      }

    void fill_fill_style_combo ()
      {
        m_fill_style_combo->remove_all ();

        int i = 0;
        for (auto t : *get_active_tileset ())
          {
            m_fill_style_combo->append (t->getName ());
            i++;
          }
        m_fill_style_combo->append (_("Autogenerated"));
        m_fill_style_combo->set_active (i);
      }

    guint32 get_active_tile_size () const
      {
        return (guint32) std::stoi (m_tile_size_combo->get_active_text ());
      }

    void setup_undo ()
      {
        m_umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
        m_umgr->signal_execute ().connect
          (sigc::mem_fun (*this, &NewMapDialog::execute_action));

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
        NewMapUndoAction *action = dynamic_cast<NewMapUndoAction*>(action2);
        UndoAction *out = NULL;

        switch (action->get_type ())
          {
          case NewMapUndoAction::TERRAIN:
              {
                auto a = dynamic_cast<NewMapUndoAction_Terrain*>(action);
                out = new NewMapUndoAction_Terrain (m_grass, m_water, m_swamp,
                                                    m_forest, m_hills,
                                                    m_mountains);
                m_grass = a->get_grass_value ();
                m_water = a->get_water_value ();
                m_swamp = a->get_swamp_value ();
                m_forest = a->get_forest_value ();
                m_hills = a->get_hills_value ();
                m_mountains = a->get_mountains_value ();
              }
            break;

          case NewMapUndoAction::CITIES:
              {
                auto a = dynamic_cast<NewMapUndoAction_Cities*>(action);
                out = new NewMapUndoAction_Cities (m_cities);
                m_cities = a->get_value ();
              }
            break;

          case NewMapUndoAction::RUINS:
              {
                auto a = dynamic_cast<NewMapUndoAction_Ruins*>(action);
                out = new NewMapUndoAction_Ruins (m_ruins);
                m_ruins = a->get_value ();
              }
            break;

          case NewMapUndoAction::TEMPLES:
              {
                auto a = dynamic_cast<NewMapUndoAction_Temples*>(action);
                out = new NewMapUndoAction_Temples (m_temples);
                m_temples = a->get_value ();
              }
            break;

          case NewMapUndoAction::SIGNPOSTS:
              {
                auto a = dynamic_cast<NewMapUndoAction_Signposts*>(action);
                out = new NewMapUndoAction_Signposts (m_signposts);
                m_signposts = a->get_value ();
              }
            break;

          case NewMapUndoAction::STONES:
              {
                auto a = dynamic_cast<NewMapUndoAction_Stones*>(action);
                out = new NewMapUndoAction_Stones (m_stones);
                m_stones = a->get_value ();
              }
            break;

          case NewMapUndoAction::MAP_SIZE:
              {
                auto a = dynamic_cast<NewMapUndoAction_MapSize*>(action);
                out = new NewMapUndoAction_MapSize (m_size, m_cities, m_ruins,
                                                    m_temples, m_signposts,
                                                    m_stones, m_width,
                                                    m_height);
                m_size = a->get_map_size_row ();
                m_cities = a->get_num_cities ();
                m_ruins = a->get_num_ruins ();
                m_temples = a->get_num_temples ();
                m_signposts = a->get_num_signposts ();
                m_stones = a->get_num_stones ();
                m_width = a->get_width ();
                m_height = a->get_height ();
              }
            break;

          case NewMapUndoAction::WIDTH:
              {
                auto a = dynamic_cast<NewMapUndoAction_Width*>(action);
                out = new NewMapUndoAction_Width (m_width);
                m_width = a->get_width ();
              }
            break;

          case NewMapUndoAction::HEIGHT:
              {
                auto a = dynamic_cast<NewMapUndoAction_Height*>(action);
                out = new NewMapUndoAction_Height (m_height);
                m_height = a->get_height ();
              }
            break;

          case NewMapUndoAction::TILESET:
              {
                auto a = dynamic_cast<NewMapUndoAction_TileSet*>(action);
                out = new NewMapUndoAction_TileSet (m_tileset_theme);
                m_tileset_theme = a->get_index ();
              }
            break;

          case NewMapUndoAction::ARMYSET:
              {
                auto a = dynamic_cast<NewMapUndoAction_ArmySet*>(action);
                out = new
                  NewMapUndoAction_ArmySet (m_player1_theme, m_player2_theme,
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

          case NewMapUndoAction::CITYSET:
              {
                auto a = dynamic_cast<NewMapUndoAction_CitySet*>(action);
                out = new NewMapUndoAction_CitySet (m_cityset_theme);
                m_cityset_theme = a->get_index ();
              }
            break;

          case NewMapUndoAction::SHIELDSET:
              {
                auto a = dynamic_cast<NewMapUndoAction_ShieldSet*>(action);
                out = new NewMapUndoAction_ShieldSet (m_shieldset_theme);
                m_shieldset_theme = a->get_index ();
              }
            break;

          case NewMapUndoAction::TILE_SIZE:
              {
                auto a = dynamic_cast<NewMapUndoAction_TileSize*>(action);
                out = new NewMapUndoAction_TileSize (m_tile_size,
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

          case NewMapUndoAction::RANDOM_ROADS:
              {
                auto a = dynamic_cast<NewMapUndoAction_RandomRoads*>(action);
                out = new NewMapUndoAction_RandomRoads (m_roads);
                m_roads = a->get_value ();
              }
            break;

          case NewMapUndoAction::FILL_STYLE:
              {
                auto a = dynamic_cast<NewMapUndoAction_FillStyle*>(action);
                out = new NewMapUndoAction_FillStyle (m_fill_style);
                m_fill_style = a->get_index ();
              }
            break;

          case NewMapUndoAction::RANDOM_NAMES:
              {
                auto a = dynamic_cast<NewMapUndoAction_RandomNames*>(action);
                out = new NewMapUndoAction_RandomNames (m_names);
                m_names = a->get_value ();
              }
            break;

          case NewMapUndoAction::PLAYER:
              {
                auto a = dynamic_cast<NewMapUndoAction_Player*>(action);
                bool active;
                switch (a->get_player_id ())
                  {
                  default:
                  case 0:
                    active = m_player1;
                    break;

                  case 1:
                    active = m_player2;
                    break;

                  case 2:
                    active = m_player3;
                    break;

                  case 3:
                    active = m_player4;
                    break;

                  case 4:
                    active = m_player5;
                    break;

                  case 5:
                    active = m_player6;
                    break;

                  case 6:
                    active = m_player7;
                    break;

                  case 7:
                    active = m_player8;
                    break;

                  }
                out = new NewMapUndoAction_Player (a->get_player_id (), active);
                switch (a->get_player_id ())
                  {
                  case 0:
                    m_player1 = a->get_active ();
                    break;

                  case 1:
                    m_player2 = a->get_active ();
                    break;

                  case 2:
                    m_player3 = a->get_active ();
                    break;

                  case 3:
                    m_player4 = a->get_active ();
                    break;

                  case 4:
                    m_player5 = a->get_active ();
                    break;

                  case 5:
                    m_player6 = a->get_active ();
                    break;

                  case 6:
                    m_player7 = a->get_active ();
                    break;

                  case 7:
                    m_player8 = a->get_active ();
                    break;

                  }
              }
            break;

          case NewMapUndoAction::STONE_ROAD_CHANCE:
              {
                auto a = dynamic_cast<NewMapUndoAction_StoneRoadChance*>(action);
                out = new NewMapUndoAction_StoneRoadChance (m_stone_road);
                m_stone_road = a->get_stone_road_chance ();
              }
            break;

          case NewMapUndoAction::MAKE_SAME:
              {
                auto a = dynamic_cast<NewMapUndoAction_MakeSame*>(action);
                out = new
                  NewMapUndoAction_MakeSame (m_make_same, m_player1_theme,
                                             m_player2_theme, m_player3_theme,
                                             m_player4_theme, m_player5_theme,
                                             m_player6_theme, m_player7_theme,
                                             m_player8_theme, m_neutral_theme);
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
        
    void update_map_size_buildings ()
      {
        switch (m_size)
          {
          case MAP_SIZE_SMALL:
            m_width = MAP_SIZE_SMALL_WIDTH;
            m_height = MAP_SIZE_SMALL_HEIGHT;
            m_cities = 15;
            m_ruins = 20;
            m_temples = 20;
            m_signposts = 20;
            m_stones = 40;
            break;

          case MAP_SIZE_TINY:
            m_width = MAP_SIZE_TINY_WIDTH;
            m_height = MAP_SIZE_TINY_HEIGHT;
            m_cities = 10;
            m_ruins = 15;
            m_temples = 15;
            m_signposts = 10;
            m_stones = 30;
            break;

          case MAP_SIZE_NORMAL:
          default:
            m_width = MAP_SIZE_NORMAL_WIDTH;
            m_height = MAP_SIZE_NORMAL_HEIGHT;
            m_cities = 30;
            m_ruins = 25;
            m_temples = 25;
            m_signposts = 25;
            m_stones = 50;
            break;

          case MAP_SIZE_CUSTOM:
            m_width = MAP_SIZE_NORMAL_WIDTH;
            m_height = MAP_SIZE_NORMAL_HEIGHT;
            m_cities = 30;
            m_ruins = 25;
            m_temples = 25;
            m_signposts = 25;
            m_stones = 50;
            break;
          }
        update ();
      }

    Gtk::Scale * get_scale (Tile::Type type)
      {
        switch (type)
          {
          case Tile::GRASS: return m_grass_scale;
          case Tile::WATER: return m_water_scale;
          case Tile::FOREST: return m_forest_scale;
          case Tile::HILLS: return m_hills_scale;
          case Tile::MOUNTAIN: return m_mountains_scale;
          case Tile::SWAMP: return m_swamp_scale;
          }
        return NULL;
      }

    void normalize (Tile::Type type)
      {
        double total = 0.0;

        total += m_grass_scale->get_value ();
        total += m_water_scale->get_value ();
        total += m_forest_scale->get_value ();
        total += m_hills_scale->get_value ();
        total += m_mountains_scale->get_value ();
        total += m_swamp_scale->get_value ();

        double err = 100.0 - total;

        if (err > 0)
          {
            double l = 100 - err;
            if (m_grass_scale->get_value () <= l && type != Tile::GRASS)
              m_grass_scale->set_value (m_grass_scale->get_value () + err);
            else if (m_water_scale->get_value () <= l && type != Tile::WATER)
              m_water_scale->set_value (m_water_scale->get_value () + err);
            else if (m_forest_scale->get_value () <= l && type != Tile::FOREST)
              m_forest_scale->set_value (m_forest_scale->get_value () + err);
            else if (m_hills_scale->get_value () <= l && type != Tile::HILLS)
              m_hills_scale->set_value (m_hills_scale->get_value () + err);
            else if (m_mountains_scale->get_value () <= l &&
                     type != Tile::MOUNTAIN)
              m_mountains_scale->set_value
                (m_mountains_scale->get_value () + err);
            else if (m_swamp_scale->get_value () <= l && type != Tile::SWAMP)
              m_swamp_scale->set_value (m_swamp_scale->get_value () + err);
          }
        else if (err < 0)
          {
            if (m_grass_scale->get_value () >= err && type != Tile::GRASS)
              m_grass_scale->set_value (m_grass_scale->get_value () + err);
            else if (m_water_scale->get_value () >= err && type != Tile::WATER)
              m_water_scale->set_value (m_water_scale->get_value () + err);
            else if (m_forest_scale->get_value () >= err && type != Tile::FOREST)
              m_forest_scale->set_value (m_forest_scale->get_value () + err);
            else if (m_hills_scale->get_value () >= err && type != Tile::HILLS)
              m_hills_scale->set_value (m_hills_scale->get_value () + err);
            else if (m_mountains_scale->get_value () >= err &&
                     type != Tile::MOUNTAIN)
              m_mountains_scale->set_value
                (m_mountains_scale->get_value () + err);
            else if (m_swamp_scale->get_value () >= err && type != Tile::SWAMP)
              m_swamp_scale->set_value (m_swamp_scale->get_value () + err);
          }
      }

    void on_scale_changed (Tile::Type changed_type)
      {
        if (m_inhibit_scales)
          return;

        m_inhibit_scales = true;

        double new_value = get_scale (changed_type)->get_value ();
        new_value = std::clamp (new_value, 0.0, 100.0);

        double old_value = m_percentages[to_index (changed_type)];
        double old_other_total = 100.0 - old_value;
        double new_other_total = 100.0 - new_value;

        double v[6];
        v[to_index (Tile::GRASS)] = get_scale (Tile::GRASS)->get_value ();
        v[to_index (Tile::WATER)] = get_scale (Tile::WATER)->get_value ();
        v[to_index (Tile::FOREST)] = get_scale (Tile::FOREST)->get_value ();
        v[to_index (Tile::HILLS)] = get_scale (Tile::HILLS)->get_value ();
        v[to_index (Tile::MOUNTAIN)] = get_scale (Tile::MOUNTAIN)->get_value ();
        v[to_index (Tile::SWAMP)] = get_scale (Tile::SWAMP)->get_value ();

        v[to_index (changed_type)] = new_value;

        if (old_other_total <= 0.0001)
          {
            // we have 5 others to distribute evenly among.
            double each = new_other_total / 5.0;

            for (int i = 0; i < 6; ++i)
              {
                if (i == to_index (changed_type))
                  continue;

                v[i] = each;
              }
          }
        else
          {
            // Preserve relative proportions of the others
            for (int i = 0; i < 6; ++i)
              {
                if (i == to_index (changed_type))
                  continue;

                double proportion = m_percentages[i] / old_other_total;

                v[i] = proportion * new_other_total;
              }
          }

        normalize (changed_type);

        if (changed_type != Tile::GRASS)
          get_scale (Tile::GRASS)->set_value (v[to_index (Tile::GRASS)]);
        if (changed_type != Tile::WATER)
          get_scale (Tile::WATER)->set_value (v[to_index (Tile::WATER)]);
        if (changed_type != Tile::FOREST)
          get_scale (Tile::FOREST)->set_value (v[to_index (Tile::FOREST)]);
        if (changed_type != Tile::HILLS)
          get_scale (Tile::HILLS)->set_value (v[to_index (Tile::HILLS)]);
        if (changed_type != Tile::MOUNTAIN)
          get_scale (Tile::MOUNTAIN)->set_value (v[to_index (Tile::MOUNTAIN)]);
        if (changed_type != Tile::SWAMP)
          get_scale (Tile::SWAMP)->set_value (v[to_index (Tile::SWAMP)]);

        m_inhibit_scales = false;
      }

    static int to_index (Tile::Type t)
      {
        return Tile::getTypeIndexForType (t);
      }

    void take_percentages ()
      {
        m_percentages[to_index (Tile::GRASS)] = m_grass_scale->get_value ();
        m_percentages[to_index (Tile::WATER)] = m_water_scale->get_value ();
        m_percentages[to_index (Tile::FOREST)] = m_forest_scale->get_value ();
        m_percentages[to_index (Tile::HILLS)] = m_hills_scale->get_value ();
        m_percentages[to_index (Tile::MOUNTAIN)] =
          m_mountains_scale->get_value ();
        m_percentages[to_index (Tile::SWAMP)] = m_swamp_scale->get_value ();
      }

    void add_player (GameParameters &g, int id, Shield::Color color,
                     CreateScenarioRandomize &random)
      {
        GameParameters::Player p;
        p.name = random.getPlayerName (color);
        p.id = id;
        g.players.push_back (p);
      }

};
#endif
