//  Copyright (C) 2007 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2012, 2014, 2015, 2017, 2020,
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

#include <assert.h>
#include <sigc++/functors/mem_fun.h>
#include <gtkmm.h>

#include "new-map-dialog.h"
#include "defs.h"
#include "File.h"
#include "tileset.h"
#include "tilesetlist.h"
#include "armysetlist.h"
#include "citysetlist.h"
#include "shieldsetlist.h"
#include "ucompose.hpp"
#include "GameMap.h"
#include "new-map-actions.h"

#define method(x) sigc::mem_fun(*this, &NewMapDialog::x)

NewMapDialog::NewMapDialog(Gtk::Window &parent)
 : LwEditorDialog(parent, "new-map-dialog.ui")
{
    umgr = new UndoMgr (UndoMgr::DELAY, UndoMgr::LIMIT);
    umgr->execute ().connect (method (executeAction));
    map_set = false;
    fill_style_combobox = NULL;
    
    xml->get_widget("dialog-vbox", dialog_vbox);
    xml->get_widget("map_size_combobox", map_size_combobox);
    xml->get_widget("width_spinbutton", width_spinbutton);
    xml->get_widget("height_spinbutton", height_spinbutton);
    xml->get_widget("random_map_container", random_map_container);
    xml->get_widget("grass_scale", grass_scale);
    xml->get_widget("water_scale", water_scale);
    xml->get_widget("swamp_scale", swamp_scale);
    xml->get_widget("forest_scale", forest_scale);
    xml->get_widget("hills_scale", hills_scale);
    xml->get_widget("mountains_scale", mountains_scale);
    xml->get_widget("cities_scale", cities_scale);
    xml->get_widget("ruins_scale", ruins_scale);
    xml->get_widget("temples_scale", temples_scale);
    xml->get_widget("signposts_scale", signposts_scale);
    xml->get_widget("stones_scale", stones_scale);
    xml->get_widget("accept_button", accept_button);
    xml->get_widget("random_roads_switch", random_roads_switch);
    xml->get_widget("random_names_switch", random_names_switch);
    xml->get_widget("num_players_spinbutton", num_players_spinbutton);
    xml->get_widget("stone_road_spinbutton", stone_road_spinbutton);
    xml->get_widget ("notebook", notebook);

    
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
    selected_tile_size_id = default_id;
    tile_size_combobox->set_active(default_id);

    xml->get_widget("tile_size_box", box);
    box->pack_start(*tile_size_combobox, Gtk::PACK_SHRINK);

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

    populate_tileset_armyset_and_cityset (get_active_tile_size ());
  
    selected_tileset_id = tile_theme_combobox->get_active_row_number ();
    selected_armyset_id = army_theme_combobox->get_active_row_number ();
    selected_cityset_id = city_theme_combobox->get_active_row_number ();

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
    selected_shieldset_id = shield_theme_combobox->get_active_row_number ();

    xml->get_widget("shield_theme_box", box);
    box->pack_start(*shield_theme_combobox, Gtk::PACK_SHRINK);

    // create fill style combobox
    fill_style_combobox = manage(new Gtk::ComboBoxText);

    add_fill_style(Tile::GRASS);
    add_fill_style(Tile::WATER);
    add_fill_style(Tile::FOREST);
    add_fill_style(Tile::HILLS);
    add_fill_style(Tile::MOUNTAIN);
    add_fill_style(Tile::SWAMP);

    fill_style_combobox->append(_("Random"));
    fill_style.push_back(-1);

    Gtk::Alignment *alignment;
    xml->get_widget("fill_style_alignment", alignment);
    alignment->add(*fill_style_combobox);

    fill_style_combobox->set_active(6);
    selected_fill_style_id = 6;

    // map size
    selected_map_size_id = MAP_SIZE_NORMAL;
    map.width = MAP_SIZE_NORMAL_WIDTH;
    map.height = MAP_SIZE_NORMAL_HEIGHT;
    map.grass = 78;
    map.water = 7;
    map.swamp = 2;
    map.forest = 3;
    map.hills = 5;
    map.signposts = 20;
    map.stones = 40;
    map.mountains = 5;
    map.cities = 20;
    map.ruins = 25;
    map.temples = 25;
    map.generate_roads = true;
    map.random_names = true;
    map.num_players = 8;
    map.stone_road_chance = ROAD_STONE_CHANCE;

    //random_names_switch->set_active(map.random_names);
    //num_players_spinbutton->set_value(map.num_players);
    //stone_road_spinbutton->set_value (map.stone_road_chance);
    //stone_road_spinbutton->set_sensitive (false);
    
    xml->get_widget("undo_button", undo_button);
    undo_button->signal_activate ().connect (method (on_undo_activated));
    xml->get_widget("redo_button", redo_button);
    redo_button->signal_activate ().connect (method (on_redo_activated));
    connect_signals ();
    update ();
}

void NewMapDialog::run()
{
  dialog->show_all();
  int response = dialog->run();
  if (response == Gtk::RESPONSE_ACCEPT)	// accepted
    {
      int row = fill_style_combobox->get_active_row_number();
      assert(row >= 0 && row < int(fill_style.size()));
      map.fill_style = fill_style[row];

      map.tileset = Tilesetlist::getInstance()->getSetDir
	(Glib::filename_from_utf8(tile_theme_combobox->get_active_text()),
	 get_active_tile_size());

      map.shieldset = Shieldsetlist::getInstance()->getSetDir
	(Glib::filename_from_utf8(shield_theme_combobox->get_active_text()));

      map.cityset = Citysetlist::getInstance()->getSetDir
	(Glib::filename_from_utf8(city_theme_combobox->get_active_text()),
	 get_active_tile_size());

      map.armyset = Armysetlist::getInstance()->getSetDir
	(Glib::filename_from_utf8(army_theme_combobox->get_active_text()),
	 get_active_tile_size());

      map_set = true;
    }
  else
    map_set = false;
}

void NewMapDialog::on_fill_style_changed()
{
  umgr->add (new NewMapAction_FillStyle (selected_fill_style_id));
  int row = fill_style_combobox->get_active_row_number();
  selected_fill_style_id = row;
  assert(row >= 0 && row < int(fill_style.size()));
  bool random_selected = fill_style[row] == -1;
  random_map_container->set_sensitive(random_selected);
  random_roads_switch->set_sensitive(random_selected);
  random_names_switch->set_sensitive(random_selected);
  update_button ();
}

void NewMapDialog::on_map_size_changed()
{
  umgr->add (new NewMapAction_MapSize (map, selected_map_size_id));
  selected_map_size_id = map_size_combobox->get_active_row_number ();
  switch (map_size_combobox->get_active_row_number())
    {
    case MAP_SIZE_SMALL:
      map.width = MAP_SIZE_SMALL_WIDTH;
      map.height = MAP_SIZE_SMALL_HEIGHT;
      map.cities = 15;
      map.ruins = 20;
      map.temples = 20;
      break;

    case MAP_SIZE_TINY:
      map.width = MAP_SIZE_TINY_WIDTH;
      map.height = MAP_SIZE_TINY_HEIGHT;
      map.cities = 10;
      map.ruins = 15;
      map.temples = 15;
      break;

    case MAP_SIZE_NORMAL:
    default:
      map.width = MAP_SIZE_NORMAL_WIDTH;
      map.height = MAP_SIZE_NORMAL_HEIGHT;
      map.cities = 20;
      map.ruins = 25;
      map.temples = 25;
      break;
    case MAP_SIZE_CUSTOM:
      map.width = MAP_SIZE_NORMAL_WIDTH;
      map.height = MAP_SIZE_NORMAL_HEIGHT;
      map.cities = 20;
      map.ruins = 25;
      map.temples = 25;
      break;
    }
  update ();
}

void NewMapDialog::add_fill_style(Tile::Type tile_type)
{
  Tileset *tileset = GameMap::getTileset();
  Tile *tile = (*tileset)[tileset->getIndex(tile_type)];
  fill_style_combobox->append(tile->getName());
  fill_style.push_back(tile_type);
}

void NewMapDialog::populate_tileset_armyset_and_cityset (guint32 ts)
{
  guint32 default_id = 0;
  guint32 counter = 0;

  tile_theme_combobox->remove_all();
  Tilesetlist *tl = Tilesetlist::getInstance();
  std::list<Glib::ustring> tile_themes = tl->getValidNames(ts);
  for (std::list<Glib::ustring>::iterator i = tile_themes.begin(),
       end = tile_themes.end(); i != end; ++i)
    {
      if (*i == _("Default"))
	default_id = counter;
      tile_theme_combobox->append(Glib::filename_to_utf8(*i));
      counter++;
    }

  tile_theme_combobox->set_active(default_id);

  army_theme_combobox->remove_all();
  Armysetlist *al = Armysetlist::getInstance();
  std::list<Glib::ustring> army_themes = al->getValidNames(ts);
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

  army_theme_combobox->set_active(default_id);

  city_theme_combobox->remove_all();
  Citysetlist *cl = Citysetlist::getInstance();
  std::list<Glib::ustring> city_themes = cl->getValidNames(ts);
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

  city_theme_combobox->set_active(default_id);
}

guint32 NewMapDialog::get_active_tile_size()
{
  return (guint32) atoi(tile_size_combobox->get_active_text().c_str());
}

void NewMapDialog::on_tile_size_changed()
{
  umgr->add (new NewMapAction_TileSize (selected_tile_size_id,
                                        selected_tileset_id,
                                        selected_armyset_id,
                                        selected_cityset_id));
  disconnect_signals ();
  selected_tile_size_id = tile_size_combobox->get_active_row_number ();
  populate_tileset_armyset_and_cityset (get_active_tile_size ());
  selected_tileset_id = tile_theme_combobox->get_active_row_number ();
  selected_armyset_id = army_theme_combobox->get_active_row_number ();
  selected_cityset_id = city_theme_combobox->get_active_row_number ();
  connect_signals ();
  update_button ();
}

void NewMapDialog::update_button ()
{
  bool sens = true;
  if (city_theme_combobox->get_model()->children().size() == 0 ||
      army_theme_combobox->get_model()->children().size() == 0 ||
      tile_theme_combobox->get_model()->children().size() == 0)
    sens = false;

  if (fill_style_combobox)
    {
      int row = fill_style_combobox->get_active_row_number();
      assert(row >= 0 && row < int(fill_style.size()));
      bool random_selected = fill_style[row] == -1;
      if (random_selected)
        accept_button->set_label (_("Create Random Map"));
      else
        {
          switch (Tile::Type (fill_style[row]))
            {
            case Tile::GRASS:
              accept_button->set_label (_("Create Grass Map"));
              break;
            case Tile::WATER:
              accept_button->set_label (_("Create Water Map"));
              break;
            case Tile::FOREST:
              accept_button->set_label (_("Create Forest Map"));
              break;
            case Tile::HILLS:
              accept_button->set_label (_("Create Hills Map"));
              break;
            case Tile::MOUNTAIN:
              accept_button->set_label (_("Create Mountains Map"));
              break;
            case Tile::SWAMP:
              accept_button->set_label (_("Create Swamp Map"));
              break;
            }
        }
    }
  accept_button->set_sensitive(sens);
}

void NewMapDialog::on_random_roads_toggled ()
{
  umgr->add (new NewMapAction_RandomRoads (map.generate_roads));
  stone_road_spinbutton->set_sensitive (random_roads_switch->get_active ());
  map.generate_roads = random_roads_switch->get_active ();
}

void NewMapDialog::on_random_names_toggled ()
{
  umgr->add (new NewMapAction_RandomNames (map.random_names));
  map.random_names = random_names_switch->get_active ();
}

void NewMapDialog::setup_progress_bar ()
{
  progress_treeview = Gtk::manage (new Gtk::TreeView ());
  progress_treeview->property_headers_visible () = false;
  progress_liststore = Gtk::ListStore::create(progress_columns);
  progress_treeview->set_model (progress_liststore);
  progress_row = *(progress_liststore->append());
  auto cell = Gtk::manage(new Gtk::CellRendererProgress());
  cell->property_text () = "";
  int cols_count = progress_treeview->append_column ("progress", *cell);
  auto pColumn = progress_treeview->get_column(cols_count -1);
  if (pColumn)
    pColumn->add_attribute(cell->property_value (), progress_columns.perc);

  dialog_vbox->pack_end (*progress_treeview, true, true);
  dialog_vbox->show_all ();
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
  dialog_vbox->set_sensitive(false);
}

void NewMapDialog::tick_progress (double p)
{
  if (!progress_treeview)
    return;
  progress_row[progress_columns.perc] = p * 100.0;
  while (g_main_context_iteration(NULL, FALSE)); //doEvents
}

NewMapDialog::~NewMapDialog()
{
  delete umgr;
  notebook->property_show_tabs () = false;
}

void NewMapDialog::on_undo_activated ()
{
  umgr->undo ();
  update ();
}

void NewMapDialog::on_redo_activated ()
{
  umgr->redo ();
  update ();
}

UndoAction *NewMapDialog::executeAction (UndoAction *action2)
{
  NewMapAction *action = dynamic_cast<NewMapAction*>(action2);
  UndoAction *out = NULL;

    switch (action->getType ())
      {
      case NewMapAction::GRASS:
          {
            NewMapAction_Grass *a =
              dynamic_cast<NewMapAction_Grass*>(action);
            out = new NewMapAction_Grass (grass_scale->get_value ());
            map.grass = a->getValue ();
          }
        break;
      case NewMapAction::WATER:
          {
            NewMapAction_Water *a =
              dynamic_cast<NewMapAction_Water*>(action);
            out = new NewMapAction_Water (water_scale->get_value ());
            map.water = a->getValue ();
          }
        break;
      case NewMapAction::SWAMP:
          {
            NewMapAction_Swamp *a =
              dynamic_cast<NewMapAction_Swamp*>(action);
            out = new NewMapAction_Swamp (swamp_scale->get_value ());
            map.swamp = a->getValue ();
          }
        break;
      case NewMapAction::FOREST:
          {
            NewMapAction_Forest *a =
              dynamic_cast<NewMapAction_Forest*>(action);
            out = new NewMapAction_Forest (forest_scale->get_value ());
            map.forest = a->getValue ();
          }
        break;
      case NewMapAction::HILLS:
          {
            NewMapAction_Hills *a =
              dynamic_cast<NewMapAction_Hills*>(action);
            out = new NewMapAction_Hills (hills_scale->get_value ());
            map.hills = a->getValue ();
          }
        break;
      case NewMapAction::MOUNTAINS:
          {
            NewMapAction_Mountains *a =
              dynamic_cast<NewMapAction_Mountains*>(action);
            out = new NewMapAction_Mountains (mountains_scale->get_value ());
            map.mountains = a->getValue ();
          }
        break;
      case NewMapAction::CITIES:
          {
            NewMapAction_Cities *a =
              dynamic_cast<NewMapAction_Cities*>(action);
            out = new NewMapAction_Cities (cities_scale->get_value ());
            map.cities = a->getValue ();
          }
        break;
      case NewMapAction::RUINS:
          {
            NewMapAction_Ruins *a =
              dynamic_cast<NewMapAction_Ruins*>(action);
            out = new NewMapAction_Ruins (ruins_scale->get_value ());
            map.ruins = a->getValue ();
          }
        break;
      case NewMapAction::TEMPLES:
          {
            NewMapAction_Temples *a =
              dynamic_cast<NewMapAction_Temples*>(action);
            out = new NewMapAction_Temples (temples_scale->get_value ());
            map.temples = a->getValue ();
          }
        break;
      case NewMapAction::SIGNPOSTS:
          {
            NewMapAction_Signposts *a =
              dynamic_cast<NewMapAction_Signposts*>(action);
            out = new NewMapAction_Signposts (signposts_scale->get_value ());
            map.signposts = a->getValue ();
          }
        break;
      case NewMapAction::STONES:
          {
            NewMapAction_Stones *a =
              dynamic_cast<NewMapAction_Stones*>(action);
            out = new NewMapAction_Stones (stones_scale->get_value ());
            map.stones = a->getValue ();
          }
        break;
      case NewMapAction::MAP_SIZE:
          {
            NewMapAction_MapSize *a =
              dynamic_cast<NewMapAction_MapSize*>(action);
            out = new NewMapAction_MapSize (map, selected_map_size_id);
            map = a->getMap ();
            selected_map_size_id = a->getMapSizeId ();
          }
        break;
      case NewMapAction::WIDTH:
          {
            NewMapAction_Width *a =
              dynamic_cast<NewMapAction_Width*>(action);
            out = new NewMapAction_Width (map.width);
            map.width = a->getWidth ();
          }
        break;
      case NewMapAction::HEIGHT:
          {
            NewMapAction_Height *a =
              dynamic_cast<NewMapAction_Height*>(action);
            out = new NewMapAction_Height (map.height);
            map.height = a->getHeight ();
          }
        break;
      case NewMapAction::TILESET:
          {
            NewMapAction_TileSet *a =
              dynamic_cast<NewMapAction_TileSet*>(action);
            out = new NewMapAction_TileSet (selected_tileset_id);
            selected_tileset_id = a->getIndex ();
          }
        break;
      case NewMapAction::ARMYSET:
          {
            NewMapAction_ArmySet *a =
              dynamic_cast<NewMapAction_ArmySet*>(action);
            out = new NewMapAction_ArmySet (selected_armyset_id);
            selected_armyset_id = a->getIndex ();
          }
        break;
      case NewMapAction::CITYSET:
          {
            NewMapAction_CitySet *a =
              dynamic_cast<NewMapAction_CitySet*>(action);
            out = new NewMapAction_CitySet (selected_cityset_id);
            selected_cityset_id = a->getIndex ();
          }
        break;
      case NewMapAction::SHIELDSET:
          {
            NewMapAction_ShieldSet *a =
              dynamic_cast<NewMapAction_ShieldSet*>(action);
            out = new NewMapAction_ShieldSet (selected_shieldset_id);
            selected_shieldset_id = a->getIndex ();
          }
        break;
      case NewMapAction::TILE_SIZE:
          {
            NewMapAction_TileSize *a =
              dynamic_cast<NewMapAction_TileSize*>(action);
            out = new NewMapAction_TileSize (selected_tile_size_id,
                                             selected_tileset_id,
                                             selected_armyset_id,
                                             selected_cityset_id);
            selected_tile_size_id = a->getTileSizeId ();
            selected_tileset_id = a->getTileSetId ();
            selected_armyset_id = a->getArmySetId ();
            selected_cityset_id = a->getCitySetId ();
            disconnect_signals ();
            tile_size_combobox->set_active(a->getTileSizeId ());
            populate_tileset_armyset_and_cityset (get_active_tile_size ());
            connect_signals ();
          }
        break;
      case NewMapAction::RANDOM_ROADS:
          {
            NewMapAction_RandomRoads *a =
              dynamic_cast<NewMapAction_RandomRoads*>(action);
            out = new NewMapAction_RandomRoads (map.generate_roads);
            map.generate_roads = a->getValue ();
          }
        break;
      case NewMapAction::FILL_STYLE:
          {
            NewMapAction_FillStyle *a =
              dynamic_cast<NewMapAction_FillStyle*>(action);
            out = new NewMapAction_FillStyle (selected_fill_style_id);
            selected_fill_style_id = a->getIndex ();
          }
        break;
      case NewMapAction::RANDOM_NAMES:
          {
            NewMapAction_RandomNames *a =
              dynamic_cast<NewMapAction_RandomNames*>(action);
            out = new NewMapAction_RandomNames (map.random_names);
            map.random_names = a->getValue ();
          }
        break;
      case NewMapAction::PLAYERS:
          {
            NewMapAction_Players *a =
              dynamic_cast<NewMapAction_Players*>(action);
            out = new NewMapAction_Players (map.num_players);
            map.num_players = a->getNumPlayers ();
          }
        break;
      case NewMapAction::STONE_ROAD_CHANCE:
          {
            NewMapAction_StoneRoadChance *a =
              dynamic_cast<NewMapAction_StoneRoadChance*>(action);
            out = new NewMapAction_StoneRoadChance (map.stone_road_chance);
            map.stone_road_chance = a->getStoneRoadChance ();
          }
        break;
      }
    return out;
}

void NewMapDialog::connect_signals ()
{
  connections.push_back
    (grass_scale->signal_value_changed ().connect (method (on_grass_changed)));
  connections.push_back
    (water_scale->signal_value_changed ().connect (method (on_water_changed)));
  connections.push_back
    (swamp_scale->signal_value_changed ().connect (method (on_swamp_changed)));
  connections.push_back
    (forest_scale->signal_value_changed ().connect
     (method (on_forest_changed)));
  connections.push_back
    (hills_scale->signal_value_changed ().connect (method (on_hills_changed)));
  connections.push_back
    (mountains_scale->signal_value_changed ().connect
     (method (on_mountains_changed)));
  connections.push_back
    (cities_scale->signal_value_changed ().connect
     (method (on_cities_changed)));
  connections.push_back
    (ruins_scale->signal_value_changed ().connect (method (on_ruins_changed)));
  connections.push_back
    (temples_scale->signal_value_changed ().connect
     (method (on_temples_changed)));
  connections.push_back
    (signposts_scale->signal_value_changed ().connect
     (method (on_signposts_changed)));
  connections.push_back
    (stones_scale->signal_value_changed ().connect
     (method (on_stones_changed)));
  connections.push_back
    (map_size_combobox->signal_changed().connect(method(on_map_size_changed)));
  connections.push_back
    (width_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_width_changed)))));
  connections.push_back
    (height_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_height_changed)))));
  connections.push_back
    (tile_theme_combobox->signal_changed().connect
     (method(on_tileset_changed)));
  connections.push_back
    (army_theme_combobox->signal_changed().connect
     (method(on_armyset_changed)));
  connections.push_back
    (city_theme_combobox->signal_changed().connect
     (method(on_cityset_changed)));
  connections.push_back
    (shield_theme_combobox->signal_changed().connect
     (method(on_shieldset_changed)));
  connections.push_back
    (tile_size_combobox->signal_changed().connect
     (method(on_tile_size_changed)));
  connections.push_back
    (random_roads_switch->property_active().signal_changed().connect
      (method(on_random_roads_toggled)));
  connections.push_back
    (fill_style_combobox->signal_changed().connect
     (method(on_fill_style_changed)));
  connections.push_back
    (random_names_switch->property_active().signal_changed().connect
      (method(on_random_names_toggled)));
  connections.push_back
    (num_players_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_players_changed)))));
  connections.push_back
    (stone_road_spinbutton->signal_insert_text().connect
     (sigc::hide(sigc::hide(method(on_stone_road_chance_changed)))));
}

void NewMapDialog::on_grass_changed ()
{
  NewMapAction_Grass *action = new NewMapAction_Grass (map.grass);
  umgr->add (action);
  map.grass = int(grass_scale->get_value ());
}

void NewMapDialog::on_water_changed ()
{
  NewMapAction_Water *action = new NewMapAction_Water (map.water);
  umgr->add (action);
  map.water = int(water_scale->get_value ());
}

void NewMapDialog::on_swamp_changed ()
{
  NewMapAction_Swamp *action = new NewMapAction_Swamp (map.swamp);
  umgr->add (action);
  map.swamp = int(swamp_scale->get_value ());
}

void NewMapDialog::on_forest_changed ()
{
  NewMapAction_Forest *action = new NewMapAction_Forest (map.forest);
  umgr->add (action);
  map.forest = int(forest_scale->get_value ());
}

void NewMapDialog::on_hills_changed ()
{
  NewMapAction_Hills *action = new NewMapAction_Hills (map.hills);
  umgr->add (action);
  map.hills = int(hills_scale->get_value ());
}

void NewMapDialog::on_mountains_changed ()
{
  NewMapAction_Mountains *action = new NewMapAction_Mountains (map.mountains);
  umgr->add (action);
  map.mountains = int(mountains_scale->get_value ());
}

void NewMapDialog::on_cities_changed ()
{
  NewMapAction_Cities *action = new NewMapAction_Cities (map.cities);
  umgr->add (action);
  map.cities = int(cities_scale->get_value ());
}

void NewMapDialog::on_ruins_changed ()
{
  NewMapAction_Ruins *action = new NewMapAction_Ruins (map.ruins);
  umgr->add (action);
  map.ruins = int(ruins_scale->get_value ());
}

void NewMapDialog::on_temples_changed ()
{
  NewMapAction_Temples *action = new NewMapAction_Temples (map.temples);
  umgr->add (action);
  map.temples = int(temples_scale->get_value ());
}

void NewMapDialog::on_signposts_changed ()
{
  NewMapAction_Signposts *action = new NewMapAction_Signposts (map.signposts);
  umgr->add (action);
  map.signposts = int(signposts_scale->get_value ());
}

void NewMapDialog::on_stones_changed ()
{
  NewMapAction_Stones *action = new NewMapAction_Stones (map.stones);
  umgr->add (action);
  map.stones = int(stones_scale->get_value ());
}

void NewMapDialog::on_tileset_changed ()
{
  NewMapAction_TileSet *action = new NewMapAction_TileSet (selected_tileset_id);
  umgr->add (action);
  selected_tileset_id = tile_theme_combobox->get_active_row_number ();
}

void NewMapDialog::on_armyset_changed ()
{
  NewMapAction_ArmySet *action = new NewMapAction_ArmySet (selected_armyset_id);
  umgr->add (action);
  selected_armyset_id = army_theme_combobox->get_active_row_number ();
}

void NewMapDialog::on_cityset_changed ()
{
  NewMapAction_CitySet *action = new NewMapAction_CitySet (selected_cityset_id);
  umgr->add (action);
  selected_cityset_id = city_theme_combobox->get_active_row_number ();
}

void NewMapDialog::on_shieldset_changed ()
{
  NewMapAction_ShieldSet *action =
    new NewMapAction_ShieldSet (selected_shieldset_id);
  umgr->add (action);
  selected_shieldset_id = shield_theme_combobox->get_active_row_number ();
}

void NewMapDialog::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

void NewMapDialog::update ()
{
  disconnect_signals ();
  grass_scale->set_value(map.grass);
  water_scale->set_value(map.water);
  swamp_scale->set_value(map.swamp);
  forest_scale->set_value(map.forest);
  hills_scale->set_value(map.hills);
  mountains_scale->set_value(map.mountains);
  cities_scale->set_value(map.cities);
  ruins_scale->set_value(map.ruins);
  temples_scale->set_value(map.temples);
  signposts_scale->set_value(map.signposts);
  stones_scale->set_value(map.stones);
  width_spinbutton->set_value (map.width);
  height_spinbutton->set_value (map.height);
  map_size_combobox->set_active (selected_map_size_id);
  width_spinbutton->set_sensitive (selected_map_size_id == MAP_SIZE_CUSTOM);
  height_spinbutton->set_sensitive (selected_map_size_id == MAP_SIZE_CUSTOM);
  tile_theme_combobox->set_active (selected_tileset_id);
  army_theme_combobox->set_active (selected_armyset_id);
  city_theme_combobox->set_active (selected_cityset_id);
  shield_theme_combobox->set_active (selected_shieldset_id);
  tile_size_combobox->set_active (selected_tile_size_id);
  random_roads_switch->set_active (map.generate_roads);
  stone_road_spinbutton->set_sensitive (map.generate_roads);

  fill_style_combobox->set_active (selected_fill_style_id);
  bool random_selected = fill_style[selected_fill_style_id] == -1;
  random_map_container->set_sensitive(random_selected);
  random_roads_switch->set_sensitive(random_selected);
  random_names_switch->set_sensitive(random_selected);

  random_names_switch->set_active (map.random_names);

  num_players_spinbutton->set_value (map.num_players);
  stone_road_spinbutton->set_value (map.stone_road_chance);
  update_button ();
  connect_signals ();
}

void NewMapDialog::on_width_changed ()
{
  umgr->add (new NewMapAction_Width (map.width));
  map.width = atoi (width_spinbutton->get_text ().c_str ());
}

void NewMapDialog::on_height_changed ()
{
  umgr->add (new NewMapAction_Width (map.height));
  map.height = atoi (height_spinbutton->get_text ().c_str ());
}

void NewMapDialog::on_players_changed ()
{
  umgr->add (new NewMapAction_Players (map.num_players));
  map.num_players = atoi (num_players_spinbutton->get_text ().c_str ());
}

void NewMapDialog::on_stone_road_chance_changed ()
{
  umgr->add (new NewMapAction_StoneRoadChance (map.stone_road_chance));
  map.stone_road_chance = atoi (stone_road_spinbutton->get_text ().c_str ());
}

