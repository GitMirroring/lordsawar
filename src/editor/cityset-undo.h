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
#ifndef CITYSET_UNDO_H
#define CITYSET_UNDO_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "defs.h"
#include "undo-action.h"
#include "city-set.h"

//! A record of an event in the cityset editor
/** 
 * The purpose of these classes is to implement undo/redo in the cityset
 * editor.
 */

class CitySetUndoAction: public UndoAction
{
public:

    //! A CitySet Undo Action can be one of the following kinds.
    enum Type
      {
        /** Modify description/copyright/license. */
        CHANGE_PROPERTIES = 1,
        /** A city/temple/ruin image has been set */
        ADD_IMAGE,
        /** An image has been cleared*/
        CLEAR_IMAGE,
        /* Modify the number of squares a city occupies */
        CITY_TILE_WIDTH,
        /* Modify the number of squares a ruin occupies */
        RUIN_TILE_WIDTH,
        /* Modify the number of squares a temple occupies */
        TEMPLE_TILE_WIDTH,
      };

    //! Default constructor.
    CitySetUndoAction (Type type, UndoAction::AggregateType aggregate =
                         UndoAction::AGGREGATE_NONE)
      : UndoAction (aggregate), d_type (type)
      {
      }

    Type getType () const
      {
        return d_type;
      }

protected:

    Type d_type;
};

//! A record of the cityset's properties changing in the editor.
/**
 * The purpose of the CitySetUndoAction_Properties class is to record
 * when a cityset's name, description, copyright, license, and tilesize have
 * changed.
 */
class CitySetUndoAction_Properties: public CitySetUndoAction
{
public:
    //! Make a new change properties action
    /**
     * Populate the properties action with the new name, description,
     * copyright, license text, and tile size.
     */
    CitySetUndoAction_Properties (Glib::ustring n, Glib::ustring d,
                                  Glib::ustring c, Glib::ustring l,
                                  guint32 ts)
      :CitySetUndoAction (CitySetUndoAction::CHANGE_PROPERTIES), d_name (n),
      d_desc (d), d_copyright (c), d_license (l), d_tile_size (ts)
  {
  }
    //! Destroy a change properties action.
    ~CitySetUndoAction_Properties ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Properties";
      }

    Glib::ustring get_name ()
      {
        return d_name;
      }

    Glib::ustring get_description ()
      {
        return d_desc;
      }

    Glib::ustring get_copyright ()
      {
        return d_copyright;
      }

    Glib::ustring get_license ()
      {
        return d_license;
      }

    guint32 get_tile_size ()
      {
        return d_tile_size;
      }

private:
    Glib::ustring d_name;
    Glib::ustring d_desc;
    Glib::ustring d_copyright;
    Glib::ustring d_license;
    guint32 d_tile_size;
};

//! A helper class for events that require saving the whole cityset
/**
 * Several actions require saving the whole tar file because it's the
 * easiest way to implement undo/redo.
 */
class CitySetUndoAction_Save: public CitySetUndoAction
{
public:
    CitySetUndoAction_Save (Cityset *c, Type t)
      :CitySetUndoAction (t)
      {
        d_cityset = new Cityset (*c);

        d_filename = File::get_tmp_file () + CITYSET_EXT;
        c->save (d_filename, CITYSET_EXT);
      }

    ~CitySetUndoAction_Save ()
      {
        File::erase (d_filename);
        delete d_cityset;
      }

    Glib::ustring get_cityset_filename () const
      {
        return d_filename;
      }

    Cityset *get_cityset () const
      {
        return d_cityset;
      }

private:
    Glib::ustring d_filename;
    Cityset *d_cityset;
};

//! A record of a cityset image being added or replaced
/**
 * The purpose of the CitySetUndoAction_AddImage class is to record
 * when we select a new file.
 *
 * We take a copy of the whole cityset.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class CitySetUndoAction_AddImage: public CitySetUndoAction_Save
{
public:
    //! Make a new add-file action
    /**
     * Populate the add image action with the cityset.
     */
    CitySetUndoAction_AddImage (Cityset *c)
      :CitySetUndoAction_Save (c, CitySetUndoAction::ADD_IMAGE)
      {
      }

    //! Destroy an add-image action, and delete the file.
    ~CitySetUndoAction_AddImage ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add Image";
      }
};

//! A record of a cityset image being cleared
/**
 * The purpose of the CitySetUndoAction_ClearImage class is to record
 * when we disassociate an image file with a city, ruin, temple, port etc.
 *
 * We take a copy of the whole cityset.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class CitySetUndoAction_ClearImage: public CitySetUndoAction_Save
{
public:
    //! Make a new clear-file action
    /**
     * Populate the clear image action with the cityset.
     */
    CitySetUndoAction_ClearImage (Cityset *c)
      :CitySetUndoAction_Save (c, CitySetUndoAction::CLEAR_IMAGE)
      {
      }

    //! Destroy an clear-image action, and delete the file.
    ~CitySetUndoAction_ClearImage ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Clear Image";
      }
};

//! A record of the tile width for cities changing in the city editor.
/**
 * The purpose of the CitySetUndoAction_CityWidth class is to record
 * when the city width has been modified.
 */
class CitySetUndoAction_CityWidth: public CitySetUndoAction
{
public:
    //! Make a new city width action
    /**
     * Populate the city width action with the width in tiles
     */
    CitySetUndoAction_CityWidth (guint32 tiles)
      :CitySetUndoAction (CitySetUndoAction::CITY_TILE_WIDTH,
                          UndoAction::AGGREGATE_DELAY), d_city_width (tiles)
  {
  }

    //! Destroy a city width action.
    ~CitySetUndoAction_CityWidth ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "City Width";
      }

    guint32 get_city_width () const
      {
        return d_city_width;
      }

private:
    guint32 d_city_width;
};

//! A record of the tile width for ruins changing in the city editor.
/**
 * The purpose of the CitySetUndoAction_RuinWidth class is to record
 * when the ruin width has been modified.
 */
class CitySetUndoAction_RuinWidth: public CitySetUndoAction
{
public:
    //! Make a new ruin width action
    /**
     * Populate the ruin width action with the width in tiles
     */
    CitySetUndoAction_RuinWidth (guint32 tiles)
      :CitySetUndoAction (CitySetUndoAction::RUIN_TILE_WIDTH,
                          UndoAction::AGGREGATE_DELAY), d_ruin_width (tiles)
  {
  }

    //! Destroy a ruin width action.
    ~CitySetUndoAction_RuinWidth ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Ruin Width";
      }

    guint32 get_ruin_width () const
      {
        return d_ruin_width;
      }
private:
    guint32 d_ruin_width;
};

//! A record of the tile width for temples changing in the city editor.
/**
 * The purpose of the CitySetUndoAction_TempleWidth class is to record
 * when the temple width has been modified.
 */
class CitySetUndoAction_TempleWidth: public CitySetUndoAction
{
public:
    //! Make a new temple width action
    /**
     * Populate the temple width action with the width in tiles
     */
    CitySetUndoAction_TempleWidth (guint32 tiles)
      :CitySetUndoAction (CitySetUndoAction::TEMPLE_TILE_WIDTH,
                          UndoAction::AGGREGATE_DELAY), d_temple_width (tiles)
  {
  }
    //! Destroy a temple width action.
    ~CitySetUndoAction_TempleWidth ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Temple Width";
      }

    guint32 get_temple_width () const
      {
        return d_temple_width;
      }

private:
    guint32 d_temple_width;
};
#endif
