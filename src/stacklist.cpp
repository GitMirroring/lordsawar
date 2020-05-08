// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2005 Andrea Paternesi
// Copyright (C) 2004 John Farrell
// Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014, 2015 Ben Asselstine
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

#include <config.h>
#include "signal.h"
#include <sigc++/functors/mem_fun.h>
#include <sigc++/adaptors/bind.h>
#include <assert.h>

#include "stacklist.h"
#include "stack.h"
#include "city.h"
#include "path.h"
#include "playerlist.h"
#include "xmlhelper.h"
#include "Item.h"
#include "hero.h"
#include "Backpack.h"
#include "LocationList.h"
#include "GameMap.h"
#include "stacktile.h"
#include "stackreflist.h"

Glib::ustring Stacklist::d_tag = "stacklist";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Vector<int> Stacklist::getPosition(guint32 id)
{
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); ++pit)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        for (const_iterator it = mylist->begin(); it !=mylist->end(); ++it)
            for (Stack::const_iterator sit = (*it)->begin(); sit != (*it)->end(); ++sit)
                if ((*sit)->getId() == id)
                    return (*it)->getPos();
    }

    return Vector<int>(-1,-1);
}

//search all player's stacklists to find this stack
bool Stacklist::deleteStack(Stack* s)
{
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); ++pit)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        for (const_iterator it = mylist->begin(); it != mylist->end(); ++it)
            if ((*it) == s)
                return mylist->flRemove(s);
    }
    return false;
}

bool Stacklist::deleteStack(guint32 id)
{
    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); ++pit)
    {
        Stacklist* mylist = (*pit)->getStacklist();
        for (const_iterator it = mylist->begin(); it != mylist->end(); ++it)
            if ((*it)->getId() == id)
                return mylist->flRemove(*it);
    }
    return false;
}

guint32 Stacklist::calculateUpkeep() const
{
  guint32 upkeep = 0;
  for (const_iterator it = begin(); it != end(); ++it)
    upkeep += (*it)->getUpkeep();
  return upkeep;
}

void Stacklist::payUpkeep(Player *p)
{
  for (iterator it = begin(); it != end(); ++it)
    (*it)->payUpkeep(p);
}

bool Stacklist::check()
{
    for (iterator it = begin(); it != end(); ++it)
      {
        if ((*it)->getOwner()->isComputer() == false)
          continue;
        std::vector<Stack*> f = GameMap::getFriendlyStacks((*it)->getPos());
        if (f.size() > 1)
          {
	      fprintf (stderr, "%lu stacks found on %d,%d\n", f.size(),
                       (*it)->getPos().x, (*it)->getPos().y);
              for (std::vector<Stack*>::iterator t = f.begin(); t != f.end(); ++t)
                {
                  Stack *stack = *t;
                  if (stack)
                    {
                    printf("stack id: %d\n", stack->getId());
                    printf("\tsize is %lu\n", stack->size());
                    }
                  else
                    printf("null stack\n");
                }
              return false;
          }
      }
    return true;
}

void Stacklist::resetStacks()
{
  for (iterator it = begin(); it != end(); ++it)
    (*it)->reset();
}

void Stacklist::nextTurn()
{
    debug("nextTurn()");
    resetStacks();

    for (iterator it = begin(); it != end(); ++it)
      for (iterator jit = begin(); jit != end(); ++jit)
	if (*jit != *it)
	  if ((*jit)->getId() == (*it)->getId())
	    {
	      fprintf (stderr, "duplicate army id %d found\n", (*it)->getId());
	      exit (1);
	    }
    //printf("checking at next-turn time\n");
    //check();
}

std::vector<Stack*> Stacklist::getDefendersInCity(const City *city)
{
    debug("getDefendersInCity()");

    std::vector<Stack*> stackvector;
    Vector<int> pos = city->getPos();

    for (unsigned int i = pos.x; i < pos.x + city->getSize(); i++)
    {
        for (unsigned int j = pos.y; j < pos.y + city->getSize(); j++)
        {
	    Vector<int> p = Vector<int>(i,j);
	    std::vector<Stack *>stacks =
	      GameMap::getFriendlyStacks(p, city->getOwner());
	    for (std::vector<Stack*>::iterator it = stacks.begin();
		 it != stacks.end(); ++it)
		stackvector.push_back(*it);
        }
    }

    return stackvector;
}

unsigned int Stacklist::getNoOfStacks()
{
    unsigned int mysize = 0;

    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); ++pit)
      mysize += (*pit)->getStacklist()->size();

    return mysize;
}

unsigned int Stacklist::getNoOfArmies()
{
    unsigned int mysize = 0;

    for (Playerlist::iterator pit = Playerlist::getInstance()->begin();
        pit != Playerlist::getInstance()->end(); ++pit)
      mysize += (*pit)->getStacklist()->countArmies();

    return mysize;
}

unsigned int Stacklist::countArmies() const
{
    unsigned int mysize = 0;

    for (const_iterator it = begin(); it != end(); ++it)
      mysize += (*it)->size();

    return mysize;
}

Stacklist::Stacklist()
    :d_activestack(0)
{
}

Stacklist::Stacklist(Stacklist *stacklist)
    :d_activestack(0)
{
    for (iterator it = stacklist->begin(); it != stacklist->end(); ++it)
      add(new Stack(**it));
}

Stacklist::Stacklist(XML_Helper* helper)
    :d_activestack(0)
{
    helper->registerTag(Stack::d_tag, sigc::mem_fun((*this), &Stacklist::load));
    load(Stacklist::d_tag, helper);
}

Stacklist::~Stacklist()
{
  //disconnect the signals
  for (auto it: d_connections)
    for (auto lit: it.second)
      lit.disconnect();
  flClear();
}

Stack* Stacklist::getNextMovable() const
{
    Player *player = Playerlist::getInstance()->getActiveplayer();
    
    const_iterator it = begin();
    
    //first, if we already have an active stack, loop through until we meet it
    if (d_activestack)
    {
        for (; *it != d_activestack; ++it);
        ++it;   //we want to start with the next stack :)
    }

    //continue looping until we meet the next not defending stack of this player
    for (; it != end(); ++it)
    {
	Stack *s = *it;
        if (s->getOwner() == player && !s->getDefending() && 
	    !s->getParked() && s->canMove())
	    return s;
    }
    
    //still not found a stack? Then start looping from the beginning until we
    //meet the activestack again. If there is no activestack, we have already
    //looped through the whole list, so stop here
    if (!d_activestack)
        return 0;

    for (it = begin(); *it != d_activestack; ++it)
      {
        Stack *s = *it;
        if (s->getOwner() == player && !s->getDefending() &&
            !s->getParked() && s->canMove())
          return s;
      }

    //still there? well, then we have only one stack left.
    if (d_activestack->getDefending() || d_activestack->getParked())
	return 0;
    else
	return d_activestack;
}

Stack *Stacklist::getStackById(guint32 id) const
{
  IdMap::const_iterator it = d_id.find(id);
  if (it != d_id.end())
    return (*it).second;
  else
    return NULL;
}

Stack *Stacklist::getArmyStackById(guint32 army) const
{
  for (Stacklist::const_iterator i = begin(), e = end(); i != e; ++i)
    if ((*i)->getArmyById(army))
      return *i;
  return NULL;
}

void Stacklist::flClear()
{
    d_activestack = 0;

    for (iterator it = begin(); it != end(); ++it)
      delete (*it);

    clear();
}

Stacklist::iterator Stacklist::flErase(iterator object)
{
    if (d_activestack == (*object))
        d_activestack = 0;
    delete (*object);
    return erase(object);
}

bool Stacklist::flRemove(guint32 id)
{
  Stack *s = getStackById(id);
  if (s == NULL)
    return false;
  return flRemove(s);
}

bool Stacklist::flRemove(Stack* object)
{
  if (object == NULL)
    return false;
  iterator stackit = find(begin(), end(), object);
  if (stackit != end())
    {
      if (d_activestack == object)
        d_activestack = 0;
      assert (object->getId() == (*stackit)->getId());
      deletePositionFromMap(object);
      delete object;
      erase(stackit);
      return true;
    }

  return false;
}

bool Stacklist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Stacklist::d_tag);
    if (d_activestack)
      retval &= helper->saveData("active", d_activestack->getId());
    else
      retval &= helper->saveData("active", 0);

    //save stacks
    for (const_iterator it = begin(); it != end(); ++it)
        retval &= (*it)->save(helper);

    retval &= helper->closeTag();

    return retval;
}

bool Stacklist::enoughMoves() const 
{
    for (const_iterator it = begin(); it != end(); ++it)
    {
        Stack* s = *it;
        if (!s->getPath()->empty() && s->enoughMoves())
            return true;
    }

    return false;
}

bool Stacklist::load(Glib::ustring tag, XML_Helper* helper)
{
    static guint32 active = 0;
    
    if (tag == Stacklist::d_tag)
    {
        helper->getData(active, "active");
        return true;
    }

    if (tag == Stack::d_tag)
    {
        Stack* s = new Stack(helper);
        if (active != 0 && s->getId() == active)
	  d_activestack = s;

        add(s);
        return true;
    }

    return false;
}

void Stacklist::getHeroes(std::vector<guint32>& dst) const
{
  for (Stacklist::const_iterator it = begin(); it != end(); ++it)
    (*it)->getHeroes(dst);
}

void Stacklist::collectTaxes(Player *p, guint32 num_cities) const
{
  std::vector<guint32> hero_ids;
  getHeroes(hero_ids);

  //now let's see if we have any items that give us gold per city
  for (std::vector<guint32>::iterator it = hero_ids.begin(); 
       it != hero_ids.end(); ++it)
    {
      Stack *stack = getArmyStackById(*it);
      Army *army = stack->getArmyById(*it);
      Hero *hero = static_cast<Hero*>(army);
      guint32 bonus = hero->getBackpack()->countGoldBonuses();
      p->addGold(bonus * num_cities);
    }
}

// do we have enough movement points to get to a place on our path
// where we can drop the stack on a suitable tile?
//suitable = empty tile, or 
//a tile with a friendly stack that has a small enough stack to merge with
//we're currently at a tile prior to a stack that's too big.
//problem point: getting into a boat.

bool Stacklist::canJumpOverTooLargeStack(Stack *s)
{
  bool found = false;
  guint32 mp = s->getMoves();
  for (Path::iterator it = s->getPath()->begin(); it != s->getPath()->end(); ++it)
    {
      guint32 moves = s->calculateTileMovementCost(*it);
      if (moves > mp)
	return false;
      mp -= moves;
      City *enemy = GameMap::getEnemyCity(*it);
      if (enemy != NULL && enemy->isBurnt() == false)
        return false;
      if (GameMap::getEnemyStack(*it) != NULL)
        return false;
      if (GameMap::canJoin(s, *it) == true)
	return true;
    }
  return found;
}

std::list<Hero*> Stacklist::getHeroes() const
{
  std::list<Hero*> heroes;
  std::vector<guint32> hero_ids;
  getHeroes(hero_ids);
  for (std::vector<guint32>::const_iterator it = hero_ids.begin(); 
       it != hero_ids.end(); ++it)
    {
        Stack *s = getArmyStackById(*it);
	if (s)
	  {
	    Hero *h = dynamic_cast<Hero*>(s->getArmyById(*it));
	    if (h)
	      heroes.push_back(h);
	  }
    }
  return heroes;
}
	
Hero *Stacklist::getNearestHero(Vector<int> pos, int dist) const
{
  std::list<Hero*> heroes = getHeroes();
  LocationList<Location*> hero_locales;
  for (std::list<Hero*>::iterator it = heroes.begin(); it != heroes.end(); ++it)
    hero_locales.push_back(new Location(getPosition((*it)->getId()), 1));
  Location *hero_locale = hero_locales.getNearestObjectBefore(pos, dist);
  if (hero_locale)
    {
      for (std::list<Hero*>::iterator it = heroes.begin(); it != heroes.end(); 
	   ++it)
	{
	  if (getPosition((*it)->getId()) == hero_locale->getPos())
	    return (*it);
	}
    }
  return NULL;
}

bool Stacklist::addPositionToMap(Stack *stack) const
{
  snewpos.emit(stack, stack->getPos());
  return true;
}

bool Stacklist::deletePositionFromMap(Stack *stack) const
{
  soldpos.emit(stack, stack->getPos());
  return true;
}

void Stacklist::add(Stack *stack)
{
  push_back(stack);
  d_id[stack->getId()] = stack;
  if (stack->getPos() != Vector<int>(-1,-1))
    {
      bool added = addPositionToMap(stack);
      if (!added)
	assert(1 == 0);
      std::list<sigc::connection> conn;
      conn.push_back(stack->smoving.connect
	 (sigc::mem_fun (this, &Stacklist::on_stack_starts_moving)));
      conn.push_back(stack->smoved.connect
	 (sigc::mem_fun (this, &Stacklist::on_stack_stops_moving)));
      conn.push_back(stack->sdying.connect
	 (sigc::mem_fun (this, &Stacklist::on_stack_died)));
      conn.push_back(stack->sgrouped.connect
	 (sigc::mem_fun (this, &Stacklist::on_stack_grouped)));
      d_connections[stack] = conn;
    }
}

void Stacklist::on_stack_grouped (Stack *stack, bool grouped)
{
  sgrouped.emit(stack, grouped);
}

void Stacklist::on_stack_died (Stack *stack)
{
  deletePositionFromMap(stack);
  ConnectionMap::iterator it = d_connections.find(stack);
  if (it != d_connections.end())
    {
      for (auto lit: (*it).second)
	lit.disconnect();
    }
  d_id.erase(d_id.find(stack->getId()));
  sstackDied.emit ();
  return;
}
void Stacklist::on_stack_starts_moving (Stack *stack)
{
  deletePositionFromMap(stack);
  return;
}
void Stacklist::on_stack_stops_moving (Stack *stack)
{
  addPositionToMap(stack);
  return;
}
void Stacklist::setActivestack(Stack* activestack)
{
  d_activestack = activestack;
}

void Stacklist::drainAllMovement()
{
  for (iterator it = begin(); it != end(); ++it)
    (*it)->drainMovement();
}

void Stacklist::changeOwnership(Player *old_owner, Player *new_owner)
{
  StackReflist *stacks = new StackReflist(old_owner->getStacklist());
  for (StackReflist::iterator it = stacks->begin(); it != stacks->end(); ++it)
    Stacklist::changeOwnership (*it, new_owner);
  delete stacks;
}

Stack* Stacklist::changeOwnership(Stack *stack, Player *new_owner)
{
  if (new_owner != stack->getOwner())
    {
      Stack *new_stack = new Stack(*stack);
      stack->getOwner()->getStacklist()->flRemove(stack);
      new_owner->addStack(new_stack);
      return new_stack;
    }
  return stack;
}

std::list<Vector<int> > Stacklist::getPositions() const
{
  std::list<Vector<int> > points;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if (std::find(points.begin(), points.end(), (*it)->getPos()) == 
          points.end())
        points.push_back((*it)->getPos());
    }
  return points;
}

std::list<Stack*> Stacklist::getStacksWithItems() const
{
  std::list<Stack*> stacks;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->countItems() > 0)
        stacks.push_back((*it));
    }
  return stacks;
}

std::list<Stack*> Stacklist::kill()
{
  std::list<Stack*> stacks;
  for (iterator it = begin(); it != end(); ++it)
    {
      (*it)->kill();
      stacks.push_back(*it);
    }
  return stacks;
}

std::list<Stack*> Stacklist::killArmyUnitsInBoats()
{
  std::list<Stack*> stacks;
  for (iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->hasShip())
        {
          if ((*it)->killArmyUnitsInBoats())
            stacks.push_back(*it);
        }
    }
  return stacks;
}
        
std::list<Stack*> Stacklist::killArmies(guint32 army_type)
{
  std::list<Stack*> stacks;
  for (iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->killArmies(army_type))
        stacks.push_back(*it);
    }
  return stacks;
}

std::list<Item*> Stacklist::getUsableItems() const
{
  std::list<Item*> items;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->hasUsableItem())
        (*it)->getUsableItems(items);
    }
  return items;
}

bool Stacklist::hasUsableItem() const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->hasUsableItem())
        return true;
    }
  return false;
}
        
bool Stacklist::getItemHolder(Item *item, Stack **stack, Hero **hero) const
{
  for (const_iterator it = begin(); it != end(); ++it)
    {
      *hero = (*it)->getHeroWithItem(item);
      if (*hero != NULL)
        {
          *stack = *it;
          return true;
        }
    }
  return false;
}

guint32 Stacklist::countMovableStacks() const
{
  guint32 count = 0;
  for (const_iterator i = begin(); i != end(); ++i)
    {
      if (!(*i)->getParked() && (*i)->canMove())
        {
          if ((*i)->getPath()->size() > 0 && (*i)->enoughMoves())
            count++;
          else if ((*i)->getPath()->size() == 0)
            count++;
        }
    }
  return count;
}

// End of file
