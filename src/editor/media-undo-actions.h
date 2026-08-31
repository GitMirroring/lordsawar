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
#ifndef MEDIA_UNDO_ACTIONS_H
#define MEDIA_UNDO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include <vector>
#include "pixmask.h"
#include "tar-file-image.h"
#include "tar-file-masked-image.h"
#include "tar-file-sound.h"
#include "tar-file.h"

//! A record of an event in the scenario media editor
/**
 * The purpose of these classes is to implement undo/redo in the scenario
 * media editor.
 */

class MediaUndoAction: public UndoAction
{
public:

    enum Type
      {
        IMAGE_SET = 1,
        MASKED_IMAGE_SET = 2,
        SOUND_SET = 3,
      };

    MediaUndoAction (Type type)
      : UndoAction (UndoAction::AGGREGATE_NONE), m_type (type)
      {
      }

    virtual ~MediaUndoAction ()
      {
      }

    Type get_type () const
      {
        return m_type;
      }

protected:

    Type m_type;
};

class MediaUndoAction_ImageSet: public MediaUndoAction
{
public:
    MediaUndoAction_ImageSet (TarFile *t, Glib::ustring ar, TarFileImage *im)
      : MediaUndoAction (IMAGE_SET), m_member (ar), m_im (im)
      {
        if (ar.empty () == false)
          {
            Glib::ustring file = t->getFileFromConfigurationFile (ar);
            if (file.empty () == false)
              {
                Glib::ustring dir = File::get_tmp_file ();
                File::create_dir (dir);
                m_filename = File::getTempFile (dir, ar);
                File::copy (file, m_filename);
              }
          }
      }

    ~MediaUndoAction_ImageSet ()
      {
        if (m_filename.empty () == false)
          {
            File::erase (m_filename);
            Glib::ustring dir = File::get_dirname (m_filename);
            File::erase_dir (dir);
          }
      }

    Glib::ustring get_action_name () const
      {
        return "ImageSet";
      }

    Glib::ustring get_archive_member () const
      {
        return m_member;
      }

    Glib::ustring get_filename () const
      {
        return m_filename;
      }

    TarFileImage *get_image () const
      {
        return m_im;
      }
private:
    Glib::ustring m_member;
    Glib::ustring m_filename;
    TarFileImage *m_im;
};

class MediaUndoAction_MaskedImageSet: public MediaUndoAction
{
public:
    MediaUndoAction_MaskedImageSet (TarFile *t, Glib::ustring ar,
                                    TarFileMaskedImage *im)
      : MediaUndoAction (MASKED_IMAGE_SET), m_member (ar), m_im (im)
      {
        if (ar.empty () == false)
          {
            Glib::ustring file = t->getFileFromConfigurationFile (ar);
            if (file.empty () == false)
              {
                Glib::ustring dir = File::get_tmp_file ();
                File::create_dir (dir);
                m_filename = File::getTempFile (dir, ar);
                File::copy (file, m_filename);
              }
          }
      }

    ~MediaUndoAction_MaskedImageSet ()
      {
        if (m_filename.empty () == false)
          {
            File::erase (m_filename);
            Glib::ustring dir = File::get_dirname (m_filename);
            File::erase_dir (dir);
          }
      }

    Glib::ustring get_action_name () const
      {
        return "MaskedImageSet";
      }

    Glib::ustring get_archive_member () const
      {
        return m_member;
      }

    Glib::ustring get_filename () const
      {
        return m_filename;
      }

    TarFileMaskedImage *get_image () const
      {
        return m_im;
      }
private:
    Glib::ustring m_member;
    Glib::ustring m_filename;
    TarFileMaskedImage *m_im;
};

class MediaUndoAction_SoundSet: public MediaUndoAction
{
public:
    MediaUndoAction_SoundSet (TarFile *t, Glib::ustring ar, TarFileSound *s)
      : MediaUndoAction (SOUND_SET), m_member (ar), m_sound (s)
      {
        if (ar.empty () == false)
          {
            Glib::ustring file = t->getFileFromConfigurationFile (ar);
            if (file.empty () == false)
              {
                Glib::ustring dir = File::get_tmp_file ();
                File::create_dir (dir);
                m_filename = File::getTempFile (dir, ar);
                File::copy (file, m_filename);
              }
          }
      }

    ~MediaUndoAction_SoundSet ()
      {
        if (m_filename.empty () == false)
          {
            File::erase (m_filename);
            Glib::ustring dir = File::get_dirname (m_filename);
            File::erase_dir (dir);
          }
      }

    Glib::ustring get_action_name () const
      {
        return "SoundSet";
      }

    Glib::ustring get_archive_member () const
      {
        return m_member;
      }

    Glib::ustring get_filename () const
      {
        return m_filename;
      }

    TarFileSound *get_sound () const
      {
        return m_sound;
      }
private:
    Glib::ustring m_member;
    Glib::ustring m_filename;
    TarFileSound *m_sound;
};
#endif
