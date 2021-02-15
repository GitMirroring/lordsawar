// Copyright (C) 2008, 2009, 2010, 2011, 2014, 2021 Ben Asselstine
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

#pragma once
#ifndef NETWORK_PLAYER_H
#define NETWORK_PLAYER_H

#include <list>
#include <gtkmm.h>

#include "player.h"

class XML_Helper;
class Action;
class Action_Move;
class Action_Split;
class Action_Fight;
class Action_Join;
class Action_Ruin;
class Action_Temple;
class Action_Occupy;
class Action_Pillage;
class Action_Sack;
class Action_Raze;
class Action_Upgrade;
class Action_Buy;
class Action_Production;
class Action_Reward;
class Action_Quest;
class Action_Equip;
class Action_Level;
class Action_Disband;
class Action_ModifySignpost;
class Action_RenameCity;
class Action_Vector;
class Action_FightOrder;
class Action_Resign;
class Action_Plant;
class Action_Produce;
class Action_ProduceVectored;
class Action_DiplomacyState;
class Action_DiplomacyProposal;
class Action_DiplomacyScore;
class Action_EndTurn;
class Action_ConquerCity;
class Action_RecruitHero;
class Action_RenamePlayer;
class Action_CityTooPoorToProduce;
class Action_InitTurn;
class Action_Loot;
class Action_UseItem;
class Action_ReorderArmies;
class Action_ResetStacks;
class Action_ResetRuins;
class Action_CollectTaxesAndPayUpkeep;
class Action_Kill;
class Action_DefendStack;
class Action_UndefendStack;
class Action_ParkStack;
class Action_UnparkStack;
class Action_SelectStack;
class Action_DeselectStack;


//! A Player that is driven by Action objects coming over the NetworkConnection.
class NetworkPlayer : public Player
{
    public:
        // CREATORS
        NetworkPlayer(Glib::ustring name, guint32 armyset,
                      std::vector<Gdk::RGBA> colors, int width, int height,
                   Player::Type type = Player::HUMAN, int player_no = -1);
        NetworkPlayer(const Player&, bool sync_ids = false);
        NetworkPlayer(XML_Helper* helper);
        ~NetworkPlayer() {};

        //! Saves the data
        virtual bool save(XML_Helper* helper) const;

	virtual bool isComputer() const {return false;};
        
        //! Actions, see player.h for explanation
	virtual void abortTurn();
        virtual bool startTurn();
        virtual void endTurn();
        virtual void invadeCity(City* c);
        virtual bool chooseHero(HeroProto *hero, City* c, int gold);

        virtual Reward *chooseReward(Ruin *ruin, Sage *sage, Stack *stack);
        virtual void heroGainsLevel(Hero * a);
	virtual bool chooseTreachery (Stack *stack, Player *player, Vector <int> pos);
        virtual Army::Stat chooseStat(Hero *hero);
        virtual bool chooseQuest(Hero *hero);
        virtual bool computerChooseVisitRuin(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool computerChoosePickupBag(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool computerChooseVisitTempleForBlessing(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool computerChooseVisitTempleForQuest(Stack *stack, Vector<int> dest, guint32 moves, guint32 turns);
        virtual bool computerChooseContinueQuest(Stack *stack, Quest *quest, Vector<int> dest, guint32 moves, guint32 turns);

	void decodeAction(const Action *action);
	void decodeActions(std::list<Action *> actions);
        
	bool isConnected() const {return d_connected;}
	void setConnected(bool connected) {d_connected = connected;}
    private:
	bool d_connected;
	bool d_abort_requested;
	void decodeActionMove(const Action_Move *action);
	void decodeActionSplit(const Action_Split *action);
	void decodeActionFight(const Action_Fight *action);
	void decodeActionJoin(const Action_Join *action);
	void decodeActionRuin(const Action_Ruin *action);
	void decodeActionTemple(const Action_Temple *action);
	void decodeActionOccupy(const Action_Occupy *action);
	void decodeActionPillage(const Action_Pillage *action);
	void decodeActionSack(const Action_Sack *action);
	void decodeActionRaze(const Action_Raze *action);
	void decodeActionBuy(const Action_Buy *action);
	void decodeActionProduction(const Action_Production *action);
	void decodeActionReward(const Action_Reward *action);
	void decodeActionQuest(const Action_Quest *action);
	void decodeActionEquip(const Action_Equip *action);
	void decodeActionLevel(const Action_Level *action);
	void decodeActionDisband(const Action_Disband *action);
	void decodeActionModifySignpost(const Action_ModifySignpost *action);
	void decodeActionRenameCity(const Action_RenameCity *action);
	void decodeActionVector(const Action_Vector *action);
	void decodeActionFightOrder(const Action_FightOrder *action);
	void decodeActionResign(const Action_Resign *action);
	void decodeActionPlant(const Action_Plant *action);
	void decodeActionProduce(const Action_Produce *action);
	void decodeActionProduceVectored(const Action_ProduceVectored *action);
	void decodeActionDiplomacyState(const Action_DiplomacyState *action);
	void decodeActionDiplomacyProposal(const Action_DiplomacyProposal *action);
	void decodeActionDiplomacyScore(const Action_DiplomacyScore *action);
	void decodeActionEndTurn(const Action_EndTurn *action);
	void decodeActionConquerCity(const Action_ConquerCity *action);
	void decodeActionRecruitHero(const Action_RecruitHero *action);
	void decodeActionCityTooPoorToProduce(const Action_CityTooPoorToProduce *action);
	void decodeActionInitTurn(const Action_InitTurn*action);
	void decodeActionLoot(const Action_Loot*action);
	void decodeActionUseItem(const Action_UseItem*action);
        void decodeActionStackOrder(const Action_ReorderArmies* action);
        void decodeActionStacksReset(const Action_ResetStacks *action);
        void decodeActionRuinsReset(const Action_ResetRuins *action);
        void decodeActionCollectTaxesAndPayUpkeep(const Action_CollectTaxesAndPayUpkeep *action);
        void decodeActionKillPlayer(const Action_Kill *action);
        void decodeActionDefendStack(const Action_DefendStack *action);
        void decodeActionUndefendStack(const Action_UndefendStack *action);
        void decodeActionParkStack(const Action_ParkStack *action);
        void decodeActionUnparkStack(const Action_UnparkStack *action);
        void decodeActionSelectStack(const Action_SelectStack *action);
        void decodeActionDeselectStack(const Action_DeselectStack *action);

};

Stack *findStackById(guint32 id);
#endif // NETWORK_PLAYER_H

// End of file
