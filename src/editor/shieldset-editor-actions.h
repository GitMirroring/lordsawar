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
#ifndef SHIELDSET_EDITOR_ACTIONS_H
#define SHIELDSET_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "defs.h"
#include "undo-action.h"

class Shieldset;

//! A record of an event in the shieldset editor
/** 
 * The purpose of these classes is to implement undo/redo in the shieldset
 * editor.
 */

class ShieldSetEditorAction: public UndoAction
{
    public:

	//! A ShieldSet Editor Action can be one of the following kinds.
        enum Type {
	        /** Modify a player's shield colour. */
                CHANGE_COLOR = 1,
	        /** Modify description/copyright/license. */
                CHANGE_PROPERTIES = 2,
	        /** Modify the non white shields to be the same as white. */
                COPY_WHITE_DOWN = 3,
                /** A shield or tartan image has been set */
                ADD_IMAGE = 4,
                /** A shield or tartan image has been cleared*/
                CLEAR_IMAGE = 5,
        };

	//! Default constructor.
        ShieldSetEditorAction(Type type,
                              UndoAction::AggregateType a = UndoAction::AGGREGATE_NONE)
          : UndoAction (a), d_type(type) {}

        //! Returns the Action::Type for this action.
        Type getType() const {return d_type;}

    protected:

        Type d_type;
};

//-----------------------------------------------------------------------------

//! A record of the player's color changing in the shieldset editor.
/**
 * The purpose of the ShieldSetEditorAction_Color class is to record
 * when a player's color has been modified.
 */
class ShieldSetEditorAction_Color: public ShieldSetEditorAction
{
    public:
	//! Make a new change color action
	/**
         * Populate the change color action with the player id and a color.
         */
        ShieldSetEditorAction_Color (guint32 id, Gdk::RGBA c)
          : ShieldSetEditorAction(ShieldSetEditorAction::CHANGE_COLOR),
          d_player_id (id), d_color (c) {}
	//! Destroy a change color action.
        ~ShieldSetEditorAction_Color () {}

        Glib::ustring getActionName () const {return _("Color");}

        guint32 getPlayerId () const {return d_player_id;}
        Gdk::RGBA getColor () const {return d_color;}

    private:
        guint32 d_player_id;
        Gdk::RGBA d_color;
};

//-----------------------------------------------------------------------------

//! A record of the shieldset's properties changing in the editor.
/**
 * The purpose of the ShieldSetEditorAction_Properties class is to record
 * when a shieldset's name, description, copyright and license have changed.
 */
class ShieldSetEditorAction_Properties: public ShieldSetEditorAction
{
    public:
	//! Make a new change properties action
	/**
         * Populate the properties action with the new name, description,
         * copyright, license text, and shield image dimensions:
         * small medium large, and width and heights.
         */
        ShieldSetEditorAction_Properties (Glib::ustring n, Glib::ustring d, Glib::ustring c, Glib::ustring l, guint32 sw, guint32 sh, guint32 mw, guint32 mh, guint32 lw, guint32 lh)
          :ShieldSetEditorAction(ShieldSetEditorAction::CHANGE_PROPERTIES),
          d_name (n), d_desc (d), d_copyright (c), d_license (l),
          d_small_width (sw), d_small_height (sh), d_medium_width (mw),
          d_medium_height (mh), d_large_width (lw), d_large_height (lh) {}
	//! Destroy a change properties action.
        ~ShieldSetEditorAction_Properties () {}

        Glib::ustring getActionName () const {return _("Properties");}

        Glib::ustring getName () {return d_name;}
        Glib::ustring getDescription () {return d_desc;}
        Glib::ustring getCopyright () {return d_copyright;}
        Glib::ustring getLicense () {return d_license;}

        guint32 getSmallWidth () {return d_small_width;}
        guint32 getSmallHeight () {return d_small_height;}
        guint32 getMediumWidth () {return d_medium_width;}
        guint32 getMediumHeight () {return d_medium_height;}
        guint32 getLargeWidth () {return d_large_width;}
        guint32 getLargeHeight  () {return d_large_height;}
    private:
        Glib::ustring d_name;
        Glib::ustring d_desc;
        Glib::ustring d_copyright;
        Glib::ustring d_license;
        guint32 d_small_width;
        guint32 d_small_height;
        guint32 d_medium_width;
        guint32 d_medium_height;
        guint32 d_large_width;
        guint32 d_large_height;
};

//-----------------------------------------------------------------------------

//! A helper class for events that require saving the whole shieldset

/**
 * Several actions require saving the whole tar file because it's the
 * easiest way to implement undo/redo.
 */
class ShieldSetEditorAction_Save: public ShieldSetEditorAction
{
    public:
        ShieldSetEditorAction_Save (Shieldset *s, Type t);
        ~ShieldSetEditorAction_Save ();

        Glib::ustring getShieldsetFilename () const {return d_filename;}
        Shieldset *getShieldset () const {return d_shieldset;}
    private:
        Glib::ustring d_filename;
        Shieldset *d_shieldset;
};

//-----------------------------------------------------------------------------

//! A record of the shieldset's shields changing in the editor en masse.
/**
 * The purpose of the ShieldSetEditorAction_WhiteDown class is to record
 * when a shieldset's white shields are copied down to the other shields.
 *
 * We take a copy of the whole shieldset to get all of the images in one
 * go.  Our copy is a file on disk and is deleted when this class is 
 * destroyed.
 */
class ShieldSetEditorAction_WhiteDown: public ShieldSetEditorAction_Save
{
    public:
	//! Make a new copy white shields down action
	/**
         * Populate the white down action with the shieldset.
         */
        ShieldSetEditorAction_WhiteDown (Shieldset *s)
          :ShieldSetEditorAction_Save(s,
                                      ShieldSetEditorAction::COPY_WHITE_DOWN){}
	//! Destroy a white down action, and delete the file.
        ~ShieldSetEditorAction_WhiteDown () {}

        Glib::ustring getActionName () const {return _("Copy White Shields");}

};

//-----------------------------------------------------------------------------

//! A record of a shieldset image being added or replaced
/**
 * The purpose of the ShieldSetEditorAction_AddImage class is to record
 * when we select a new file.
 *
 * We take a copy of the whole shieldset.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class ShieldSetEditorAction_AddImage: public ShieldSetEditorAction_Save
{
    public:
	//! Make a new add-file action
	/**
         * Populate the add image action with the shieldset.
         */
        ShieldSetEditorAction_AddImage (Shieldset *s)
          :ShieldSetEditorAction_Save(s, ShieldSetEditorAction::ADD_IMAGE) {}
	//! Destroy an add-image action, and delete the file.
        ~ShieldSetEditorAction_AddImage () {}

        Glib::ustring getActionName () const {return _("Add Image");}
};

//-----------------------------------------------------------------------------

//! A record of a shieldset image being cleared
/**
 * The purpose of the ShieldSetEditorAction_ClearImage class is to record
 * when we disassociate an image file with a shield or tartan.
 *
 * We take a copy of the whole shieldset.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class ShieldSetEditorAction_ClearImage: public ShieldSetEditorAction_Save
{
    public:
	//! Make a new clear-file action
	/**
         * Populate the clear image action with the shieldset.
         */
        ShieldSetEditorAction_ClearImage (Shieldset *s)
          :ShieldSetEditorAction_Save(s, ShieldSetEditorAction::CLEAR_IMAGE){}
	//! Destroy an clear-image action, and delete the file.
        ~ShieldSetEditorAction_ClearImage () {}

        Glib::ustring getActionName () const {return _("Clear Image");}
};
#endif //SHIELDSET_EDITOR_ACTIONS_H
