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
#ifndef TILESET_SELECTOR_EDITOR_ACTIONS_H
#define TILESET_SELECTOR_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include <vector>
#include "PixMask.h"
#include "TarFileMaskedImage.h"
#include "tarfile.h"

//! A record of an event in the tileset selector editor
/** 
 * The purpose of these classes is to implement undo/redo in the tileset
 * selector editor.
 */

class TileSetSelectorEditorAction: public UndoAction
{
public:

    enum Type {
      SET = 1,
      SHIELD = 2,
      SIZE = 3,
    };

    TileSetSelectorEditorAction(Type type)
     : UndoAction (UndoAction::AGGREGATE_NONE), d_type (type) {}

    virtual ~TileSetSelectorEditorAction() {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class TileSetSelectorEditorAction_Set: public TileSetSelectorEditorAction
{
    public:
        TileSetSelectorEditorAction_Set (TarFile *t,
                                         bool large, Glib::ustring ar)
          : TileSetSelectorEditorAction (SET), d_large (large),
          d_member (ar)
          {
            if (ar.empty () == false)
              {
                Glib::ustring file = t->getFileFromConfigurationFile (ar);
                if (file.empty () == false)
                  {
                    Glib::ustring dir = File::get_tmp_file ();
                    File::create_dir (dir);
                    d_filename = File::getTempFile(dir, ar);
                    File::copy (file, d_filename);
                  }
              }
          }
        ~TileSetSelectorEditorAction_Set ()
          {
            if (d_filename.empty () == false)
              {
                File::erase (d_filename);
                Glib::ustring dir = File::get_dirname (d_filename);
                File::erase_dir (dir);
              }
          }

        Glib::ustring getActionName () const {return "Set";}

        Glib::ustring getArchiveMember () const {return d_member;}
        Glib::ustring getFileName () const {return d_filename;}
        bool getLarge () const {return d_large;}
    private:
        bool d_large;
        Glib::ustring d_member;
        Glib::ustring d_filename;
};

class TileSetSelectorEditorAction_Shield: public TileSetSelectorEditorAction
{
    public:
        TileSetSelectorEditorAction_Shield (int s)
          : TileSetSelectorEditorAction (SHIELD), d_shield (s) {}
        ~TileSetSelectorEditorAction_Shield () {}

        Glib::ustring getActionName () const {return "Shield";}

        int getShield () const {return d_shield;}

    private:
        int d_shield;
};

class TileSetSelectorEditorAction_Size: public TileSetSelectorEditorAction
{
    public:
        TileSetSelectorEditorAction_Size (bool l)
          : TileSetSelectorEditorAction (SIZE), d_large (l) {}
        ~TileSetSelectorEditorAction_Size () {}

        Glib::ustring getActionName () const {return "Size";}

        bool getLarge () const {return d_large;}

    private:
        int d_large;
};

#endif //TILESET_SELECTOR_EDITOR_ACTIONS_H
