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
#ifndef ARMYSET_SELECTOR_EDITOR_ACTIONS_H
#define ARMYSET_SELECTOR_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"
#include <vector>
#include "PixMask.h"
#include "TarFileMaskedImage.h"
#include "tarfile.h"

//! A record of an event in the armyset selector editor
/** 
 * The purpose of these classes is to implement undo/redo in the armyset
 * selector editor.
 */

class ArmySetSelectorEditorAction: public UndoAction
{
public:

    enum Type {
      SET = 1,
      SHIELD = 2,
      SIZE = 3,
      OWNER = 4,
    };

    ArmySetSelectorEditorAction(Type type)
     : UndoAction (UndoAction::AGGREGATE_NONE), d_type (type) {}

    virtual ~ArmySetSelectorEditorAction() {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class ArmySetSelectorEditorAction_Set: public ArmySetSelectorEditorAction
{
    public:
        ArmySetSelectorEditorAction_Set (TarFile *t, Shield::Colour c,
                                         bool large, Glib::ustring ar)
          : ArmySetSelectorEditorAction (SET), d_owner (c), d_large (large),
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
        ~ArmySetSelectorEditorAction_Set ()
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

        Shield::Colour getOwner () const {return d_owner;}
        bool getLarge () const {return d_large;}
    private:
        Shield::Colour d_owner;
        bool d_large;
        Glib::ustring d_member;
        Glib::ustring d_filename;
};

class ArmySetSelectorEditorAction_Shield: public ArmySetSelectorEditorAction
{
    public:
        ArmySetSelectorEditorAction_Shield (int s)
          : ArmySetSelectorEditorAction (SHIELD), d_shield (s) {}
        ~ArmySetSelectorEditorAction_Shield () {}

        Glib::ustring getActionName () const {return "Shield";}

        int getShield () const {return d_shield;}

    private:
        int d_shield;
};

class ArmySetSelectorEditorAction_Size: public ArmySetSelectorEditorAction
{
    public:
        ArmySetSelectorEditorAction_Size (bool l)
          : ArmySetSelectorEditorAction (SIZE), d_large (l) {}
        ~ArmySetSelectorEditorAction_Size () {}

        Glib::ustring getActionName () const {return "Size";}

        bool getLarge () const {return d_large;}

    private:
        int d_large;
};

class ArmySetSelectorEditorAction_Owner: public ArmySetSelectorEditorAction
{
    public:
        ArmySetSelectorEditorAction_Owner (int o)
          : ArmySetSelectorEditorAction (OWNER), d_owner (o) {}
        ~ArmySetSelectorEditorAction_Owner () {}

        Glib::ustring getActionName () const {return "Owner";}

        int getOwner () const {return d_owner;}

    private:
        int d_owner;
};

#endif //ARMYSET_SELECTOR_EDITOR_ACTIONS_H
