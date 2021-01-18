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
#ifndef CITYSET_EDITOR_ACTIONS_H
#define CITYSET_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "defs.h"

class Cityset;

//! A record of an event in the cityset editor
/** 
 * The purpose of these classes is to implement undo/redo in the cityset
 * editor.
 */

class CitySetEditorAction
{
    public:

	//! An CitySet Editor Action can be one of the following kinds.
        enum Type {
	        /** Modify description/copyright/license. */
                CHANGE_PROPERTIES = 1,
                /** A city/temple/ruin image has been set */
                ADD_IMAGE = 2,
                /** An image has been cleared*/
                CLEAR_IMAGE = 3,
                /* Modify the number of squares a city occupies */
                CITY_TILE_WIDTH = 4,
                /* Modify the number of squares a ruin occupies */
                RUIN_TILE_WIDTH = 5,
                /* Modify the number of squares a temple occupies */
                TEMPLE_TILE_WIDTH = 6,
        };

	//! Default constructor.
        CitySetEditorAction(Type type);

	//! Destructor.
        virtual ~CitySetEditorAction() {}

        //! Get the name of this action for the undo/redo menuitem.
        virtual Glib::ustring getActionName () {return "";}

        //! Returns the Action::Type for this action.
        Type getType() const {return d_type;}

    protected:

        Type d_type;
};

//-----------------------------------------------------------------------------

//! A record of the cityset's properties changing in the editor.
/**
 * The purpose of the CitySetEditorAction_Properties class is to record
 * when a cityset's name, description, copyright, license, and tilesize have
 * changed.
 */
class CitySetEditorAction_Properties: public CitySetEditorAction
{
    public:
	//! Make a new change properties action
	/**
         * Populate the properties action with the new name, description,
         * copyright, license text, and tile size.
         */
        CitySetEditorAction_Properties (Glib::ustring n, Glib::ustring d, Glib::ustring c, Glib::ustring l, guint32 ts);
	//! Destroy a change properties action.
        ~CitySetEditorAction_Properties () {};

        Glib::ustring getActionName () {return _("Properties");}

        Glib::ustring getName () {return d_name;}
        Glib::ustring getDescription () {return d_desc;}
        Glib::ustring getCopyright () {return d_copyright;}
        Glib::ustring getLicense () {return d_license;}
        guint32 getTileSize () {return d_tile_size;}

    private:
        Glib::ustring d_name;
        Glib::ustring d_desc;
        Glib::ustring d_copyright;
        Glib::ustring d_license;
        guint32 d_tile_size;
};

//-----------------------------------------------------------------------------

//! A helper class for events that require saving the whole cityset

/**
 * Several actions require saving the whole tar file because it's the
 * easiest way to implement undo/redo.
 */
class CitySetEditorAction_Save: public CitySetEditorAction
{
    public:
        CitySetEditorAction_Save (Cityset *s, Type t);
        ~CitySetEditorAction_Save ();

        Glib::ustring getCitysetFilename () const {return d_filename;}
        Cityset *getCityset () const {return d_cityset;}
    private:
        Glib::ustring d_filename;
        Cityset *d_cityset;
};

//-----------------------------------------------------------------------------

//! A record of a cityset image being added or replaced
/**
 * The purpose of the CitySetEditorAction_AddImage class is to record
 * when we select a new file.
 *
 * We take a copy of the whole cityset.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class CitySetEditorAction_AddImage: public CitySetEditorAction_Save
{
    public:
	//! Make a new add-file action
	/**
         * Populate the add image action with the cityset.
         */
        CitySetEditorAction_AddImage (Cityset *s);
	//! Destroy an add-image action, and delete the file.
        ~CitySetEditorAction_AddImage () {};

        Glib::ustring getActionName () {return _("Add Image");}
};

//-----------------------------------------------------------------------------

//! A record of a cityset image being cleared
/**
 * The purpose of the CitySetEditorAction_ClearImage class is to record
 * when we disassociate an image file with a city, ruin, temple, port etc.
 *
 * We take a copy of the whole cityset.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class CitySetEditorAction_ClearImage: public CitySetEditorAction_Save
{
    public:
	//! Make a new clear-file action
	/**
         * Populate the clear image action with the cityset.
         */
        CitySetEditorAction_ClearImage (Cityset *s);
	//! Destroy an clear-image action, and delete the file.
        ~CitySetEditorAction_ClearImage () {};

        Glib::ustring getActionName () {return _("Clear Image");}
};

//-----------------------------------------------------------------------------

//! A record of the tile width for cities changing in the city editor.
/**
 * The purpose of the CitySetEditorAction_CityWidth class is to record
 * when the city width has been modified.
 */
class CitySetEditorAction_CityWidth: public CitySetEditorAction
{
    public:
	//! Make a new city width action
	/**
         * Populate the city width action with the width in tiles
         */
        CitySetEditorAction_CityWidth  (guint32 tiles);
	//! Destroy a city width action.
        ~CitySetEditorAction_CityWidth () {};

        Glib::ustring getActionName () {return _("City Width");}

        guint32 getCityWidth () const {return d_city_width;}

    private:
        guint32 d_city_width;
};

//-----------------------------------------------------------------------------

//! A record of the tile width for ruins changing in the city editor.
/**
 * The purpose of the CitySetEditorAction_RuinWidth class is to record
 * when the ruin width has been modified.
 */
class CitySetEditorAction_RuinWidth: public CitySetEditorAction
{
    public:
	//! Make a new ruin width action
	/**
         * Populate the ruin width action with the width in tiles
         */
        CitySetEditorAction_RuinWidth  (guint32 tiles);
	//! Destroy a ruin width action.
        ~CitySetEditorAction_RuinWidth () {};

        Glib::ustring getActionName () {return _("Ruin Width");}

        guint32 getRuinWidth () const {return d_ruin_width;}
    private:
        guint32 d_ruin_width;
};

//-----------------------------------------------------------------------------

//! A record of the tile width for temples changing in the city editor.
/**
 * The purpose of the CitySetEditorAction_TempleWidth class is to record
 * when the temple width has been modified.
 */
class CitySetEditorAction_TempleWidth: public CitySetEditorAction
{
    public:
	//! Make a new temple width action
	/**
         * Populate the temple width action with the width in tiles
         */
        CitySetEditorAction_TempleWidth  (guint32 tiles);
	//! Destroy a temple width action.
        ~CitySetEditorAction_TempleWidth () {};

        Glib::ustring getActionName () {return _("Temple  Width");}

        guint32 getTempleWidth () const {return d_temple_width;}

    private:
        guint32 d_temple_width;
};
#endif //CITYSET_EDITOR_ACTIONS_H
