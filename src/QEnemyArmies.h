// Copyright (C) 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2004 Andrea Paternesi
// Copyright (C) 2007, 2008, 2009, 2014, 2021 Ben Asselstine
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
#ifndef QUEST_ENEMY_ARMIES_H
#define QUEST_ENEMY_ARMIES_H

#include <sigc++/trackable.h>

#include <list>
#include "Quest.h"

class Army;
class Player;

//! A Quest to kill a certain number of another Player's Army objects.
/**
 * A hero that receives this quest has to kill a number of armies.  The Quest 
 * is completed when this happens, or the quest is expired if enemy Player 
 * dies.
 */
class QuestEnemyArmies : public Quest, public sigc::trackable
{
public:
    //! Default constructor.
    /**
     * Make a new kill-armies quest.
     *
     * @param q_mgr  The quests manager to associate this quest with.
     * @param hero   The Id of the Hero who is responsible for the quest.
     */
    QuestEnemyArmies(QuestsManager& q_mgr, guint32 hero);

    //! Copy constructor.
    QuestEnemyArmies (const QuestEnemyArmies &q);

    // Construct from remote action.
    QuestEnemyArmies(QuestsManager& q_mgr, guint32 hero,
		     guint32 armies_to_kill, guint32 victim_player);

    //! Destructor.
    ~QuestEnemyArmies() {};

    //! Loading constructor.
    /**
     * @param q_mgr   The quests manager to associate this quest with.
     * @param helper  The opened saved-game file to load this quest from.
     */
    QuestEnemyArmies(QuestsManager& q_mgr, XML_Helper* helper);


    // Get Methods

    //! Return a description of how many armies have been killed so far.
    Glib::ustring getProgress() const;

    //! Return a queue of strings to show when the quest is compeleted.
    void getSuccessMsg(std::queue<Glib::ustring>& msgs) const;

    //! Return a queue of strings to show when the quest has expired.
    void getExpiredMsg(std::queue<Glib::ustring>& msgs) const;

    //! Returns the number of Army objects to be killed in this Quest.
    guint32 getArmiesToKill() {return d_to_kill;}

    //! Returns the enemy player whose Army objects are to be killed.
    guint32 getVictimPlayerId();

    // Methods that operate on the class data but do not modify the class.

    //! Saves the kill-armies quest data to an opened saved-game file.
    bool save(XML_Helper* helper) const;


    // Methods that need to be implemented from the superclass.

    //! Callback for when an Army object is killed.
    /**
     * This method is used to account for the number armies killed by the
     * Hero.
     *
     * @param army           A pointer to the Army object that has been
     *                       killed.
     * @param heroIsCulprit  Whether or not the Hero object responsible for
     *                       this Quest was involved with the killing of
     *                       the given Army object.
     */
    void armyDied(Army *army, bool heroIsCulprit);

    //! Callback for when a City is defeated.
    /**
     * @note This method is not used.
     */
    void cityAction(City *city, CityDefeatedAction action, 
		    bool heroIsCulprit, int gold);


    // Static Methods

    //! Returns whether or not this quest is impossible.
    /**
     * Scans all Player objects in the Playerlist to see if there is one 
     * that is alive that isn't the neutral player.
     *
     * @note This method is static because it is executed before the
     *       Quest is instantiated.  It is also called from within the
     *       instantiated Quest.
     *
     * @param heroId  The Id of the Hero responsible for the kill-armies 
     *                quest.
     *
     * @return Whether or not the quest is possible.
     */
    static bool isFeasible(guint32 heroId);

private:

    //! Generate a description of the Quest.
    void initDescription();

    //! Recalculate the positions on the map of the target Army objects.
    void update_targets();

    //! The number of Army objects the Hero must kill to succeed.
    guint32 d_to_kill;

    //! The number of Army objects the Hero has already killed.
    guint32 d_killed;

    //! The victim player who the Hero is targeting Army objects of.
    guint32 d_victim_player_id;
};

Player* getVictimPlayer(Player *p);

#endif
