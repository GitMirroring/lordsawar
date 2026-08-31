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
#ifndef TAR_FILE_MASKED_IMAGE_EDITOR_ACTIONS_H
#define TAR_FILE_MASKED_IMAGE_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include <vector>
#include "PixMask.h"
#include "TarFileMaskedImage.h"

//! A record of an event in the image editor
/** 
 * The purpose of these classes is to implement undo/redo in the tar-file
 * masked image editor.
 */

class TarFileMaskedImageEditorAction: public UndoAction
{
public:

    enum Type {
      SET = 1,
      SHIELD = 2,
    };

    TarFileMaskedImageEditorAction(Type type)
     : UndoAction (UndoAction::AGGREGATE_NONE), d_type (type) {}

    virtual ~TarFileMaskedImageEditorAction() {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class TarFileMaskedImageEditorAction_Set: public TarFileMaskedImageEditorAction
{
    public:
        TarFileMaskedImageEditorAction_Set (TarFileMaskedImage *i,
                                            Glib::ustring f)
          : TarFileMaskedImageEditorAction (SET),
          d_image (new TarFileMaskedImage (*i)), d_filename (f) {}
        ~TarFileMaskedImageEditorAction_Set () {delete d_image;}

        Glib::ustring getActionName () const {return "Set";}

        Glib::ustring getFileName () const {return d_filename;}
        TarFileMaskedImage* getTarFileMaskedImage() const {return d_image;}
    private:
        TarFileMaskedImage *d_image;
        Glib::ustring d_filename;
};

class TarFileMaskedImageEditorAction_Shield: public TarFileMaskedImageEditorAction
{
    public:
        TarFileMaskedImageEditorAction_Shield (int s)
          : TarFileMaskedImageEditorAction (SHIELD), d_shield (s) {}
        ~TarFileMaskedImageEditorAction_Shield () {}

        Glib::ustring getActionName () const {return "Shield";}

        int getShield () const {return d_shield;}

    private:
        int d_shield;
};
#endif //TAR_FILE_MASKED_IMAGE_EDITOR_ACTIONS_H
