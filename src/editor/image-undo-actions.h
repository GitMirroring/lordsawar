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
#ifndef IMAGE_UNDO_ACTIONS_H
#define IMAGE_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include <vector>
#include "pixmask.h"

//! A record of an event in the image editor
/**
 * The purpose of these classes is to implement undo/redo in the image
 * editor.
 */

class ImageUndoAction: public UndoAction
{
public:

    enum Type
      {
        SET = 1,
      };

    ImageUndoAction (Type type)
     : UndoAction (UndoAction::AGGREGATE_NONE), m_type (type)
      {
      }

    virtual ~ImageUndoAction ()
      {
      }

    Type get_type () const
      {
        return m_type;
      }

protected:

    Type m_type;
};

class ImageUndoAction_Set: public ImageUndoAction
{
    public:
        ImageUndoAction_Set (Glib::ustring f, std::vector<PixMask *> im)
          : ImageUndoAction (SET), m_file (f)
          {
            for (auto i : im)
              m_frames.push_back (i->copy ());
          }

        ~ImageUndoAction_Set ()
          {
            for (auto i : m_frames)
              delete i;
          }

        Glib::ustring get_action_name () const
          {
            return "Set";
          }

        Glib::ustring get_file () const
          {
            return m_file;
          }

        std::vector<PixMask *> get_frames () const
          {
            return m_frames;
          }

    private:
        Glib::ustring m_file;
        std::vector<PixMask *> m_frames;
};
#endif
