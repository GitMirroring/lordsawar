//  Copyright (C) 2007, 2008, 2009, 2011, 2014, 2017 Ben Asselstine
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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <config.h>

#include <gtkmm.h>
#include <sigc++/functors/mem_fun.h>

#include "game-options-dialog.h"

#include "Configuration.h"
#include "GameScenarioOptions.h"

#define method(x) sigc::mem_fun(*this, &GameOptionsDialog::x)

GameOptionsDialog::GameOptionsDialog(Gtk::Window &parent, bool readonly)
 : LwDialog(parent, "game-options-dialog.ui")
{
    d_readonly = readonly;
    xml->get_widget("difficultoptionstable", difficultoptionstable);
    xml->get_widget("notdifficultoptionstable", notdifficultoptionstable);
    xml->get_widget("view_enemies_switch", view_enemies_switch);
    xml->get_widget("view_production_switch", view_production_switch);
    xml->get_widget("quests_combobox", quests_combobox);
    xml->get_widget("hidden_map_switch", hidden_map_switch);
    xml->get_widget("neutral_combobox", neutral_cities_combobox);
    xml->get_widget("vectoring_combobox", vectoring_combobox);
    xml->get_widget("build_production_combobox", build_production_combobox);
    xml->get_widget("sack_combobox", sack_combobox);
    xml->get_widget("razing_combobox", razing_cities_combobox);
    xml->get_widget("diplomacy_switch", diplomacy_switch);
    xml->get_widget("military_advisor_switch", 
                    military_advisor_switch);
    xml->get_widget("quick_start_combobox", quick_start_combobox);
    xml->get_widget("cusp_of_war_switch", cusp_of_war_switch);
    xml->get_widget("intense_combat_switch", intense_combat_switch);
    xml->get_widget("random_turns_switch", random_turns_switch);
    xml->get_widget("notebook", notebook);
}

void GameOptionsDialog::fill_in_options()
{
    neutral_cities_combobox->set_active(GameScenarioOptions::s_neutral_cities);
    vectoring_combobox->set_active(GameScenarioOptions::s_vectoring_mode);
    build_production_combobox->set_active(GameScenarioOptions::s_build_production_mode);
    sack_combobox->set_active(GameScenarioOptions::s_sacking_mode);
    razing_cities_combobox->set_active(GameScenarioOptions::s_razing_cities);

    view_enemies_switch->set_active(GameScenarioOptions::s_see_opponents_stacks);
    view_production_switch->set_active(GameScenarioOptions::s_see_opponents_production);
    quests_combobox->set_active(int(GameScenarioOptions::s_play_with_quests));
    hidden_map_switch->set_active(GameScenarioOptions::s_hidden_map);
    razing_cities_combobox->set_active(int(GameScenarioOptions::s_razing_cities));
    diplomacy_switch->set_active(GameScenarioOptions::s_diplomacy);
    military_advisor_switch->set_active(GameScenarioOptions::s_military_advisor);
    quick_start_combobox->set_active(Configuration::s_quick_start);
    cusp_of_war_switch->set_active(GameScenarioOptions::s_cusp_of_war);
    cusp_of_war_switch->set_sensitive(diplomacy_switch->get_active());
    intense_combat_switch->set_active(GameScenarioOptions::s_intense_combat);
    random_turns_switch->set_active(GameScenarioOptions::s_random_turns);
    if (d_readonly)
      {
	difficultoptionstable->set_sensitive(false);
	notdifficultoptionstable->set_sensitive(false);
      }
}

bool GameOptionsDialog::run()
{
  std::list<sigc::connection> connections;
    GameParameters g;
    fill_in_options();
    connections.push_back (view_enemies_switch->property_active().signal_changed().connect
                           (method (on_view_enemies_switch_clicked)));
    connections.push_back (view_production_switch->property_active().signal_changed().connect
                           (method(on_view_production_switch_clicked)));
    connections.push_back (quests_combobox->signal_changed().connect
                           (method(on_quests_combobox_changed)));
    connections.push_back (hidden_map_switch->property_active().signal_changed().connect
                           (method(on_hidden_map_switch_clicked)));
    connections.push_back (neutral_cities_combobox->signal_changed().connect
                           (method(on_neutral_cities_combobox_changed)));
    connections.push_back (vectoring_combobox->signal_changed().connect
                           (method(on_vectoring_combobox_changed)));
    connections.push_back (build_production_combobox->signal_changed().connect
                           (method(on_build_production_combobox_changed)));
    connections.push_back (sack_combobox->signal_changed().connect
                           (method(on_sacking_combobox_changed)));
    connections.push_back (razing_cities_combobox->signal_changed().connect
                           (method (on_razing_cities_combobox_changed)));
    connections.push_back (diplomacy_switch->property_active().signal_changed().connect
                           (method (on_diplomacy_switch_clicked)));
    connections.push_back (cusp_of_war_switch->property_active().signal_changed().connect
                           (method(on_cusp_of_war_switch_clicked)));
    connections.push_back (random_turns_switch->property_active().signal_changed().connect
                           (method (on_random_turns_switch_clicked)));
    connections.push_back (quick_start_combobox->signal_changed().connect
                           (method (on_quick_start_combobox_changed)));
    connections.push_back (intense_combat_switch->property_active().signal_changed().connect
                           (method (on_intense_combat_switch_clicked)));
    connections.push_back (military_advisor_switch->property_active().signal_changed().connect
                           (method (on_military_advisor_switch_clicked)));

    dialog->run();
  
    for (std::list<sigc::connection>::iterator it = connections.begin();
         it != connections.end(); ++it) 
      (*it).disconnect();
    connections.clear();
    
    g.see_opponents_stacks = view_enemies_switch->get_active();
    GameScenarioOptions::s_see_opponents_stacks = g.see_opponents_stacks;
    g.see_opponents_production = view_production_switch->get_active();
    GameScenarioOptions::s_see_opponents_production = g.see_opponents_production;
    g.play_with_quests = GameParameters::QuestPolicy (
	quests_combobox->get_active_row_number());
    GameScenarioOptions::s_play_with_quests = g.play_with_quests;

    g.hidden_map = hidden_map_switch->get_active();
    GameScenarioOptions::s_hidden_map = g.hidden_map;

    g.neutral_cities = GameParameters::NeutralCities (
	neutral_cities_combobox->get_active_row_number());
    GameScenarioOptions::s_neutral_cities = g.neutral_cities;
    g.vectoring_mode = GameParameters::VectoringMode(
	vectoring_combobox->get_active_row_number());
    GameScenarioOptions::s_vectoring_mode = g.vectoring_mode;
    g.build_production_mode = GameParameters::BuildProductionMode(
	build_production_combobox->get_active_row_number());
    GameScenarioOptions::s_build_production_mode = g.build_production_mode;
    g.sacking_mode = GameParameters::SackingMode(
	sack_combobox->get_active_row_number());
    GameScenarioOptions::s_sacking_mode = g.sacking_mode;
    g.razing_cities = GameParameters::RazingCities (
	razing_cities_combobox->get_active_row_number());
    GameScenarioOptions::s_razing_cities = g.razing_cities;

    g.diplomacy = diplomacy_switch->get_active();
    GameScenarioOptions::s_diplomacy = g.diplomacy;
    g.random_turns = random_turns_switch->get_active();
    GameScenarioOptions::s_random_turns = g.random_turns;
    g.quick_start = GameParameters::QuickStartPolicy(
	quick_start_combobox->get_active_row_number());
    Configuration::s_quick_start = g.quick_start;
    g.cusp_of_war = cusp_of_war_switch->get_active();
    GameScenarioOptions::s_cusp_of_war = g.cusp_of_war;
    g.intense_combat = intense_combat_switch->get_active();
    GameScenarioOptions::s_intense_combat = g.intense_combat;
    g.military_advisor = military_advisor_switch->get_active();
    GameScenarioOptions::s_military_advisor = g.military_advisor;
    //save it all to Configuration too
    Configuration::s_see_opponents_stacks = 
      GameScenarioOptions::s_see_opponents_stacks;
    Configuration::s_see_opponents_production = 
      GameScenarioOptions::s_see_opponents_production;
    Configuration::s_play_with_quests = GameScenarioOptions::s_play_with_quests;
    Configuration::s_hidden_map = GameScenarioOptions::s_hidden_map;
    Configuration::s_neutral_cities = GameScenarioOptions::s_neutral_cities;
    Configuration::s_vectoring_mode = GameScenarioOptions::s_vectoring_mode;
    Configuration::s_build_production_mode = GameScenarioOptions::s_build_production_mode;
    Configuration::s_sacking_mode = GameScenarioOptions::s_sacking_mode;
    Configuration::s_razing_cities = GameScenarioOptions::s_razing_cities;
    Configuration::s_diplomacy = GameScenarioOptions::s_diplomacy;
    Configuration::s_random_turns = GameScenarioOptions::s_random_turns;
    Configuration::s_cusp_of_war = GameScenarioOptions::s_cusp_of_war;
    Configuration::s_intense_combat = GameScenarioOptions::s_intense_combat;
    Configuration::s_military_advisor = GameScenarioOptions::s_military_advisor;
    Configuration::saveConfigurationFile();
    dialog->hide();
    return true;
}

void GameOptionsDialog::on_view_enemies_switch_clicked()
{
  GameScenarioOptions::s_see_opponents_stacks =
    view_enemies_switch->get_active();
  difficulty_option_changed.emit();
}

void GameOptionsDialog::on_view_production_switch_clicked()
{
  GameScenarioOptions::s_see_opponents_production =
    view_production_switch->get_active();
  difficulty_option_changed.emit();
}

void GameOptionsDialog::on_quests_combobox_changed()
{
  GameScenarioOptions::s_play_with_quests = GameParameters::QuestPolicy
    (quests_combobox->get_active_row_number());
  difficulty_option_changed.emit();
}

void GameOptionsDialog::on_hidden_map_switch_clicked()
{
  GameScenarioOptions::s_hidden_map = hidden_map_switch->get_active();
  difficulty_option_changed.emit();
}

void GameOptionsDialog::on_neutral_cities_combobox_changed()
{
  GameScenarioOptions::s_neutral_cities = GameParameters::NeutralCities 
    (neutral_cities_combobox->get_active_row_number());
  difficulty_option_changed.emit();
}

void GameOptionsDialog::on_vectoring_combobox_changed()
{
  GameScenarioOptions::s_vectoring_mode = GameParameters::VectoringMode
    (vectoring_combobox->get_active_row_number());
}

void GameOptionsDialog::on_build_production_combobox_changed()
{
  GameScenarioOptions::s_build_production_mode =
    GameParameters::BuildProductionMode
    (build_production_combobox->get_active_row_number());
}

void GameOptionsDialog::on_sacking_combobox_changed()
{
  GameScenarioOptions::s_sacking_mode =
    GameParameters::SackingMode (sack_combobox->get_active_row_number());
}

void GameOptionsDialog::on_razing_cities_combobox_changed()
{
  GameScenarioOptions::s_razing_cities = GameParameters::RazingCities 
    (razing_cities_combobox->get_active_row_number());
  difficulty_option_changed.emit();
}

void GameOptionsDialog::on_diplomacy_switch_clicked()
{
  if (diplomacy_switch->get_active() == true)
    cusp_of_war_switch->set_sensitive(true);
  else
    cusp_of_war_switch->set_sensitive(false);
  GameScenarioOptions::s_diplomacy = diplomacy_switch->get_active();
  difficulty_option_changed.emit();
}

void GameOptionsDialog::on_cusp_of_war_switch_clicked()
{
  GameScenarioOptions::s_cusp_of_war = cusp_of_war_switch->get_active();
}

void GameOptionsDialog::on_random_turns_switch_clicked()
{
  GameScenarioOptions::s_random_turns = random_turns_switch->get_active();
}

void GameOptionsDialog::on_quick_start_combobox_changed()
{
  Configuration::s_quick_start = GameParameters::QuickStartPolicy
    (quick_start_combobox->get_active_row_number());
  difficulty_option_changed.emit();
}

void GameOptionsDialog::on_intense_combat_switch_clicked()
{
  GameScenarioOptions::s_intense_combat = intense_combat_switch->get_active();
}

void GameOptionsDialog::on_military_advisor_switch_clicked()
{
  GameScenarioOptions::s_military_advisor = military_advisor_switch->get_active();
}

GameOptionsDialog::~GameOptionsDialog()
{
  notebook->property_show_tabs () = false;
}
