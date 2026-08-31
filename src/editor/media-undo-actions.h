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
#ifndef MEDIA_ACTIONS_H
#define MEDIA_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include <vector>
#include "PixMask.h"
#include "TarFileMaskedImage.h"
#include "tarfile.h"

class TarFileImage;
class TarFileMaskedImage;
//! A record of an event in the scenario media editor
/** 
 * The purpose of these classes is to implement undo/redo in the scenario
 * media editor.
 */

class MediaAction: public UndoAction
{
public:

    enum Type {
      IMAGE_SET = 1,
      MASKED_IMAGE_SET = 2,
    };

    MediaAction(Type type)
     : UndoAction (UndoAction::AGGREGATE_NONE), d_type (type) {}

    virtual ~MediaAction() {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class MediaAction_ImageSet: public MediaAction
{
    public:
        MediaAction_ImageSet (TarFile *t, Glib::ustring ar, TarFileImage *im)
          : MediaAction (IMAGE_SET), d_member (ar), d_im (im)
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
        ~MediaAction_ImageSet ()
          {
            if (d_filename.empty () == false)
              {
                File::erase (d_filename);
                Glib::ustring dir = File::get_dirname (d_filename);
                File::erase_dir (dir);
              }
          }

        Glib::ustring getActionName () const {return "ImageSet";}

        Glib::ustring getArchiveMember () const {return d_member;}
        Glib::ustring getFileName () const {return d_filename;}
        TarFileImage *getImage () const {return d_im;}
    private:
        Glib::ustring d_member;
        Glib::ustring d_filename;
        TarFileImage *d_im;
};

class MediaAction_MaskedImageSet: public MediaAction
{
    public:
        MediaAction_MaskedImageSet (TarFile *t, Glib::ustring ar,
                                    TarFileMaskedImage *im)
          : MediaAction (MASKED_IMAGE_SET), d_member (ar), d_im (im)
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
        ~MediaAction_MaskedImageSet ()
          {
            if (d_filename.empty () == false)
              {
                File::erase (d_filename);
                Glib::ustring dir = File::get_dirname (d_filename);
                File::erase_dir (dir);
              }
          }

        Glib::ustring getActionName () const {return "MaskedImageSet";}

        Glib::ustring getArchiveMember () const {return d_member;}
        Glib::ustring getFileName () const {return d_filename;}
        TarFileMaskedImage *getImage () const {return d_im;}
    private:
        Glib::ustring d_member;
        Glib::ustring d_filename;
        TarFileMaskedImage *d_im;
};

#endif //MEDIA_ACTIONS_H
