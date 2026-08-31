//  Copyright (C) 2021, 2026 Ben Asselstine
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
        UndoAction (AggregateType a)
          : d_time (Glib::DateTime ()), d_aggregate (a)
          {
            d_time = Glib::DateTime::create_now_local ();
          }

	//! Destructor.
        virtual ~UndoAction ()
          {
          }

        //! Get the name of this action for the undo/redo menuitem.
        virtual Glib::ustring get_action_name () const
          {
            return "";
          }

        void set_time (Glib::DateTime t)
          {
            d_time = t;
          }

        Glib::DateTime get_time () const
          {
            return d_time;
          }

        AggregateType get_aggregate () const
          {
            return d_aggregate;
          }

    private:
        Glib::DateTime d_time;
        AggregateType d_aggregate;
};
#endif
