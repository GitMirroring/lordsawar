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
#ifndef TILESET_EXPLOSION_PICTURE_EDITOR_ACTIONS_H
#define TILESET_EXPLOSION_PICTURE_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include <vector>
#include "PixMask.h"
#include "TarFileMaskedImage.h"
#include "tarfile.h"

//! A record of an event in the tileset explosion picture editor
/** 
 * The purpose of these classes is to implement undo/redo in the tileset
 * explosion picture editor.
 */

class TileSetExplosionPictureEditorAction: public UndoAction
{
public:

    enum Type {
      SET = 1,
      SIZE = 2,
    };

    TileSetExplosionPictureEditorAction(Type type)
     : UndoAction (UndoAction::AGGREGATE_NONE), d_type (type) {}

    virtual ~TileSetExplosionPictureEditorAction() {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class TileSetExplosionPictureEditorAction_Set: public TileSetExplosionPictureEditorAction
{
    public:
        TileSetExplosionPictureEditorAction_Set (TarFile *t, Glib::ustring ar)
          : TileSetExplosionPictureEditorAction (SET), d_member (ar)
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
        ~TileSetExplosionPictureEditorAction_Set ()
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
    private:
        Glib::ustring d_member;
        Glib::ustring d_filename;
};

class TileSetExplosionPictureEditorAction_Size: public TileSetExplosionPictureEditorAction
{
    public:
        TileSetExplosionPictureEditorAction_Size (bool l)
          : TileSetExplosionPictureEditorAction (SIZE), d_large (l) {}
        ~TileSetExplosionPictureEditorAction_Size () {}

        Glib::ustring getActionName () const {return "Size";}

        bool getLarge () const {return d_large;}

    private:
        int d_large;
};

#endif //TILESET_EXPLOSION_PICTURE_EDITOR_ACTIONS_H
