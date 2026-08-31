//  Copyright (C) 2000, 2001, 2003 Michael Bartl
//  Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
//  Copyright (C) 2002 Mark L. Amidon
//  Copyright (C) 2005 Andrea Paternesi
//  Copyright (C) 2006, 2007, 2008, 2009, 2011, 2014, 2015, 2021,
//  2026 Ben Asselstine
//  Copyright (C) 2008 Ole Laursen
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

#include <stdio.h>
#include <sstream>
#include "city.h"
#include "path.h"
#include "army.h"
#include "army-prod-base.h"
#include "hero.h"
#include "stack-list.h"
#include "stack.h"
#include "player-list.h"
#include "army-set-list.h"
#include "city-list.h"
#include "game-map.h"
#include "vectored-unit-list.h"
#include "vectored-unit.h"
#include "action.h"
#include "xml-helper.h"
#include "rnd.h"
#include "game-scenario-options.h"

Glib::ustring City::d_tag = "city";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

City::City(Vector<int> pos, guint32 width, Glib::ustring name, guint32 gold, 
	   guint32 numslots)
    :OwnerId((Player *)0), NamedLocation(pos, width, name, ""),
    ProdSlotlist(numslots), d_gold(gold), d_defense_level(1), d_burnt(false), 
    d_vectoring(false), d_vector(Vector<int>(-1,-1)), 
    d_capital(false), d_capital_owner(0), d_build_production (true)

{
  // set the tiles to city type
  for (unsigned int i = 0; i < getSize(); i++)
    for (unsigned int j = 0; j < getSize(); j++)
      {
	Vector<int> npos = getPos() + Vector<int>(i, j);
	GameMap::instance()->getTile(npos)->setBuilding(Maptile::CITY);
      }
}

City::City(XML_Helper* helper, guint32 width)
    :OwnerId(helper), NamedLocation(helper, width),
    ProdSlotlist(helper)
{
    //initialize the city

    helper->get(d_defense_level, "defense");
    
    helper->get(d_gold, "gold");
    helper->get(d_burnt, "burnt");
    helper->get(d_build_production, "build_production");
    helper->get(d_capital, "capital");
    if (d_capital)
      {
	guint32 ui;
        helper->get(ui, "capital_owner");
        d_capital_owner = Playerlist::instance()->get (ui);
      }
    else
      d_capital_owner = NULL;


    std::istringstream svect;
    Glib::ustring s;

    helper->get(s, "vectoring");
     svect.str(s);
    svect >> d_vector.x;
    svect >> d_vector.y;

    if (d_vector.x!=-1 && d_vector.y!=-1) 
        d_vectoring=true;
    else 
      d_vectoring=false;

    //mark the positions on the map as being occupied by a city
    for (unsigned int i = 0; i < d_size; i++)
        for (unsigned int j = 0; j < d_size; j++)
            GameMap::instance()->getTile(getPos().x+i, getPos().y+j)
                                  ->setBuilding(Maptile::CITY);
}

City::City(const City& c, bool sync_id)
    :OwnerId(c), NamedLocation(c, sync_id), ProdSlotlist(c),
    d_gold(c.d_gold), d_defense_level(c.d_defense_level), d_burnt(c.d_burnt),
    d_vectoring(c.d_vectoring),d_vector(c.d_vector), d_capital(c.d_capital), 
    d_capital_owner(c.d_capital_owner), d_build_production(c.d_build_production)
{
}

City::City(const City& c, Vector<int> pos)
    :OwnerId(c), NamedLocation(c, pos), ProdSlotlist(c),
    d_gold(c.d_gold), d_defense_level(c.d_defense_level), d_burnt(c.d_burnt),
    d_vectoring(c.d_vectoring),d_vector(c.d_vector), d_capital(c.d_capital), 
    d_capital_owner(c.d_capital_owner), d_build_production(c.d_build_production)
{
}

bool City::save(XML_Helper* helper) const
{
    bool retval = true;

    std::stringstream svect;

    svect << d_vector.x << " " << d_vector.y;

    retval &= helper->open_tag(City::d_tag);
    retval &= helper->save("id", d_id);
    retval &= helper->save("x", getPos().x);
    retval &= helper->save("y", getPos().y);
    retval &= helper->save("name", getName(false));
    retval &= helper->save("description", getDescription());
    retval &= helper->save("owner", d_owner_id);
    retval &= helper->save("defense", d_defense_level);
    retval &= helper->save("gold", d_gold);
    retval &= helper->save("burnt", d_burnt);
    retval &= helper->save("build_production", d_build_production);
    retval &= helper->save("capital", d_capital);
    if (d_capital)
      retval &= helper->save("capital_owner", d_capital_owner->getId());
    retval &= helper->save("vectoring", svect.str());

    retval &= ProdSlotlist::save(helper);
    retval &= helper->close_tag();
    return retval;
}

void City::conquer(Player* newowner)
{
  Citylist::instance()->stopVectoringTo(this);

  setOwner(newowner);

    // remove vectoring info 
    setVectoring(Vector<int>(-1,-1));

    deFog(newowner);

    VectoredUnitlist::instance()->removeVectoredUnitsGoingTo(this);
    VectoredUnitlist::instance()->removeVectoredUnitsComingFrom(this);
}

void City::produceStrongestProductionBase()
{
  debug("produceStrongestProductionBase()");

  if (getNoOfProductionBases() == 0)
    return;

  if (!isFull(getOwner ()))
    {
      unsigned int max_strength = 0;
      int strong_idx = -1;
      for (unsigned int i = 0; i < getMaxNoOfProductionBases(); i++)
	{
	  if ((*this)[i]->getArmyProdBase() == NULL)
	    continue;
	  if (getProductionBase(i)->getStrength() > max_strength)
	    {
	      strong_idx = i;
	      max_strength = getProductionBase(i)->getStrength();
	    }
	}
      if (strong_idx == -1)
	return;

      int savep = d_active_production_slot;
      setActiveProductionSlot(strong_idx);
      Stack *s = NULL;
      produceArmy(s);
      setActiveProductionSlot(savep);
      return;
    }
}

void City::produceWeakestQuickestArmyInArmyset()
{
  guint32 set = getOwner ()->getArmyset();
  ArmyProto *scout = Armysetlist::instance()->lookupWeakestQuickestArmy(set);
  Army *a = new Army(*scout, getOwner ());
  GameMap::instance()->addArmy(this, a);
}

void City::produceWeakestProductionBase()
{
  debug("produceWeakestProductionBase()");

  if (getNoOfProductionBases() == 0)
    return;

  if (!isFull(getOwner ()))
    {
      unsigned int min_strength = 100;
      int weak_idx = -1;
      for (unsigned int i = 0; i < getMaxNoOfProductionBases(); i++)
	{
	  if ((*this)[i]->getArmyProdBase() == NULL)
	    continue;
	  if (getProductionBase(i)->getStrength() < min_strength)
	    {
	      weak_idx = i;
	      min_strength = getProductionBase(i)->getStrength();
	    }
	}
      if (weak_idx == -1)
	return;

      int savep = d_active_production_slot;
      setActiveProductionSlot(weak_idx);
      Stack *s = NULL;
      produceArmy(s);
      setActiveProductionSlot(savep);
      return;
    }
}

const Army *City::armyArrives(Stack *& stack)
{
  // vector the army to the new spot
  if (d_vectoring)
    {
      int turns = VectoredUnit::get_travel_turns (getPos(), d_vector);
      VectoredUnit *v = 
        new VectoredUnit (getPos(), d_vector, 
                          (*this)[d_active_production_slot]->getArmyProdBase(),
                          turns, getOwner ());
      VectoredUnitlist::instance()->push_back(v);
      getOwner ()->cityChangeProduction(this, d_active_production_slot);
      //we don't return an army when we've vectored it.
      //it doesn't really exist until it lands at the destination.
      return NULL;
    }
  else //or make it here
    {
      return produceArmy(stack);
    }
  return NULL;
}

void City::nextTurn()
{
  if (d_burnt)
    return;

  // check if an army should be produced
  if (d_active_production_slot >= 0 && --d_duration == 0) 
    {
      if (getOwner ()->getGold() <= 0)
	{
	  //dont make or vector the unit
	  //and also stop production
	  getOwner ()->cityChangeProduction(this, -1);
	  getOwner ()->vectorFromCity(this, Vector<int>(-1,-1));
	  return;
	}
      getOwner ()->cityProducesArmy(this);
    }
}

void City::setVectoring(Vector<int> p) 
{
  d_vector = p;
  d_vectoring = true;

  if (p.x == -1 || p.y == -1)
    {
      d_vectoring=false;
      d_vector.x = -1;
      d_vector.y = -1;
    }
}

Army *City::produceArmy(Stack *& stack)
{
  // add produced army to stack
  if (d_active_production_slot == -1)
    return NULL;

  debug("produce_army()\n");

  // do not produce an army if the player has no gold.
  // unless it's the neutrals
  if (getOwner () != Playerlist::getNeutral() && 
      getOwner ()->getGold() < 0) 
    return NULL;

  Army *a =
    new Army(*(getProductionBase(d_active_production_slot)), getOwner ());
  stack = GameMap::instance()->addArmy(this, a);

  if (getOwner ()== Playerlist::getNeutral()) 
    {
      //we're an active neutral city
      //check to see if we've made 5 or not.
      //stop producing if we've made 5 armies in our neutral city
      if (countDefenders() >= MAX_ARMIES_PRODUCED_IN_NEUTRAL_CITY)
	setActiveProductionSlot(-1);
      else
	setActiveProductionSlot(d_active_production_slot);
    }
  else // start producing next army of same type
    setActiveProductionSlot(d_active_production_slot);
  return a;
}

bool City::canAcceptMoreVectoring() const
{
  return canAcceptMoreVectoring(0);
}

bool City::canAcceptMoreVectoring(guint32 number_of_cities) const
{
  //here we presume that it's one unit per city
  guint32 num = Citylist::instance()->countCitiesVectoringTo(this);
  if (num + number_of_cities >= MAX_CITIES_VECTORED_TO_ONE_CITY)
    return false;
  return true;
}

bool City::changeVectorDestination(Vector<int> dest)
{
  setVectoring(dest);
  VectoredUnitlist::instance()->changeDestination(this, dest);
  return true;
}

std::vector<Stack *> City::getDefenders() const
{
  if (isBurnt() == true)
    {
      std::vector<Stack *> e;
      return e;
    }
  return getOwner()->getStacklist()->getDefendersInCity(this);
}

guint32 City::countDefenders() const
{
  std::vector<Stack*> defenders;
  defenders = getDefenders();

  guint32 armies = 0;
  for (std::vector<Stack*>::iterator it = defenders.begin();
       it != defenders.end(); ++it)
    armies += (*it)->size();

  return armies;
}

void City::randomlyImproveOrDegradeArmy(ArmyProdBase *army)
{
  if (Rnd::rand() % 30 == 0) //random chance of improving strength
      army->setStrength(army->getStrength() + 1);
  if (Rnd::rand() % 25 == 0) //random chance of improving turns
    {
      if (army->getProduction() > 1)
	army->setProduction(army->getProduction() - 1);
    }
  if (Rnd::rand() % 50 == 0) //random chance of degrading strength
    {
      if (army->getStrength() > 1)
	army->setStrength(army->getStrength() - 1);
    }
  if (Rnd::rand() % 45 == 0) //random chance of improving turns
    {
      if (army->getProduction() < 5)
	army->setProduction(army->getProduction() + 1);
    }
}

bool armyCompareStrength (const ArmyProdBase *lhs, const ArmyProdBase *rhs)
{
  guint32 lhs_strength = lhs->getStrength();
  guint32 rhs_strength = rhs->getStrength();
  return lhs_strength < rhs_strength; 
}

void City::sortProduction()
{
  //sort them by strength
  if (getNoOfProductionBases() > 1)
    {
      std::list<ArmyProdBase*> productibles;
      unsigned int j;
      for (j = 0; j < getMaxNoOfProductionBases(); j++)
	{
	  if ((*this)[j]->getArmyProdBase())
	    productibles.push_back((*this)[j]->getArmyProdBase());
	}
      productibles.sort(armyCompareStrength);
      j = 0;
      for (std::list<ArmyProdBase*>::iterator it = productibles.begin();
	   it != productibles.end(); ++it, j++)
       	(*this)[j]->setArmyProdBase(*it);
    }
  return;
}

void City::setRandomArmytypes(bool produce_allies, int likely)
{
  //remove armies any that happen to be being produced
  for (unsigned int i = 0; i < getMaxNoOfProductionBases(); i++)
    removeProductionBase(i);

  guint32 set = getOwner ()->getArmyset();

  int army_type;
  int num = Rnd::rand() % 10;
  if (num < 7)
    army_type = 1;
  else if (num < 9 && likely == 0)
    army_type = 0;
  else
    army_type = 1 + likely + (Rnd::rand () % 11);
  ArmyProto *template_army = Armysetlist::instance()->getArmy(set, army_type);
  if (!template_army || 
      (template_army->getAwardable() == true && produce_allies == false) ||
      template_army->isHero())
    {
      produceWeakestQuickestArmyInArmyset();
      return;
    }
  ArmyProdBase *army = new ArmyProdBase (*template_army);
  randomlyImproveOrDegradeArmy(army);
  addProductionBase(0, army);

  if (((Rnd::rand() % 10) < 3 && !isCapital() && likely < 1) ||
      template_army->getAwardable())
    {
      sortProduction();
      return;
    }

  army_type += 1 + (Rnd::rand() % (2 + (produce_allies ? 2 : 0)));
  template_army = Armysetlist::instance()->getArmy(set, army_type);
  if (!template_army ||
      (template_army->getAwardable() == true && produce_allies == false) ||
      template_army->isHero())
    {
      sortProduction();
      return;
    }
  army = new ArmyProdBase (*template_army);
  randomlyImproveOrDegradeArmy(army);
  addProductionBase(1, army);

  if (((Rnd::rand() % 10) < 4 && !isCapital() && likely < 2) ||
      template_army->getAwardable())
    {
      sortProduction();
      return;
    }

  if (army_type < 5)
    army_type += 1 + (Rnd::rand() % (7 + (produce_allies ? 2 : 0)));
  else
    army_type += 1 + (Rnd::rand() % (2 + (produce_allies ? 2 : 0)));
  template_army = Armysetlist::instance()->getArmy(set, army_type);
  if (!template_army ||
      (template_army->getAwardable() == true && produce_allies == false) ||
      template_army->isHero())
    {
      sortProduction();
      return;
    }
  army = new ArmyProdBase (*template_army);
  randomlyImproveOrDegradeArmy(army);
  addProductionBase(2, army);

  if (((Rnd::rand() % 10) < 6 && !isCapital() && likely < 3) ||
      template_army->getAwardable())
    {
      sortProduction();
      return;
    }

  army_type += 1 + (Rnd::rand() % (3 + (produce_allies ? 2 : 0)));
  template_army = Armysetlist::instance()->getArmy(set, army_type);
  if (!template_army ||
      (template_army->getAwardable() == true && produce_allies == false) ||
      template_army->isHero())
    {
      sortProduction();
      return;
    }
  army = new ArmyProdBase (*template_army);
  randomlyImproveOrDegradeArmy(army);
  addProductionBase(3, army);
  sortProduction();
}

int City::getDefenseLevel() const
{
  int num_production_bases = getNoOfProductionBases();
  if (isBurnt()) 
    return 0;
  else if (num_production_bases <= 2 && 
	   getOwner() == Playerlist::getNeutral())
    return 1;
  else if (num_production_bases <= 2)
    return 2;
  else if (num_production_bases > 2 && 
	   getOwner() == Playerlist::getNeutral())
    return 2;
  else if (num_production_bases > 2)
    return 3;
  return 0;
}

std::list<Stack*> City::diseaseDefenders(double percent_to_kill)
{
  std::list<Stack*> affected;
  std::vector<Stack*> stacks = 
    getOwner()->getStacklist()->getDefendersInCity(this);
  double percent;
  if (percent_to_kill > 100.0)
    percent = 100;
  else if (percent_to_kill <= 0.0)
    percent = 0;
  else
    percent = percent_to_kill;
  guint32 num_armies_to_kill = (double)countDefenders() * (percent  / 100.0);
  std::vector<guint32> ids;
  for (unsigned int i = 0; i < stacks.size(); i++)
    {
      for (Stack::iterator j = stacks[i]->begin(); j != stacks[i]->end(); ++j)
        ids.push_back((*j)->getId());
    }
  std::shuffle(ids.begin(), ids.end(), Rnd::gen ());
  for (unsigned int i = 0; i < num_armies_to_kill; i++)
    {
      Stack *s = getOwner()->getStacklist()->getArmyStackById(ids[i]);
      if (s)
        {
          Army *a = s->getArmyById(ids[i]);
          if (a)
            a->kill();
        }
    }
  for (unsigned int i = 0; i < stacks.size(); i++)
    {
      if (stacks[i]->hasDeadArmies())
        affected.push_back(stacks[i]);
    }
  return affected;
}

void City::persuadeDefenders(Player *new_owner)
{
  std::vector<Stack*> stacks = getDefenders();
  for (unsigned int i = 0; i < stacks.size(); i++)
    getOwner()->getStacklist()->changeOwnership(stacks[i], new_owner);
  switch (GameScenarioOptions::s_build_production_mode)
    {
    case GameParameters::BUILD_PRODUCTION_ALWAYS:
    case GameParameters::BUILD_PRODUCTION_NEVER:
      break;
    case GameParameters::BUILD_PRODUCTION_USUALLY:
    case GameParameters::BUILD_PRODUCTION_SELDOM:
      setBuildProduction(true);
      break;
    }
  conquer(new_owner);
}

void City::squeezeProductionSlots ()
{
  //sort the production slot ist whether or not
  //the slot is empty
  std::sort
    (begin (), end (),
     [](const ProdSlot* a, const ProdSlot* b)
     {
       bool a_null = (a->getArmyProdBase () == nullptr);
       bool b_null = (b->getArmyProdBase () == nullptr);

       if (a_null != b_null)
         return !a_null;

       return a->getArmyProdBase () < b->getArmyProdBase ();
     });
}
