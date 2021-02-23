//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2014, 2015, 2017, 2020,
//  2021 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>
#include <gtkmm.h>
#include <algorithm>
#include <map>
#include <vector>

#include "new-random-map-dialog.h"

#include "defs.h"
#include "Configuration.h"
#include "File.h"
#include "tileset.h"
#include "tilesetlist.h"
#include "armysetlist.h"
#include "citysetlist.h"
#include "shieldsetlist.h"
#include "ucompose.hpp"
#include "GameScenarioOptions.h"
#include "CreateScenarioRandomize.h"
#include "CreateScenario.h"
#include "player.h"
#include "counter.h"
#include "rnd.h"

#define method(x) sigc::mem_fun(*this, &NewRandomMapDialog::x)

NewRandomMapDialog::NewRandomMapDialog(Gtk::Window &parent)
        : LwDialog(parent, "new-random-map-dialog.ui")
{
  xml->get_widget("dialog-vbox1", dialog_vbox);
  xml->get_widget("dialog-action_area1", dialog_action_area);
  xml->get_widget("map_size_combobox", map_size_combobox);
  xml->get_widget("grass_scale", grass_scale);
  ActiveTerrainType terrain = GRASS;
  grass_scale->signal_value_changed().connect
    (sigc::bind(sigc::mem_fun (this, &NewRandomMapDialog::on_value_changed), terrain));
  xml->get_widget("water_scale", water_scale);
  terrain = WATER;
  water_scale->signal_value_changed().connect
    (sigc::bind(sigc::mem_fun (this, &NewRandomMapDialog::on_value_changed), terrain));
  xml->get_widget("swamp_scale", swamp_scale);
  terrain = SWAMP;
  swamp_scale->signal_value_changed().connect
    (sigc::bind(sigc::mem_fun (this, &NewRandomMapDialog::on_value_changed), terrain));
  xml->get_widget("forest_scale", forest_scale);
  terrain = FOREST;
  forest_scale->signal_value_changed().connect
    (sigc::bind(sigc::mem_fun (this, &NewRandomMapDialog::on_value_changed), terrain));
  xml->get_widget("hills_scale", hills_scale);
  terrain = HILLS;
  hills_scale->signal_value_changed().connect
    (sigc::bind(sigc::mem_fun (this, &NewRandomMapDialog::on_value_changed), terrain));
  xml->get_widget("mountains_scale", mountains_scale);
  terrain = MOUNTAINS;
  mountains_scale->signal_value_changed().connect
    (sigc::bind(sigc::mem_fun (this, &NewRandomMapDialog::on_value_changed), terrain));
  xml->get_widget("cities_scale", cities_scale);
  xml->get_widget("accept2_button", accept_button);
  accept_button->signal_clicked().connect (method(on_accept_clicked));
  xml->get_widget("cancel2_button", cancel_button);
  cancel_button->signal_clicked().connect (method(on_cancel_clicked));
  xml->get_widget("grass_random_switch", grass_random_switch);
  grass_random_switch->property_active().signal_changed().connect
    (method(on_grass_random_toggled));
  xml->get_widget("water_random_switch", water_random_switch);
  water_random_switch->property_active().signal_changed().connect
    (method(on_water_random_toggled));
  xml->get_widget("swamp_random_switch", swamp_random_switch);
  swamp_random_switch->property_active().signal_changed().connect
    (method(on_swamp_random_toggled));
  xml->get_widget("forest_random_switch", forest_random_switch);
  forest_random_switch->property_active().signal_changed().connect
    (method(on_forest_random_toggled));
  xml->get_widget("hills_random_switch", hills_random_switch);
  hills_random_switch->property_active().signal_changed().connect
    (method(on_hills_random_toggled));
  xml->get_widget("mountains_random_switch", mountains_random_switch);
  mountains_random_switch->property_active().signal_changed().connect
    (method(on_mountains_random_toggled));
  xml->get_widget("cities_random_switch", cities_random_switch);
  cities_random_switch->property_active().signal_changed().connect
    (method(on_cities_random_toggled));
  xml->get_widget("progress_treeview", progress_treeview);

  // fill in tile themes combobox

  guint32 counter = 0;
  guint32 default_id = 0;
  Gtk::Box *box;

  //fill in tile sizes combobox
  tile_size_combobox = manage(new Gtk::ComboBoxText);
  std::list<guint32> sizes;
  Tilesetlist::getInstance()->getSizes(sizes);
  Citysetlist::getInstance()->getSizes(sizes);
  Armysetlist::getInstance()->getSizes(sizes);
  for (std::list<guint32>::iterator it = sizes.begin(); it != sizes.end();
       ++it)
    {
      Glib::ustring s = String::ucompose("%1x%1", *it);
      tile_size_combobox->append(s);
      if ((*it) == Tileset::getDefaultTileSize())
        default_id = counter;
      counter++;
    }
  tile_size_combobox->set_active(default_id);
  xml->get_widget("tile_size_box", box);
  box->pack_start(*tile_size_combobox, Gtk::PACK_SHRINK);
  tile_size_combobox->signal_changed().connect (method(on_tile_size_changed));

  // make new tile themes combobox
  tile_theme_combobox = manage(new Gtk::ComboBoxText);
  xml->get_widget("tile_theme_box", box);
  box->pack_start(*tile_theme_combobox, Gtk::PACK_SHRINK);

  // make new army themes combobox
  army_theme_combobox = manage(new Gtk::ComboBoxText);
  xml->get_widget("army_theme_box", box);
  box->pack_start(*army_theme_combobox, Gtk::PACK_SHRINK);

  // make new city themes combobox
  city_theme_combobox = manage(new Gtk::ComboBoxText);
  xml->get_widget("city_theme_box", box);
  box->pack_start(*city_theme_combobox, Gtk::PACK_SHRINK);

  counter = 0;
  default_id = 0;
  shield_theme_combobox = manage(new Gtk::ComboBoxText);
  Shieldsetlist *sl = Shieldsetlist::getInstance();
  std::list<Glib::ustring> shield_themes = sl->getValidNames();
  for (std::list<Glib::ustring>::iterator i = shield_themes.begin(),
       end = shield_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
        default_id = counter;
      shield_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  shield_theme_combobox->set_active(default_id);

  xml->get_widget("shield_theme_box", box);
  box->pack_start(*shield_theme_combobox, Gtk::PACK_SHRINK);

  on_tile_size_changed();

  // map size
  map_size_combobox->set_active(MAP_SIZE_NORMAL);
  map_size_combobox->signal_changed().connect(method(on_map_size_changed));

  xml->get_widget("cities_can_produce_allies_checkbutton", 
                  cities_can_produce_allies_checkbutton);

  xml->get_widget("notebook", notebook);

  grass_scale->set_value(78);
  water_scale->set_value(7);
  swamp_scale->set_value(2);
  forest_scale->set_value(3);
  hills_scale->set_value(5);
  mountains_scale->set_value(5);
  on_map_size_changed();
  dialog_response = Gtk::RESPONSE_CANCEL;
  d_active_terrain = NONE;
  d_inhibit_scales = false;

  //progressbar, what a pain
  progress_treeview->property_headers_visible () = false;
  progress_liststore = Gtk::ListStore::create(progress_columns);
  progress_treeview->set_model (progress_liststore);
  row = *(progress_liststore->append());
  auto cell = Gtk::manage (new Gtk::CellRendererProgress());
  cell->property_text () = "";
  int cols_count = progress_treeview->append_column ("progress", *cell);
  auto pColumn = progress_treeview->get_column(cols_count -1);
  if (pColumn)
    pColumn->add_attribute(cell->property_value (), progress_columns.perc);

}

int NewRandomMapDialog::run()
{
  dialog->show_all();
  dialog_action_area->hide();
  progress_treeview->hide();
  //we're not using the buttons from the action area.
  //we have our own buttons so that we can show a progress bar after the
  //button is clicked.
  dialog->run();
  return dialog_response;
}

void NewRandomMapDialog::on_map_size_changed()
{
  switch (map_size_combobox->get_active_row_number()) {
  case MAP_SIZE_SMALL:
    map.width = MAP_SIZE_SMALL_WIDTH;
    map.height = MAP_SIZE_SMALL_HEIGHT;
    cities_scale->set_value(15);
    break;

  case MAP_SIZE_TINY:
    map.width = MAP_SIZE_TINY_WIDTH;
    map.height = MAP_SIZE_TINY_HEIGHT;
    cities_scale->set_value(10);
    break;

  case MAP_SIZE_NORMAL:
  default:
    map.width = MAP_SIZE_NORMAL_WIDTH;
    map.height = MAP_SIZE_NORMAL_HEIGHT;
    cities_scale->set_value(20);
    break;
  }
}

guint32 NewRandomMapDialog::get_active_tile_size()
{
  return (guint32) atoi(tile_size_combobox->get_active_text().c_str());
}

void NewRandomMapDialog::on_tile_size_changed()
{
  guint32 default_id = 0;
  guint32 counter = 0;

  accept_button->set_sensitive(true);
  tile_theme_combobox->remove_all();

  Tilesetlist *tl = Tilesetlist::getInstance();
  std::list<Glib::ustring> tile_themes = tl->getValidNames(get_active_tile_size());
  for (std::list<Glib::ustring>::iterator i = tile_themes.begin(),
       end = tile_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
        default_id = counter;
      tile_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  if (counter > 0)
    tile_theme_combobox->set_active(default_id);
  else
    accept_button->set_sensitive(false);


  army_theme_combobox->remove_all();

  Armysetlist *al = Armysetlist::getInstance();
  std::list<Glib::ustring> army_themes = al->getValidNames(get_active_tile_size());
  counter = 0;
  default_id = 0;
  for (std::list<Glib::ustring>::iterator i = army_themes.begin(),
       end = army_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
        default_id = counter;
      army_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  if (counter > 0)
    army_theme_combobox->set_active(default_id);
  else
    accept_button->set_sensitive(false);

  city_theme_combobox->remove_all();

  Citysetlist *cl = Citysetlist::getInstance();
  std::list<Glib::ustring> city_themes = cl->getValidNames(get_active_tile_size());
  counter = 0;
  default_id = 0;
  for (std::list<Glib::ustring>::iterator i = city_themes.begin(),
       end = city_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
        default_id = counter;
      city_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  if (counter > 0)
    city_theme_combobox->set_active(default_id);
  else
    accept_button->set_sensitive(false);
}

void NewRandomMapDialog::assign_random_terrain (GameParameters &g)
{
  double sum = 0;
  std::vector<ActiveTerrainType> ter;

  if (!grass_random_switch->get_active())
    sum += grass_scale->get_value ();
  else
    {
      ter.push_back(GRASS);
      g.map.grass = 0;
    }


  if (!water_random_switch->get_active())
    sum += water_scale->get_value ();
  else
    {
      ter.push_back(WATER);
      g.map.water = 0;
    }

  if (!forest_random_switch->get_active())
    sum += forest_scale->get_value ();
  else
    {
      ter.push_back(FOREST);
      g.map.forest = 0;
    }

  if (!hills_random_switch->get_active())
    sum += hills_scale->get_value ();
  else
    {
      ter.push_back(HILLS);
      g.map.hills = 0;
    }

  if (!swamp_random_switch->get_active())
    sum += swamp_scale->get_value ();
  else
    {
      ter.push_back(SWAMP);
      g.map.swamp = 0;
    }

  if (!mountains_random_switch->get_active())
    sum += mountains_scale->get_value ();
  else
    {
      ter.push_back(MOUNTAINS);
      g.map.mountains= 0;
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
      for (int i = 0; i < int(excess); i++)
        {
          ActiveTerrainType type = ter[Rnd::rand() % ter.size()];
          switch (type)
            {
            case GRASS:
              g.map.grass++;
              break;
            case WATER:
              g.map.water++;
              break;
            case FOREST:
              g.map.forest++;
              break;
            case HILLS:
              g.map.hills++;
              break;
            case SWAMP:
              g.map.swamp++;
              break;
            case MOUNTAINS:
              g.map.mountains++;
              break;
            default:
              break;
            }
        }
    }
}

GameParameters NewRandomMapDialog::getParams()
{
  CreateScenarioRandomize random;
  GameParameters g;
  GameParameters::Player p;
  p.type = GameParameters::Player::HUMAN;
  g.players.clear();
  p.name = random.getPlayerName(Shield::WHITE);
  p.id = int(Shield::WHITE);
  g.players.push_back(p);
  p.id = int(Shield::GREEN);
  p.name = random.getPlayerName(Shield::Color(p.id));
  g.players.push_back(p);
  p.id = int(Shield::YELLOW);
  p.name = random.getPlayerName(Shield::Color(p.id));
  g.players.push_back(p);
  p.id = int(Shield::DARK_BLUE);
  p.name = random.getPlayerName(Shield::Color(p.id));
  g.players.push_back(p);
  p.id = int(Shield::ORANGE);
  p.name = random.getPlayerName(Shield::Color(p.id));
  g.players.push_back(p);
  p.id = int(Shield::LIGHT_BLUE);
  p.name = random.getPlayerName(Shield::Color(p.id));
  g.players.push_back(p);
  p.id = int(Shield::RED);
  p.name = random.getPlayerName(Shield::Color(p.id));
  g.players.push_back(p);
  p.id = int(Shield::BLACK);
  p.name = random.getPlayerName(Shield::Color(p.id));
  g.players.push_back(p);

  g.map_path = "";
  switch (map_size_combobox->get_active_row_number()) {
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
    CreateScenario::calculateNumberOfSignposts(g.map.width, g.map.height,
                                               int(grass_scale->get_value()));

  if (!grass_random_switch->get_active())
    g.map.grass = int(grass_scale->get_value());

  if (!water_random_switch->get_active())
    g.map.water = int(water_scale->get_value());

  if (!swamp_random_switch->get_active())
    g.map.swamp = int(swamp_scale->get_value());

  if (!forest_random_switch->get_active())
    g.map.forest = int(forest_scale->get_value());

  if (!hills_random_switch->get_active())
    g.map.hills = int(hills_scale->get_value());

  if (!mountains_random_switch->get_active())
    g.map.mountains = int(mountains_scale->get_value());

  assign_random_terrain (g);

  if (cities_random_switch->get_active())
    g.map.cities =  
      int(cities_scale->get_adjustment()->get_lower()) + 
      (Rnd::rand() % (int(cities_scale->get_adjustment()->get_upper()) -
		 int(cities_scale->get_adjustment()->get_lower()) + 1));
  else
    g.map.cities = int(cities_scale->get_value());

  Tilesetlist *tl = Tilesetlist::getInstance();
  Armysetlist *al = Armysetlist::getInstance();
  Shieldsetlist *sl = Shieldsetlist::getInstance();
  Citysetlist *cl = Citysetlist::getInstance();
  g.tile_theme = tl->getSetDir 
    (Glib::filename_from_utf8(tile_theme_combobox->get_active_text()),
     get_active_tile_size());

  g.army_theme = al->getSetDir
    (Glib::filename_from_utf8(army_theme_combobox->get_active_text()),
     get_active_tile_size());

  g.shield_theme = sl->getSetDir 
    (Glib::filename_from_utf8(shield_theme_combobox->get_active_text()));

  g.city_theme = cl->getSetDir 
    (Glib::filename_from_utf8(city_theme_combobox->get_active_text()),
     get_active_tile_size());

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
  g.cities_can_produce_allies = 
    cities_can_produce_allies_checkbutton->get_active();
  g.cusp_of_war = GameScenarioOptions::s_cusp_of_war;
  g.vectoring_mode = GameScenarioOptions::s_vectoring_mode;
  g.build_production_mode = GameScenarioOptions::s_build_production_mode;
  g.sacking_mode = GameScenarioOptions::s_sacking_mode;

  g.name = _("Autogenerated");
  random.cleanup();
  return g;
}

void NewRandomMapDialog::on_grass_random_toggled()
{
  grass_scale->set_sensitive(!grass_random_switch->get_active());
}

void NewRandomMapDialog::on_water_random_toggled()
{
  water_scale->set_sensitive(!water_random_switch->get_active());
}

void NewRandomMapDialog::on_swamp_random_toggled()
{
  swamp_scale->set_sensitive(!swamp_random_switch->get_active());
}

void NewRandomMapDialog::on_forest_random_toggled()
{
  forest_scale->set_sensitive(!forest_random_switch->get_active());
}

void NewRandomMapDialog::on_hills_random_toggled()
{
  hills_scale->set_sensitive(!hills_random_switch->get_active());
}

void NewRandomMapDialog::on_mountains_random_toggled()
{
  mountains_scale->set_sensitive(!mountains_random_switch->get_active());
}

void NewRandomMapDialog::on_cities_random_toggled()
{
  cities_scale->set_sensitive(!cities_random_switch->get_active());
}

Glib::ustring NewRandomMapDialog::create_and_dump_scenario(const Glib::ustring &file,
                                                         const GameParameters &g, sigc::slot<void> *pulse)
{
  CreateScenario creator (g.map.width, g.map.height);

  // then fill the other players
  Armyset *as = Armysetlist::getInstance()->get(g.army_theme);
  int army_id = as->getId();
  Shieldsetlist *ssl = Shieldsetlist::getInstance();
  guint32 id = ssl->get(g.shield_theme)->getId();
  for (std::vector<GameParameters::Player>::const_iterator
       i = g.players.begin(), end = g.players.end();
       i != end; ++i) {

    if (i->type == GameParameters::Player::OFF)
      {
        fl_counter->getNextId();
        continue;
      }

    Player::Type type;
    if (i->type == GameParameters::Player::EASY)
      type = Player::AI_FAST;
    else if (i->type == GameParameters::Player::HARD)
      type = Player::AI_SMART;
    else
      type = Player::HUMAN;

    creator.addPlayer(i->name, army_id, ssl->getColors(id, i->id), type);
  }


  CreateScenarioRandomize random;
  // the neutral player must come last so it has the highest id among players
  creator.addNeutral(random.getPlayerName(Shield::NEUTRAL), army_id, 
                     ssl->getColors(id, MAX_PLAYERS), Player::AI_DUMMY);

  // now fill in some map information
  creator.setMapTiles(g.tile_theme);
  creator.setShieldset(g.shield_theme);
  creator.setCityset(g.city_theme);
  creator.setNoCities(g.map.cities);
  creator.setNoRuins(g.map.ruins);
  creator.setNoTemples(g.map.temples);
  int num_signposts = g.map.signposts;
  if (num_signposts == -1)
    num_signposts = CreateScenario::calculateNumberOfSignposts(g.map.width,
                                                               g.map.height,
                                                               g.map.grass);
  creator.setNoSignposts(num_signposts);

  // terrain: the scenario generator also accepts input with a sum of
  // more than 100%, so the thing is rather easy here
  creator.setPercentages(g.map.grass, g.map.water, g.map.forest, g.map.swamp,
                         g.map.hills, g.map.mountains);

  // now create the map and dump the created map
  Glib::ustring path = File::getSaveFile(file);

  if (pulse)
    creator.progress.connect(*pulse);
  
  creator.create(g);
  creator.dump(path);
  random.cleanup();
  return path;
}

void NewRandomMapDialog::on_accept_clicked()
{
  dialog_vbox->set_sensitive(false);
  progress_treeview->show_all();

  while (g_main_context_iteration(NULL, FALSE)); //doEvents
  GameParameters g = getParams();

  sigc::slot<void> progress = method(pulse);
  g.difficulty = GameScenarioOptions::calculate_difficulty_rating(g);
  d_filename = create_and_dump_scenario("random.map", g, &progress);

  //finish off the progressbar
  while (row[progress_columns.perc] < 100)
    {
      row[progress_columns.perc] = row[progress_columns.perc] + 1;
      while (g_main_context_iteration(NULL, FALSE)); //doEvents
      Glib::usleep (10000);
    }
  row[progress_columns.perc] = 100;
  progress_treeview->queue_draw ();
  Glib::usleep (20000);
  while (g_main_context_iteration(NULL, FALSE)); //doEvents

  dialog_response = Gtk::RESPONSE_ACCEPT;
  dialog->hide();
}

void NewRandomMapDialog::on_cancel_clicked()
{
  dialog_response = Gtk::RESPONSE_CANCEL;
  dialog->hide();
}
void NewRandomMapDialog::pulse()
{
  if (row[progress_columns.perc] < 98)
    {
      row[progress_columns.perc] = row[progress_columns.perc] + 3;
      while (g_main_context_iteration(NULL, FALSE)); //doEvents
    }
}

void NewRandomMapDialog::take_percentages ()
{
  percentages[GRASS] = grass_scale->get_value();
  percentages[WATER] = water_scale->get_value();
  percentages[FOREST] = forest_scale->get_value();
  percentages[HILLS] = hills_scale->get_value();
  percentages[MOUNTAINS] = mountains_scale->get_value();
  percentages[SWAMP] = swamp_scale->get_value();
}

void NewRandomMapDialog::alter_grass ()
{
  double excess = 100.0 - grass_scale->get_value() - water_scale->get_value() -
    forest_scale->get_value() - hills_scale->get_value() -
    swamp_scale->get_value() - mountains_scale->get_value();

  std::map<int,double> per;
  //go get the sum
  double sum = 0;
  for (int i = GRASS + 1; i < MAX_TERRAINS; i++)
    sum += percentages[i];

  // how much is each of the other terrains of the whole, multiplied by excess
  for (int i = GRASS + 1; i < MAX_TERRAINS; i++)
    {
      if (percentages[i] > 0)
        per[i] = (double)percentages[i] / sum * excess;
    }

  //we can't sort the map, so we load a copy of it into a vector.
  std::vector<std::pair<int, double> > percopy(per.begin(), per.end());

  std::sort(percopy.begin(), percopy.end(), cmp);

  sum = 0;
  for (auto p : percopy)
    {
      p.second = round (p.second);
      sum += p.second;
    }

  if (sum > excess)
    {
      //decrement from the bottom
      int extra = sum - excess;
      for (std::vector<std::pair<int, double> >::reverse_iterator it = percopy.rbegin();
           it != percopy.rend(); ++it)
        {
          if ((*it).second == 0)
            continue;
          (*it).second -= 1;
          extra--;
          if (extra <= 0)
            break;
        }
    }
  else if (sum < excess)
    {
      int extra = excess - sum;
      //add to the top.
      for (auto it : percopy)
        {
          it.second++;
          extra--;
          if (extra <= 0)
            break;
        }
    }
  for (auto p : percopy)
    augment_scale_value_by_type (ActiveTerrainType(p.first), p.second);
}

void NewRandomMapDialog::augment_scale_value_by_type (ActiveTerrainType type, double amt)
{
  switch (type)
    {
    case GRASS:
      grass_scale->set_value(grass_scale->get_value() + amt);
      break;
    case WATER:
      water_scale->set_value(water_scale->get_value() + amt);
      break;
    case FOREST:
      forest_scale->set_value(forest_scale->get_value() + amt);
      break;
    case HILLS:
      hills_scale->set_value(hills_scale->get_value() + amt);
      break;
    case SWAMP:
      swamp_scale->set_value(swamp_scale->get_value() + amt);
      break;
    case MOUNTAINS:
      mountains_scale->set_value(mountains_scale->get_value() + amt);
      break;
    default:
      break;
    }
}

int NewRandomMapDialog::cmp (std::pair<int,double> const &a, std::pair<int,double> const &b)
{
  return a.second != b.second ?  fabs(a.second) > fabs(b.second) : a.first > b.first;
}

void NewRandomMapDialog::alter_terrain (ActiveTerrainType type)
{
  double grass = 100.0 - water_scale->get_value() - forest_scale->get_value() -
    hills_scale->get_value() - swamp_scale->get_value() -
    mountains_scale->get_value();
  int excess = 0;
  if (grass < 0)
    {
      excess = int(grass) * -1;
      grass = 0;
    }
  grass_scale->set_value (grass);
  if (!excess)
    return;
  //okay we have EXCESS to take away from every other terrain that isn't TYPE
  //and isn't grass.
  std::map<int,double> per;
  //go get the sum
  double sum = 0;
  for (int i = GRASS + 1; i < MAX_TERRAINS; i++)
    {
      if (ActiveTerrainType(i) == type)
        continue;
      sum += percentages[i];
    }

  // how much is each of the other terrains of the whole, multiplied by excess
  for (int i = GRASS + 1; i < MAX_TERRAINS; i++)
    {
      if (ActiveTerrainType (i) == type)
        continue;
      if (percentages[i] > 0)
        per[i] = (double)percentages[i] / sum * excess;
    }

  //we can't sort the map, so we load a copy of it into a vector.
  std::vector<std::pair<int, double> > percopy(per.begin(), per.end());

  std::sort(percopy.begin(), percopy.end(), cmp);

  sum = 0;
  for (auto p : percopy)
    {
      p.second = round (p.second);
      sum += p.second;
    }
  if (sum > excess)
    {
      //decrement from the bottom
      int extra = sum - excess;
      for (std::vector<std::pair<int, double> >::reverse_iterator it = percopy.rbegin();
           it != percopy.rend(); ++it)
        {
          if ((*it).second == 0)
            continue;
          (*it).second -= 1;
          extra--;
          if (extra <= 0)
            break;
        }
    }
  else if (sum < excess)
    {
      int extra = excess - sum;
      //add to the top.
      for (auto it : percopy)
        {
          it.second++;
          extra--;
          if (extra <= 0)
            break;
        }
    }
  for (auto p : percopy)
    augment_scale_value_by_type (ActiveTerrainType(p.first), -p.second);
}

void NewRandomMapDialog::on_value_changed (ActiveTerrainType type)
{
  if (d_inhibit_scales)
    return;
  if (type != d_active_terrain)
    {
      take_percentages ();
      d_active_terrain = type;
    }
  switch (type)
    {
    case GRASS:
      d_inhibit_scales = true;
      alter_grass ();
      d_inhibit_scales = false;
      break;
    case WATER:
    case FOREST:
    case HILLS:
    case MOUNTAINS:
    case SWAMP:
      d_inhibit_scales = true;
      alter_terrain (type);
      d_inhibit_scales = false;
      break;
    default:
      break;
    }
}

NewRandomMapDialog::~NewRandomMapDialog()
{
  notebook->property_show_tabs () = false;
}
