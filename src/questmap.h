//  Copyright (C) 2007, 2008, 2009, 2012, 2014 Ben Asselstine
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
#ifndef QUESTMAP_H
#define QUESTMAP_H

#include <sigc++/signal.h>

#include "overviewmap.h"
#include "vector.h"

class Player;
class Quest;

//! Draw a Quest objective onto a miniature map graphic.
/** 
 * This is a map where you can depict a Quest.
 * The depiction is different for each kind of Quest (Quest::Type).
 *
 * @note This class is also used in a special case to depict the quest 
 * completion when the reward is a hidden ruin (Reward::RUIN).
 */
class QuestMap : public OverviewMap
{
 public:

     //! Default constructor.  Make a new QuestMap.
     /**
      * @param quest  The quest to depict on the miniature map graphic.
      */
    QuestMap(Quest *quest);

    //! Destructor.
    ~QuestMap() {};

    // Set Methods

    //! Point to another position on the miniature map graphic.
    /**
     * @note This is used to point to a hidden map Reward after a Quest has
     * been completed.
     */
    void set_target(Vector<int>target){ d_target = target;}


    // Signals

    //! Emitted when the quest is finished being drawn on the map surface.
    /**
     * Classes that use QuestMap must catch this signal to display the map.
     */
    sigc::signal<void, Cairo::RefPtr<Cairo::Surface> > map_changed;
    
 private:

    //! Draw the given positions on the map in the color of the given player.
    void draw_stacks(Player *p, std::list< Vector<int> > targets);

    //! Draw a line to a boxed target.
    void draw_target(Vector<int> start, Vector<int> target);

    //! Draw a box around a target.
    void draw_target();
    
    //! Draw the Quest onto the miniature map graphic.
    /**
     * This method is automatically called by the QuestMap::draw method.
     * Either draws the given stacks, a line to a target with a target, or a 
     * target, or nothing at all depending on the kind of Quest.
     */
    virtual void after_draw();

    // DATA

    //! The Quest to depict on the miniature map graphic.
    Quest *quest;

    //! The new position to point to on the miniature map graphic.
    Vector<int> d_target;

};

#endif
