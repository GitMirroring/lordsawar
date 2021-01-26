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
#ifndef SIGNPOST_EDITOR_ACTIONS_H
#define SIGNPOST_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "Tile.h"
#include "SmallTile.h"
#include "tilestyle.h"
#include "defs.h"
#include "undo-action.h"

//! A record of an event in the signpost editor
/** 
 * The purpose of these classes is to implement undo/redo in the signpost
 * editor.
 */

class SignpostEditorAction: public UndoAction
{
public:

    //! A Signpost Editor Action can be one of the following kinds.
    enum Type {
      MESSAGE = 1,
    };

    //! Default constructor.
    SignpostEditorAction(Type type, bool agg = false)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY :
                   UndoAction::AGGREGATE_NONE), d_type (type) {}

    //! Destructor.
    virtual ~SignpostEditorAction() {}

    //! Returns the Action::Type for this action.
    Type getType() const {return d_type;}

protected:

    Type d_type;
};

//-----------------------------------------------------------------------------

//! A record of a signpost's text being changed
/**
 * The purpose of the SignpostEditorAction_Name class is to record
 * when we change the text changes.  This happens letter by letter.
 *
 */
class SignpostEditorAction_Message: public SignpostEditorAction
{
    public:
	//! Make a new message action
	/**
         * Populate the action with the sign's message.
         */
        SignpostEditorAction_Message (Glib::ustring m)
          : SignpostEditorAction (MESSAGE, true), d_message (m) {}
	//! Destroy a message action.
        ~SignpostEditorAction_Message () {}

        Glib::ustring getActionName () const {return "Message";}

        Glib::ustring getMessage () {return d_message;}

    private:
        Glib::ustring d_message;
};

#endif //SIGNPOST_EDITOR_ACTIONS_H
