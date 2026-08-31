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
#ifndef SIGNPOST_UNDO_ACTIONS_H
#define SIGNPOST_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include "undo-mgr.h"

//! A record of an event in the signpost editor
/**
 * The purpose of these classes is to implement undo/redo in the signpost
 * editor.
 */

class SignpostUndoAction: public UndoAction
{
public:

    //! A Signpost Editor Action can be just one kind.
    enum Type
      {
        MESSAGE = 1,
      };

    //! Default constructor.
    SignpostUndoAction (Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), m_type (type)
       {
       }

    //! Destructor.
    virtual ~SignpostUndoAction ()
      {
      }

    Type get_type () const
      {
        return m_type;
      }

protected:

    Type m_type;
};

//-----------------------------------------------------------------------------

//! A record of a signpost's text being changed
/**
 * The purpose of the SignpostUndoAction_Name class is to record
 * when we change the text changes.  This happens letter by letter.
 *
 */
class SignpostUndoAction_Message: public SignpostUndoAction, public UndoCursor
{
    public:
	//! Make a new message action
	/**
         * Populate the action with the sign's message.
         */
        SignpostUndoAction_Message (Glib::ustring m, UndoMgr *u,
                                    Gtk::TextView *v)
          : SignpostUndoAction (MESSAGE, true),
          UndoCursor (u->get_pos (v), v), m_message (m)
  {
  }

	//! Destroy a message action.
        ~SignpostUndoAction_Message ()
          {
          }

        Glib::ustring get_action_name () const
          {
            return "Message";
          }

        Glib::ustring get_message () const
          {
            return m_message;
          }

    private:
        Glib::ustring m_message;
};

#endif
