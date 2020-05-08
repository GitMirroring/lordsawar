//  Copyright (C) 2020 Ben Asselstine
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

#include <fstream>
#include <sstream>
#include "scenario-details.h"
#include "GameScenario.h"

ScenarioDetails::ScenarioDetails(Glib::ustring id, guint32 num_cities, 
                                 guint32 num_players, Glib::ustring name,
                                 Glib::ustring desc, Glib::ustring filename)
: d_id(id), d_number_of_cities(num_cities), d_number_of_players(num_players),
    d_name(name), d_desc (desc), d_filename(filename)
{
}

ScenarioDetails::ScenarioDetails (Glib::ustring filename, bool &broken)
 : d_filename (filename)
{
  guint32 player_count, city_count;
  Glib::ustring id, comment, name;
  GameScenario::loadDetails(filename, broken, player_count, city_count, name, comment, id);
  if (!broken)
    {
      d_number_of_players = player_count;
      d_number_of_cities = city_count;
      d_name = name;
      d_id = id;
      d_desc = comment;
    }
}
