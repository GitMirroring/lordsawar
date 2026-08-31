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
#ifndef SHIELDSET_INFO_ACTIONS_H
#define SHIELDSET_INFO_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>
#include "undo-action.h"

class UndoMgr;

class ShieldSetInfoAction: public UndoAction
{
public:

    enum Type {
      DESCRIPTION = 1,
      COPYRIGHT = 2,
      LICENSE = 3,
      NAME = 4,
      SMALL_WIDTH = 5,
      SMALL_HEIGHT = 6,
      MEDIUM_WIDTH = 7,
      MEDIUM_HEIGHT = 8,
      LARGE_WIDTH = 9,
      LARGE_HEIGHT = 10,
      FIT = 11,
    };

    ShieldSetInfoAction(Type type, bool agg = true)
     : UndoAction (agg ? UndoAction::AGGREGATE_DELAY : UndoAction::AGGREGATE_NONE), d_type (type) {}

    Type getType() const {return d_type;}

protected:

    Type d_type;
};

class ShieldSetInfoAction_Message: public ShieldSetInfoAction, public UndoCursor
{
    public:
        ShieldSetInfoAction_Message (Type t, Glib::ustring m, UndoMgr *u,
                                     Gtk::TextView *v)
          : ShieldSetInfoAction (t), UndoCursor (u, v), d_message (m) {}
        Glib::ustring getMessage () {return d_message;}
    private:
        Glib::ustring d_message;
};

class ShieldSetInfoAction_Description: public ShieldSetInfoAction_Message
{
    public:
        ShieldSetInfoAction_Description (Glib::ustring m, UndoMgr *u,
                                         Gtk::TextView *v)
          : ShieldSetInfoAction_Message (DESCRIPTION, m, u, v) {}
        ~ShieldSetInfoAction_Description () {}

        Glib::ustring getActionName () const {return "Description";}
};

class ShieldSetInfoAction_Copyright: public ShieldSetInfoAction_Message
{
    public:
        ShieldSetInfoAction_Copyright (Glib::ustring m, UndoMgr *u,
                                       Gtk::TextView *v)
          : ShieldSetInfoAction_Message (COPYRIGHT, m, u, v) {}
        ~ShieldSetInfoAction_Copyright () {}

        Glib::ustring getActionName () const {return "Copyright";}
};

class ShieldSetInfoAction_License: public ShieldSetInfoAction_Message
{
    public:
        ShieldSetInfoAction_License (Glib::ustring m, UndoMgr *u,
                                     Gtk::TextView *v)
          : ShieldSetInfoAction_Message (LICENSE, m, u, v) {}
        ~ShieldSetInfoAction_License () {}

        Glib::ustring getActionName () const {return "License";}
};

class ShieldSetInfoAction_Name: public ShieldSetInfoAction, public UndoCursor
{
    public:
        ShieldSetInfoAction_Name (Glib::ustring n, UndoMgr *u, Gtk::Entry *e)
          : ShieldSetInfoAction (NAME), UndoCursor (u, e), d_name (n) {}
        ~ShieldSetInfoAction_Name () {}

        Glib::ustring getActionName () const {return "Name";}
        Glib::ustring getName () {return d_name;}

    private:
        Glib::ustring d_name;
};

class ShieldSetInfoAction_Size: public ShieldSetInfoAction
{
    public:
        ShieldSetInfoAction_Size (Type t, int p)
          : ShieldSetInfoAction (t), d_size (p) {};

        int getSize () {return d_size;}

    private:
        int d_size;
};

class ShieldSetInfoAction_SmallWidth: public ShieldSetInfoAction_Size
{
    public:
        ShieldSetInfoAction_SmallWidth (int p)
          : ShieldSetInfoAction_Size (SMALL_WIDTH, p) {}
        ~ShieldSetInfoAction_SmallWidth () {}

        Glib::ustring getActionName () const {return "SmallWidth";}
};
class ShieldSetInfoAction_SmallHeight: public ShieldSetInfoAction_Size
{
    public:
        ShieldSetInfoAction_SmallHeight (int p)
          : ShieldSetInfoAction_Size (SMALL_HEIGHT, p) {}
        ~ShieldSetInfoAction_SmallHeight () {}

        Glib::ustring getActionName () const {return "SmallHeight";}
};
class ShieldSetInfoAction_MediumWidth: public ShieldSetInfoAction_Size
{
    public:
        ShieldSetInfoAction_MediumWidth (int p)
          : ShieldSetInfoAction_Size (MEDIUM_WIDTH, p) {}
        ~ShieldSetInfoAction_MediumWidth () {}

        Glib::ustring getActionName () const {return "MediumWidth";}
};
class ShieldSetInfoAction_MediumHeight: public ShieldSetInfoAction_Size
{
    public:
        ShieldSetInfoAction_MediumHeight (int p)
          : ShieldSetInfoAction_Size (MEDIUM_HEIGHT, p) {}
        ~ShieldSetInfoAction_MediumHeight () {}

        Glib::ustring getActionName () const {return "MediumHeight";}
};
class ShieldSetInfoAction_LargeWidth: public ShieldSetInfoAction_Size
{
    public:
        ShieldSetInfoAction_LargeWidth (int p)
          : ShieldSetInfoAction_Size (LARGE_WIDTH, p) {}
        ~ShieldSetInfoAction_LargeWidth () {}

        Glib::ustring getActionName () const {return "LargeWidth";}
};
class ShieldSetInfoAction_LargeHeight: public ShieldSetInfoAction_Size
{
    public:
        ShieldSetInfoAction_LargeHeight (int p)
          : ShieldSetInfoAction_Size (LARGE_HEIGHT, p) {}
        ~ShieldSetInfoAction_LargeHeight () {}

        Glib::ustring getActionName () const {return "LargeHeight";}
};

class ShieldSetInfoAction_Fit: public ShieldSetInfoAction
{
    public:
        ShieldSetInfoAction_Fit (guint32 a, guint32 b, guint32 c, guint32 d,
                                 guint32 e, guint32 f)
          : ShieldSetInfoAction (FIT, false), d_small_width (a),
          d_small_height (b), d_medium_width (c), d_medium_height (d),
          d_large_width (e), d_large_height (f) {}

        ~ShieldSetInfoAction_Fit () {}
        Glib::ustring getActionName () const {return "Fit";}

        guint32 getSmallWidth () const {return d_small_width;}
        guint32 getSmallHeight () const {return d_small_height;}
        guint32 getMediumWidth () const {return d_medium_width;}
        guint32 getMediumHeight () const {return d_medium_height;}
        guint32 getLargeWidth () const {return d_large_width;}
        guint32 getLargeHeight () const {return d_large_height;}
    private:
        guint32 d_small_width;
        guint32 d_small_height;
        guint32 d_medium_width;
        guint32 d_medium_height;
        guint32 d_large_width;
        guint32 d_large_height;
};

#endif //SHIELDSET_INFO_ACTIONS_H
