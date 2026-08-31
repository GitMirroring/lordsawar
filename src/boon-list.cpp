//  Copyright (C) 2004 John Farrell
//  Copyright (C) 2004, 2005 Ulf Lorenz
//  Copyright (C) 2005 Andrea Paternesi
//  Copyright (C) 2007, 2009, 2010, 2014, 2026 Ben Asselstine
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

#include <iostream>

#include "boon-list.h"
#include "ruin.h"
#include "city.h"
#include "temple.h"
#include "map-backpack.h"
#include "quest.h"
#include "player-list.h"
#include "game-map.h"
#include "path-calculator.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<flush<<std::endl<<std::flush;}
#define debug(x)

Boonlist::Boonlist ()
{
}

Boonlist::~Boonlist ()
{
  fl_clear ();
}

bool Boonlist::compare_value (const Boon *lhs, const Boon *rhs)
{
  return lhs->get_value () > rhs->get_value ();
}

void Boonlist::sort_by_value ()
{
  sort (compare_value);
}

void Boonlist::add (Ruin *ruin)
{
  //if the ruin is already searched, it is not a boon.
  if (ruin->isSearched ())
    return;

  push_back (new Boon (ruin));
}

void Boonlist::add (Temple *temple)
{
  push_back (new Boon (temple));
}

void Boonlist::add (City *city)
{
  // if city is guarded or it's razed it is not a boon.
  if (city->countDefenders () != 0 || city->isBurnt ())
    return;
  // it has to be someone else's empty city, not our own.
  if (city->getOwner () == Playerlist::getActiveplayer ())
    return;
  push_back (new Boon (city));
}

void Boonlist::add (MapBackpack *backpack)
{
  // if bag is empty it is not a boon.
  if (backpack->size () == 0)
    return;
  push_back (new Boon (backpack));
}

void Boonlist::add (Quest *quest)
{
  //if quest is already completed it is not a boon.
  if (quest->isPendingDeletion ())
    return;
  push_back (new Boon (quest));
}

Glib::ustring Boonlist::to_string () const
{
  Glib::ustring result = "{";
  bool first = true;
  for (const_iterator it = begin (); it != end (); ++it)
    {
      if (first)
        first = false;
      else
        result += ",";
      result = result + " " + (*it)->to_string ();
    }
  result += " }";
  return result;
}

void Boonlist::fl_clear ()
{
  for (iterator it = begin (); it != end (); ++it)
    delete (*it);

  clear ();
}

Boonlist::iterator Boonlist::fl_erase (iterator object)
{
    delete (*object);
    return erase (object);
}

bool Boonlist::fl_remove (Boon* object)
{
  iterator it = find (begin (), end (), object);
  if (it != end ())
    {
      delete object;
      erase (it);
      return true;
    }
  return false;
}

void Boonlist::calculate (Stack *s)
{
  for (iterator it = begin (); it != end (); ++it)
    (*it)->calculate (s);
}

Boon *Boonlist::get_first_city_boon ()
{
  for (iterator it = begin (); it != end (); ++it)
    if ((*it)->is_city ())
      return *it;
  return NULL;
}

Boon *Boonlist::get_first_ruin_boon ()
{
  for (iterator it = begin (); it != end (); ++it)
    if ((*it)->is_ruin ())
      return *it;
  return NULL;
}

Boon *Boonlist::get_first_temple_boon ()
{
  for (iterator it = begin (); it != end (); ++it)
    if ((*it)->is_temple ())
      return *it;
  return NULL;
}

Boon *Boonlist::get_first_quest_boon ()
{
  for (iterator it = begin (); it != end (); ++it)
    if ((*it)->is_quest ())
      return *it;
  return NULL;
}

Boon *Boonlist::get_first_backpack_boon ()
{
  for (iterator it = begin (); it != end (); ++it)
    if ((*it)->is_backpack ())
      return *it;
  return NULL;
}

std::list<Boon> Boonlist::get_best_boons (Stack *s)
{
  calculate (s);
  sort_by_value ();

  std::list<Boon> boons;

  auto boon = get_first_city_boon ();
  if (boon && boon->get_value () > 0)
    boons.push_back (*boon);

  boon = get_first_ruin_boon ();
  if (boon && boon->get_value () > 0)
    boons.push_back (*boon);

  boon = get_first_temple_boon ();
  if (boon && boon->get_value () > 0)
    boons.push_back (*boon);

  boon = get_first_backpack_boon ();
  if (boon && boon->get_value () > 0)
    boons.push_back (*boon);

  boon = get_first_quest_boon ();
  if (boon && boon->get_value () > 0)
    boons.push_back (*boon);

  boons.sort
    ([](const auto& a, const auto& b)
     {
       return a.get_value () < b.get_value ();
     });
  return boons;
}
      
std::list<Boon> Boonlist::get_best_city_boons () const
{
  std::list<Boon> boons;
  for (auto it = begin (); it != end (); it++)
    if ((*it)->is_city () && (*it)->get_value () > 0)
      boons.push_back (**it);
  boons.sort
    ([](const auto& a, const auto& b)
     {
       return a.get_value () > b.get_value ();
     });
  return boons;
}

std::list<Boon> Boonlist::get_best_ruin_boons () const
{
  std::list<Boon> boons;
  for (auto it = begin (); it != end (); it++)
    if ((*it)->is_ruin () && (*it)->get_value () > 0)
      boons.push_back (**it);
  boons.sort
    ([](const auto& a, const auto& b)
     {
       return a.get_value () > b.get_value ();
     });
  return boons;
}

std::list<Boon> Boonlist::get_best_temple_boons () const
{
  std::list<Boon> boons;
  for (auto it = begin (); it != end (); it++)
    if ((*it)->is_temple () && (*it)->get_value () > 0)
      boons.push_back (**it);
  boons.sort
    ([](const auto& a, const auto& b)
     {
       return a.get_value () > b.get_value ();
     });
  return boons;
}

std::list<Boon> Boonlist::get_best_backpack_boons () const
{
  std::list<Boon> boons;
  for (auto it = begin (); it != end (); it++)
    if ((*it)->is_backpack () && (*it)->get_value () > 0)
      boons.push_back (**it);
  boons.sort
    ([](const auto& a, const auto& b)
     {
       return a.get_value () > b.get_value ();
     });
  return boons;
}

std::list<Boon> Boonlist::get_best_quest_boons () const
{
  std::list<Boon> boons;
  for (auto it = begin (); it != end (); it++)
    if ((*it)->is_quest () && (*it)->get_value () > 0)
      boons.push_back (**it);
  boons.sort
    ([](const auto& a, const auto& b)
     {
       return a.get_value () > b.get_value ();
     });
  return boons;
}
        
Boon* Boonlist::getClosestBoon (Stack *st, std::vector<Vector<int>> points)
{
  //cull the points into a smaller set
  std::vector<Vector<int>> pts;
  for (auto p : points)
    {
      auto building = GameMap::instance ()->getBuilding (p);
      if (building != Maptile::NONE)
        pts.push_back (p);
      else
        {
          Stack *s = GameMap::instance ()->getEnemyStack (p);
          if (s)
            pts.push_back (p);
          else
            {
              if (GameMap::getBackpack (p)->empty () == false)
                pts.push_back (p);
            }
        }
    }
  if (pts.empty ())
    return NULL;

  //second pass, only keep the boon positions
  std::vector<Vector<int>> boonpoints;
  for (auto p : pts)
    {
      for (auto it = begin (); it != end (); ++it)
        {
          if ((*it)->get_destination (st) == p &&
              (*it)->get_value () > 0)
            boonpoints.push_back (p);
        }
    }

  PathCalculator pc (st);
  Vector<int> pos = pc.getClosestPoint (boonpoints);
      
  for (auto it = begin (); it != end (); ++it)
    {
      if ((*it)->get_destination (st) == pos)
        return *it;
    }
    
  return NULL;
}
