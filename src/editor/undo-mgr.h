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
#ifndef UNDO_MGR_H
#define UNDO_MGR_H

#include <gtkmm.h>
#include "undo-action.h"

#include <sigc++/signal.h>
#include <sigc++/trackable.h>

/**
 * The UndoMgr is handles the lists of undo and redo actions.
 * New'd undo actions are given to the manager, and then it later deletes them.
 * Callers connect to the execute signal for callback.
 */
class UndoMgr: public sigc::trackable
{
    public:

        static const int LIMIT;
        static const double DELAY;

	//! Default constructor.
        UndoMgr (double delay, guint32 limit)
          : d_delay (delay), d_limit (limit) {}

        void undo ();
        void redo ();
        void add (UndoAction *a);

        bool undoEmpty () {return undos.empty ();}
        bool redoEmpty () {return redos.empty ();}

        void setDelay (double d) {d_delay = d;}
        void setLimit (guint32 l) {d_limit = l;}

        Glib::ustring getUndoName () const
          {return UndoMgr::getActionName (undos);}
        Glib::ustring getRedoName () const
          {return UndoMgr::getActionName (redos);}

        void updateMenuItems (Gtk::MenuItem *undo, Gtk::MenuItem *redo);
        void clear ();

	//! Destructor.
        ~UndoMgr();

        sigc::signal<UndoAction*, UndoAction*> execute ()
          {return execute_signal;}

        void dump ();
    private:
        double d_delay;
        guint32 d_limit;
        std::list<UndoAction*> undos;
        std::list<UndoAction*> redos;
        sigc::signal<UndoAction*, UndoAction*> execute_signal;

        std::list<UndoAction*> popLastGroup (std::list<UndoAction*> *list, bool bottom = false);
        std::list<UndoAction*> popGroup (std::list<UndoAction*> *list, bool bottom = false);
        bool isGrouped (UndoAction *l, UndoAction *r, bool top = false);
        guint32 countUndoBlocks (std::list<UndoAction*> list, bool top = false);
        static Glib::ustring getActionName (std::list<UndoAction*> list);

};
#endif //UNDO_MGR_H
