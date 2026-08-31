//  Copyright (C) 2026 Ben Asselstine
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

#include "boon.h"
#include <iostream>
#include "city.h"
#include "ruin.h"
#include "temple.h"
#include "map-backpack.h"
#include "stack.h"
#include "quest.h"
#include "quest-manager.h"
#include "player-list.h"
#include "game-scenario-options.h"
#include "game-map.h"
#include "stack-list.h"
#include "path.h"

using GSO = ::GameScenarioOptions;

Boon::Boon (const Boon &t)
 : LocationBox (t), m_city (t.m_city), m_ruin (t.m_ruin), m_temple (t.m_temple),
    m_backpack (t.m_backpack), m_quest (t.m_quest), m_value (t.m_value),
    m_expiry (t.m_expiry), m_turn_factor (t.m_turn_factor)

{
}

Boon::Boon (City *c)
 : LocationBox (*c), m_city (c), m_ruin (NULL), m_temple (NULL),
    m_backpack (NULL), m_quest (NULL), m_value (0), m_expiry (1),
    m_turn_factor (0)
{
}

Boon::Boon (Ruin *r)
 : LocationBox (*r), m_city (NULL), m_ruin (r), m_temple (NULL),
    m_backpack (NULL), m_quest (NULL), m_value (0), m_expiry (5),
    m_turn_factor (0)
{
}

Boon::Boon (Temple *t)
 : LocationBox (*t), m_city (NULL), m_ruin (NULL), m_temple (t),
    m_backpack (NULL), m_quest (NULL), m_value (0), m_expiry (0),
    m_turn_factor (0)
{
}

Boon::Boon (MapBackpack *b)
 : LocationBox (b->getPos ()), m_city (NULL), m_ruin (NULL), m_temple (NULL),
    m_backpack (b), m_quest (NULL), m_value (0), m_expiry (0), m_turn_factor (0)
{
}

Boon::Boon (Quest *q)
 : LocationBox (Vector<int>(0,0)), m_city (NULL), m_ruin (NULL),
    m_temple (NULL), m_backpack (NULL), m_quest (q), m_value (0), m_expiry (0),
    m_turn_factor (0)
{
}

Boon::~Boon ()
{
}

Glib::ustring Boon::to_string () const
{
  Glib::ustring expires = "";
  if (m_expiry > 0)
    expires = String::ucompose (" (expires in %1 turns)", m_expiry);
  if (m_city)
    {
      Glib::ustring bags =
        m_city->has_backpack () ? " has a backpack" : "";
      return
        "City " + m_city->getName () + " owned by " + m_city->getName () +
        bags + expires;
    }
  else if (m_ruin)
    {
      Glib::ustring bags =
        m_ruin->has_backpack () ? " has a backpack" : "";
      return "Ruin " + m_ruin->getName () + bags + expires;
    }
  else if (m_temple)
    {
      Glib::ustring bags =
        m_temple->has_backpack () ? " has a backpack" : "";
      return "Temple " + m_temple->getName () + bags + expires;
    }
  else if (m_backpack)
    return 
      String::ucompose ("backpack at %1,%2%3",
                        getPos ().x, getPos ().y, expires);
  else if (m_quest)
    {
      auto s =
        Playerlist::getActiveplayer ()->getStacklist ()->getArmyStackById
        (m_quest->getHeroId ());
      if (s)
        {
          Vector<int> pos = get_destination (s);
          return String::ucompose ("quest at %1,%2 belonging to %3%4 (%5)",
                                   pos.x, pos.y, m_quest->getHeroName (),
                                   expires, m_quest->getDescription ());
        }
      else
        return String::ucompose ("quest belonging to %1%2 (%3)",
                                 m_quest->getHeroName (), expires,
                                 m_quest->getDescription ());
    }
  return "";
}

void Boon::calculate (Stack *stack)
{
  m_value = calculate_value (stack, m_turn_factor);
}

float Boon::calculate_value (Stack *stack, float &turn_factor) const
{
  turn_factor = 0;
  auto dest = get_destination (stack);
  if (dest == Vector<int>(-1,-1))
    return 0;

  float value = 0;
  if (m_city)
    {
      value = calculate_city_value (stack);
      if (m_city->contains (stack->getPos ()))
        return value;
    }
  else if (m_ruin)
    {
      value = calculate_ruin_value (stack);
      if (m_ruin->contains (stack->getPos ()))
        return value;
    }
  else if (m_temple)
    {
      value = calculate_temple_value (stack);
      if (m_temple->contains (stack->getPos ()))
        return value;
    }
  else if (m_backpack)
    {
      value = calculate_backpack_value (stack);
      if (dest == stack->getPos ())
        return value;
    }
  else if (m_quest)
    {
      value = calculate_quest_value (stack);
      if (dest == stack->getPos ())
        return value;
    }

  guint32 moves = 0, turns = 0;
  Path p;
  moves = p.calculate (stack, dest, turns);

  if (moves == 0 || p.size () == 0) // if we can't get there
    return 0;
  if (m_expiry && turn_factor >= m_expiry) // if it takes too long
    return 0;

  // if we can get there, the value is the value.
  if (moves <= stack->getMaxMoves ())
    return value;

  turn_factor = (double)moves / (double)stack->getMaxMoves ();

  // if we can't get there we reduce the value
  // by how many turns away it is

  // turn factor isn't quite the same thing as turns
  // bc of ships and what not, but we like that it
  // captures partial turns.
  return value / turn_factor;
}

float Boon::calculate_city_value (Stack *stack) const
{
  float value = 100;
  if (m_city && m_city->has_backpack ())
    value += calculate_backpack_value (stack);
  return value;
}

float Boon::calculate_ruin_value (Stack *s) const
{
  if (s->hasHero () == false)
    return 0.0;
  float value = 65;
  if (m_ruin->getOwner () == s->getOwner ())
    value -= 20;
  if (m_ruin && m_ruin->has_backpack ())
    value += calculate_backpack_value (s);
  return value;
}

float Boon::calculate_temple_value (Stack *s) const
{
  float value = 0.0;
  Hero *h = s->getFirstHeroWithoutAQuest ();
  bool can_get_quest = false;
  if (h)
    {
      if (GSO::s_play_with_quests == GameParameters::ONE_QUEST_PER_HERO)
        can_get_quest = true;
      else if (GSO::s_play_with_quests == GameParameters::ONE_QUEST_PER_PLAYER)
        {
          auto p = Playerlist::getActiveplayer ();
          auto quests = QuestsManager::instance ()->getPlayerQuests (p);
          if (quests.size () == 0)
            can_get_quest = true;
        }
    }
  
  if (can_get_quest)
    value = 35;

  guint32 already_blessed = s->countArmiesBlessedAtTemple (m_temple->getId ());
  int num_to_be_blessed = s->size () - already_blessed;

  switch (num_to_be_blessed)
    {
    case 0:
      break;
    case 1:
      value += 10;
      break;
    case 2:
      value += 15;
      break;
    case 3:
      value += 17.5; 
      break;
    case 4:
      value += 20; 
      break;
    case 5:
      value += 22.5; 
      break;
    case 6:
      value += 25.0; 
      break;
    case 7:
      value += 27.5; 
      break;
    default:
      value += 30; 
      break;
    }

  if (m_temple && m_temple->has_backpack ())
    value += calculate_backpack_value (s);

  return value;
}

float Boon::calculate_backpack_value (Stack *s) const
{
  if (s->hasHero () == false)
    return 0.0;
  return 50;
}

float Boon::calculate_quest_value (Stack *s) const
{
  bool found = s->hasQuest (m_quest);
  if (!found)
    return 0;

  float value = 65;

  auto pos = get_destination (s);
  City *city = GameMap::getCity (pos);
  Ruin *ruin = GameMap::getRuin (pos);
  Temple *temple = GameMap::getTemple (pos);
  if (city && city->has_backpack ())
    value += calculate_backpack_value (s);
  else if (ruin && ruin->has_backpack ())
    value += calculate_backpack_value (s);
  else if (temple && temple->has_backpack ())
    value += calculate_backpack_value (s);
  else if (GameMap::getBackpack (pos)->size () > 0)
    value += calculate_backpack_value (s);
  return value;
}

Vector<int> Boon::get_destination (Stack *s) const
{
  // okay we've remembered all the positions but
  // for quests we need to look it up based on the
  // position and owner of a stack
  Vector<int> result (-1,-1);
  if (m_city && m_city->isVisible (s->getOwner ()))
    result = getNearestPos (s->getPos ());
  else if (m_ruin && m_ruin->isVisible (s->getOwner ()))
    result = getNearestPos (s->getPos ());
  else if (m_temple && m_temple->isVisible (s->getOwner ()))
    result = getNearestPos (s->getPos ());
  else if (m_backpack && GameMap::isVisible (getPos ()))
    result = getPos ();
  else if (m_quest)
    {
      auto dest = m_quest->getDestination (s);
      if (dest.getPos () != Vector<int>(-1,-1) && 
          dest.isVisible (s->getOwner ()))
        result = dest.getNearestPos (s->getPos ());
    }

  return result;
}
