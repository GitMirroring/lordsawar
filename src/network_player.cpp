// Copyright (C) 2008, 2009, 2010, 2011, 2014, 2015, 2017, 2020,
// 2021 Ben Asselstine
// Copyright (C) 2008 Ole Laursen
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
#include <assert.h>

#include "network_player.h"
#include "playerlist.h"
#include "armysetlist.h"
#include "stacklist.h"
#include "citylist.h"
#include "portlist.h"
#include "templelist.h"
#include "ruinlist.h"
#include "signpostlist.h"
#include "Itemlist.h"
#include "SightMap.h"
#include "rewardlist.h"
#include "QuestsManager.h"
#include "Quest.h"
#include "path.h"
#include "GameMap.h"
#include "army.h"
#include "armyprodbase.h"
#include "hero.h"
#include "heroproto.h"
#include "action.h"
#include "MoveResult.h"
#include "Configuration.h"
#include "FogMap.h"
#include "xmlhelper.h"
#include "game-parameters.h"
#include "signpost.h"
#include "history.h"
#include "vectoredunit.h"
#include "Backpack.h"
#include "MapBackpack.h"
#include "stackreflist.h"
#include "city.h"
#include "game-actionlist.h"
#include "keeper.h"

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

NetworkPlayer::NetworkPlayer(Glib::ustring name, guint32 armyset,
                             std::vector<Gdk::RGBA> colors, int width,
                             int height, Player::Type type, int player_no)
    :Player(name, armyset, colors, width, height, type, player_no),
    d_connected(false), d_abort_requested(false)
{
}

NetworkPlayer::NetworkPlayer(const Player& player, bool sync_ids)
    :Player(player, sync_ids), d_connected(false)
{
    d_type = Player::NETWORKED;
    d_abort_requested = false;
}

NetworkPlayer::NetworkPlayer(XML_Helper* helper)
    :Player(helper), d_connected(false)
{
    d_abort_requested = false;
}

bool NetworkPlayer::save(XML_Helper* helper) const
{
    // This may seem a bit dumb, but allows derived players (especially
    // AI's) to save additional data, such as character types or so.
    bool retval = true;
    retval &= helper->openTag(Player::d_tag);
    retval &= Player::save(helper);
    retval &= helper->closeTag();
    return retval;
}

void NetworkPlayer::abortTurn()
{
  d_abort_requested = true;
    aborted_turn.emit();
}

bool NetworkPlayer::startTurn()
{
  return false;
}

void NetworkPlayer::endTurn()
{
}

void NetworkPlayer::invadeCity(City* c)
{
  (void) c;
  assert(false);
}
bool NetworkPlayer::chooseHero(HeroProto *hero, City *city, int gold)
{
  (void) hero;
  (void) city;
  (void) gold;
  assert(false);
  return true;
}

Reward *NetworkPlayer::chooseReward(Ruin *ruin, Sage *sage, Stack *stack)
{
  (void) ruin;
  (void) sage;
  (void) stack;
  assert(false);
  return NULL;
}

void NetworkPlayer::heroGainsLevel(Hero * a)
{
  (void) a;
  assert(false);
}

bool NetworkPlayer::chooseTreachery (Stack *stack, Player *player, Vector <int> pos)
{
  (void) stack;
  (void) player;
  (void) pos;
  assert(false);
  return true;
}

Army::Stat NetworkPlayer::chooseStat(Hero *hero)
{
  (void) hero;
  assert(false);
  return Army::STRENGTH;
}

bool NetworkPlayer::chooseQuest(Hero *hero)
{
  (void) hero;
  assert(false);
  return true;
}

bool NetworkPlayer::computerChooseVisitRuin(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) dest;
  (void) moves;
  (void) turns;
  assert (false);
  return true;
}

bool NetworkPlayer::computerChoosePickupBag(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) dest;
  (void) moves;
  (void) turns;
  assert (false);
  return true;
}

bool NetworkPlayer::computerChooseVisitTempleForBlessing(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) dest;
  (void) moves;
  (void) turns;
  assert (false);
  return true;
}

bool NetworkPlayer::computerChooseVisitTempleForQuest(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) dest;
  (void) moves;
  (void) turns;
  assert (false);
  return true;
}

bool NetworkPlayer::computerChooseContinueQuest(Stack *stack, Quest *quest, Vector<int> dest, guint32 moves, guint32 turns)
{
  (void) stack;
  (void) quest;
  (void) dest;
  (void) moves;
  (void) turns;
  assert (false);
  return true;
}

void NetworkPlayer::decodeActions(std::list<Action *> actions)
{
  if (isDead())
    return;
  pruneActionlist();
  for (std::list<Action*>::iterator it = actions.begin();
       it != actions.end(); ++it)
    decodeAction(*it);
}

void NetworkPlayer::decodeAction(const Action *a)
{
  sbusy.emit ();
  d_actions.push_back(Action::copy(a));
  switch(a->getType())
    {
    case Action::STACK_MOVE:
      return decodeActionMove(dynamic_cast<const Action_Move*>(a));
    case Action::STACK_SPLIT:
      return decodeActionSplit(dynamic_cast<const Action_Split*>(a));
    case Action::STACK_FIGHT:
      return decodeActionFight(dynamic_cast<const Action_Fight*>(a));
    case Action::STACK_JOIN:
      return decodeActionJoin(dynamic_cast<const Action_Join*>(a));
    case Action::RUIN_SEARCH:
      return decodeActionRuin(dynamic_cast<const Action_Ruin*>(a));
    case Action::TEMPLE_SEARCH:
      return decodeActionTemple(dynamic_cast<const Action_Temple*>(a));
    case Action::CITY_OCCUPY:
      return decodeActionOccupy(dynamic_cast<const Action_Occupy*>(a));
    case Action::CITY_PILLAGE:
      return decodeActionPillage
	(dynamic_cast<const Action_Pillage*>(a));
    case Action::CITY_SACK:
      return decodeActionSack(dynamic_cast<const Action_Sack*>(a));
    case Action::CITY_RAZE:
      return decodeActionRaze(dynamic_cast<const Action_Raze*>(a));
    case Action::CITY_BUY:
      return decodeActionBuy(dynamic_cast<const Action_Buy*>(a));
    case Action::CITY_PROD:
      return decodeActionProduction 
	(dynamic_cast<const Action_Production*>(a));
    case Action::REWARD: 
      return decodeActionReward(dynamic_cast<const Action_Reward*>(a));
    case Action::QUEST:
      return decodeActionQuest(dynamic_cast<const Action_Quest*>(a));
    case Action::HERO_EQUIP:
      return decodeActionEquip(dynamic_cast<const Action_Equip*>(a));
    case Action::UNIT_ADVANCE:
      return decodeActionLevel(dynamic_cast<const Action_Level*>(a));
    case Action::STACK_DISBAND:
      return decodeActionDisband (dynamic_cast<const Action_Disband*>(a));
    case Action::MODIFY_SIGNPOST:
      return decodeActionModifySignpost
	(dynamic_cast<const Action_ModifySignpost*>(a));
    case Action::CITY_RENAME:
      return decodeActionRenameCity 
	(dynamic_cast<const Action_RenameCity*>(a));
    case Action::CITY_VECTOR:
      return decodeActionVector (dynamic_cast<const Action_Vector*>(a));
    case Action::FIGHT_ORDER:
      return decodeActionFightOrder 
	(dynamic_cast<const Action_FightOrder*>(a));
    case Action::RESIGN:
      return decodeActionResign(dynamic_cast<const Action_Resign*>(a));
    case Action::ITEM_PLANT:
      return decodeActionPlant(dynamic_cast<const Action_Plant*>(a));
    case Action::PRODUCE_UNIT:
      return decodeActionProduce (dynamic_cast<const Action_Produce*>(a));
    case Action::PRODUCE_VECTORED_UNIT:
      return decodeActionProduceVectored
	(dynamic_cast<const Action_ProduceVectored*>(a));
    case Action::DIPLOMATIC_STATE:
      return decodeActionDiplomacyState 
	(dynamic_cast<const Action_DiplomacyState*>(a));
    case Action::DIPLOMATIC_PROPOSAL:
      return decodeActionDiplomacyProposal
	(dynamic_cast<const Action_DiplomacyProposal*>(a));
    case Action::DIPLOMATIC_SCORE:
      return decodeActionDiplomacyScore
	(dynamic_cast<const Action_DiplomacyScore*>(a));
    case Action::END_TURN:
      return decodeActionEndTurn
        (dynamic_cast<const Action_EndTurn*>(a));
    case Action::CITY_CONQUER:
      return decodeActionConquerCity
        (dynamic_cast<const Action_ConquerCity*>(a));
    case Action::RECRUIT_HERO:
      return decodeActionRecruitHero
        (dynamic_cast<const Action_RecruitHero*>(a));
    case Action::CITY_DESTITUTE:
      return decodeActionCityTooPoorToProduce
	(dynamic_cast<const Action_CityTooPoorToProduce*>(a));
    case Action::INIT_TURN:
      return decodeActionInitTurn
	(dynamic_cast<const Action_InitTurn*>(a));
    case Action::CITY_LOOT:
      return decodeActionLoot
	(dynamic_cast<const Action_Loot*>(a));
    case Action::USE_ITEM:
      return decodeActionUseItem
	(dynamic_cast<const Action_UseItem*>(a));
    case Action::STACK_ORDER:
      return decodeActionStackOrder(dynamic_cast<const Action_ReorderArmies*>(a));
    case Action::STACKS_RESET:
      return decodeActionStacksReset(dynamic_cast<const Action_ResetStacks*>(a));
    case Action::RUINS_RESET:
      return decodeActionRuinsReset(dynamic_cast<const Action_ResetRuins*>(a));
    case Action::COLLECT_TAXES_AND_PAY_UPKEEP:
      return decodeActionCollectTaxesAndPayUpkeep(dynamic_cast<const Action_CollectTaxesAndPayUpkeep*>(a));
    case Action::KILL_PLAYER:
      return decodeActionKillPlayer(dynamic_cast<const Action_Kill*>(a));
    case Action::STACK_DEFEND:
      return decodeActionDefendStack(dynamic_cast<const Action_DefendStack*>(a));
    case Action::STACK_UNDEFEND:
      return decodeActionUndefendStack(dynamic_cast<const Action_UndefendStack*>(a));
    case Action::STACK_PARK:
      return decodeActionParkStack(dynamic_cast<const Action_ParkStack*>(a));
    case Action::STACK_UNPARK:
      return decodeActionUnparkStack(dynamic_cast<const Action_UnparkStack*>(a));
    case Action::STACK_SELECT:
      return decodeActionSelectStack(dynamic_cast<const Action_SelectStack*>(a));
    case Action::STACK_DESELECT:
      return decodeActionDeselectStack(dynamic_cast<const Action_DeselectStack*>(a));
    }
  return;
}

// decoders

void NetworkPlayer::decodeActionMove(const Action_Move *action)
{
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  if (stack == NULL)
    {
      debug ("couldn't find stack with id " <<  action->getStackId());
      debug ("is there a stack near the ending position?");
      for (int x = -1; x <= 1; x++)
	for (int y = -1; y <= 1; y++)
	  {
	    Vector<int> dest = action->getEndingPosition() + Vector<int>(x,y);
	    Stack *s = GameMap::getFriendlyStack(dest);
	    debug ("stack at position " << dest.x << "," << dest.y << " is " << s);
	    if (s)
              {
                debug ("stack id is " << s->getId());
              }
	  }
    }
  assert (stack != NULL);

  assert (stack->getPos() == (action->getEndingPosition() - action->getPositionDelta()));

  if (stack->hasShip() != action->getHadShip())
    {
      printf("expected %d for hadship, but got %d\n", action->getHadShip(), stack->hasShip());
    }
  assert (stack->hasShip() == action->getHadShip());
  /*
    {
      Vector<int> dest= action->getEndingPosition();
  Maptile::Building dst_building = GameMap::getInstance()->getBuilding(dest);
  Maptile::Building src_building = GameMap::getInstance()->getBuilding(stack->getPos());
  bool to_city = dst_building == Maptile::CITY;
  bool on_port = src_building == Maptile::PORT;
  bool on_bridge = src_building == Maptile::BRIDGE;
  bool on_water = (GameMap::getInstance()->getTerrainType(stack->getPos()) == Tile::WATER);
  bool to_water = (GameMap::getInstance()->getTerrainType(dest) == Tile::WATER);
      printf("has ship is %d\n", stack->hasShip());
      printf("to_city is %d, on_water is %d\n", to_city, on_water);
      printf("on_port is %d, on_bridge is %d\n", on_port, on_bridge);
      printf("to_water is %d\n", to_water);
            }
  */
  bool skipping = false;
  if (!stack->isFlying())
    {
      bool on_ship = stack->hasShip();
      if (stack->isMovingToOrFromAShip(action->getEndingPosition(), on_ship) == true)
        {
          //are we skipping?
          if (GameMap::countArmyUnits(action->getEndingPosition()) + stack->size() > MAX_STACK_SIZE && GameMap::getFriendlyStack(action->getEndingPosition()) != NULL)
            skipping = true;
        }
    }

  Playerlist::getActiveplayer()->setActivestack (stack);

  stack->moveToDest(action->getEndingPosition(), skipping);
  if (stack->hasShip() != action->getHasShip())
    {
      printf("expected %d for ship, but we got %d\n", action->getHasShip(), stack->hasShip());
    }
  assert (stack->hasShip() == action->getHasShip());
  if (stack->getMoves () != action->getMovesLeft())
    {

      printf("expected %d moves left, but we got %d\n", action->getMovesLeft(), stack->getMoves());
      printf("it is on a tile of type %d, with a building of %d\n", GameMap::getInstance()->getTile(action->getEndingPosition())->getType(), GameMap::getInstance()->getTile(action->getEndingPosition())->getBuilding());
    }
  assert (stack->getMoves() == action->getMovesLeft());
  supdatingStack.emit(stack);
}

void NetworkPlayer::decodeActionSplit(const Action_Split *action)
{
  guint32 stack_id = action->getStackId();
  Stack *stack = d_stacklist->getStackById(stack_id);
  assert (stack != NULL);
  
  std::list<guint32> armies;
  for (unsigned int i = 0; i < MAX_STACK_SIZE; ++i) 
    if (action->getGroupedArmyId(i) != 0)
      armies.push_back(action->getGroupedArmyId(i));

  Stack *new_stack = NULL;
  doStackSplitArmies(stack, armies, new_stack);
  assert (new_stack != NULL);
  if (new_stack->getId() != action->getNewStackId())
    {
      debug ("created stack with id " << new_stack->getId() << ", but expected " << action->getNewStackId());
    }
  assert (new_stack->getId() == action->getNewStackId());
}

void NetworkPlayer::decodeActionFight(const Action_Fight *action)
{
  debug ("performing action: " <<  action->dump());
  std::list<Stack *> attackers, defenders;
  std::list<guint32> attacker_stack_ids = action->getAttackerStackIds();
  for (std::list<guint32>::const_iterator i = attacker_stack_ids.begin(),
         end = attacker_stack_ids.end(); i != end; ++i)
    attackers.push_back(d_stacklist->getStackById(*i));

  std::list<guint32> defender_stack_ids = action->getDefenderStackIds();
  for (std::list<guint32>::const_iterator i = defender_stack_ids.begin(),
         end = defender_stack_ids.end(); i != end; ++i)
    defenders.push_back(Playerlist::getInstance()->getStackById(*i));

  if (action->getAttackerStackIds().empty())
    printf("expected the attacker stack to be non empty!\n");
  assert (action->getAttackerStackIds().empty() == false);
  if (action->getAttackerArmyIds().empty())
    printf("expected the attacker armies to be non empty!\n");
  assert (action->getAttackerArmyIds().empty() == false);

  for (auto f: action->getBattleHistory())
    {
      bool attacker = action->is_army_id_in_stacks(f.id, attacker_stack_ids);
      bool defender = action->is_army_id_in_stacks(f.id, defender_stack_ids);
      if (!attacker && ! defender)
        {
          printf("army id %d is not in attackers or defenders.\n", f.id);
        }
      assert (attacker || defender);
    }
  for (auto f: action->getAttackerArmyIds())
    {
      bool attacker = action->is_army_id_in_stacks(f, attacker_stack_ids);
      if (!attacker)
        {
          printf("army id %d is not in attackers!\n", f);
        }
      assert (action->is_army_id_in_stacks(f, attacker_stack_ids) == true);
    }
  for (auto f: action->getDefenderArmyIds())
    {
      bool defender = action->is_army_id_in_stacks(f, defender_stack_ids);
      if (!defender)
        {
          printf("army id %d is not in defenders!\n", f);
        }
      assert (action->is_army_id_in_stacks(f, defender_stack_ids) == true);
    }

  Fight fight(attackers, defenders, action->getBattleHistory());
  Fight::Result result = fight.battleFromHistory();
  fight_started.emit(fight);

  std::list<History*> attacker_history;
  std::list<History*> defender_history;
  cleanupAfterFight(attackers, defenders, attacker_history, defender_history);
  clearHistorylist(attacker_history);
  clearHistorylist(defender_history);
  if (result == Fight::ATTACKER_WON)
    {
      debug ("there are " << (&*attackers.front())->size() << " attackers left in " << (&*attackers.front())->getId() << " at " << (&*attackers.front())->getPos().x << "," << (&*attackers.front())->getPos().y);
    }
}

void NetworkPlayer::decodeActionJoin(const Action_Join *action)
{
  Stack *receiver = d_stacklist->getStackById(action->getReceivingStackId());
  Stack *joining = d_stacklist->getStackById(action->getJoiningStackId());

  assert (receiver != NULL);
  assert (joining != NULL);
  doStackJoin(receiver, joining);
  supdatingStack.emit(0);
}

void NetworkPlayer::decodeActionRuin(const Action_Ruin *action)
{
  Stack *explorer = d_stacklist->getStackById(action->getStackId());
  Ruin *r = Ruinlist::getInstance()->getById(action->getRuinId());
  bool searched = action->getSearchSuccessful();
  Keeper *keeper = r->getOccupant();
  Stack *stack = NULL;
  if (keeper)
    stack = keeper->getStack ();

  Fight::Result result = Fight::ATTACKER_WON;
  if (searched == false)
    result = Fight::DEFENDER_WON;

  if (searched == false && keeper == NULL)
    {
      std::cerr << "whoops, we have an impossible situation here." << std::endl;
      exit(0);
    }
  // now simulate the fight that might have happened on the other side
  if (keeper) 
    {
      if (result == Fight::ATTACKER_WON) 
        {
          // whack the keeper
          for (Stack::iterator i = stack->begin(); i != stack->end(); ++i)
            (*i)->setHP(0);
        }
      else if (result == Fight::DEFENDER_WON)
        {
          // whack the hero
          explorer->getFirstHero()->setHP(0);
        }

      std::list<Stack *> attackers, defenders;
      attackers.push_back(explorer);
      defenders.push_back(stack);

      std::list<History*> attacker_history;
      std::list<History*> defender_history;
      cleanupAfterFight(attackers, defenders, attacker_history, defender_history);
      clearHistorylist(attacker_history);
      clearHistorylist(defender_history);
    }

   doStackSearchRuin(explorer, r, result);

  //the reward is given to the player via the decodeActionReward method.
  supdatingStack.emit(0);
}

void NetworkPlayer::decodeActionTemple(const Action_Temple *action)
{
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  doStackVisitTemple(stack);
}

void NetworkPlayer::decodeActionOccupy(const Action_Occupy *action)
{
  doCityOccupy(Citylist::getInstance()->getById(action->getCityId()));
}

void NetworkPlayer::decodeActionPillage(const Action_Pillage *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  int gold, pillaged_army_type;
  doCityPillage(city, gold, &pillaged_army_type);
}

void NetworkPlayer::decodeActionSack(const Action_Sack *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  int gold;
  std::list<guint32> sacked_types;
  doCitySack(city, gold, &sacked_types);
  // FIXME: the game class doesn't listen for sack signals, so it doesn't
  // redraw the map as it should
}

void NetworkPlayer::decodeActionRaze(const Action_Raze *action)
{
  doCityRaze(Citylist::getInstance()->getById(action->getCityId()));
}

void NetworkPlayer::decodeActionBuy(const Action_Buy *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  doCityBuyProduction(city, action->getProductionSlot(), 
		      action->getBoughtArmyTypeId());
}

void NetworkPlayer::decodeActionProduction(const Action_Production *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  doCityChangeProduction(city, action->getSlot());
}

void NetworkPlayer::decodeActionReward(const Action_Reward *action)
{
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  StackReflist *stacks = new StackReflist();
  doGiveReward(stack, action->getReward(), stacks);
  delete stacks;
  supdatingStack.emit(stack); // make sure we get a redraw
}

void NetworkPlayer::decodeActionQuest(const Action_Quest *action)
{
  QuestsManager *qm = QuestsManager::getInstance();
  switch (Quest::Type(action->getQuestType()))
    {
    case Quest::KILLHERO: 
      qm->createNewKillHeroQuest(action->getHeroId(), action->getData());
      break;
    case Quest::KILLARMIES:
      qm->createNewEnemyArmiesQuest(action->getHeroId(), action->getData(), 
				    action->getVictimPlayerId());
      break;
    case Quest::CITYSACK:
      qm->createNewCitySackQuest(action->getHeroId(), action->getData());
      break;
    case Quest::CITYRAZE:
      qm->createNewCityRazeQuest(action->getHeroId(), action->getData());
      break;
    case Quest::CITYOCCUPY:
      qm->createNewCityOccupyQuest(action->getHeroId(), action->getData());
      break;
    case Quest::KILLARMYTYPE:
      qm->createNewEnemyArmytypeQuest(action->getHeroId(), action->getData());
      break;
    case Quest::PILLAGEGOLD:
      qm->createNewPillageGoldQuest(action->getHeroId(), action->getData());
      break;
    }
}

void NetworkPlayer::decodeActionEquip(const Action_Equip *action)
{
  Stack *stack = d_stacklist->getArmyStackById(action->getHeroId());
  if (stack == NULL)
    {
      debug ("couldn't find hero with id " <<  action->getHeroId());
    }
  assert (stack != NULL);
  Hero *hero = dynamic_cast<Hero *>(stack->getArmyById(action->getHeroId()));
  Item *item = 0;

  bool splash = false;
  switch (action->getToBackpackOrToGround())
  {
  case Action_Equip::BACKPACK:
    item = GameMap::getInstance()->getTile(action->getItemPos())->getBackpack()->getItemById(action->getItemId());
    doHeroPickupItem(hero, item, action->getItemPos());
    break;

  case Action_Equip::GROUND:
    item = hero->getBackpack()->getItemById(action->getItemId());
    doHeroDropItem(hero, item, action->getItemPos(), splash);
    break;
  }
}

void NetworkPlayer::decodeActionLevel(const Action_Level *action)
{
  Stack *stack = d_stacklist->getArmyStackById(action->getArmyId());
  Hero*hero= dynamic_cast<Hero*>(stack->getArmyById(action->getArmyId()));

  doHeroGainsLevel(hero, Army::Stat(action->getStatToIncrease()));
  debug ("army is hero? " << hero->isHero());
  debug ("new level is " << hero->getLevel());
}

void NetworkPlayer::decodeActionDisband(const Action_Disband *action)
{
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  if (stack == NULL)
    {
      debug ("couldn't find stack with id " << action->getStackId());
    }
  assert (stack != NULL);
  bool found = doStackDisband(stack);
  assert (found == true);
}

void NetworkPlayer::decodeActionModifySignpost(const Action_ModifySignpost *act)
{
  Signpost *sign = Signpostlist::getInstance()->getById(act->getSignpostId());
  doSignpostChange(sign, act->getSignContents());
}

void NetworkPlayer::decodeActionRenameCity(const Action_RenameCity *action)
{
  doCityRename(Citylist::getInstance()->getById(action->getCityId()), 
               action->getNewCityName());
}

void NetworkPlayer::decodeActionVector(const Action_Vector *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  doVectorFromCity(city, action->getVectoringDestination());
}

void NetworkPlayer::decodeActionFightOrder(const Action_FightOrder *action)
{
  doSetFightOrder(action->getFightOrder());
}

void NetworkPlayer::decodeActionResign(const Action_Resign *action)
{
  (void) action;
  std::list<History*> history;
  doResign(history);
  clearHistorylist(history);
}

void NetworkPlayer::decodeActionPlant(const Action_Plant *action)
{
  Stack *stack = d_stacklist->getArmyStackById(action->getHeroId());
  Hero *hero = dynamic_cast<Hero *>(stack->getArmyById(action->getHeroId()));
  Item *item = hero->getBackpack()->getItemById(action->getItemId());
    
  doHeroPlantStandard(hero, item, stack->getPos());
}

void NetworkPlayer::decodeActionProduce(const Action_Produce *action)
{
  //if it was vectored, we just wait for the Action_ProduceVectored later on.
  if (action->getVectored() == true)
    {
      debug ("produced unit but it's vectored.");
      debug ("we could put it in the vectored unit list, but why bother eh.");
      debug ("We can just make one on demand when it \"shows up\".");
    return;
    }
  //ArmyProdBase *a = action->getArmy();
  City *c = Citylist::getInstance()->getById(action->getCityId());
  //Army *army = new Army (*a, this);
  //Stack *s = c->addArmy(army);
  Stack *s = NULL;
  bool vectored = false;
  const Army *army = doCityProducesArmy(c, s, vectored);
  if (army)
    {
      debug ("created army id " << army->getId() << " in stack " << s->getId() << " of size " << s->size());
    }
  else
    {
      debug ("we got a null army! how?");
        int cost = c->getActiveProductionBase()->getProductionCost();
          if (cost > d_gold)
            {
              debug ("we can't afford it.");
            }
          else
            {
              debug ("we can afford it");
            }

    exit(0);
    }
  debug ("expecting it to be in stack id " << action->getStackId());
  Stack *expected = getStacklist()->getStackById(action->getStackId());
  debug ("expected stack is " << expected);
  if (expected)
    {
    debug ("expected stack has position " << expected->getPos().x << "," << expected->getPos().y);
    }
  assert (s != NULL);
  assert (s->getId() == action->getStackId());
  assert (s->getPos() == action->getDestination());
  assert (army->getId() == action->getArmyId());
}

void NetworkPlayer::decodeActionProduceVectored(const Action_ProduceVectored *action)
{
  //create a vectored unit.
  VectoredUnit v(action->getOrigination(), action->getDestination(),
		 action->getArmy(), 0, this);
  Stack *s = NULL;
  Army *army = doVectoredUnitArrives(&v, s);
  debug ("army is " << army);
  debug ("stack is " << s);
  assert (army != NULL);
  assert (s != NULL);
  //printf("vectored unit landed in stack %d, and we expected %d\n", s->getId(), action->getTargetStackId());
  //printf("new id of unit is %d, and we expected %d\n", army->getId(), action->getTargetArmyId());
  assert (army->getId() == action->getTargetArmyId());
  assert (s->getId() == action->getTargetStackId());
}

void NetworkPlayer::decodeActionDiplomacyState(const Action_DiplomacyState *action)
{
  Player *player = Playerlist::getInstance()->getPlayer(action->getOpponentId());
  doDeclareDiplomacy(action->getDiplomaticState(), player);
}

void NetworkPlayer::decodeActionDiplomacyProposal(const Action_DiplomacyProposal *action)
{
  Player *player = Playerlist::getInstance()->getPlayer(action->getOpponentId());
  doProposeDiplomacy(action->getDiplomaticProposal(), player);
}

void NetworkPlayer::decodeActionDiplomacyScore(const Action_DiplomacyScore *action)
{
  (void) action;
  Player *player = Playerlist::getInstance()->getPlayer(action->getOpponentId());
  alterDiplomaticRelationshipScore(player, action->getAmountChange());
}

void NetworkPlayer::decodeActionEndTurn(const Action_EndTurn *action)
{
  (void) action;
  debug ("ending turn!!");
  ending_turn.emit();
}

void NetworkPlayer::decodeActionConquerCity(const Action_ConquerCity *action)
{
  doConquerCity(Citylist::getInstance()->getById(action->getCityId()));
}

void NetworkPlayer::decodeActionRecruitHero(const Action_RecruitHero *action)
{
  City *city = Citylist::getInstance()->getById(action->getCityId());
  ArmyProto *ally = 0;
  if (action->getNumAllies())
    ally = Armysetlist::getInstance()->getArmy(getArmyset(),
                                               action->getAllyArmyType());
  StackReflist *stacks = new StackReflist();
  Hero *hero = doRecruitHero(action->getHero(), city, action->getCost(), 
			     action->getNumAllies(), ally, stacks);
  if (hero)
    {
      debug ("created hero with id " << hero->getId() << ", in stack " << d_stacklist->getArmyStackById(hero->getId())->getId());
    }

  if (stacks->size())
    supdatingStack.emit(stacks->front()); // make sure we get a redraw
  delete stacks;
}

void NetworkPlayer::decodeActionCityTooPoorToProduce(const Action_CityTooPoorToProduce *action)
{
  (void) action;
  //this action is only used for reporting purposes.
}

void NetworkPlayer::decodeActionInitTurn(const Action_InitTurn*action)
{
  (void) action;
  debug ("remote: dumping " << d_actions.size() << " actions");
  for (std::list<Action*>::iterator i = d_actions.begin(); i != d_actions.end(); ++i)
    {
      debug ("\t" << Action::actionTypeToString((*i)->getType()) << " " << (*i)->dump().c_str());
    }

  GameActionlist::getInstance()->add(new TurnActionlist(this, d_actions));
  clearActionlist();
}

void NetworkPlayer::decodeActionLoot (const Action_Loot *action)
{
  guint32 player_id = action->getLootedPlayerId();
  Player *looted = Playerlist::getInstance()->getPlayer(player_id);
  doLootCity(looted, action->getAmountToAdd(), action->getAmountToSubtract());
}

void NetworkPlayer::decodeActionUseItem(const Action_UseItem *action)
{
  Stack *stack = d_stacklist->getArmyStackById(action->getHeroId());
  if (stack == NULL)
    {
      debug ("couldn't find hero with id " << action->getHeroId());
    }
  assert (stack != NULL);
  Hero *hero = dynamic_cast<Hero *>(stack->getArmyById(action->getHeroId()));
  assert (hero != NULL);
  Item *item = hero->getBackpack()->getItemById(action->getItemId());
  assert (item != NULL);
  Player *victim = Playerlist::getInstance()->getPlayer(action->getVictimPlayerId());
  City *friendly_city = 
    Citylist::getInstance()->getById(action->getFriendlyCityId());
  City *enemy_city = 
    Citylist::getInstance()->getById(action->getEnemyCityId());
  City *neutral_city = 
    Citylist::getInstance()->getById(action->getNeutralCityId());
  City *city = 
    Citylist::getInstance()->getById(action->getCityId());
  doHeroUseItem(hero, item, victim, friendly_city, enemy_city, neutral_city, 
                city);
}
      
void NetworkPlayer::decodeActionStackOrder(const Action_ReorderArmies* action)
{
  //sort the buggers.
  Player *p = Playerlist::getInstance()->getPlayer(action->getPlayerId());
  if (!p)
    {
      debug ("we don't have player id " << action->getPlayerId());
      exit(0);
    }
  Stack *s = p->getStacklist()->getStackById(action->getStackId());
  if (!s)
    {
      debug ("we don't have stack id %d" <<  action->getStackId());
      exit(0);
    }
  assert (action->getArmyIds().size() == s->size());
  std::list<guint32> ids = action->getArmyIds();
  bool success = true;
  for (std::list<guint32>::iterator i = ids.begin(); i != ids.end(); ++i)
    {
      if (s->getArmyById(*i) == NULL)
        {
          debug ("stack " << s->getId() << " does not have army id " << *i);
          success = false;
        }
    }
  if (!success)
    {
      exit(0);
    }
  printf("started out with this ordering: ");
  for (Stack::iterator i = s->begin(); i != s->end(); ++i)
    printf ("%d ", (*i)->getId());
  printf("\n");
  printf("we say order like: ");
  for (std::list<guint32>::iterator i = ids.begin(); i != ids.end(); ++i)
    printf ("%d ", *i);
  printf("\n");
  doStackSort(s, ids);
  printf("changed it to: ");
  for (Stack::iterator i = s->begin(); i != s->end(); ++i)
    printf ("%d ", (*i)->getId());
  printf("\n");
}

void NetworkPlayer::decodeActionStacksReset(const Action_ResetStacks *action)
{
  (void) action;
  Player *p = Playerlist::getInstance()->getPlayer(action->getPlayerId());
  if (!p)
    {
      debug ("couldn't find player " << action->getPlayerId());
      exit(0);
    }
  if (p->getId() != getId())
    {
      debug ("can't heal another player's stacks?");
      exit(0);
    }
  doStacksReset();
}

void NetworkPlayer::decodeActionRuinsReset(const Action_ResetRuins *action)
{
  (void) action;
  doRuinsReset();
}

void NetworkPlayer::decodeActionCollectTaxesAndPayUpkeep(const Action_CollectTaxesAndPayUpkeep *action)
{
  (void) action;
  doCollectTaxesAndPayUpkeep();
}

void NetworkPlayer::decodeActionKillPlayer(const Action_Kill *action)
{
  (void) action;
  if (isDead() == false)
    {
      doKill();
      Playerlist::getInstance()->splayerDead.emit(this);
    }
}

void NetworkPlayer::decodeActionDefendStack(const Action_DefendStack *action)
{
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  if (!stack)
    {
      debug ("couldn't find stack id " << action->getStackId());
      exit(0);
    }
  doStackDefend(stack);
}

void NetworkPlayer::decodeActionUndefendStack(const Action_UndefendStack *action)
{
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  if (!stack)
    {
      debug ("couldn't find stack id " << action->getStackId());
      exit(0);
    }
  doStackUndefend(stack);
}

void NetworkPlayer::decodeActionParkStack(const Action_ParkStack *action)
{
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  if (!stack)
    {
      debug ("couldn't find stack id " << action->getStackId());
      exit(0);
    }
  doStackPark(stack);
}

void NetworkPlayer::decodeActionUnparkStack(const Action_UnparkStack *action)
{
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  if (!stack)
    {
      debug ("couldn't find stack id " << action->getStackId());
      exit(0);
    }
  doStackUnpark(stack);
}

void NetworkPlayer::decodeActionSelectStack(const Action_SelectStack *action)
{
  Stack *stack = d_stacklist->getStackById(action->getStackId());
  if (!stack)
    {
      debug ("couldn't find stack id " << action->getStackId());
      exit(0);
    }
  doStackSelect(stack);
  supdatingStack.emit(stack);
}

void NetworkPlayer::decodeActionDeselectStack(const Action_DeselectStack *action)
{
  (void) action;
  doStackDeselect();
  supdatingStack.emit(0);
}
// End of file
