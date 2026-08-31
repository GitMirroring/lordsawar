// Copyright (C) 2009, 2014 Ben Asselstine
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
#ifndef COMMENTATOR_H
#define COMMENTATOR_H

#include <vector>
#include <gtkmm.h>

class Player;
//! A Commentator is the in-game commentator.
/** 
 * 
 * Periodically a message will be conveyed to human players at the start of 
 * their turn.  This Commentator object determines appropriate messages to
 * be conveyed.
 */

class Commentator
{
    public:

        //! Returns the singleton instance.
	static Commentator* getInstance();
        
        //! Explicitly deletes the singleton instance.
        static void deleteInstance();

        bool hasComment() const;

        std::vector<Glib::ustring> getComments(Player *player) const;

    protected:
	//! Creates a new Commentator from scratch.
        Commentator();

        //! Destructor.
        virtual ~Commentator() {};
    private:

        static Commentator* d_instance;
};

#endif //COMMENTATOR_H
