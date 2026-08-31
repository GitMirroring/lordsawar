//  Copyright (C) 2021, 2026 Ben Asselstine
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

#pragma once
#ifndef SCENARIO_H
#define SCENARIO_H

#include <gtkmm.h>

class GameScenario;
class GameMap;
class ScenarioMedia;
class ID_Counter;
class Itemlist;
class Playerlist;
class Citylist;
class Templelist;
class Ruinlist;
class Rewardlist;
class Signpostlist;
class Roadlist;
class Stonelist;
class Portlist;
class Bridgelist;
class HeroTemplates;

#include "game-scenario.h"
#include "game-map.h"
#include "scenario-media.h"
#include "counter.h"
#include "item-list.h"
#include "player-list.h"
#include "city-list.h"
#include "temple-list.h"
#include "ruin-list.h"
#include "reward-list.h"
#include "signpost-list.h"
#include "road-list.h"
#include "stone-list.h"
#include "port-list.h"
#include "bridge-list.h"
#include "hero-templates.h"
//! A copy of the game model
/**
 * Holds all of the objects that comprise the data model of a scenario.
 * citylists, game scenario, game map, and so on.
 *
 */
class Scenario
{
public:

    //! Default constructor.
    Scenario (const GameScenario *g)
      : m_game_scenario (new GameScenario (*g, false)),
      m_game_map (GameMap::instance ()->copy ()),
      m_scenario_media (ScenarioMedia::instance ()->copy ()),
      m_id_counter (id_counter->copy ()),
      m_itemlist (Itemlist::instance ()->copy ()),
      m_playerlist (Playerlist::instance ()->copy ()),
      m_citylist (Citylist::instance ()->copy ()),
      m_templelist (Templelist::instance ()->copy ()),
      m_ruinlist (Ruinlist::instance ()->copy ()),
      m_rewardlist (Rewardlist::instance ()->copy ()),
      m_signpostlist (Signpostlist::instance ()->copy ()),
      m_roadlist (Roadlist::instance ()->copy ()),
      m_stonelist (Stonelist::instance ()->copy ()),
      m_portlist (Portlist::instance ()->copy ()),
      m_bridgelist (Bridgelist::instance ()->copy ()),
      m_hero_templates (HeroTemplates::instance ()->copy ())
  {
    m_resetted = false;
  }

    //! Copy constructor.
    Scenario (const Scenario &s)
      : m_game_scenario (new GameScenario (*s.m_game_scenario, false)),
      m_game_map (s.m_game_map->copy ()),
      m_scenario_media (s.m_scenario_media->copy ()),
      m_id_counter (s.m_id_counter->copy ()),
      m_itemlist (s.m_itemlist->copy ()),
      m_playerlist (s.m_playerlist->copy ()),
      m_citylist (s.m_citylist->copy ()),
      m_templelist (s.m_templelist->copy ()),
      m_ruinlist (s.m_ruinlist->copy ()),
      m_rewardlist (s.m_rewardlist->copy ()),
      m_signpostlist (s.m_signpostlist->copy ()),
      m_roadlist (s.m_roadlist->copy ()),
      m_stonelist (s.m_stonelist->copy ()),
      m_portlist (s.m_portlist->copy ()),
      m_bridgelist (s.m_bridgelist->copy ()),
      m_hero_templates (s.m_hero_templates->copy ())
  {
    m_resetted = s.m_resetted;
  }

    //! Destructor.
    ~Scenario ()
      {
        if (!m_resetted)
          {
            delete m_game_scenario;
            delete m_game_map;
            delete m_scenario_media;
            delete m_id_counter;
            delete m_itemlist;
            delete m_playerlist;
            delete m_citylist;
            delete m_templelist;
            delete m_ruinlist;
            delete m_rewardlist;
            delete m_signpostlist;
            delete m_roadlist;
            delete m_stonelist;
            delete m_portlist;
            delete m_bridgelist;
            delete m_hero_templates;
          }
      }

    GameScenario *get_game_scenario () const
      {
        return m_game_scenario;
      }

    static void reset (Scenario *s)
      {
        GameMap::instance ()->reset (s->m_game_map);
        ScenarioMedia::instance ()->reset (s->m_scenario_media);
        id_counter->reset (s->m_id_counter);
        Itemlist::instance ()->reset (s->m_itemlist);
        Playerlist::instance ()->reset (s->m_playerlist);
        Citylist::instance ()->reset (s->m_citylist);
        Templelist::instance ()->reset (s->m_templelist);
        Ruinlist::instance ()->reset (s->m_ruinlist);
        Rewardlist::instance ()->reset (s->m_rewardlist);
        Signpostlist::instance ()->reset (s->m_signpostlist);
        Roadlist::instance ()->reset (s->m_roadlist);
        Stonelist::instance ()->reset (s->m_stonelist);
        Portlist::instance ()->reset (s->m_portlist);
        Bridgelist::instance ()->reset (s->m_bridgelist);
        HeroTemplates::instance ()->reset (s->m_hero_templates);
        s->m_resetted = true;
      }
protected:

    // DATA

    GameScenario *m_game_scenario;
    GameMap *m_game_map;
    ScenarioMedia *m_scenario_media;
    ID_Counter *m_id_counter;
    Itemlist *m_itemlist;
    Playerlist *m_playerlist;
    Citylist *m_citylist;
    Templelist *m_templelist;
    Ruinlist *m_ruinlist;
    Rewardlist *m_rewardlist;
    Signpostlist *m_signpostlist;
    Roadlist *m_roadlist;
    Stonelist *m_stonelist;
    Portlist *m_portlist;
    Bridgelist *m_bridgelist;
    HeroTemplates *m_hero_templates;
    bool m_resetted;
};

#endif
