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
#ifndef MASKED_IMAGE_UNDO_ACTIONS_H
#define MASKED_IMAGE_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include <vector>
#include "pixmask.h"
#include "tar-file-masked-image.h"

//! A record of an event in the masked image editor
/** 
 * The purpose of these classes is to implement undo/redo in the tar-file
 * masked image editor.
 */

class MaskedImageUndoAction: public UndoAction
{
public:

    enum Type
      {
        SET = 1,
        SHIELD,
      };

    MaskedImageUndoAction(Type type)
      : UndoAction (UndoAction::AGGREGATE_NONE), m_type (type)
      {
      }

    virtual ~MaskedImageUndoAction ()
      {
      }

    Type get_type () const
      {
        return m_type;
      }

protected:
    Type m_type;
};

class MaskedImageUndoAction_Set: public MaskedImageUndoAction
{
public:
    MaskedImageUndoAction_Set (TarFileMaskedImage *i, Glib::ustring f)
      : MaskedImageUndoAction (SET),
      m_image (new TarFileMaskedImage (*i)), m_filename (f)
  {
  }
    ~MaskedImageUndoAction_Set ()
      {
        delete m_image;
      }

    Glib::ustring get_action_name () const
      {
        return "Set";
      }

    Glib::ustring get_file_name () const
      {
        return m_filename;
      }

    TarFileMaskedImage* get_image () const
      {
        return m_image;
      }
private:
    TarFileMaskedImage *m_image;
    Glib::ustring m_filename;
};

class MaskedImageUndoAction_Shield: public MaskedImageUndoAction
{
    public:
        MaskedImageUndoAction_Shield (int s)
          : MaskedImageUndoAction (SHIELD), m_shield (s)
          {
          }

        ~MaskedImageUndoAction_Shield ()
          {
          }

        Glib::ustring get_action_name () const
          {
            return "Shield";
          }

        int get_shield () const
          {
            return m_shield;
          }

    private:
        int m_shield;
};
#endif
