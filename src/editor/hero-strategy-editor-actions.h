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
#ifndef HERO_STRATEGY_EDITOR_ACTIONS_H
#define HERO_STRATEGY_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>

#include "undo-action.h"
#include "hero-strategy.h"
#include "undo-mgr.h"

//! A record of an event in the hero strategy editor
/** 
 * The purpose of these classes is to implement undo/redo in the hero
 * strategy editor.
 */

class HeroStrategyEditorAction: public UndoAction
{
    public:

        enum Type
          {
            TYPE = 1,
            TURNS = 2,
            NUM_HELPERS = 3,
            FALLBACK_TYPE = 4,
          };

	//! Default constructor.
        HeroStrategyEditorAction(Type type, UndoAction::AggregateType aggregate = UndoAction::AGGREGATE_NONE) : UndoAction (aggregate), d_type(type) {}

        Type getType() const {return d_type;}

    protected:

        Type d_type;
};

class HeroStrategyEditorAction_Strategy: public HeroStrategyEditorAction
{
    public:
        HeroStrategyEditorAction_Strategy (Type t, HeroStrategy *s, bool agg = false)
          : HeroStrategyEditorAction (t, agg ? UndoAction::AGGREGATE_DELAY : UndoAction::AGGREGATE_NONE), d_strategy (HeroStrategy::copy (s)) { }
        ~HeroStrategyEditorAction_Strategy () { delete d_strategy; }

        HeroStrategy *getStrategy () const {return d_strategy;}
    private:
        HeroStrategy *d_strategy;
};

class HeroStrategyEditorAction_Type : public HeroStrategyEditorAction_Strategy
{
    public:
        HeroStrategyEditorAction_Type (HeroStrategy *s)
          :HeroStrategyEditorAction_Strategy (TYPE, s, false) {}
        ~HeroStrategyEditorAction_Type () {}

        Glib::ustring getActionName () const {return "Type";}
};

class HeroStrategyEditorAction_Turns : public HeroStrategyEditorAction_Strategy
{
    public:
        HeroStrategyEditorAction_Turns (HeroStrategy *s)
          :HeroStrategyEditorAction_Strategy (TURNS, s, false) {}
        ~HeroStrategyEditorAction_Turns () {}

        Glib::ustring getActionName () const {return "Turns";}
};

class HeroStrategyEditorAction_NumHelpers : public HeroStrategyEditorAction_Strategy
{
    public:
        HeroStrategyEditorAction_NumHelpers (HeroStrategy *s)
          :HeroStrategyEditorAction_Strategy (NUM_HELPERS, s, false) {}
        ~HeroStrategyEditorAction_NumHelpers () {}

        Glib::ustring getActionName () const {return "NumHelpers";}
};

class HeroStrategyEditorAction_FallbackType : public HeroStrategyEditorAction_Strategy
{
    public:
        HeroStrategyEditorAction_FallbackType (HeroStrategy *s)
          :HeroStrategyEditorAction_Strategy (FALLBACK_TYPE, s, false) {}
        ~HeroStrategyEditorAction_FallbackType () {}

        Glib::ustring getActionName () const {return "FallbackType";}
};
#endif //HERO_STRATEGY_EDITOR_ACTIONS_H
