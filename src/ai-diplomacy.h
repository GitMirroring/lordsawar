//  Copyright (C) 2007, 2008, 2014 Ben Asselstine
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
#ifndef AI_DIPLOMACY_H
#define AI_DIPLOMACY_H

#include <list>

class Player;

//! Artificial intelligence for managing diplomatic relations.
class AI_Diplomacy
{
    public:
        // Initializes the object 
        AI_Diplomacy(Player *owner);

        void considerCuspOfWar();
        
	void makeProposals();
	void needNewEnemy(Player *player);
        ~AI_Diplomacy();

    private:
        // the analysis currently in use
        static AI_Diplomacy *instance;

        void makeFriendsAndEnemies();
        void makeRequiredEnemies();
        void neutralsDwindlingNeedFirstEnemy();
        void gangUpOnTheBully();
       
        // DATA
        Player *d_owner;
	std::list<Player *> new_enemies;
};

#endif // AI_DIPLOMACY_H

// End of file
