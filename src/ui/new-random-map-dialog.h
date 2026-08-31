//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2017, 2020, 2021,
//  2026 Ben Asselstine
#include <gtkmm.h>
#include <array>
#include "tile.h"
#include <map>
#include <vector>
#include "rnd.h"
#include "lw-dialog-base.h"
#include "game-scenario.h"
#include "create-scenario.h"
#include "startup.h"
#include "lw-combo.h"
#ifndef NEW_RANDOM_MAP_DIALOG_H
#define NEW_RANDOM_MAP_DIALOG_H
class NewRandomMapDialog: public LwDialogBase
{
public:

    enum
      {
        MAP_SIZE_NORMAL = 0,
        MAP_SIZE_SMALL,
        MAP_SIZE_TINY
      };

    static std::string get_resource_name ()
      {
        return "new-random-map.ui";
      }

    NewRandomMapDialog (BaseObjectType* o,
                        const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_inhibit_scales = false;
        m_percentages.resize (6);

        m_dialog_vbox = load <Gtk::Box> ("dialog-vbox1");

        auto c = load <Gtk::ComboBox> ("map_size_combobox");
        m_map_size_combobox = LwCombo::replace (c);
        m_grass_scale = load <Gtk::Scale> ("grass_scale");
        m_water_scale = load <Gtk::Scale> ("water_scale");
        m_swamp_scale = load <Gtk::Scale> ("swamp_scale");
        m_forest_scale = load <Gtk::Scale> ("forest_scale");
        m_hills_scale = load <Gtk::Scale> ("hills_scale");
        m_mountains_scale = load <Gtk::Scale> ("mountains_scale");
        m_cities_scale = load <Gtk::Scale> ("cities_scale");
        m_accept_button = load <Gtk::Button> ("accept2_button");
        set_default_widget (*m_accept_button);
        m_cancel_button = load <Gtk::Button> ("cancel2_button");
        m_grass_random_switch = load <Gtk::Switch> ("grass_random_switch");
        m_water_random_switch = load <Gtk::Switch> ("water_random_switch");
        m_swamp_random_switch = load <Gtk::Switch> ("swamp_random_switch");
        m_forest_random_switch = load <Gtk::Switch> ("forest_random_switch");
        m_hills_random_switch = load <Gtk::Switch> ("hills_random_switch");
        m_mountains_random_switch =
          load <Gtk::Switch> ("mountains_random_switch");
        m_cities_random_switch = load <Gtk::Switch> ("cities_random_switch");
        auto cc = load <Gtk::ComboBoxText> ("tile_size_combobox");
        m_tile_size_combobox = LwCombo::replace (cc);
        cc = load <Gtk::ComboBoxText> ("tileset_combobox");
        m_tileset_combobox = LwCombo::replace (cc);
        cc = load <Gtk::ComboBoxText> ("cityset_combobox");
        m_cityset_combobox = LwCombo::replace (cc);
        cc = load <Gtk::ComboBoxText> ("shieldset_combobox");
        m_shieldset_combobox = LwCombo::replace (cc);
        cc = load <Gtk::ComboBoxText> ("player_1_army_theme_combobox");
        m_player_1_army_theme_combobox = LwCombo::replace (cc);
        m_make_all_same_switch = load <Gtk::Switch> ("make_all_same_switch");
        cc = load <Gtk::ComboBoxText> ("player_2_army_theme_combobox");
        m_player_2_army_theme_combobox = LwCombo::replace (cc);
        cc = load <Gtk::ComboBoxText> ("player_3_army_theme_combobox");
        m_player_3_army_theme_combobox = LwCombo::replace (cc);
        cc = load <Gtk::ComboBoxText> ("player_4_army_theme_combobox");
        m_player_4_army_theme_combobox = LwCombo::replace (cc);
        cc = load <Gtk::ComboBoxText> ("player_5_army_theme_combobox");
        m_player_5_army_theme_combobox = LwCombo::replace (cc);
        cc = load <Gtk::ComboBoxText> ("player_6_army_theme_combobox");
        m_player_6_army_theme_combobox = LwCombo::replace (cc);
        cc = load <Gtk::ComboBoxText> ("player_7_army_theme_combobox");
        m_player_7_army_theme_combobox = LwCombo::replace (cc);
        cc = load <Gtk::ComboBoxText> ("player_8_army_theme_combobox");
        m_player_8_army_theme_combobox = LwCombo::replace (cc);
        cc = load <Gtk::ComboBoxText> ("neutral_army_theme_combobox");
        m_neutral_army_theme_combobox = LwCombo::replace (cc);
        m_overlay = load <Gtk::Overlay> ("overlay");
        m_revealer = load <Gtk::Revealer> ("revealer");
        m_progressbar = load <Gtk::ProgressBar> ("progressbar");
        m_name_entry = load <Gtk::Entry> ("name_entry");
        m_description_entry = load <Gtk::Entry> ("description_entry");
      }

    ~NewRandomMapDialog ()
      {
        m_coming_up.disconnect ();
      }

    void setup (LwCombo *combo, int id)
      {
        fill_army_theme_combo (combo, id);
        combo->signal_changed ().connect
          ([combo, id, this] ()
           {
             std::string name = combo->get_active_text ();
             Armyset *as =
               Armysetlist::instance ()->get (name, get_active_tile_size ());
             if (as)
               m_army_themes[id] = as->getBaseName ();
           });
      }

    void setup ()
      {
        m_coming_up = 
          Startup::instance ()->signal_game_window_coming_up ().connect
          ([this] ()
           {
             hide ();
             delete this;
           });

        m_overlay->add_overlay (*m_revealer);
        m_overlay->set_clip_overlay (*m_revealer, false);

        std::array<std::string, 9> themes =
          {
            "default",
            "default",
            "default",
            "default",
            "default",
            "default",
            "default",
            "default",
            "default"
          };
        m_army_themes = themes;

        setup_scale (m_grass_scale, Tile::GRASS);
        setup_scale (m_water_scale, Tile::WATER);
        setup_scale (m_forest_scale, Tile::FOREST);
        setup_scale (m_hills_scale, Tile::HILLS);
        setup_scale (m_mountains_scale, Tile::MOUNTAIN);
        setup_scale (m_swamp_scale, Tile::SWAMP);

        set_response (m_accept_button, Gtk::ResponseType::ACCEPT);
        set_response (m_cancel_button, Gtk::ResponseType::CLOSE);
        m_grass_random_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             m_grass_scale->set_sensitive
               (!m_grass_random_switch->get_active ());
           });
        m_water_random_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             m_water_scale->set_sensitive
               (!m_water_random_switch->get_active ());
           });
        m_forest_random_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             m_forest_scale->set_sensitive
               (!m_forest_random_switch->get_active ());
           });
        m_hills_random_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             m_hills_scale->set_sensitive
               (!m_hills_random_switch->get_active ());
           });
        m_mountains_random_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             m_mountains_scale->set_sensitive
               (!m_mountains_random_switch->get_active ());
           });
        m_swamp_random_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             m_swamp_scale->set_sensitive
               (!m_swamp_random_switch->get_active ());
           });
        m_cities_random_switch->property_active ().signal_changed ().connect
          ([this] ()
           {
             m_cities_scale->set_sensitive
               (!m_cities_random_switch->get_active ());
           });

        std::list<guint32> sizes;
        Tilesetlist::instance ()->getSizes (sizes);
        Citysetlist::instance ()->getSizes (sizes);
        Armysetlist::instance ()->getSizes (sizes);
        int counter = 0, default_id = 0;
        for (auto it = sizes.begin (); it != sizes.end (); ++it)
          {
            Glib::ustring s = String::ucompose ("%1x%1", *it);
            m_tile_size_combobox->append (s);
            if ((*it) == Tileset::getDefaultTileSize ())
              default_id = counter;
            counter++;
          }
        m_tile_size_combobox->set_active (default_id);
        on_tile_size_changed ();
        m_tile_size_combobox->signal_changed ().connect
          ([this] ()
           {
             on_tile_size_changed ();
           });

        // fill in shieldsets
        counter = 0;
        auto shield_themes = Shieldsetlist::instance ()->getValidIds ();
        for (auto i : shield_themes)
          {
            auto s = Shieldsetlist::instance ()->get (i);
            if (s->getId () == 1)
              default_id = counter;
            m_shieldset_combobox->append
              (Glib::filename_to_utf8 (s->getName ()));
            counter++;
          }
        m_shieldset_combobox->set_active (default_id);

        //fill in army themes
        setup (m_player_1_army_theme_combobox, 0);
        m_player_1_army_theme_combobox->signal_changed ().connect
          ([this] ()
           {
             int i = m_player_1_army_theme_combobox->get_active_row_number ();
             bool active = m_make_all_same_switch->get_active ();
             if (active)
               {
                 m_player_2_army_theme_combobox->set_active (i);
                 m_player_3_army_theme_combobox->set_active (i);
                 m_player_4_army_theme_combobox->set_active (i);
                 m_player_5_army_theme_combobox->set_active (i);
                 m_player_6_army_theme_combobox->set_active (i);
                 m_player_7_army_theme_combobox->set_active (i);
                 m_player_8_army_theme_combobox->set_active (i);
                 m_neutral_army_theme_combobox->set_active (i);
               }
           });
        setup (m_player_2_army_theme_combobox, 1);
        setup (m_player_3_army_theme_combobox, 2);
        setup (m_player_4_army_theme_combobox, 3);
        setup (m_player_5_army_theme_combobox, 4);
        setup (m_player_6_army_theme_combobox, 5);
        setup (m_player_7_army_theme_combobox, 6);
        setup (m_player_8_army_theme_combobox, 7);
        setup (m_neutral_army_theme_combobox, 8);

        bool make_all_same =
          std::all_of
          (m_army_themes.begin (), m_army_themes.end (),
           [&](const std::string& s)
           {
             return s == m_army_themes[0];
           });

        m_make_all_same_switch->property_active ().signal_changed ().connect
          ([this]()
           {
             bool active = m_make_all_same_switch->get_active ();
             int i = m_player_1_army_theme_combobox->get_active_row_number ();
             if (active)
               m_player_2_army_theme_combobox->set_active (i);
             m_player_2_army_theme_combobox->set_sensitive (!active);
             if (active)
               m_player_3_army_theme_combobox->set_active (i);
             m_player_3_army_theme_combobox->set_sensitive (!active);
             if (active)
               m_player_4_army_theme_combobox->set_active (i);
             m_player_4_army_theme_combobox->set_sensitive (!active);
             if (active)
               m_player_5_army_theme_combobox->set_active (i);
             m_player_5_army_theme_combobox->set_sensitive (!active);
             if (active)
               m_player_6_army_theme_combobox->set_active (i);
             m_player_6_army_theme_combobox->set_sensitive (!active);
             if (active)
               m_player_7_army_theme_combobox->set_active (i);
             m_player_7_army_theme_combobox->set_sensitive (!active);
             if (active)
               m_player_8_army_theme_combobox->set_active (i);
             m_player_8_army_theme_combobox->set_sensitive (!active);
             if (active)
               m_neutral_army_theme_combobox->set_active (i);
             m_neutral_army_theme_combobox->set_sensitive (!active);
           });

        m_make_all_same_switch->set_active (make_all_same);

        m_map_size_combobox->signal_changed ().connect
          ([this] ()
           {
             switch (m_map_size_combobox->get_active_row_number ())
               {
               case MAP_SIZE_SMALL:
                 m_cities_scale->set_value (15);
                 break;

               case MAP_SIZE_TINY:
                 m_cities_scale->set_value (10);
                 break;

               case MAP_SIZE_NORMAL:
               default:
                 m_cities_scale->set_value (20);
                 break;
               }
           });
        m_map_size_combobox->set_active (MAP_SIZE_NORMAL);

        signal_response ().connect
          ([this](Gtk::ResponseType response)
           {
             switch (response)
               {
               case Gtk::ResponseType::ACCEPT:
                   {
                     std::string filename = File::getSaveFile ("random.map");

                     m_revealer->set_reveal_child (true);

                     m_dialog_vbox->set_sensitive (false);
                     GameParameters g = get_params ();
                     sigc::slot<void(double)> progress =
                       [this, filename](double fraction)
                         {
                           m_progressbar->set_fraction (fraction);
                           Lw::do_events ();
                         };
                     g.difficulty =
                       GameScenarioOptions::calculate_difficulty_rating (g);

                     GameScenario::create_and_dump
                       (filename, g, &progress,
                        [this, filename] ()
                        {
                          m_scenario_generated.emit (filename,
                                                     GameScenario::HOTSEAT);
                          hide ();
                        });
                   }
                 break;

               default:
                 hide ();
                 break;
               }
           });
        m_accept_button->grab_focus ();
      }

    sigc::signal<void(std::string,GameScenario::PlayMode)> signal_scenario_generated ()
      {
        return m_scenario_generated;
      }
private:

    Gtk::Box *m_dialog_vbox;
    LwCombo *m_map_size_combobox;
    Gtk::Scale *m_grass_scale;
    Gtk::Scale *m_water_scale;
    Gtk::Scale *m_forest_scale;
    Gtk::Scale *m_hills_scale;
    Gtk::Scale *m_mountains_scale;
    Gtk::Scale *m_swamp_scale;
    Gtk::Scale *m_cities_scale;
    Gtk::Button *m_accept_button;
    Gtk::Button *m_cancel_button;
    Gtk::Switch *m_grass_random_switch;
    Gtk::Switch *m_water_random_switch;
    Gtk::Switch *m_forest_random_switch;
    Gtk::Switch *m_hills_random_switch;
    Gtk::Switch *m_mountains_random_switch;
    Gtk::Switch *m_swamp_random_switch;
    Gtk::Switch *m_cities_random_switch;
    LwCombo *m_tile_size_combobox;
    LwCombo *m_tileset_combobox;
    LwCombo *m_cityset_combobox;
    LwCombo *m_shieldset_combobox;
    LwCombo *m_player_1_army_theme_combobox;
    LwCombo *m_player_2_army_theme_combobox;
    LwCombo *m_player_3_army_theme_combobox;
    LwCombo *m_player_4_army_theme_combobox;
    LwCombo *m_player_5_army_theme_combobox;
    LwCombo *m_player_6_army_theme_combobox;
    LwCombo *m_player_7_army_theme_combobox;
    LwCombo *m_player_8_army_theme_combobox;
    LwCombo *m_neutral_army_theme_combobox;
    Gtk::Entry *m_name_entry;
    Gtk::Entry *m_description_entry;
    Gtk::Switch *m_make_all_same_switch;
    Gtk::Overlay *m_overlay = NULL;
    Gtk::Revealer *m_revealer = NULL;
    Gtk::ProgressBar *m_progressbar = NULL;
    sigc::connection m_coming_up;

    std::array<std::string, 9> m_army_themes;

    bool m_inhibit_scales;
    std::vector<double> m_percentages;
    sigc::signal<void(std::string,GameScenario::PlayMode)> m_scenario_generated;

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

    guint32 get_active_tile_size ()
      {
        return (guint32) std::stoi (m_tile_size_combobox->get_active_text ());
      }

    void on_tile_size_changed ()
      {
        guint32 default_id = 0;
        guint32 counter = 0;

        m_accept_button->set_sensitive (true);
        m_tileset_combobox->remove_all ();

        Tilesetlist *tl = Tilesetlist::instance ();
        auto tile_themes = tl->getValidIds (get_active_tile_size ());
        for (auto i : tile_themes)
          {
            auto t = Tilesetlist::instance ()->get (i);
            if (t->getId () == 1)
              default_id = counter;
            m_tileset_combobox->append (Glib::filename_to_utf8 (t->getName ()));
            counter++;
          }

        if (counter > 0)
          m_tileset_combobox->set_active (default_id);
        else
          m_accept_button->set_sensitive (false);

        auto themes =
         Armysetlist::instance ()->getValidIds (get_active_tile_size ());
        m_accept_button->set_sensitive (!themes.empty ());
        if (themes.empty ())
          {
            std::array<std::string, 9> empty =
              {
                "", "", "",
                "", "", "",
                "", "", ""
              };
            m_army_themes = empty;
            fill_army_theme_combo (m_player_1_army_theme_combobox, 0);
            fill_army_theme_combo (m_player_2_army_theme_combobox, 1);
            fill_army_theme_combo (m_player_3_army_theme_combobox, 2);
            fill_army_theme_combo (m_player_4_army_theme_combobox, 3);
            fill_army_theme_combo (m_player_5_army_theme_combobox, 4);
            fill_army_theme_combo (m_player_6_army_theme_combobox, 5);
            fill_army_theme_combo (m_player_7_army_theme_combobox, 6);
            fill_army_theme_combo (m_player_8_army_theme_combobox, 7);
            fill_army_theme_combo (m_neutral_army_theme_combobox, 8);
          }
        else
          {
            auto as = Armysetlist::instance ()->get (*themes.begin ());
            if (as)
              {
                auto name = as->getBaseName ();
                std::array<std::string, 9> newthemes =
                  {
                    name, name, name,
                    name, name, name,
                    name, name, name
                  };
                m_army_themes = newthemes;
                fill_army_theme_combo (m_player_1_army_theme_combobox, 0);
                fill_army_theme_combo (m_player_2_army_theme_combobox, 1);
                fill_army_theme_combo (m_player_3_army_theme_combobox, 2);
                fill_army_theme_combo (m_player_4_army_theme_combobox, 3);
                fill_army_theme_combo (m_player_5_army_theme_combobox, 4);
                fill_army_theme_combo (m_player_6_army_theme_combobox, 5);
                fill_army_theme_combo (m_player_7_army_theme_combobox, 6);
                fill_army_theme_combo (m_player_8_army_theme_combobox, 7);
                fill_army_theme_combo (m_neutral_army_theme_combobox, 8);
                m_player_1_army_theme_combobox->set_active (0);
                m_player_2_army_theme_combobox->set_active (0);
                m_player_3_army_theme_combobox->set_active (0);
                m_player_4_army_theme_combobox->set_active (0);
                m_player_5_army_theme_combobox->set_active (0);
                m_player_6_army_theme_combobox->set_active (0);
                m_player_7_army_theme_combobox->set_active (0);
                m_player_8_army_theme_combobox->set_active (0);
                m_neutral_army_theme_combobox->set_active (0);
              }
          }

        m_cityset_combobox->remove_all ();

        Citysetlist *cl = Citysetlist::instance ();
        auto city_themes = cl->getValidIds (get_active_tile_size ());
        counter = 0;
        default_id = 0;
        for (auto i : city_themes)
          {
            auto c = Citysetlist::instance ()->get (i);
            if (c->getId () == 1)
              default_id = counter;
            m_cityset_combobox->append (Glib::filename_to_utf8 (c->getName ()));
            counter++;
          }

        if (counter > 0)
          m_cityset_combobox->set_active (default_id);
        else
          m_accept_button->set_sensitive (false);
      }

    void assign_random_terrain (GameParameters &g)
      {
        double sum = 0;
        std::vector<Tile::Type> ter;

        if (!m_grass_random_switch->get_active ())
          sum += m_grass_scale->get_value ();
        else
          {
            ter.push_back (Tile::GRASS);
            g.map.grass = 0;
          }

        if (!m_water_random_switch->get_active ())
          sum += m_water_scale->get_value ();
        else
          {
            ter.push_back (Tile::WATER);
            g.map.water = 0;
          }

        if (!m_forest_random_switch->get_active ())
          sum += m_forest_scale->get_value ();
        else
          {
            ter.push_back (Tile::FOREST);
            g.map.forest = 0;
          }

        if (!m_hills_random_switch->get_active ())
          sum += m_hills_scale->get_value ();
        else
          {
            ter.push_back (Tile::HILLS);
            g.map.hills = 0;
          }

        if (!m_mountains_random_switch->get_active ())
          sum += m_mountains_scale->get_value ();
        else
          {
            ter.push_back (Tile::MOUNTAIN);
            g.map.mountains = 0;
          }

        if (!m_swamp_random_switch->get_active ())
          sum += m_swamp_scale->get_value ();
        else
          {
            ter.push_back (Tile::SWAMP);
            g.map.swamp = 0;
          }

        double excess = 100 - sum;
        if (excess <= 0)
          return;
        if (excess == 1)
          {
            g.map.grass++;
            return;
          }
        if (ter.empty () == false)
          {
            for (int i = 0; i < int (excess); i++)
              {
                Tile::Type type = ter[Rnd::rand () % ter.size ()];
                switch (type)
                  {
                  case Tile::GRASS:
                    g.map.grass++;
                    break;

                  case Tile::WATER:
                    g.map.water++;
                    break;

                  case Tile::FOREST:
                    g.map.forest++;
                    break;

                  case Tile::HILLS:
                    g.map.hills++;
                    break;

                  case Tile::MOUNTAIN:
                    g.map.mountains++;
                    break;

                  case Tile::SWAMP:
                    g.map.swamp++;
                    break;
                  }
              }
          }
      }

    void add_player (GameParameters &g, int id, Shield::Color color,
                     CreateScenarioRandomize &random)
      {
        GameParameters::Player p;
        p.name = random.getPlayerName (color);
        p.id = id;
        g.players.push_back (p);
      }

    GameParameters get_params ()
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
        switch (m_map_size_combobox->get_active_row_number ())
          {
          case MAP_SIZE_SMALL:
            g.map.width = MAP_SIZE_SMALL_WIDTH;
            g.map.height = MAP_SIZE_SMALL_HEIGHT;
            g.map.ruins = 20;
            g.map.temples = 4;
            break;

          case MAP_SIZE_TINY:
            g.map.width = MAP_SIZE_TINY_WIDTH;
            g.map.height = MAP_SIZE_TINY_HEIGHT;
            g.map.ruins = 15;
            g.map.temples = 4;
            break;

          case MAP_SIZE_NORMAL:
          default:
            g.map.width = MAP_SIZE_NORMAL_WIDTH;
            g.map.height = MAP_SIZE_NORMAL_HEIGHT;
            g.map.ruins = 25;
            g.map.temples = 4;
            break;
          }

        g.map.signposts =
          CreateScenario::calculateNumberOfSignposts
          (g.map.width, g.map.height, int (m_grass_scale->get_value ()));

        if (!m_grass_random_switch->get_active ())
          g.map.grass = int (m_grass_scale->get_value ());

        if (!m_water_random_switch->get_active ())
          g.map.water = int (m_water_scale->get_value ());

        if (!m_forest_random_switch->get_active ())
          g.map.forest = int (m_forest_scale->get_value ());

        if (!m_hills_random_switch->get_active ())
          g.map.hills = int (m_hills_scale->get_value ());

        if (!m_mountains_random_switch->get_active ())
          g.map.mountains = int (m_mountains_scale->get_value ());

        if (!m_swamp_random_switch->get_active ())
          g.map.swamp = int (m_swamp_scale->get_value ());

        assign_random_terrain (g);

        if (m_cities_random_switch->get_active ())
          {
            auto adjust = m_cities_scale->get_adjustment ();
            g.map.cities =  
              int (adjust->get_lower ()) + 
              (Rnd::rand () % (int (adjust->get_upper ()) -
                               int (adjust->get_lower ()) + 1));
          }
        else
          g.map.cities = int (m_cities_scale->get_value ());

        Tilesetlist *tl = Tilesetlist::instance ();
        Shieldsetlist *sl = Shieldsetlist::instance ();
        Citysetlist *cl = Citysetlist::instance ();
        g.tile_theme = tl->getSetDir 
          (Glib::filename_from_utf8 (m_tileset_combobox->get_active_text ()),
           get_active_tile_size ());

        for (guint32 i = 0; i < MAX_PLAYERS + 1; i++)
          g.army_theme[i] = m_army_themes[i];

        g.shield_theme = sl->getSetDir 
          (Glib::filename_from_utf8 (m_shieldset_combobox->get_active_text ()));

        g.city_theme = cl->getSetDir 
          (Glib::filename_from_utf8 (m_cityset_combobox->get_active_text ()),
           get_active_tile_size ());

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

        if (m_name_entry->get_text () == "")
          g.name = _("Autogenerated");
        else
          g.name = m_name_entry->get_text ();

        g.comment = m_description_entry->get_text ();
        random.cleanup ();
        return g;
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

    void setup_scale (Gtk::Scale *scale, Tile::Type type)
      {
        scale->signal_value_changed ().connect
          ([this, type] ()
           {
             on_scale_changed (type);
           }); 
        auto gesture = Gtk::GestureClick::create ();
        gesture->set_button (0);
        gesture->signal_pressed ().connect
          ([this] (int, double, double)
           {
             take_percentages ();
           });
        scale->add_controller (gesture);
      }

    void fill_army_theme_combo (LwCombo *combo, guint32 id)
      {
        combo->remove_all ();
        guint32 ts = get_active_tile_size ();
        guint32 counter = 0;
        guint32 default_id = 0;
        auto default_name = m_army_themes[id];
        auto das = Armysetlist::instance ()->get (default_name);
        for (auto i : Armysetlist::instance ()->getValidIds (ts))
          {
            Armyset *as = Armysetlist::instance ()->get (i);
            if (as)
              {
                if (das && as->getId () == das->getId ())
                  default_id = counter;
                combo->append (Glib::filename_to_utf8 (as->getName ()));
                counter++;
              }
          }

        if (counter > 0)
          combo->set_active (default_id);
      }
};
#endif
