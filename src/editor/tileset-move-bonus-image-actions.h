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
#ifndef TILESET_MOVE_BONUS_IMAGE_ACTIONS_H
#define TILESET_MOVE_BONUS_IMAGE_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include <vector>
#include "PixMask.h"
#include "TarFileMaskedImage.h"
#include "tarfile.h"

//! A record of an event in the tileset move bonus image editor
/** 
 * The purpose of these classes is to implement undo/redo in the tileset
 * move bonus image editor.
 */

class TileSetMoveBonusImageAction: public UndoAction
{
public:

    Gtk::Button *all_imagechooser_button;
    Gtk::Button *water_imagechooser_button;
    Gtk::Button *forest_imagechooser_button;
    Gtk::Button *hills_imagechooser_button;
    Gtk::Button *mountains_imagechooser_button;
    Gtk::Button *swamp_imagechooser_button;
    enum Type {
      ALL = 1,
      WATER = 2,
      FOREST = 3,
      HILLS = 4,
      MOUNTAINS = 5,
      SWAMP = 6,
    };

    TileSetMoveBonusImageAction(Type type)
     : UndoAction (UndoAction::AGGREGATE_NONE), d_type (type) {}

    virtual ~TileSetMoveBonusImageAction() {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};


class TileSetMoveBonusImageAction_Set: public TileSetMoveBonusImageAction
{
    public:
        TileSetMoveBonusImageAction_Set (Type type, TarFile *t, Glib::ustring ar)
          : TileSetMoveBonusImageAction (type), d_member (ar)
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
        ~TileSetMoveBonusImageAction_Set ()
          {
            if (d_filename.empty () == false)
              {
                File::erase (d_filename);
                Glib::ustring dir = File::get_dirname (d_filename);
                File::erase_dir (dir);
              }
          }

        Glib::ustring getArchiveMember () const {return d_member;}
        Glib::ustring getFileName () const {return d_filename;}
    private:
        Glib::ustring d_member;
        Glib::ustring d_filename;
};

class TileSetMoveBonusImageAction_All: public TileSetMoveBonusImageAction_Set
{
    public:
        TileSetMoveBonusImageAction_All (TarFile *t, Glib::ustring ar)
          : TileSetMoveBonusImageAction_Set (ALL, t, ar) {}
        ~TileSetMoveBonusImageAction_All () {}
        Glib::ustring getActionName () const {return "All";}

};
class TileSetMoveBonusImageAction_Water: public TileSetMoveBonusImageAction_Set
{
    public:
        TileSetMoveBonusImageAction_Water (TarFile *t, Glib::ustring ar)
          : TileSetMoveBonusImageAction_Set (WATER, t, ar) {}
        ~TileSetMoveBonusImageAction_Water () {}
        Glib::ustring getActionName () const {return "Water";}

};
class TileSetMoveBonusImageAction_Forest: public TileSetMoveBonusImageAction_Set
{
    public:
        TileSetMoveBonusImageAction_Forest (TarFile *t, Glib::ustring ar)
          : TileSetMoveBonusImageAction_Set (FOREST, t, ar) {}
        ~TileSetMoveBonusImageAction_Forest () {}
        Glib::ustring getActionName () const {return "Forest";}

};
class TileSetMoveBonusImageAction_Hills: public TileSetMoveBonusImageAction_Set
{
    public:
        TileSetMoveBonusImageAction_Hills (TarFile *t, Glib::ustring ar)
          : TileSetMoveBonusImageAction_Set (HILLS, t, ar) {}
        ~TileSetMoveBonusImageAction_Hills () {}
        Glib::ustring getActionName () const {return "Hills";}

};
class TileSetMoveBonusImageAction_Mountains: public TileSetMoveBonusImageAction_Set
{
    public:
        TileSetMoveBonusImageAction_Mountains (TarFile *t, Glib::ustring ar)
          : TileSetMoveBonusImageAction_Set (MOUNTAINS, t, ar) {}
        ~TileSetMoveBonusImageAction_Mountains () {}
        Glib::ustring getActionName () const {return "Mountains";}

};
class TileSetMoveBonusImageAction_Swamp: public TileSetMoveBonusImageAction_Set
{
    public:
        TileSetMoveBonusImageAction_Swamp (TarFile *t, Glib::ustring ar)
          : TileSetMoveBonusImageAction_Set (SWAMP, t, ar) {}
        ~TileSetMoveBonusImageAction_Swamp () {}
        Glib::ustring getActionName () const {return "Swamp";}

};
#endif //TILESET_MOVE_BONUS_IMAGE_ACTIONS_H
