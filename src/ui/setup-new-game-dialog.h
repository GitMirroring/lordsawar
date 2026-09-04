//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2011, 2012, 2014, 2015, 2020,
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
#include "image-cache.h"
#include "game-options-dialog.h"
#include "characters-dialog.h"
#include "lw-dialog.h"
#include "lw-combo.h"
#include "lw.h"
#ifndef SETUP_NEW_GAME_DIALOG_H
#define SETUP_NEW_GAME_DIALOG_H
static bool s_inhibit_difficulty_combobox = false;

class SetupNewGameDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "setup-new-game.ui";
      }

    SetupNewGameDialog (BaseObjectType* o,
                        const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_cancel_button = load <Gtk::Button> ("cancel_button");
        m_dialog_vbox = load <Gtk::Box> ("dialog-vbox1");
        m_start_game_button = load <Gtk::Button> ("start_game_button");
        m_start_game_button->set_receives_default (true);
        set_default_widget (*m_start_game_button);
        m_edit_options_button = load <Gtk::Button> ("edit_options_button");
        m_characters_button = load <Gtk::Button> ("characters_button");
        m_scenario_name_entry = load <Gtk::Entry> ("scenario_name_entry");
        m_scenario_description_entry = load <Gtk::Entry> ("scenario_description_entry");
        m_network_game_box = load <Gtk::Box> ("network_game_box");
        m_difficulty_label = load <Gtk::Label> ("difficulty_label");
        auto combobox = load <Gtk::ComboBox> ("difficulty_combobox");
        m_difficulty_combobox = LwCombo::replace (combobox);

        m_players_vbox = load <Gtk::Box> ("players_vbox");
        m_overlay = load <Gtk::Overlay> ("overlay");
        m_revealer = load <Gtk::Revealer> ("revealer");
        m_progressbar = load <Gtk::ProgressBar> ("progressbar");
      }

    ~SetupNewGameDialog ()
      {
        m_coming_up.disconnect ();
        m_load_tick.disconnect ();
      }

    void setup (std::string f, GameScenario::PlayMode m)
      {
        bool broken = false;
        m_filename = f;
        m_load_map_parameters =
          GameScenario::loadGameParameters (m_filename, broken);

        m_scenario_name_entry->set_text (m_load_map_parameters.name);
        m_scenario_description_entry->set_text (m_load_map_parameters.comment);
        m_network_game_box->set_visible (m == GameScenario::NETWORKED);

        m_coming_up =
          Startup::instance ()->signal_game_window_coming_up ().connect
          ([this] ()
           {
             hide ();
             Glib::signal_timeout ().connect_once
               ([this]()
                {
                  //this is how we avoid the race
                  delete this;
                }, 3000);
           });

        m_overlay->add_overlay (*m_revealer);
        m_overlay->set_clip_overlay (*m_revealer, false);

        m_mode = m;

        m_max_players = 0;

        m_difficulty_combobox->set_active (CUSTOM);
        m_difficulty_combobox->signal_changed ().connect
          ([this] ()
           {
             int type_num = 0;
             switch (m_difficulty_combobox->get_active_row_number ()) 
               {
               case BEGINNER:
                 GameScenarioOptions::s_see_opponents_stacks = true;
                 GameScenarioOptions::s_see_opponents_production = true;
                 GameScenarioOptions::s_play_with_quests =
                   GameParameters::NO_QUESTING;
                 GameScenarioOptions::s_hidden_map = false;
                 GameScenarioOptions::s_neutral_cities =
                   GameParameters::AVERAGE;
                 GameScenarioOptions::s_razing_cities = GameParameters::ALWAYS;
                 GameScenarioOptions::s_diplomacy = false;
                 GameScenarioOptions::s_cusp_of_war = false;
                 type_num = 1;
                 break;

               case INTERMEDIATE:
                 GameScenarioOptions::s_see_opponents_stacks = false;
                 GameScenarioOptions::s_see_opponents_production = true;
                 GameScenarioOptions::s_play_with_quests = 
                   GameParameters::ONE_QUEST_PER_PLAYER;
                 GameScenarioOptions::s_hidden_map = false;
                 GameScenarioOptions::s_neutral_cities = GameParameters::STRONG;
                 GameScenarioOptions::s_razing_cities = GameParameters::ALWAYS;
                 GameScenarioOptions::s_diplomacy = true;
                 GameScenarioOptions::s_cusp_of_war = false;
                 type_num = 1;
                 break;

               case ADVANCED:
                 GameScenarioOptions::s_see_opponents_stacks = false;
                 GameScenarioOptions::s_see_opponents_production = false;
                 GameScenarioOptions::s_play_with_quests = 
                   GameParameters::ONE_QUEST_PER_PLAYER;
                 GameScenarioOptions::s_hidden_map = true;
                 GameScenarioOptions::s_neutral_cities = GameParameters::ACTIVE;
                 GameScenarioOptions::s_razing_cities =
                   GameParameters::ON_CAPTURE;
                 GameScenarioOptions::s_diplomacy = true;
                 GameScenarioOptions::s_cusp_of_war = false;
                 type_num = 2;
                 break;

               case I_AM_THE_GREATEST:
                 GameScenarioOptions::s_see_opponents_stacks = false;
                 GameScenarioOptions::s_see_opponents_production = false;
                 GameScenarioOptions::s_play_with_quests = 
                   GameParameters::ONE_QUEST_PER_PLAYER;
                 GameScenarioOptions::s_hidden_map = true;
                 GameScenarioOptions::s_neutral_cities =
                   GameParameters::DEFENSIVE;
                 GameScenarioOptions::s_razing_cities = GameParameters::NEVER;
                 GameScenarioOptions::s_diplomacy = true;
                 GameScenarioOptions::s_cusp_of_war = true;
                 type_num = 2;
                 break;

               case CUSTOM:
                 break;
               }

             if (s_inhibit_difficulty_combobox == false)
               {
                 if (type_num)
                   {
                     for (auto c = m_player_types.begin ();
                          c != m_player_types.end (); ++c)
                       {
                         if ((*c)->get_active_row_number () != 3) //if OFF
                           (*c)->set_active (type_num);
                       }
                   }
                 update_difficulty_rating ();
                 update_buttons ();
               }
           });

        m_edit_options_button->signal_clicked ().connect
          ([this] ()
           {
             s_inhibit_difficulty_combobox = true;
             auto d = LwDialog::build<GameOptionsDialog> (this);
             d->setup (false);
             d->difficulty_option_changed.connect
               ([this] ()
                {
                  update_difficulty_rating ();
                  update_difficulty_combobox ();
                });
             d->signal_response ().connect
               ([this, d] (Gtk::ResponseType r)
                {
                  (void) r;
                  s_inhibit_difficulty_combobox = false; 
                  delete d;
                });
           });

        if (broken)
          m_start_game_button->set_sensitive (false);

        broken = false;
        Shieldsetlist::instance ()->instantiateImages (broken);
        auto shieldset =
          Shieldsetlist::instance ()->get (m_load_map_parameters.shield_theme);
        m_shieldset = shieldset->getId ();
        for (unsigned int i = 0; i < MAX_PLAYERS; i++)
          add_player (GameParameters::Player::HUMAN, "", i);

        auto e = m_player_names.begin ();
        //disable all names, and types
        for (auto c = m_player_types.begin (); c != m_player_types.end ();
             ++c, ++e)
          {
            (*c)->set_sensitive (true);
            (*c)->set_active (GameParameters::Player::OFF);
            (*c)->set_sensitive (false);
            (*e)->set_sensitive (false);
          }
        //parse load map parameters.
        guint32 b;
        for (auto i = m_load_map_parameters.players.begin (), 
             end = m_load_map_parameters.players.end (); i != end; ++i) 
          {
            auto c = m_player_types.begin ();
            e = m_player_names.begin ();
            //zip to correct combobox, entry
            for (b = 0; b < (*i).id; b++, ++c, ++e)
              ;
            (*c)->set_sensitive (true);
            (*c)->set_active ((*i).type);
            (*e)->set_sensitive (true);
            (*e)->set_text ((*i).name);
            m_max_players++;
          }

        //load the game options from the config file.
        GameScenarioOptions::s_see_opponents_stacks = 
          Configuration::s_see_opponents_stacks;
        GameScenarioOptions::s_see_opponents_production = 
          Configuration::s_see_opponents_production;
        GameScenarioOptions::s_play_with_quests =
          Configuration::s_play_with_quests;
        GameScenarioOptions::s_vectoring_mode = Configuration::s_vectoring_mode;
        GameScenarioOptions::s_sacking_mode = Configuration::s_sacking_mode;
        GameScenarioOptions::s_build_production_mode =
          Configuration::s_build_production_mode;
        GameScenarioOptions::s_hidden_map = Configuration::s_hidden_map;
        GameScenarioOptions::s_neutral_cities = Configuration::s_neutral_cities;
        GameScenarioOptions::s_razing_cities = Configuration::s_razing_cities;
        GameScenarioOptions::s_diplomacy = Configuration::s_diplomacy ;
        GameScenarioOptions::s_random_turns = Configuration::s_random_turns;
        GameScenarioOptions::s_cusp_of_war = Configuration::s_cusp_of_war;
        GameScenarioOptions::s_intense_combat = Configuration::s_intense_combat;
        GameScenarioOptions::s_military_advisor =
          Configuration::s_military_advisor;
        update_difficulty_rating ();
        update_difficulty_combobox ();

        setup_player_types ();

        m_characters_button->signal_clicked ().connect
          ([this] ()
           {
             auto d = LwDialog::build<CharactersDialog> (this);
             d->setup (m_player_shields, m_filename);
             d->signal_response ().connect
               ([d] (Gtk::ResponseType)
                {
                  delete d;
                });
           });

        set_response (m_cancel_button, Gtk::ResponseType::CLOSE);
        set_response (m_start_game_button, Gtk::ResponseType::ACCEPT);
        signal_response ().connect
          ([this](Gtk::ResponseType  response)
           {
             switch (response)
               {
               case Gtk::ResponseType::ACCEPT:
                 kick_off ();
                 break;

               default:
                 break;
               }
             hide ();
           });
        m_start_game_button->grab_focus ();
      }

    sigc::signal<void(GameScenario*, GameParameters)> signal_game_setup ()
      {
        return m_game_setup;
      }
private:
    GameScenario::PlayMode m_mode;
    Glib::ustring m_filename;
    Gtk::Button *m_start_game_button;
    Gtk::Button *m_cancel_button;
    Gtk::Box *m_dialog_vbox;
    Gtk::Button *m_edit_options_button;
    Gtk::Button *m_characters_button;
    Gtk::Box *m_network_game_box;
    Gtk::Entry *m_scenario_name_entry;
    Gtk::Entry *m_scenario_description_entry;
    Gtk::Label *m_difficulty_label;
    LwCombo *m_difficulty_combobox;
    Gtk::Box *m_players_vbox;
    Gtk::Overlay *m_overlay;
    Gtk::Revealer *m_revealer;
    Gtk::ProgressBar *m_progressbar;
    guint32 m_shieldset;
    guint32 m_max_players;
    sigc::connection m_coming_up;
    sigc::connection m_load_tick;
        
    GameParameters m_load_map_parameters = {};

    enum { BEGINNER = 0, INTERMEDIATE, ADVANCED, I_AM_THE_GREATEST, CUSTOM};

    std::list<LwCombo *> m_player_types;
    std::list<Gtk::Entry *> m_player_names;
    std::list<Gtk::Image *> m_player_shields;

    sigc::signal<void(GameScenario*, GameParameters)> m_game_setup;

    void add_player (GameParameters::Player::Type type,
                     const Glib::ustring &name, int i)
      {
        //okay, add a new hbox, with a combo and an entry in it
        //add it to players_vbox
        auto player_hbox =
          Gtk::make_managed<Gtk::Box> (Gtk::Orientation::HORIZONTAL);
        player_hbox->set_spacing (6);

        Gtk::Image *shield = Gtk::make_managed<Gtk::Image> ();
        shield->set
          (ImageCache::instance ()->getShieldPic
           (m_shieldset, 2, i, false)->to_pixbuf ());
        shield->set_pixel_size (LW_BUTTON_SIZE);

        auto player_type = Gtk::make_managed<LwCombo> ();
        player_type->append (HUMAN_PLAYER_TYPE);
        player_type->append (EASY_PLAYER_TYPE);
        player_type->append (HARD_PLAYER_TYPE);
        player_type->append (NO_PLAYER_TYPE);

        auto player_name = Gtk::make_managed<Gtk::Entry> ();
        player_name->signal_changed ().connect
          ([this] ()
           {
             update_buttons ();
             update_difficulty_rating ();
           });
        player_name->set_text (name);

        if (type == GameParameters::Player::HUMAN)
          player_type->set_active (0);
        else if (type == GameParameters::Player::EASY)
          player_type->set_active (1);
        else if (type == GameParameters::Player::HARD)
          player_type->set_active (2);
        else if (type== GameParameters::Player::OFF)
          player_type->set_active (3);

        player_type->set_valign (Gtk::Align::CENTER);
        m_player_types.push_back (player_type);
        player_name->set_valign (Gtk::Align::CENTER);
        m_player_names.push_back (player_name);
        m_player_shields.push_back (shield);
        player_hbox->append (*shield);
        player_hbox->append (*player_name);
        player_hbox->append (*player_type);
        m_players_vbox->append (*player_hbox);
      }

    void update_buttons ()
      {
        std::map<guint32, bool> offplayers;
        guint32 offcount = 0;
        guint32 count = 0;
        for (auto c = m_player_types.begin (); c != m_player_types.end (); ++c)
          {
            GameParameters::Player::Type match = 
              GameParameters::player_param_string_to_player_param
              ((*c)->get_active_text ());
            if (match == GameParameters::Player::OFF)
              {
                offplayers[count] = true;
                offcount++;
              }
            else
              offplayers[count] = false;
            count++;
          }
        bool found_empty_name = false;
        count = 0;
        for (auto e = m_player_names.begin (); e != m_player_names.end (); ++e)
          {
            if (offplayers[count] == true)
              continue;
            if (String::utrim ((*e)->get_text ())== "")
              {
                found_empty_name = true;
                break;
              }
            count++;
          }

        bool human_player = at_least_one_human_player ();

        if (offcount > m_player_types.size () - 2 ||
            found_empty_name || !human_player)
          m_start_game_button->set_sensitive (false);
        else
          m_start_game_button->set_sensitive (true);
      }

    void update_difficulty_rating ()
      {
        GameParameters g;
        for (auto c = m_player_types.begin (); c != m_player_types.end (); ++c)
          {
            GameParameters::Player p;
            auto t = (*c)->get_active_text ();
            p.type = GameParameters::player_param_string_to_player_param (t);
            g.players.push_back (p);
          }

        g.see_opponents_stacks = GameScenarioOptions::s_see_opponents_stacks;
        g.see_opponents_production =
          GameScenarioOptions::s_see_opponents_production;
        g.play_with_quests = GameScenarioOptions::s_play_with_quests;
        g.vectoring_mode = GameScenarioOptions::s_vectoring_mode;
        g.sacking_mode = GameScenarioOptions::s_sacking_mode;
        g.build_production_mode = GameScenarioOptions::s_build_production_mode;
        g.hidden_map = GameScenarioOptions::s_hidden_map;
        g.neutral_cities = GameScenarioOptions::s_neutral_cities;
        g.razing_cities = GameScenarioOptions::s_razing_cities;
        g.diplomacy = GameScenarioOptions::s_diplomacy;
        g.cusp_of_war = GameScenarioOptions::s_cusp_of_war;
        g.random_turns = GameScenarioOptions::s_random_turns;
        g.quick_start = Configuration::s_quick_start;
        g.intense_combat = GameScenarioOptions::s_intense_combat;
        g.military_advisor = GameScenarioOptions::s_military_advisor;

        int difficulty = GameScenario::calculate_difficulty_rating (g);
        g.players.clear ();

        m_difficulty_label->set_text (String::ucompose ("%1%%", difficulty));
      }

    bool is_beginner ()
      {
        return (GameScenarioOptions::s_see_opponents_stacks == true &&
                GameScenarioOptions::s_see_opponents_production == true &&
                GameScenarioOptions::s_play_with_quests == 
                GameParameters::NO_QUESTING &&
                GameScenarioOptions::s_hidden_map == false &&
                GameScenarioOptions::s_neutral_cities ==
                GameParameters::AVERAGE &&
                GameScenarioOptions::s_razing_cities ==
                GameParameters::ALWAYS &&
                GameScenarioOptions::s_diplomacy == false &&
                GameScenarioOptions::s_cusp_of_war == false);
      }

    bool is_intermediate ()
      {
        return (GameScenarioOptions::s_see_opponents_stacks == false &&
                GameScenarioOptions::s_see_opponents_production == true &&
                GameScenarioOptions::s_play_with_quests == 
                GameParameters::ONE_QUEST_PER_PLAYER &&
                GameScenarioOptions::s_hidden_map == false &&
                GameScenarioOptions::s_neutral_cities ==
                GameParameters::STRONG &&
                GameScenarioOptions::s_razing_cities ==
                GameParameters::ALWAYS &&
                GameScenarioOptions::s_diplomacy == true &&
                GameScenarioOptions::s_cusp_of_war == false);
      }

    bool is_advanced ()
      {
        return (GameScenarioOptions::s_see_opponents_stacks == false &&
                GameScenarioOptions::s_see_opponents_production == false &&
                GameScenarioOptions::s_play_with_quests == 
                GameParameters::ONE_QUEST_PER_PLAYER &&
                GameScenarioOptions::s_hidden_map == true &&
                GameScenarioOptions::s_neutral_cities ==
                GameParameters::ACTIVE &&
                GameScenarioOptions::s_razing_cities ==
                GameParameters::ON_CAPTURE &&
                GameScenarioOptions::s_diplomacy == true &&
                GameScenarioOptions::s_cusp_of_war == false);
      }

    bool is_greatest ()
      {
        return (GameScenarioOptions::s_see_opponents_stacks == false &&
                GameScenarioOptions::s_see_opponents_production == false &&
                GameScenarioOptions::s_play_with_quests == 
                GameParameters::ONE_QUEST_PER_PLAYER &&
                GameScenarioOptions::s_hidden_map == true &&
                GameScenarioOptions::s_neutral_cities ==
                GameParameters::DEFENSIVE &&
                GameScenarioOptions::s_razing_cities == GameParameters::NEVER &&
                GameScenarioOptions::s_diplomacy == true &&
                GameScenarioOptions::s_cusp_of_war == true);
      }

    void update_difficulty_combobox()
      {
        s_inhibit_difficulty_combobox = true;
        if (at_least_one_human_player ())
          m_difficulty_combobox->set_active (CUSTOM);
        else if (is_greatest ())
          m_difficulty_combobox->set_active (I_AM_THE_GREATEST);
        else if (is_advanced ())
          m_difficulty_combobox->set_active (ADVANCED);
        else if (is_intermediate ())
          m_difficulty_combobox->set_active (INTERMEDIATE);
        else if (is_beginner ())
          m_difficulty_combobox->set_active (BEGINNER);
        else
          m_difficulty_combobox->set_active (CUSTOM);
        s_inhibit_difficulty_combobox = false;
      }

    void setup_player_types ()
      {
        for (auto player_type : m_player_types)
          {
            player_type->signal_changed ().connect
              ([this, player_type] ()
               {
                 update_buttons ();
                 update_difficulty_rating ();
                 update_difficulty_combobox ();
               });
          }
      }

    bool at_least_one_human_player ()
      {
        bool human_player = false;
        for (auto c = m_player_types.begin (); c != m_player_types.end (); ++c)
          {
            GameParameters::Player::Type match = 
              GameParameters::player_param_string_to_player_param
              ((*c)->get_active_text ());
            if (match == GameParameters::Player::HUMAN)
              human_player = true;
          }
        return human_player;
      }
                 
    void kick_off ()
      {
        GameParameters g = m_load_map_parameters;
        g.players.clear ();
        g.map_path = m_filename;
        int id = 0;
        auto c = m_player_types.begin ();
        auto e = m_player_names.begin ();
        for (; c != m_player_types.end (); ++c, ++e, id++)
          {
            GameParameters::Player p;
            p.type =
              GameParameters::player_param_string_to_player_param
              ((*c)->get_active_text ());
            Glib::ustring name = String::utrim ((*e)->get_text ());
            p.name = name;
            p.id = id;
            g.players.push_back( p);
          }

        g.see_opponents_stacks = GameScenarioOptions::s_see_opponents_stacks;
        g.see_opponents_production =
          GameScenarioOptions::s_see_opponents_production;
        g.play_with_quests = GameScenarioOptions::s_play_with_quests;
        g.vectoring_mode = GameScenarioOptions::s_vectoring_mode;
        g.sacking_mode = GameScenarioOptions::s_sacking_mode;
        g.build_production_mode = GameScenarioOptions::s_build_production_mode;
        g.hidden_map = GameScenarioOptions::s_hidden_map;
        g.neutral_cities = GameScenarioOptions::s_neutral_cities;
        g.razing_cities = GameScenarioOptions::s_razing_cities;
        g.diplomacy = GameScenarioOptions::s_diplomacy;
        g.random_turns = GameScenarioOptions::s_random_turns;
        g.quick_start = Configuration::s_quick_start;
        g.cusp_of_war = GameScenarioOptions::s_cusp_of_war;
        g.intense_combat = GameScenarioOptions::s_intense_combat;
        g.military_advisor = GameScenarioOptions::s_military_advisor;
        g.cities_can_produce_allies =
          GameScenarioOptions::s_cities_can_produce_allies;

        g.difficulty = GameScenario::calculate_difficulty_rating(g);
          
        if (m_scenario_name_entry->get_text () != "")
          g.name = String::utrim (m_scenario_name_entry->get_text ());
        if (m_scenario_description_entry->get_text () != "")
          g.comment = String::utrim (m_scenario_description_entry->get_text ());

        m_revealer->set_reveal_child (true);
        m_dialog_vbox->set_sensitive (false);
        Lw::do_events ();
        m_load_tick = GameScenario::load_tick.connect
          ([this] (double fraction)
           {
             m_progressbar->set_fraction (fraction);
             Lw::do_events ();
           });
        GameScenario *game_scenario = create_new_scenario (g, m_mode);
        if (m_mode == GameScenario::HOTSEAT)
          m_game_setup.emit (game_scenario, g);
        else if (m_mode == GameScenario::NETWORKED)
          m_game_setup.emit (game_scenario, g);
      }

    GameScenario *create_new_scenario (GameParameters &g,
                                       GameScenario::PlayMode m)
      {
        bool update_uuid = false;
        if (g.map_path.empty ()) 
          {
            // construct new random scenario if we're not going to load the game
            Glib::ustring path = File::getSaveFile ("random.map");
            GameScenario::create_and_dump (path, g, NULL,
                                           [] ()
                                           {
                                           });
            g.map_path = path;
          }
        else
          update_uuid = true;

        bool broken = false;
        Glib::ustring err;
        GameScenario* game_scenario = new GameScenario (g.map_path, broken, err);
        if (broken)
          return NULL;

        GameScenarioOptions::s_see_opponents_stacks = g.see_opponents_stacks;
        GameScenarioOptions::s_see_opponents_production =
          g.see_opponents_production;
        GameScenarioOptions::s_play_with_quests = g.play_with_quests;
        GameScenarioOptions::s_vectoring_mode = g.vectoring_mode;
        GameScenarioOptions::s_sacking_mode = g.sacking_mode;
        GameScenarioOptions::s_build_production_mode = g.build_production_mode;
        GameScenarioOptions::s_hidden_map = g.hidden_map;
        GameScenarioOptions::s_diplomacy = g.diplomacy;
        GameScenarioOptions::s_cusp_of_war = g.cusp_of_war;
        GameScenarioOptions::s_neutral_cities = g.neutral_cities;
        GameScenarioOptions::s_razing_cities = g.razing_cities;
        GameScenarioOptions::s_intense_combat = g.intense_combat;
        GameScenarioOptions::s_military_advisor = g.military_advisor;
        GameScenarioOptions::s_random_turns = g.random_turns;
        GameScenarioOptions::s_cities_can_produce_allies =
          g.cities_can_produce_allies;

        game_scenario->setName (g.name);
        game_scenario->setComment (g.comment);
        game_scenario->setPlayMode (m);

        if (game_scenario->getRound () == 0)
          {
            if (update_uuid)
              game_scenario->setNewRandomId ();
            Playerlist::instance ()->syncPlayers (g);
            game_scenario->initialize (g);
          }
        return game_scenario;
      }
};

#endif
