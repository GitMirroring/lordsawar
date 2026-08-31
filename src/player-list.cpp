// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004 John Farrell
// Copyright (C) 2005 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2010, 2014, 2015, 2017, 2020,
// 2021 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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
#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "playerlist.h"
#include "armysetlist.h"
#include "citylist.h"
#include "ruinlist.h"
#include "vectoredunitlist.h"
#include "xmlhelper.h"
#include "history.h"
#include "stacklist.h"
#include "FogMap.h"
#include "real_player.h"
#include "ai_smart.h"
#include "ai_fast.h"
#include "ai_dummy.h"
#include "network_player.h"
#include "GameMap.h"
#include "shieldset.h"
#include "shieldsetlist.h"
#include "ucompose.hpp"
#include "stack.h"
#include "rnd.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Glib::ustring Playerlist::d_tag = "playerlist";
Playerlist* Playerlist::s_instance = 0;

Playerlist* Playerlist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Playerlist();

    return s_instance;
}

Playerlist* Playerlist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Playerlist(helper);

    return s_instance;
}

void Playerlist::deleteInstance()
{
    if (s_instance)
    {
        delete s_instance;
        s_instance = 0;
    }
}

Playerlist::Playerlist()
    :d_neutral(0), d_activeplayer (0), viewingplayer (0)
{
}

Playerlist::Playerlist (const Playerlist &plist, bool sync_ids)
 : std::list<Player*> (), sigc::trackable (plist),
    d_neutral(0), d_activeplayer (0), viewingplayer (0)
{
  for (auto p : plist)
    {
      switch (p->getType ())
        {
        case Player::HUMAN:
          add (new RealPlayer (*p, sync_ids));
          break;
        case Player::AI_FAST:
          add (new AI_Fast (*p, sync_ids));
          break;
        case Player::AI_DUMMY:
          add (new AI_Dummy (*p, sync_ids));
          break;
        case Player::AI_SMART:
          add (new AI_Smart (*p, sync_ids));
          break;
        case Player::NETWORKED:
          add (new NetworkPlayer (*p, sync_ids));
          break;
        }
      if (plist.getNeutral () == p)
        d_neutral = back ();
      if (plist.getViewingplayer () == p)
        viewingplayer = back ();
      if (plist.getActiveplayer () == p)
        d_activeplayer = back ();
      d_id[back ()->getId ()] = back ();
    }
}

Playerlist::Playerlist(XML_Helper* helper)
    :d_neutral(0)
{
    d_activeplayer=0;
    viewingplayer=0;
    //load data. This consists currently of two values: size (for checking
    //the size; we don't use it yet) and active(which player is active)
    //we do it by calling load with playerlist as string
    load(Playerlist::d_tag, helper);

    helper->registerTag(Player::d_tag, sigc::mem_fun(this, &Playerlist::load));
}

Playerlist::~Playerlist()
{
    iterator it = begin();
    while (!empty())
        it = flErase(it);
}

bool Playerlist::checkPlayers()
{
    bool last = false;
    bool dead = false;
    debug("checkPlayers()");
    iterator it = begin ();
	  
    while (it != end ())
    {
        debug("checkPlayers() iter");
        //ignore the neutral player as well as dead and immortal ones
        if ((*it) == d_neutral || (*it)->isDead() || (*it)->isImmortal())
        {
            debug("checkPlayers() dead?");
            ++it;
            continue;
        }

        if (!Citylist::getInstance()->countCities((*it)))
        {
            debug("checkPlayers() city?");
            iterator nextit = it;
            ++nextit;

            (*it)->kill();
	    if (getNoOfPlayers() == 1)
	      last = true;
            splayerDead.emit(*it);
            dead = true;
	    if (last)
	      break;

            it = nextit;    // do this at the end to catch abuse of invalid it
        } else {
            debug("checkPlayers() inc");
            ++it;
        }
    }
  return dead;
}

void Playerlist::nextPlayer()
{
    debug("nextPlayer()");

    iterator it;

    if (!d_activeplayer)
      it = begin();
    else
    {
        for (it = begin(); it != end(); ++it)
        {
            if ((*it) == d_activeplayer)
            {
                ++it;
                break;
            }
        }
    }

    // in case we have got a dead player, continue iterating. This breaks
    // if we have ONLY dead players which we assume never happens.
    while ((it == end()) || ((*it)->isDead()))
    {
        if (it == end())
        {
            it = begin();
            continue;
        }
        ++it;
    }

    d_activeplayer = (*it);
    updateViewingPlayer();
    debug("got player: " <<d_activeplayer->getName())
}

Player* Playerlist::getPlayer(Glib::ustring name) const
{
    debug("getPlayer()");
    for (const_iterator it = begin(); it != end(); ++it)
      if ((*it)->getName() == name) return (*it);
    return 0;
}

Player* Playerlist::getPlayer(guint32 id) const
{
  IdMap::const_iterator it = d_id.find(id);
  if (it == d_id.end())
    return NULL;
  return (*it).second;
}

guint32 Playerlist::getNoOfPlayers() const
{
    unsigned int number = 0;

    for (const_iterator it = begin(); it != end(); ++it)
    {
        if (((*it) != d_neutral) && !(*it)->isDead())
            number++;
    }

    return number;
}

Player* Playerlist::getFirstLiving() const
{
    for (const_iterator it = begin(); ; ++it)
        if (!(*it)->isDead() && *it != d_neutral)
            return (*it);
}

bool Playerlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Playerlist::d_tag);
    if (d_activeplayer)
      retval &= helper->saveData("active", d_activeplayer->getId());
    else
      retval &= helper->saveData("active", d_neutral->getId ());
    retval &= helper->saveData("neutral", d_neutral->getId());

    for (const_iterator it = begin(); it != end(); ++it)
        retval &= (*it)->save(helper);

    retval &= helper->closeTag();

    return retval;
}

void Playerlist::add(Player *player)
{
  push_back(player);
  d_id[player->getId()] = player;
}

bool Playerlist::load(Glib::ustring tag, XML_Helper* helper)
{
    static guint32 active = 0;
    static guint32 neutral = 0;

    if (tag == Playerlist::d_tag) //only called in the constructor
    {
        helper->getData(active, "active");
        helper->getData(neutral, "neutral");
        return true;
    }

    if (tag != Player::d_tag)
        return false;

    Player* p = Player::loadPlayer(helper);
    if(p == 0)
        return false;

    //insert player...
    add(p);

    //set neutral and active
    if (p->getId() == neutral)
      d_neutral = p;
    if (p->getId() == active)
      d_activeplayer = p;

    updateViewingPlayer();

    return true;
}

Playerlist::iterator Playerlist::flErase(Playerlist::iterator it)
{
    if ((*it) == d_neutral)
        d_neutral = 0;

    delete (*it);
    return erase (it);
}

bool compareDiplomaticScores (const struct rankable_t lhs,
			      const struct rankable_t rhs)
{
  /* make ties prefer normal player order */
  if (lhs.score == rhs.score) 
    return lhs.player->getId() > rhs.player->getId();
  else
    return lhs.score < rhs.score;
}

Glib::ustring Playerlist::get_title(int rank)
{
  if (rank == 0)
    return _("Statesman");
  else if (rank == 1)
    return _("Diplomat");
  else if (rank == 2)
    return _("Pragmatist");
  else if (rank == 3)
    return _("Politician");
  else if (rank == 4)
    return _("Deceiver");
  else if (rank == 5)
    return _("Scoundrel");
  else if (rank == 6)
    return _("Turncoat");
  else if (rank == 7)
    return _("Running Dog");
  return _("unknown");
}

void Playerlist::calculateDiplomaticRankings()
{
  unsigned int i = 0;
  int used_titles[MAX_PLAYERS];
  memset (used_titles, 0, sizeof (used_titles));

  //determine the rank for each player
  //add up the scores for all living players, and sort
  std::list<struct rankable_t> rankables;
  for (iterator pit = begin (); pit != end (); ++pit)
    {
      if ((*pit) == d_neutral)
	continue;
      if ((*pit)->isDead () == true)
	continue;
      struct rankable_t rankable;
      rankable.score = 0;
      for (iterator it = begin (); it != end (); ++it)
	{
	  if ((*it) == d_neutral)
	    continue;
	  if ((*it)->isDead () == true)
	    continue;
	  if (*pit == *it)
	    continue;
	  rankable.score += (*it)->getDiplomaticScore(*pit);
	}
      rankable.player = *pit;
      rankables.push_back(rankable);
    }
  rankables.sort (compareDiplomaticScores);
  std::reverse (rankables.begin (), rankables.end ());

  i = 1;
  for (std::list<struct rankable_t>::iterator rit = rankables.begin (); 
       rit != rankables.end (); ++rit)
    {
      (*rit).player->setDiplomaticRank(i);
      i++;
    }

  // given the rankings, what are the titles?
  // the titles are depleted from the middle as players die.
  // 7 means deplete first, 0 means deplete last.
  unsigned int deplete[MAX_PLAYERS] = { 0, 2, 4, 6, 7, 5, 3, 1 };

  unsigned int numAlive = countPlayersAlive ();
  // okay, we take the first numAlive titles

  std::vector<Glib::ustring> available_titles;
  for (i = numAlive; i < MAX_PLAYERS ; i++)
    {
      for (unsigned int j = 0; j < MAX_PLAYERS; j++)
	{
	  if (deplete[j] == i)
            used_titles[j] = 1;
	}
    }
  for (unsigned int j = 0; j < MAX_PLAYERS; j++)
    {
      if (used_titles[j] != 1)
	available_titles.push_back (get_title(j));
    }

  for (const_iterator it = begin (); it != end (); ++it)
    {
      if ((*it) == d_neutral)
	continue;
      if ((*it)->isDead () == true)
	continue;
      guint32 rank = (*it)->getDiplomaticRank();
      // recall that the first rank is 1, and not 0.
      (*it)->setDiplomaticTitle (available_titles[rank - 1]);
    }

  return;
}

void Playerlist::calculateWinners()
{
    guint32 score;
    guint32 total_gold = 0;
    guint32 total_armies = 0;
    guint32 total_cities = 0;
    for (const_iterator it = begin(); it != end(); ++it)
      {
	if ((*it) == d_neutral)
	  continue;
	if ((*it)->isDead() == true)
	  continue;
	total_gold += (*it)->getGold();
	total_armies += (*it)->getStacklist()->countArmies();
      }
    total_cities = Citylist::getInstance()->size();

    for (const_iterator it = begin(); it != end(); ++it)
      {
	if ((*it) == d_neutral)
	  continue;
	if ((*it)->isDead() == true)
	  continue;

	float city_component = (float)
	  ((float) Citylist::getInstance()->countCities(*it)/ (float)total_cities) * 70.0;
	float gold_component = (float)
	  ((float) (*it)->getGold() / (float)total_gold) * 10.0;
	float army_component = (float)
	  ((float) (*it)->getStacklist()->countArmies() / 
	   (float)total_armies) * 20.0;
	score = (guint32) (city_component + gold_component + army_component);
        (*it)->reportEndOfRound(score);
      }

    return;
}

guint32 Playerlist::countHumanPlayersAlive() const
{
  guint32 retval = 0;
  for (const_iterator it = begin(); it != end(); ++it)
    if ((*it)->isDead() == false && (*it)->getType() == Player::HUMAN)
      retval++;
  return retval;
}

guint32 Playerlist::countPlayersAlive () const
{
  guint32 numAlive = 0; 

  for (const_iterator it = begin (); it != end (); ++it)
    {
      if ((*it) == d_neutral)
	continue;
      if ((*it)->isDead () == true)
	continue;
      numAlive++;
    }
  return numAlive;
}

void Playerlist::negotiateDiplomacy()
{
  // hold diplomatic talks, and determine diplomatic outcomes
  for (iterator pit = begin(); pit != end(); ++pit)
    {
      if ((*pit)->isDead())
	continue;

      if ((*pit) == getNeutral())
	continue;
  
      for (iterator it = begin(); it != end(); ++it)
	{
      
	  if ((*it)->isDead())
	    continue;

	  if ((*it) == getNeutral())
	    continue;

	  if ((*it) == (*pit))
	    break;
  
	  Player::DiplomaticState old_state = (*pit)->getDiplomaticState(*it);
	  Player::DiplomaticState new_state = (*pit)->negotiateDiplomacy(*it);
	  (*pit)->declareDiplomacy (new_state, (*it), false);
	  (*pit)->proposeDiplomacy (Player::NO_PROPOSAL, (*it));
	  (*it)->declareDiplomacy (new_state, (*pit), false);
	  (*it)->proposeDiplomacy (Player::NO_PROPOSAL, (*pit));
	  if (old_state != new_state)
	    {
	      Player *me = *pit;
	      Player *them = *it;
	      if (new_state == Player::AT_PEACE)
		{
		  //their view of me goes up
		  them->improveDiplomaticRelationship(me, 1);
		  //their allies think better of me
		  me->improveAlliesRelationship (them, 1, Player::AT_PEACE);
		  //their enemies think less of me
		  them->deteriorateAlliesRelationship (me, 1, Player::AT_WAR);
		}
	      else if (new_state == Player::AT_WAR)
		{
		  //their view of me goes down
		  them->deteriorateDiplomaticRelationship(me, 1);
		  //their allies view of me goes down
		  them->deteriorateAlliesRelationship(me, 1, Player::AT_PEACE);
		  //their enemies view of me goes up
		  me->improveAlliesRelationship(them, 1, Player::AT_WAR);
		}
	    }
	}
    }
}

void Playerlist::swap(Player *old_player, Player *new_player)
{
  std::replace(begin(), end(), old_player, new_player);
  //point cities to the new owner
  Citylist::getInstance()->changeOwnership (old_player, new_player);
  Ruinlist::getInstance()->changeOwnership (old_player, new_player);
  VectoredUnitlist::getInstance()->changeOwnership (old_player, new_player);
  AI_Analysis::changeOwnership(old_player, new_player);
  if (old_player == d_activeplayer)
    {
      d_activeplayer = new_player;
      d_activeplayer->setActivestack(0);
    }
  if (old_player == viewingplayer)
    viewingplayer = new_player;
  d_id[new_player->getId()] = new_player;
  GameMap::getInstance()->clearStackPositions();
  GameMap::getInstance()->updateStackPositions();
  /* note, we don't have to change the player associated with flag graphics
     because it's stored as an id. */
}

bool Playerlist::randomly(const Player *lhs, const Player *rhs)  
{
  (void) rhs;
  if (lhs == Playerlist::getInstance()->getNeutral())
    return false;
  if (Rnd::rand() % 2 == 0)
    return true;
  else
    return false;
}

bool Playerlist::inOrderOfId(const Player *lhs, const Player *rhs)  
{
  if (lhs->getId() > rhs->getId())
    return false;
  else
    return true;
}

//randomly reorder the player list, but keeping neutral last.
void Playerlist::randomizeOrder()
{
  sort(randomly);
  d_activeplayer = NULL;
  viewingplayer = NULL;
}

void Playerlist::nextRound(bool diplomacy, bool *surrender_already_offered)
{
  // update diplomacy
  if (diplomacy)
    {
      negotiateDiplomacy();
      calculateDiplomaticRankings();
    }

  // update winners
  calculateWinners();

  // offer surrender
  if (countHumanPlayersAlive() == 1 &&
      *surrender_already_offered == 0)
    {
      for (iterator it = begin(); it != end(); ++it)
	{
	  if ((*it)->getType() == Player::HUMAN)
	    {
	      int target_level = Citylist::getInstance()->size() / 2;
	      if (Citylist::getInstance()->countCities(*it) > target_level)
		{
		  *surrender_already_offered = 1;
		  ssurrender.emit(*it);
		  break;
		}
	    }
	}
    }
}

void Playerlist::syncPlayer(GameParameters::Player player)
{
  Player *p = getPlayer((guint32)player.id);
  if (!p)
    {
      //player was off originally, but now it's on
      guint32 armyset = d_neutral->getArmyset();
      int width = d_neutral->getFogMap()->getWidth();
      int height = d_neutral->getFogMap()->getHeight();
      int gold = d_neutral->getGold();
      Shieldset *shieldset = GameMap::getShieldset();
      switch (player.type)
	{
	case GameParameters::Player::HUMAN:
	  p = new RealPlayer(player.name, armyset,
			     shieldset->getColors(player.id),
			     width, height, Player::HUMAN, player.id);
	  break;
	case GameParameters::Player::EASY:
	  p = new AI_Fast(player.name, armyset,
		       	  shieldset->getColors(player.id),
			  width, height, player.id);
	  break;
	case GameParameters::Player::HARD:
	  p = new AI_Smart(player.name, armyset,
			   shieldset->getColors(player.id),
			   width, height, player.id);
	  break;
	case GameParameters::Player::OFF:
	  //was off, now it's still off.
	  break;
	default:
          std::cerr << String::ucompose("could not make player with type %1", player.type) << std::endl;
	  exit (1);
	  break;
	}
      if (p)
	{
	  p->setGold(gold);
	  add(p);

	  sort(inOrderOfId);
	  d_activeplayer = getFirstLiving();
          updateViewingPlayer();
	}
      return;
    }
  else
    p->setName(player.name);

          
  switch (player.type)
    {
    case GameParameters::Player::HUMAN:
      if (p->getType() != Player::HUMAN)
	{
	  RealPlayer *new_p = new RealPlayer(*p, true);
	  swap(p, new_p);
	}
      break;
    case GameParameters::Player::EASY:
      if (p->getType() != Player::AI_FAST)
	{
	  AI_Fast *new_p = new AI_Fast(*p, true);
	  swap(p, new_p);
	}
      break;
    case GameParameters::Player::HARD:
      if (p->getType() != Player::AI_SMART)
	{
	  AI_Smart *new_p = new AI_Smart(*p, true);
	  swap(p, new_p);
	}
      break;
    case GameParameters::Player::OFF:
	{
	  //point owned cities to neutral
          Citylist::getInstance()->changeOwnership (p, d_neutral);
	  //point owned ruins to neutral
          Ruinlist::getInstance()->changeOwnership (p, d_neutral);
          //also copy over the stacks to neutral
          p->getStacklist()->changeOwnership(p, d_neutral);
	  //now get rid of the player entirely
          GameMap::getInstance()->clearStackPositions();
          if (d_id.find(p->getId()) != d_id.end())
            d_id.erase(d_id.find(p->getId()));
	  flErase(find(begin(), end(), p));
          GameMap::getInstance()->updateStackPositions();
	}
      break;
    default:
      std::cerr << String::ucompose("could not sync player with type %1", player.type) << std::endl;
      exit (1);
      break;
    }

  sort(inOrderOfId);
  d_activeplayer = getFirstLiving();
  updateViewingPlayer();
  return;
}

void Playerlist::syncPlayers(std::vector<GameParameters::Player> players)
{
  std::vector<GameParameters::Player>::const_iterator i = players.begin();
  for (; i != players.end(); ++i)
    syncPlayer(*i);
}
	
guint32 Playerlist::turnHumansIntoNetworkPlayers()
{
  guint32 count = 0;
  std::list<Player*> p;
  for (iterator i = begin(); i != end(); ++i)
    {
      if ((*i)->getType() == Player::HUMAN)
	{
          count++;
	  NetworkPlayer *new_p = new NetworkPlayer(**i, true);
          p.push_back(*i);
	  swap((*i), new_p);
	  i = begin();
	  continue;
	}
    }
  for (std::list<Player*>::iterator j = p.begin(); j != p.end(); ++j)
    delete *j;
  return count;
}

guint32 Playerlist::turnHumansInto(Player::Type type, int number_of_players)
{
  int count = 0;
  for (iterator i = begin(); i != end(); ++i)
    {
      if (count >= number_of_players && number_of_players > 0)
	break;
      if ((*i)->getType() == Player::HUMAN)
	{
	  switch (type)
	    {
	    case Player::AI_DUMMY:
		{
		  AI_Dummy *new_p = new AI_Dummy(**i, true);
		  swap((*i), new_p);
		  //delete *i; fixme
		  i = begin();
		  count++;
		  continue;
		}
	      break;
	    case Player::AI_FAST:
		{
		  AI_Fast *new_p = new AI_Fast(**i, true);
		  swap((*i), new_p);
		  //delete *i; fixme
		  i = begin();
		  count++;
		  continue;
		}
	      break;
	    case Player::AI_SMART:
		{
		  AI_Smart *new_p = new AI_Smart(**i, true);
		  swap((*i), new_p);
		  //delete *i; fixme
		  i = begin();
		  count++;
		  continue;
		}
	      break;
	    case Player::NETWORKED:
		{
		  NetworkPlayer *new_p = new NetworkPlayer(**i, true);
		  swap((*i), new_p);
		  //delete *i; fixme
		  i = begin();
		  count++;
		  continue;
		}
	      break;
	    case Player::HUMAN:
	      break;
	    }
	}
    }
  return count;
}

std::list<guint32> given_turn_order;
bool Playerlist::inGivenOrder(const Player *lhs, const Player *rhs)  
{
  if (given_turn_order.size() == 0)
    return true;

  int count = 0;
  for(std::list<guint32>::iterator it = given_turn_order.begin(); 
      it != given_turn_order.end(); ++it)
    {
      count++;
      if (lhs->getId() == (*it))
	break;
    }
  int lhs_rank = count;
  count = 0;
  for(std::list<guint32>::iterator it = given_turn_order.begin(); 
      it != given_turn_order.end(); ++it)
    {
      count++;
      if (rhs->getId() == (*it))
	break;
    }
  int rhs_rank = count;
  return lhs_rank < rhs_rank;
}

void Playerlist::reorder(const std::list<guint32> &order)
{
  given_turn_order = order;
  sort(inGivenOrder);
  given_turn_order.clear();
  d_activeplayer = getFirstLiving();
  updateViewingPlayer();
}

std::list<History *>Playerlist::getHistoryForHeroId(guint32 id) const
{
  std::list<History*> no_events;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      std::list<History *>events = (*it)->getHistoryForHeroId(id);
      if (events.size() > 0)
	return events;
    }
  return no_events;
}

void Playerlist::surrender()
{
  //the last human player has accepted surrender
  for (iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getType() != Player::HUMAN)
        (*it)->setSurrendered(true);
    }
}

bool Playerlist::isEndOfRound() const
{
  //check to see if all players have moved this round.
  //do all players have the same number of history:end_turn events?
  if (d_activeplayer == NULL)
    return false;
  guint32 count = d_activeplayer->countEndTurnHistoryEntries();
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if (*it == d_activeplayer)
	continue;
      if (count != (*it)->countEndTurnHistoryEntries())
	return false;
    }
  return true;
}

void Playerlist::setWinningPlayer(Player *winner)
{
  //only for humans
  d_activeplayer = winner;
}

Player *Playerlist::getWinningPlayer() const
{
  guint32 best_score = 0;
  Player *winning_player = NULL;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      Player *p = (*it);
      if (p->isDead() == false)
        continue;
      if (p == getNeutral())
        continue;
      if (p->getScore() >= best_score)
        {
          best_score = p->getScore();
          winning_player = p;
        }
    }
  return winning_player;
}

bool Playerlist::hasArmyset(guint32 id) const
{
  return getPlayersWithArmyset (id).empty() == false;
}

std::vector<Player*> Playerlist::getPlayersWithArmyset(guint32 id) const
{
  std::vector<Player *> players;
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getArmyset() == id)
        players.push_back (*it);
    }
  return players;
}

void Playerlist::setNewColors(Shieldset *shieldset)
{
  for (iterator it = begin(); it != end(); ++it)
    (*it)->setColors(shieldset->getColors((*it)->getId()));
}

void Playerlist::clearAllActions()
{
  for (iterator it = begin(); it != end(); ++it)
    (*it)->clearActionlist();
}

void Playerlist::syncNeutral()
{
  if (d_neutral == NULL)
    return;
  if (d_neutral->getType() == Player::AI_DUMMY)
    return;
  //okay, let's sync it.
  Player *p = new AI_Dummy(*d_neutral);
  swap(d_neutral, p);
  d_neutral = p;
}

Stack *Playerlist::getStackById(guint32 id) const
{
  //this method checks each stacklist for the given stack id.
  //the stack ids in each stacklist are in a std::map, so that's how the
  //stack pointers are gotten.
  for (const_iterator j = begin(), jend = end(); j != jend; ++j) 
    {
      Stack *s = (*j)->getStacklist()->getStackById(id);
      if (s)
        return s;
    }
  return NULL;
}

void Playerlist::updateViewingPlayer ()
{
  /*
   * the idea here is that the viewing player is related to the
   * hidden map.
   *
   * imagine a computer player moving and then it walks into the area of
   * your map that you have uncovered.
   *
   * i think the smallmap happens to be getting blanked right now,
   * but this funciton is about retaining the last human player as the
   * player who can see movements through his or her territory.
   *
   * the blanking of the smallmap/bigmap is because:
   * it's not really fair to show one player some enemy units going through
   * his or her territory, and not all players.  why should one player be
   * rewarded in this way just by fluke?
   *
   * this is the story for hotseat anyway.  i'm not sure how it relates to
   * network play.
   */
  if (d_activeplayer && d_activeplayer->getType() == Player::HUMAN)
    viewingplayer = d_activeplayer;
  else
    viewingplayer = getNeutral();
}

guint32 Playerlist::getTurnOrderNumber(const Player *p)
{
  guint32 count = 1;
  for (const_iterator i = begin(); i != end(); ++i)
    {
      if ((*i) == p)
        break;
      count++;
    }
  return count;
}

guint32 Playerlist::countAllStacks () const
{
  guint32 count = 0;
  for (const_iterator i = begin(); i != end(); ++i)
    count += (*i)->getStacklist ()->size ();
  return count;
}

bool Playerlist::playerHasNoCapitalCity () const
{
  for (const_iterator i = begin (); i != end (); ++i)
    if (*i != d_neutral &&
        Citylist::getInstance ()->getCapitalCity (*i) == NULL)
      return true;
  return false;
}
        
std::list<guint32> Playerlist::getArmysets() const
{
  std::list<guint32> ids;
  for (const_iterator i = begin (); i != end (); ++i)
    ids.push_back ((*i)->getArmyset ());
  ids.unique ();
  return ids;
}

void Playerlist::reset (Playerlist *p)
{
  delete s_instance;
  s_instance = p;
}
