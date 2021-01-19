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
#ifndef EDITOR_ACTIONS_H
#define EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "defs.h"

//! A record of an event in the shieldset editor
/** 
 * The purpose of these classes is to implement undo/redo in the shieldset
 * editor.
 */

class Scenario;
class GameScenario;

class EditorAction
{
    public:

	//! An Editor Action can be one of the following kinds.
        enum Type {
	        /** Modify description/copyright/license. */
                CHANGE_PROPERTIES = 1,
                /** The scenario media has been modified */
                SCENARIO_MEDIA = 2,
        };

	//! Default constructor.
        EditorAction(Type type) : d_type (type) {}

	//! Destructor.
        virtual ~EditorAction() {}

        //! Get the name of this action for the undo/redo menuitem.
        virtual Glib::ustring getActionName () {return "";}

        //! Returns the Action::Type for this action.
        Type getType() const {return d_type;}

    protected:

        Type d_type;
};

//-----------------------------------------------------------------------------

//! A record of the shieldset's properties changing in the editor.
/**
 * The purpose of the EditorAction_Properties class is to record
 * when a map's name, description, copyright and license have changed.
 */
class EditorAction_Properties: public EditorAction
{
    public:
	//! Make a new change properties action
	/**
         * Populate the properties action with the new name, description,
         * copyright, and license text.
         */
        EditorAction_Properties (Glib::ustring n, Glib::ustring d, Glib::ustring c, Glib::ustring l)
          :EditorAction(EditorAction::CHANGE_PROPERTIES), d_name (n),
          d_desc (d), d_copyright (c), d_license (l) {}
	//! Destroy a change properties action.
        ~EditorAction_Properties () {}

        Glib::ustring getActionName () {return _("Properties");}

        Glib::ustring getName () {return d_name;}
        Glib::ustring getDescription () {return d_desc;}
        Glib::ustring getCopyright () {return d_copyright;}
        Glib::ustring getLicense () {return d_license;}

    private:
        Glib::ustring d_name;
        Glib::ustring d_desc;
        Glib::ustring d_copyright;
        Glib::ustring d_license;
};

//-----------------------------------------------------------------------------

//! A helper class for events that require saving the whole map

/**
 * Several actions require saving the whole tar file because it's the
 * easiest way to implement undo/redo.
 */
class EditorAction_Save: public EditorAction
{
    public:
        EditorAction_Save (Type t, GameScenario *g);
        ~EditorAction_Save ();

        Glib::ustring getScenarioFilename () const {return d_filename;}
        Scenario *getScenario () const {return d_scenario;}
    private:
        Glib::ustring d_filename;
        Scenario *d_scenario;
};

//-----------------------------------------------------------------------------

//! A record of the scenario media being modified
/**
 * The purpose of the EditorAction_ScenarioMedia class is to record
 * when an image or sound file in the scenario media is changed.
 *
 * We take a copy of the whole scenario.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class EditorAction_ScenarioMedia: public EditorAction_Save
{
    public:
	//! Make a new scenario media action
	/**
         * Populate the scenario media action with the game scenario.
         */
        EditorAction_ScenarioMedia (GameScenario *g)
          :EditorAction_Save(EditorAction::SCENARIO_MEDIA, g) {}
	//! Destroy an scenario media action, and delete the file.
        ~EditorAction_ScenarioMedia () {}

        Glib::ustring getActionName () {return _("Scenario Media");}
};
#endif //EDITOR_ACTIONS_H
