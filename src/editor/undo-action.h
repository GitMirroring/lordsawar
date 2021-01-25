// Copyright (C) 2021 Ben Asselstine
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
#ifndef UNDO_ACTION_H
#define UNDO_ACTION_H

#include <gtkmm.h>

//! A base class for undo/redo events
/** 
 * The idea here is that the various editors have all these actions that
 * need to be undone, and this class helps with naming and dating the events.
 * Also there are 'aggregated' actions that can be undone together in a block
 * through the Undo Manager.
 *
 * Aggregation happens by a series of same named actions followed by a blank,
 * or by a time delay.  The former is for things like terrain and the latter is
 * for things like spinboxes and text entries.
 */

class UndoAction
{
    public:
        enum AggregateType
          {
            AGGREGATE_NONE,
            AGGREGATE_BLANK, //aggregates end with a terminating blank
            AGGREGATE_DELAY, //aggregates end after a delay
          };

	//! Default constructor.
        UndoAction(AggregateType a);

	//! Destructor.
        virtual ~UndoAction() {}

        //! Get the name of this action for the undo/redo menuitem.
        virtual Glib::ustring getActionName () const {return "";}

        void setTime (Glib::TimeVal t) {d_time = t;}
        Glib::TimeVal getTime () const {return d_time;}

        AggregateType getAggregate () const {return d_aggregate;}

    private:
        Glib::TimeVal d_time;
        AggregateType d_aggregate;
};
#endif //UNDO_ACTION_H
