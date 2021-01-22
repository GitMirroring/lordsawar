// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2000, Anluan O'Brien
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2020,
// 2021 Ben Asselstine
// Copyright (C) 2007, 2008 Ole Laursen
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

#include <sigc++/functors/mem_fun.h>
#include <assert.h>

#include "stack.h"
#include "playerlist.h"
#include "path.h"
#include "armysetlist.h"
#include "counter.h"
#include "army.h"
#include "hero.h"
#include "GameMap.h"
#include "vector.h"
#include "xmlhelper.h"
#include "FogMap.h"
#include "player.h"
#include "Backpack.h"
#include "AI_Analysis.h"
#include "ruin.h"
#include "Item.h"

Glib::ustring Stack::d_tag = "stack";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Stack::Stack(Player* player, Vector<int> pos)
    : UniquelyIdentified(), Movable(pos), OwnerId(player), d_defending(false), 
    d_parked(false), d_deleting(false)
{
    d_path = new Path();
}

Stack::Stack(guint32 id, Player* player, Vector<int> pos)
    : UniquelyIdentified(id), Movable(pos), OwnerId(player), 
    d_defending(false), d_parked(false), d_deleting(false)
{
    d_path = new Path();
}

Stack::Stack(const Stack& s, bool uniq)
    : UniquelyIdentified(s, !uniq), Movable(s), OwnerId(s), std::list<Army*>(),
    sigc::trackable(s), d_defending(s.d_defending), d_parked(s.d_parked), 
    d_deleting(false)
{
  d_unique = uniq;
  if (s.d_path == NULL)
    {
      printf("Stack %d has a null path!\n", d_id);
    }
    d_path = new Path(*s.d_path);

    for (const_iterator sit = s.begin(); sit != s.end(); ++sit)
    {
	if ((*sit)->isHero())
          push_back(new Hero(dynamic_cast<Hero&>(**sit), !uniq));
	else
          push_back(new Army((**sit), !uniq, (*sit)->getOwner()));
    }
}

Stack::Stack(XML_Helper* helper)
  : UniquelyIdentified(helper), Movable(helper), OwnerId(helper), 
    d_deleting(false)
{
  helper->getData(d_defending, "defending");
  helper->getData(d_parked, "parked");

  helper->registerTag(Path::d_tag, sigc::mem_fun((*this), &Stack::load));
  helper->registerTag(Army::d_tag, sigc::mem_fun((*this), &Stack::load));
  helper->registerTag(Hero::d_hero_tag, sigc::mem_fun((*this), &Stack::load));
}

Stack::~Stack()
{
  d_deleting = true;
  if (d_unique)
    sdying.emit(this);

  delete d_path;
  flClear();
}

void Stack::setPlayer(Player* p)
{
  // we need to change the armies' loyalties as well!!
  setOwnerId(p->getId ());
  for (iterator it = begin(); it != end(); ++it)
    (*it)->setOwner(p);
}

void Stack::moveOneStep(bool skipping)
{
  debug("moveOneStep()");

  Vector<int> dest = getFirstPointInPath();
  moveToDest(dest, skipping);

  //now remove first point of the path
  d_path->eraseFirstPoint();
}

bool Stack::isMovingToOrFromAShip(Vector<int> dest, bool &on_ship) const
{
  Vector<int> pos = getPos();
  Maptile::Building src_building = GameMap::getInstance()->getBuilding(pos);
  Maptile::Building dst_building = GameMap::getInstance()->getBuilding(dest);

  bool to_city = dst_building == Maptile::CITY;
  bool on_city = src_building == Maptile::CITY;

  bool on_port = src_building == Maptile::PORT;
  bool on_bridge = src_building == Maptile::BRIDGE;
  bool to_bridge = dst_building == Maptile::BRIDGE;
  bool on_water = (GameMap::getInstance()->getTerrainType(pos) == Tile::WATER);
  bool to_water = (GameMap::getInstance()->getTerrainType(dest) == Tile::WATER);
  //here we mark the armies as being on or off a boat
  /* skipping refers to when we have to move over another friendly stack
   * of a size that's too big to join with. */
  if ((on_water && to_city && !on_bridge) || 
      (on_water && on_port && !to_water && on_ship) ||
      ((on_city || on_port) && to_water && !to_bridge) ||
      (on_bridge && to_water && !to_bridge) ||
      (on_bridge && !to_water && on_ship) ||
      (on_water && to_water && !on_bridge && !on_port && !to_bridge &&
       on_ship == false) ||
      (!on_water && !to_water && on_ship == true))
    {
      on_ship = !on_ship;
      return true;
    }
  return false;
}

void Stack::drainMovement()
{
  for (Stack::iterator it = begin(); it != end(); ++it)
    (*it)->decrementMoves((*it)->getMoves());
}

void Stack::moveToDest(Vector<int> dest, bool skipping)
{
  bool ship_load_unload = false;
  if (!isFlying())
    {
      bool on_ship = hasShip();
      if (isMovingToOrFromAShip(dest, on_ship) == true)
	{
	  if (!skipping)
	    {
              updateShipStatus(dest);
	      ship_load_unload = true;
	    }
	}
    }
  else
    {
      for (Stack::iterator it = begin(); it != end(); ++it)
	(*it)->setInShip(false);
    }

  guint32 maptype = GameMap::getInstance()->getTile(dest.x,dest.y)->getType();
  //how many moves does the stack need to travel to dest?
  int needed_moves = calculateTileMovementCost(dest);

  if (ship_load_unload)
    drainMovement();
  else
    {
      for (Stack::iterator it = begin(); it != end(); ++it)
        {
          //maybe the army has a natural movement ability
          if ((*it)->getStat(Army::MOVE_BONUS) & maptype && needed_moves > 1)
            (*it)->decrementMoves(2);
          else
            (*it)->decrementMoves(needed_moves);
        }
    }

  //update position and status
  smoving.emit(this);
  setPos(dest);

  //update fogmap
  deFog();

  smoved.emit(this);

  setFortified(false);
  setDefending(false);
  setParked(false);
}

void Stack::deFog()
{
  getOwner()->getFogMap()->alterFogRadius(getPos(), getMaxSight(), 
					  FogMap::OPEN);
  return;
}

// return the maximum moves of this stack by checking the moves of each army
guint32 Stack::getMoves() const
{
  if (empty())
    return 0;

  assert(!empty());

  int min = -1;

  for (const_iterator it = begin(); it != end(); ++it)
    {
      if (min == -1)
	min = int((*it)->getMoves());
      else
	min = std::min(min, int((*it)->getMoves()));
    }

  if (min <= -1)
    return 0;
  return min;
}

int Stack::getMinTileMoves() const
{
  LwRectangle bounds = GameMap::getInstance()->get_boundary();

  std::vector<Vector<int> > tiles;
  tiles.push_back(Vector<int>(getPos().x + 1, getPos().y - 1));
  tiles.push_back(Vector<int>(getPos().x,     getPos().y - 1));
  tiles.push_back(Vector<int>(getPos().x - 1, getPos().y - 1));
  tiles.push_back(Vector<int>(getPos().x + 1, getPos().y + 1));
  tiles.push_back(Vector<int>(getPos().x,     getPos().y + 1));
  tiles.push_back(Vector<int>(getPos().x - 1, getPos().y + 1));
  tiles.push_back(Vector<int>(getPos().x + 1, getPos().y));
  tiles.push_back(Vector<int>(getPos().x - 1, getPos().y));

  int min = -1;

  for (auto tile: tiles)
    if (is_inside(bounds, tile))
      {
	int v = GameMap::getInstance()->getTile(tile)->getMoves();
	if (min == -1)
	  min = v;
	else
	  min = std::min(min, v);
      }

  return min;
}

// decrement each armys moves by needed moves to travel

void Stack::decrementMoves(guint32 moves)
{
  debug("decrement_moves()");

  for (iterator it = begin(); it != end(); ++it)
    (*it)->decrementMoves(moves);
}

void Stack::incrementMoves(guint32 moves)
{
  debug("increment_moves()");

  for (iterator it = begin(); it != end(); ++it)
    (*it)->incrementMoves(moves);
}

// Purpose: Return the strongest army of a group
// Note: If a hero is present return it. If there are two similar armies
// (two of the same strength, or two heroes) return the first in the sequence.
// heroes in boats must be considered to be less strong than heroes who are
// not in boats.

Army* Stack::getStrongestArmy() const
{
  assert(!empty());
  return getStrongestArmy(false);
}

Army* Stack::getStrongestHero() const
{
  Army *strongest = 0;
  guint32 highest_strength = 0;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->isHero())
        {
          if ((*it)->getStat(Army::STRENGTH) > highest_strength)

            {
              highest_strength = (*it)->getStat(Army::STRENGTH);
              strongest = *it;
            }
        }
    }
  return strongest;
}

Army* Stack::getStrongestArmy(bool hero) const
{
  Army *strongest = 0;
  guint32 highest_strength = 0;

  for (const_iterator it = begin(); it != end(); ++it)
    {
      if (((*it)->isHero() && hero) || !hero)
	{
	  if ((*it)->getStat(Army::STRENGTH) > highest_strength)
	    {
	      highest_strength = (*it)->getStat(Army::STRENGTH);
	      strongest = *it;
	    }
	}
    }
  return strongest;
}

Army *Stack::getArmyById(guint32 id) const
{
  for (Stack::const_iterator i = begin(), e = end(); i != e; ++i)
    if ((*i)->getId() == id)
      return *i;
  
  return 0;
}

bool Stack::hasHero() const
{
  for (const_iterator it = begin(); it != end(); ++it)
    if ((*it)->isHero())
      return true;

  return false;
}

Army* Stack::getFirstHero() const
{
  for (const_iterator it = begin(); it != end(); ++it)
    if ((*it)->isHero())
      return (*it);

  return 0;
}

void Stack::getHeroes(std::vector<guint32>& dst) const
{
  debug("getHeroes - stack = " << this)
    for (const_iterator it = begin(); it != end(); ++it)
      {
	// if hero - add it to the vector
	debug("Army type: " << (*it)->getTypeId())
	  if ((*it)->isHero() && (*it)->getHP() > 0)
	    dst.push_back((*it)->getId());
      }
}

int Stack::bless()
{
  int count = 0;
  for (iterator it = begin(); it != end(); ++it)
    {
      Temple *temple = GameMap::getTemple(this);
      if ((*it)->bless(temple))
	count++;
    }
  return count;
}

guint32 Stack::calculateTileMovementCost(Vector<int> pos) const
{
  Maptile* tile = GameMap::getInstance()->getTile(pos);
  guint32 moves = tile->getMoves();
  guint32 bonus = calculateMoveBonus();
  if (bonus & tile->getType() && moves > 1)
    moves = 2;
  else if (isFlying() && moves > 1)
    moves = 2;
  return moves;
}

Vector<int> Stack::getFirstPointInPath() const
{
  if (hasPath() == false)
    return Vector<int>(-1,-1);
  Vector<int> p = *(d_path->begin());
  return p;
}

Vector<int> Stack::getLastReachablePointInPath() const
{
  if (d_path->size() == 0)
    return Vector<int>(-1,-1);
  unsigned int count = 0;
  for (Path::iterator it = d_path->begin(); it != d_path->end(); ++it)
    {
      count++;
      if (count == d_path->getMovesExhaustedAtPoint())
	return (*it);
    }
  return Vector<int>(-1,-1);
}

Vector<int> Stack::getLastPointInPath() const
{
  if (d_path->size() == 0)
    return Vector<int>(-1,-1);
  Vector<int> p = d_path->back();
  return p;
}

bool Stack::enoughMoves() const
{
  if (hasPath() == false)
    return true; //we have enough moves to move nowhere!

  Vector<int> p = getFirstPointInPath();
  guint32 needed = calculateTileMovementCost(p);

  if (getMoves() >= needed)
    return true;

  return false;
}

bool Stack::canMove() const
{
  int tile_moves = getMinTileMoves();
  int group_moves = getMoves();

  assert (tile_moves != -1);
  return group_moves > 0 && tile_moves >= 0 && group_moves >= tile_moves;
}

guint32 Stack::getMaxSight() const
{
  guint32 max = 0;
  for (const_iterator it = begin(); it != end(); ++it)
    if ((*it)->getStat(Army::SIGHT) > max)
      max = (*it)->getStat(Army::SIGHT);

  return max;
}

void Stack::payUpkeep(Player *p)
{
  for (iterator it = begin(); it != end(); ++it)
      p->withdrawGold((*it)->getUpkeep());
}

void Stack::reset(bool recalculate_path)
{
  guint32 movement_multiplier = 1;

  //count the number of items that double the movement in the stack.
  for (const_iterator it = begin(); it != end(); ++it)
    if ((*it)->isHero())
      {
	Hero *hero = dynamic_cast<Hero*>(*it);
	guint32 bonus = hero->getBackpack()->countMovementDoublers();
	for (guint32 i = 0; i < bonus; i++)
	  movement_multiplier*=2;
      }

  if (movement_multiplier > 1024)
    movement_multiplier = 1024;

  //set the multipler on all armies in the stack
  for (const_iterator it = begin(); it != end(); ++it)
    (*it)->setStat(Army::MOVES_MULTIPLIER, movement_multiplier);

  if (d_defending == true)
    setFortified(true);

  for (iterator it = begin(); it != end(); ++it)
    {
      (*it)->resetMoves();
      (*it)->heal();
    }

  //recalculate paths

  if (recalculate_path)
    d_path->recalculate(this);
  //we need to unpark stacks here (at the end of a round), at the very least
  //because it makes ai stacks move in the field after they've been parked.
  //we aren't sending the un-parked action here because this is part of the
  //reset stacks action.
  setParked(false);
}

bool Stack::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(Stack::d_tag);
  retval &= helper->saveData("id", d_id);
  retval &= helper->saveData("x", getPos().x);
  retval &= helper->saveData("y", getPos().y);
  if (isOwnerIdSet ())
    retval &= helper->saveData("owner", d_owner_id);
  else
    retval &= helper->saveData("owner", -1);
  retval &= helper->saveData("defending", d_defending);
  retval &= helper->saveData("parked", d_parked);


  //save path
  retval &= d_path->save(helper);

  //save armies
  for (const_iterator it = begin(); it != end(); ++it)
    retval &= (*it)->save(helper);

  retval &= helper->closeTag();

  return retval;
}

bool Stack::load(Glib::ustring tag, XML_Helper* helper)
{
  if (tag == Path::d_tag)
    {
      d_path = new Path(helper);
      return true;
    }

  if (tag == Army::d_tag)
    {
      Army* a = new Army(helper);
      a->setOwner(getOwner ());
      push_back(a);
      return true;
    }

  if (tag == Hero::d_hero_tag)
    {
      Hero* h = new Hero(helper);
      h->setOwner(getOwner ());
      push_back(h);

      return true;
    }

  return false;
}

void Stack::flClear()
{
  for (iterator it = begin(); it != end(); ++it)
    {
      Army *a = *it;
      if (a->isHero())
        {
          Hero *h = dynamic_cast<Hero*>(a);
          delete h;
        }
      else
        delete a;
    }
  clear();
}

Stack::iterator Stack::flErase(Stack::iterator object)
{
  Army *a = *object;
  if (a->isHero())
    {
      Hero *h = dynamic_cast<Hero*>(a);
      delete h;
    }
  else
    delete (a);
  return erase(object);
}

guint32 Stack::calculateMoveBonus() const
{
  guint32 d_bonus = 0;

  bool landed = false;
  guint32 bonus;
  // check to see if we're all flying
  int num_landedhero = 0;
  int num_flyer = 0;
  int num_landedother = 0;
  if (size() == 0)
    return 0;
  for (const_iterator it = this->begin(); it != this->end(); ++it)
    {
      bonus = (*it)->getStat(Army::MOVE_BONUS);
      if (bonus == Tile::GRASS || (bonus & Tile::WATER) == 0 || 
	  (bonus & Tile::FOREST) == 0 || (bonus & Tile::HILLS) == 0 ||
	  (bonus & Tile::MOUNTAIN) == 0 || (bonus & Tile::SWAMP) == 0)
	{
	  landed = true;
	  if ((*it)->isHero())
	    num_landedhero++;
	  else
	    num_landedother++;
	}
      else
	num_flyer++;

    }
  //if we're all flying or we have enough flyers to carry landbound heroes
  if (landed == false ||
      (num_landedother == 0 && num_landedhero <= num_flyer)) 
    {
      d_bonus = Tile::isFlying();
      return d_bonus;
    }

  //or maybe we have an item that lets us all fly
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->isHero())
	{
	  Hero *h = dynamic_cast<Hero*>(*it);
	  if (h->isFlyer())
	    {
	      d_bonus = Tile::isFlying();
	      return d_bonus;
	    }
	}
    }

  //calculate move bonuses for non-flying stacks
  
  //first we see if any have a move bonus that confers to the whole stack
  for (Stack::const_iterator it = this->begin(); it != this->end(); ++it)
    {
      bonus = (*it)->getStat(Army::MOVE_BONUS);
      if (((*it)->getStat(Army::ARMY_BONUS) & ArmyBase::CONFER_MOVE_BONUS) > 0)
        d_bonus |= bonus;
    }

  //check if all army units have the ability to move through forest
  bool found = true;
  for (Stack::const_iterator it = this->begin(); it != this->end(); ++it)
    {
      bonus = (*it)->getStat(Army::MOVE_BONUS);
      if ((bonus & Tile::FOREST) == 0)
        found = false;
    }
  if (found)
    d_bonus |= Tile::FOREST;

  //check if all army units have the ability to move through hills
  found = true;
  for (Stack::const_iterator it = this->begin(); it != this->end(); ++it)
    {
      bonus = (*it)->getStat(Army::MOVE_BONUS);
      if ((bonus & Tile::HILLS) == 0)
        found = false;
    }
  if (found)
    d_bonus |= Tile::HILLS;

  //check if all army units have the ability to move through mountains
  found = true;
  for (Stack::const_iterator it = this->begin(); it != this->end(); ++it)
    {
      bonus = (*it)->getStat(Army::MOVE_BONUS);
      if ((bonus & Tile::MOUNTAIN) == 0)
        found = false;
    }
  if (found)
    d_bonus |= Tile::MOUNTAIN;

  //check if all army units have the ability to move through swamp
  found = true;
  for (Stack::const_iterator it = this->begin(); it != this->end(); ++it)
    {
      bonus = (*it)->getStat(Army::MOVE_BONUS);
      if ((bonus & Tile::SWAMP) == 0)
        found = false;
    }
  if (found)
    d_bonus |= Tile::SWAMP;

  return d_bonus;
}

bool Stack::isFlying () const
{
  guint32 d_bonus = calculateMoveBonus();
  if (d_bonus == Tile::isFlying())
    return true;
  else
    return false;
}

/*if any stack member is in a boat, then the whole stack appears to be in
 * a boat */
bool Stack::hasShip () const
{
  for (Stack::const_iterator it = this->begin(); it != this->end(); ++it)
    {
      if ((*it)->getStat(Army::SHIP))
	return true;
    }
  return false;
}

guint32 getFightOrder(std::list<guint32> values, guint32 value)
{
  guint32 count = 0;
  for (std::list<guint32>::const_iterator it = values.begin(); 
       it != values.end(); ++it)
    {
      count++;
      if (*it == value)
	return count;
    }
  return 0;
}

bool Stack::armyCompareStrength (const Army *lhs, const Army *rhs)  
{
  return lhs->getStat(Army::STRENGTH) < rhs->getStat(Army::STRENGTH);
}

bool Stack::armyCompareFightOrder (const Army *lhs, const Army *rhs)  
{
  std::list<guint32> lhs_fight_order = lhs->getOwner()->getFightOrder();
  std::list<guint32> rhs_fight_order = rhs->getOwner()->getFightOrder();
  guint32 lhs_rank = getFightOrder (lhs_fight_order, lhs->getTypeId());
  guint32 rhs_rank = getFightOrder (rhs_fight_order, rhs->getTypeId());
  if (lhs_rank == rhs_rank)
    return lhs->getId() < rhs->getId();
  return lhs_rank < rhs_rank; 
}

void Stack::sortByStrength(bool rev)
{
  sort(armyCompareStrength);
  if (rev)
    std::reverse(begin(), end());
}

void Stack::sortForViewing (bool rev)
{
  sort(armyCompareFightOrder);
  if (rev)
    std::reverse(begin(), end());
}

void Stack::setFortified(bool fortified)
{
  if (empty())
    return;
  for (iterator it = begin(); it != end(); ++it)
    (*it)->setFortified(false);

  (*begin())->setFortified(fortified);
}

bool Stack::getFortified() const
{
  if (empty())
    return false;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getFortified())
	return true;
    }
  return false;
}

guint32 Stack::getUpkeep() const
{
  guint32 upkeep = 0;
  for (const_iterator it = begin(); it != end(); ++it)
    upkeep += (*it)->getUpkeep();
  return upkeep;
}

guint32 Stack::getMaxArmiesToJoin() const
{
  return MAX_STACK_SIZE - size();
}

bool Stack::canJoin(const Stack *stack) const
{
  if ((stack->size() + size()) > MAX_STACK_SIZE)
    return false;

  return true;

}

//take the weakest units where their strengths add up to strength.
std::list<guint32> Stack::determineArmiesByStrength(float strength) const
{
  std::list<guint32> armies;
  float remaining = strength; 
  Stack *stack = new Stack(*this);
  stack->sortByStrength(false);
  for (iterator it = stack->begin(); it != stack->end(); ++it)
    {
      float score = AI_Analysis::assessArmyStrength(*it);
      if (score > remaining)
        continue;
      else
        {
          remaining -= score;
          armies.push_back((*it)->getId());
        }
    }
  delete stack;
  return armies;
}

std::list<guint32> Stack::determineStrongArmies(float strength) const
{
  return determineArmiesByStrength(strength);
}

std::list<guint32> Stack::determineWeakArmies(float strength) const
{
  return determineArmiesByStrength(strength);
}

std::list<guint32> Stack::determineReachableArmies(Vector<int> dest) const
{
  std::list<guint32> ids;
  //try each army individually to see if it reaches
	  
  Stack *stack = Stack::createNonUniqueStack(getOwner(), getPos());
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getMoves() > 0)
	{
	  stack->push_back(*it);
	  if (stack->getMoves() >= stack->getPath()->calculate(stack, dest))
	    ids.push_back((*it)->getId());
	  stack->clear();
	}
    }
  delete stack;
  if (ids.size() == 0)
    return ids;

  //now try to see if any army units can tag along
  stack = Stack::createNonUniqueStack(getOwner(), getPos());
  for (const_iterator it = begin(); it != end(); ++it)
    {
      //skip over armies that are already known to be reachable
      if (find(ids.begin(), ids.end(), (*it)->getId()) != ids.end())
	continue;
      if ((*it)->getMoves() > 0)
	{
	  stack->push_back(*it);
	  //also push back the rest of the known reachables
	  for (std::list<guint32>::iterator iit = ids.begin();
               iit != ids.end(); ++iit)
	    {
	      Army *army = getArmyById(*iit);
	      if (army)
		stack->push_back(army);
	    }
	  if (stack->getMoves() >= 
	      stack->getPath()->calculate(stack, dest))
	    ids.push_back((*it)->getId());
	  stack->clear();
	}
    }
  delete stack;

  return ids;
}

guint32 Stack::countArmiesBlessedAtTemple(guint32 temple_id) const
{
  guint32 blessed = 0;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->blessedAtTemple(temple_id))
	blessed++;
    }
    return blessed;
}
	
Stack* Stack::createNonUniqueStack(Player *player, Vector<int> pos)
{
  return new Stack(0, player, pos);
}

guint32 Stack::getMaxMoves() const
{
  if (GameMap::getInstance()->getTile(getPos())->getType() != Tile::WATER)
    return getMaxLandMoves();
  else
    return getMaxBoatMoves();
}

guint32 Stack::getMaxLandMoves() const
{
  if (empty())
    return 0;

  assert(!empty());

  //copy the stack, reset the moves and return the group moves
  Stack *copy = new Stack (*this);
  copy->getPath()->clear(); //this prevents triggering path recalc in reset
  copy->decrementMoves(copy->getMoves());
  copy->reset(false);
  guint32 moves = copy->getMoves();
  if (isFlying() == true)
    {
      delete copy;
      return moves;
    }

  //alright, we're not flying.  what would our group moves be if we were on land
  //remove ship status from all army units
  copy->decrementMoves(copy->getMoves());
  for (Stack::iterator it = copy->begin(); it != copy->end(); ++it)
    (*it)->setInShip(false);
  copy->reset(false);

  moves = copy->getMoves();
  delete copy;
  return moves;
}

guint32 Stack::getMaxBoatMoves() const
{
  if (empty())
    return 0;

  assert(!empty());

  //copy the stack, reset the moves and return the group moves
  Stack *copy = new Stack (*this);
  copy->getPath()->clear(); //this prevents triggering path recalc in reset
  copy->reset();
  guint32 moves = copy->getMoves();
  if (isFlying() == true)
    {
      delete copy;
      return moves;
    }
  //alright, we're not flying.  what would our group moves be if we were on water?
  copy->decrementMoves(copy->getMoves());
	      
  for (Stack::iterator it = copy->begin(); it != copy->end(); ++it)
    {
      if (((*it)->getStat(Army::MOVE_BONUS) & Tile::WATER) == 0)
	(*it)->setInShip(true);
      else
	(*it)->setInShip(false);
    }
  copy->reset();

  moves = copy->getMoves();
  delete copy;
  return moves;
}
	
void Stack::setPath(const Path p)
{
  if (d_path)
    delete d_path;
  d_path = new Path(p);
}

void Stack::add(Army *army)
{
  push_back(army);
}

//! split the given army from this stack, into a brand new stack.
Stack *Stack::splitArmy(Army *army)
{
  if (size() == 1) //we can't split the last army.
    return NULL;

  assert (army != NULL);
  Stack *new_stack = NULL;
  for (iterator it = begin(); it != end(); ++it)
    {
      if (*it == army || (*it)->getId() == army->getId())
	{
	  new_stack = new Stack(getOwner(), getPos());
	  new_stack->add(*it);
	  it = erase(it);
	  break;
	}
    }

  return new_stack;
}

//! split the given armies from this stack, into a brand new stack.
Stack *Stack::splitArmies(const std::list<Army*> &armies)
{
  std::list<guint32> ids;
  for (std::list<Army*>::const_iterator i = armies.begin(); i != armies.end(); ++i)
    ids.push_back((*i)->getId());
  return splitArmies(ids);
}

Stack *Stack::splitArmies(const std::list<guint32> & armies)
{
  if (armies.size() == 0) //we can't split 0 armies into a new stack.
    return NULL;
  if (armies.size() >= size()) //we can't split everyone into a new stack.
    return NULL;
  Stack *new_stack = NULL;
  for (std::list<guint32>::const_iterator i = armies.begin(); i != armies.end(); ++i)
    {
      bool found = false;
      iterator found_army_it = end();
      for (iterator it = begin(); it != end(); ++it)
	{
	  if ((*it)->getId() == *i)
	    {
	      found = true;
	      found_army_it = it;
	      break;
	    }
	}
      if (found)
	{
	  if (new_stack == NULL)
	    new_stack = new Stack(getOwner(), getPos());
	  new_stack->push_back(*found_army_it);
	  erase(found_army_it);
	}
    }
  return new_stack;
}

void Stack::join(Stack *s)
{
  for (iterator i = s->begin(); i != s->end(); ++i)
    push_back(*i);
  s->clear();
}

bool Stack::validate() const
{
  if (size() > MAX_STACK_SIZE)
    return false;
  if (size() == 0)
    return false;
  return true;
}

bool Stack::isFull() const
{
  if (size() >= MAX_STACK_SIZE)
    return true;
  return false;
}

bool Stack::clearPath()
{
  if (getPath())
    {
      if (getPath()->size() > 0)
	{
	  getPath()->clear();
	  return true;
	}
      else
	return false;
    }
  else
    return false;
  return true;
}

bool Stack::isOnCity() const
{
  if (GameMap::getInstance()->getBuilding(getPos()) == Maptile::CITY)
    return true;
  return false;
}

bool Stack::hasPath() const
{
  if (getPath() && getPath()->size() > 0)
    return true;
  return false;
}

bool Stack::hasQuest() const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->isHero() == true)
        {
          Hero *hero = dynamic_cast<Hero*>(*it);
          if (hero->hasQuest() == true)
            return true;

        }
    }
  return false;
}

bool Stack::hasArmyType(guint32 army_type) const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getTypeId() == army_type)
        return true;
    }
  return false;
}

Hero *Stack::getFirstHeroWithoutAQuest() const
{
  Hero *hero = NULL;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->isHero() == false)
        continue;
      hero = dynamic_cast<Hero*>(*it);
      if (hero->hasQuest() == false)
        return hero;
    }
  return NULL;
}

Hero *Stack::getFirstHeroWithAQuest() const
{
  Hero *hero = NULL;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->isHero() == false)
        continue;
      hero = dynamic_cast<Hero*>(*it);
      if (hero->hasQuest() == true)
        return hero;
    }
  return NULL;
}

guint32 Stack::countItems() const
{
  guint32 count = 0;
  Hero *hero = NULL;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->isHero() == false)
        continue;
      hero = dynamic_cast<Hero*>(*it);
      Backpack *backpack = hero->getBackpack();
      count += backpack->size();
    }
  return count;
}

bool Stack::hasUsableItem() const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->isHero() == false)
        continue;
      Hero *hero = dynamic_cast<Hero*>(*it);
      Backpack *backpack = hero->getBackpack();
      if (backpack->hasUsableItem() == true)
        return true;
    }
  return false;
}
        
void Stack::getUsableItems(std::list<Item*> &items) const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->isHero() == false)
        continue;
      Hero *hero = dynamic_cast<Hero*>(*it);
      Backpack *backpack = hero->getBackpack();
      std::list<Item*> backpack_items;
      backpack->getUsableItems(backpack_items);
      //now we dwindle the items from the backpack, depending on whether or
      //not they're actually usable.
      for (std::list<Item*>::iterator i = backpack_items.begin(); 
           i !=backpack_items.end(); ++i)
        {
          Maptile::Building b = GameMap::getInstance()->getBuilding(getPos());
          Ruin *ruin = GameMap::getInstance()->getRuin(getPos());
          bool ruin_has_occupant = false;
          if (ruin)
            {
              if (ruin->isSearched() == false && ruin->getOccupant() != NULL)
                ruin_has_occupant = true;
            }
          bool victims = Playerlist::getInstance()->countPlayersAlive() > 1;
          if ((*i)->isCurrentlyUsable(b, !GameMap::getInstance()->getBackpacks().empty(),
                                      victims, ruin_has_occupant,
                                      GameMap::friendlyCitiesPresent(),
                                      GameMap::enemyCitiesPresent(),
                                      GameMap::neutralCitiesPresent()) == false)
            i = backpack_items.erase(i);
        }
      if (backpack_items.size() > 0)
        items.insert(std::end(items),
                     std::begin(backpack_items), std::end(backpack_items));
    }
  return;
}

Hero* Stack::getHeroWithItem(Item *item) const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->isHero() == false)
        continue;
      Hero *hero = dynamic_cast<Hero*>(*it);
      Backpack *backpack = hero->getBackpack();
      if (backpack->getItemById(item->getId()) != NULL)
        return hero;
    }
  return NULL;
}

void Stack::kill()
{
  for (iterator it = begin(); it != end(); ++it)
    (*it)->kill();
}

//! Sink the stack.  return true if we've sunk any part of the stack.
bool Stack::killArmyUnitsInBoats()
{
  bool retval = false;
  if (GameMap::getInstance()->getTile(getPos())->getType() != Tile::WATER)
    return retval;
  if (isFlying())
    return retval;
  std::list<Army*> flyers;
  std::list<Army*> landedother;
  std::list<Army*> landedhero;
  for (iterator it = this->begin(); it != this->end(); ++it)
    {
      guint32 bonus = (*it)->getStat(Army::MOVE_BONUS);
      if (bonus == Tile::GRASS || (bonus & Tile::WATER) == 0 || 
	  (bonus & Tile::FOREST) == 0 || (bonus & Tile::HILLS) == 0 ||
	  (bonus & Tile::MOUNTAIN) == 0 || (bonus & Tile::SWAMP) == 0)
	{
	  if ((*it)->isHero())
            landedhero.push_back(*it);
	  else
            landedother.push_back(*it);
	}
      else
        flyers.push_back(*it);
    }
  //sink the landed others.
  for (std::list<Army*>::iterator it = landedother.begin(); 
       it != landedother.end(); ++it)
    {
      retval = true;
      (*it)->kill();
    }
  int num_heroes_to_sink = landedhero.size() - flyers.size();
  if (num_heroes_to_sink > 0)
    {
      //sink the unlucky heroes and any items they might have.
      for (std::list<Army*>::reverse_iterator it = landedhero.rbegin();
           it != landedhero.rend(); ++it)
        {
          retval = true;
          (*it)->kill();
          num_heroes_to_sink--;
          if (num_heroes_to_sink <= 0)
            break;
        }
    }
      
  for (std::list<Army*>::iterator it = landedhero.begin(); 
       it != landedhero.end(); ++it)
    {
      if ((*it)->getHP() > 0)
        (*it)->setInShip(false); //we're being carried by a flyer
    }
  return retval;
}
          
//! Kill the army units that are the given army type
bool Stack::killArmies(guint32 army_type)
{
  bool killed = false;
  for (iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getTypeId() == army_type)
        {
          (*it)->kill();
          killed = true;
        }
    }
  return killed;
}

std::list<guint32> compare_ids;
bool Stack::compareIds(const Army *lhs, const Army *rhs)
{
  guint32 lhs_rank = MAX_STACK_SIZE + 1;
  guint32 rhs_rank = MAX_STACK_SIZE + 1;
  int count = 0;
  for (std::list<guint32>::iterator i = compare_ids.begin(); 
       i != compare_ids.end(); ++i)
    {
      if (lhs && *i == lhs->getId())
        lhs_rank = count;
      if (rhs && *i == rhs->getId())
        rhs_rank = count;
      count++;
    }
  return lhs_rank < rhs_rank;
}

void Stack::sortByIds(const std::list<guint32> &ids)
{
  compare_ids = ids;
  sort(compareIds);
}

void Stack::updateShipStatus(Vector<int> dest)
{
  bool to_water = (GameMap::getInstance()->getTile(dest)->getType() == Tile::WATER);
  bool to_bridge = (GameMap::getBridge(dest) != NULL);
  for (Stack::iterator it = begin(); it != end(); ++it)
    {
      if (to_water && !to_bridge && 
          ((*it)->getStat(Army::MOVE_BONUS) & Tile::WATER) == 0)
        (*it)->setInShip(true);
      else
        (*it)->setInShip(false);
    }
}

bool Stack::hasDeadArmies() const
{
  for (const_iterator i = begin(); i != end(); ++i)
    if ((*i)->getHP() == 0)
      return true;
  return false;
}

bool Stack::removeArmiesWithoutArmyType(guint32 armyset)
{
  bool removedArmy = false;
  for (iterator i = begin(); i != end(); ++i)
    {
      Armyset *a = Armysetlist::getInstance()->get(armyset);
      ArmyProto *armyproto = a->lookupArmyByType((*i)->getTypeId());
      if (armyproto == NULL)
        {
          i = flErase(i);
          if (size() > 0)
            --i;
          removedArmy = true;
          continue;
        }
    }
  return removedArmy;
}

/*
 * the idea here is that a hero has a stack flight item, and we don't want
 * to split the stack when it is in the precarious position of stranding
 * army units in water or mountains.
 *
 * this routine just detects when splitting a stack would strand an army unit.
 *
 * we could do a better job detecting the non-flyers in the stack.
 */
bool Stack::fliesWithItemAndNonFlyersOverWaterOrMountains() const
{
  Maptile *mtile = GameMap::getInstance()->getTile(getPos());
  bool on_water = mtile->getType() == Tile::WATER &&
    mtile->getBuilding() != Maptile::BRIDGE;
  bool on_mountains = mtile->getType() == Tile::MOUNTAIN &&
    mtile->getBuilding() != Maptile::ROAD;
  if (!on_water && !on_mountains)
    return false;
  bool flies_with_item = false;
  std::vector<guint32> ids;
  getHeroes(ids);
  for (auto id : ids)
    {
      Army *a = getArmyById(id);
      Hero *h = dynamic_cast<Hero*>(a);
      if (h->getBackpack()->countStackFlightGivers() > 0)
        flies_with_item = true;
    }
  return flies_with_item && size() > 1 && (on_water || on_mountains);
}

bool Stack::canMoveThroughMountains () const
{
  guint32 bonus = calculateMoveBonus();
  if (bonus == Tile::isFlying())
    return false;
  return (bonus & Tile::MOUNTAIN) > 0;
}
// End of file
