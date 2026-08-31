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
#ifndef SHIELDSET_UNDO_H
#define SHIELDSET_UNDO_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "defs.h"
#include "undo-action.h"
#include "file.h"
#include "shield-set.h"

class Shieldset;

//! A record of an event in the shieldset editor
/**
 * The purpose of these classes is to implement undo/redo in the shieldset
 * editor.
 */

class ShieldSetUndoAction: public UndoAction
{
public:

    //! A ShieldSet Undo Action can be one of the following kinds.
    enum Type
      {
        /** Modify a player's shield colors. */
        CHANGE_COLORS = 1,
        /** Modify description/copyright/license. */
        CHANGE_PROPERTIES,
        /** Modify the non white shields to be the same as white. */
        COPY_WHITE_DOWN,
        /** A shield or tartan image has been set */
        ADD_IMAGE,
        /** A shield or tartan image has been cleared*/
        CLEAR_IMAGE,
      };

    //! Default constructor.
    ShieldSetUndoAction (Type type,
                         UndoAction::AggregateType a =
                         UndoAction::AGGREGATE_NONE)
      : UndoAction (a), m_type (type)
      {
      }

    Type get_type() const
      {
        return m_type;
      }

protected:

    Type m_type;
};

class ShieldSetUndoAction_ShieldIndex: public ShieldSetUndoAction
{
public:
    ShieldSetUndoAction_ShieldIndex (Type t, guint32 i, bool agg = false)
      : ShieldSetUndoAction (t, agg ? UndoAction::AGGREGATE_DELAY :
                             UndoAction::AGGREGATE_NONE), m_index (i)
      {
      }

    ~ShieldSetUndoAction_ShieldIndex ()
      {
      }

    guint32 get_index () const
      {
        return m_index;
      }
private:
    guint32 m_index;
};
//-----------------------------------------------------------------------------

//! A record of the player's colors changing in the shieldset editor.
/**
 * The purpose of the ShieldSetUndoAction_Color class is to record
 * when a player's colors have been modified.
 */
class ShieldSetUndoAction_Colors: public ShieldSetUndoAction_ShieldIndex
{
public:
    //! Make a new change color action
    /**
     * Populate the change color action with the player id and the
     * set of colors.
     */
    ShieldSetUndoAction_Colors (guint32 id, std::vector<Gdk::RGBA> c)
      : ShieldSetUndoAction_ShieldIndex (CHANGE_COLORS, id),
      m_colors (c)
      {
      }

    //! Destroy a change color action.
    ~ShieldSetUndoAction_Colors ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Color";
      }

    std::vector<Gdk::RGBA> get_colors () const
      {
        return m_colors;
      }

private:
    std::vector<Gdk::RGBA> m_colors;
};

//-----------------------------------------------------------------------------

//! A record of the shieldset's properties changing in the editor.
/**
 * The purpose of the ShieldSetUndoAction_Properties class is to record
 * when a shieldset's name, description, copyright and license have changed.
 */
class ShieldSetUndoAction_Properties: public ShieldSetUndoAction
{
public:
    //! Make a new change properties action
    /**
     * Populate the properties action with the new name, description,
     * copyright, and license text
     */
    ShieldSetUndoAction_Properties (Glib::ustring n, Glib::ustring d,
                                    Glib::ustring c, Glib::ustring l)
      :ShieldSetUndoAction (ShieldSetUndoAction::CHANGE_PROPERTIES),
      m_name (n), m_desc (d), m_copyright (c), m_license (l)
      {
      }

    //! Destroy a change properties action.
    ~ShieldSetUndoAction_Properties ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Properties";
      }

    Glib::ustring get_name () const
      {
        return m_name;
      }

    Glib::ustring get_description () const
      {
        return m_desc;
      }

    Glib::ustring get_copyright () const
      {
        return m_copyright;
      }

    Glib::ustring get_license () const
      {
        return m_license;
      }

private:
    Glib::ustring m_name;
    Glib::ustring m_desc;
    Glib::ustring m_copyright;
    Glib::ustring m_license;
};

//-----------------------------------------------------------------------------

//! A helper class for events that require saving the whole shieldset

/**
 * Several actions require saving the whole tar file because it's the
 * easiest way to implement undo/redo.
 */
class ShieldSetUndoAction_Save: public ShieldSetUndoAction
{
public:
    ShieldSetUndoAction_Save (Shieldset *s, Type t)
      :ShieldSetUndoAction (t)
      {
        m_shieldset = new Shieldset (*s);

        m_filename = File::get_tmp_file () + SHIELDSET_EXT;
        s->save (m_filename, SHIELDSET_EXT);
      }

    ~ShieldSetUndoAction_Save ()
      {
        File::erase (m_filename);
        delete m_shieldset;
      }

    Glib::ustring get_shieldset_filename () const
      {
        return m_filename;
      }

    Shieldset *get_shieldset () const
      {
        return m_shieldset;
      }
private:
    Glib::ustring m_filename;
    Shieldset *m_shieldset;
};

//-----------------------------------------------------------------------------

//! A record of the shieldset's shields changing in the editor en masse.
/**
 * The purpose of the ShieldSetUndoAction_WhiteDown class is to record
 * when a shieldset's white shields are copied down to the other shields.
 *
 * We take a copy of the whole shieldset to get all of the images in one
 * go.  Our copy is a file on disk and is deleted when this class is
 * destroyed.
 */
class ShieldSetUndoAction_WhiteDown: public ShieldSetUndoAction_Save
{
public:
    //! Make a new copy white shields down action
    /**
     * Populate the white down action with the shieldset.
     */
    ShieldSetUndoAction_WhiteDown (Shieldset *s)
      :ShieldSetUndoAction_Save (s, ShieldSetUndoAction::COPY_WHITE_DOWN)
      {
      }

    //! Destroy a white down action, and delete the file.
    ~ShieldSetUndoAction_WhiteDown ()
      {
      }

    Glib::ustring getActionName () const
      {
        return "Copy White Shields";
      }

};

//-----------------------------------------------------------------------------

//! A record of a shieldset image being added or replaced
/**
 * The purpose of the ShieldSetUndoAction_AddImage class is to record
 * when we select a new file.
 *
 * We take a copy of the whole shieldset.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class ShieldSetUndoAction_AddImage: public ShieldSetUndoAction_Save
{
public:
    //! Make a new add-file action
    /**
     * Populate the add image action with the shieldset.
     */
    ShieldSetUndoAction_AddImage (Shieldset *s)
      :ShieldSetUndoAction_Save (s, ShieldSetUndoAction::ADD_IMAGE)
      {
      }

    //! Destroy an add-image action, and delete the file.
    ~ShieldSetUndoAction_AddImage ()
      {
      }

    Glib::ustring getActionName () const
      {
        return "Add Image";
      }
};

//-----------------------------------------------------------------------------

//! A record of a shieldset image being cleared
/**
 * The purpose of the ShieldSetUndoAction_ClearImage class is to record
 * when we disassociate an image file with a shield or tartan.
 *
 * We take a copy of the whole shieldset.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class ShieldSetUndoAction_ClearImage: public ShieldSetUndoAction_Save
{
public:
    //! Make a new clear-file action
    /**
     * Populate the clear image action with the shieldset.
     */
    ShieldSetUndoAction_ClearImage (Shieldset *s)
      :ShieldSetUndoAction_Save (s, ShieldSetUndoAction::CLEAR_IMAGE)
      {
      }

    //! Destroy an clear-image action, and delete the file.
    ~ShieldSetUndoAction_ClearImage ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Clear Image";
      }
};
#endif
