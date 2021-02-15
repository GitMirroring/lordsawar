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
#ifndef MASK_VALIDATION_ACTIONS_H
#define MASK_VALIDATION_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include <vector>
#include "PixMask.h"

//! A record of an event in the mask validation dialog
/** 
 * The purpose of these classes is to implement undo/redo in the 
 * mask validation dialog.
 */

class MaskValidationAction: public UndoAction
{
public:

    enum Type {
      MASKS = 1,
    };

    MaskValidationAction(Type type)
     : UndoAction (UndoAction::AGGREGATE_NONE), d_type (type) {}

    virtual ~MaskValidationAction() {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class MaskValidationAction_Masks: public MaskValidationAction
{
    public:
        MaskValidationAction_Masks (guint32 n)
          : MaskValidationAction (MASKS), d_num_masks(n) {}
        ~MaskValidationAction_Masks () {}

        Glib::ustring getActionName () const {return "Masks";}

        guint32 getNumMasks () const {return d_num_masks;}

    private:
        guint32 d_num_masks;
};
#endif //MASK_VALIDATION_ACTIONS_H
