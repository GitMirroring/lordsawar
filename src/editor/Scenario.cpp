//  Copyright (C) 2021 Ben Asselstine
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

#include "Scenario.h"
#include "GameScenario.h"
#include "GameMap.h"
#include "ScenarioMedia.h"
#include "counter.h"
#include "Itemlist.h"
#include "playerlist.h"
#include "citylist.h"
#include "templelist.h"
#include "ruinlist.h"
#include "rewardlist.h"
#include "signpostlist.h"
#include "roadlist.h"
#include "stonelist.h"
#include "portlist.h"
#include "bridgelist.h"
#include "herotemplates.h"

Scenario::Scenario (const GameScenario *g)
 : d_game_scenario (new GameScenario (*g, false)),
  d_game_map (GameMap::getInstance ()->copy ()),
  d_scenario_media (ScenarioMedia::getInstance ()->copy ()),
  d_fl_counter (fl_counter->copy ()),
  d_itemlist (Itemlist::getInstance ()->copy ()),
  d_playerlist (Playerlist::getInstance ()->copy ()),
  d_citylist (Citylist::getInstance ()->copy ()),
  d_templelist (Templelist::getInstance ()->copy ()),
  d_ruinlist (Ruinlist::getInstance ()->copy ()),
  d_rewardlist (Rewardlist::getInstance ()->copy ()),
  d_signpostlist (Signpostlist::getInstance ()->copy ()),
  d_roadlist (Roadlist::getInstance ()->copy ()),
  d_stonelist (Stonelist::getInstance ()->copy ()),
  d_portlist (Portlist::getInstance ()->copy ()),
  d_bridgelist (Bridgelist::getInstance ()->copy ()),
  d_hero_templates (HeroTemplates::getInstance ()->copy ())
{
  d_resetted = false;
}
        
Scenario::Scenario (const Scenario &s)
 : d_game_scenario (new GameScenario (*s.d_game_scenario, false)),
  d_game_map (s.d_game_map->copy ()),
  d_scenario_media (d_scenario_media->copy ()),
  d_fl_counter (s.d_fl_counter->copy ()),
  d_itemlist (s.d_itemlist->copy ()),
  d_playerlist (s.d_playerlist->copy ()),
  d_citylist (s.d_citylist->copy ()),
  d_templelist (s.d_templelist->copy ()),
  d_ruinlist (s.d_ruinlist->copy ()),
  d_rewardlist (s.d_rewardlist->copy ()),
  d_signpostlist (s.d_signpostlist->copy ()),
  d_roadlist (s.d_roadlist->copy ()),
  d_stonelist (s.d_stonelist->copy ()),
  d_portlist (s.d_portlist->copy ()),
  d_bridgelist (s.d_bridgelist->copy ()),
  d_hero_templates (s.d_hero_templates->copy ())
{
  d_resetted = s.d_resetted;
}

Scenario::~Scenario ()
{
  if (!d_resetted)
    {
      delete d_game_scenario;
      delete d_game_map;
      delete d_scenario_media;
      delete d_fl_counter;
      delete d_itemlist;
      delete d_playerlist;
      delete d_citylist;
      delete d_templelist;
      delete d_ruinlist;
      delete d_rewardlist;
      delete d_signpostlist;
      delete d_roadlist;
      delete d_stonelist;
      delete d_portlist;
      delete d_bridgelist;
      delete d_hero_templates;
    }
}

void Scenario::reset (Scenario *s)
{
  GameMap::getInstance()->reset (s->d_game_map);
  ScenarioMedia::getInstance()->reset (s->d_scenario_media);
  fl_counter->reset (s->d_fl_counter);
  Itemlist::getInstance()->reset (s->d_itemlist);
  Playerlist::getInstance()->reset (s->d_playerlist);
  Citylist::getInstance()->reset (s->d_citylist);
  Templelist::getInstance()->reset (s->d_templelist);
  Ruinlist::getInstance()->reset (s->d_ruinlist);
  Rewardlist::getInstance()->reset (s->d_rewardlist);
  Signpostlist::getInstance()->reset (s->d_signpostlist);
  Roadlist::getInstance()->reset (s->d_roadlist);
  Stonelist::getInstance()->reset (s->d_stonelist);
  Portlist::getInstance()->reset (s->d_portlist);
  Bridgelist::getInstance()->reset (s->d_bridgelist);
  HeroTemplates::getInstance()->reset (s->d_hero_templates);
  s->d_resetted = true;
}
