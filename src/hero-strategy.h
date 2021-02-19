//  Copyright (C) 2021 Ben Asselstine
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
#ifndef HERO_STRATEGY_H
#define HERO_STRATEGY_H

#include <gtkmm.h>
#include <glibmm.h>
#include "xmlhelper.h"
#include "defs.h"

//! How heroes go about their day
/**
 * A set of strategies that heroes can have for playing their turns.
 *
 */
class HeroStrategy 
{
    public:

	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! The different kinds of HeroStrategy objects.
        enum Type {
	  //! Moves like any other stack
	  NONE = 0, 
	  //! After a number of turns switch strategies
	  RANDOM = 1, 
	  //! Goes on quest with help
	  SIMPLE_QUESTER = 2, 
	};

        HeroStrategy (Type type);
        HeroStrategy (XML_Helper* helper);
        HeroStrategy (const HeroStrategy& orig);
        virtual ~HeroStrategy() {};

        Type getType () const { return d_type; }

        virtual Glib::ustring getDescription () const {return "";}

        virtual bool save (XML_Helper* helper) const = 0;


	// Static Methods

        static HeroStrategy* handle_load (XML_Helper* helper);

	static Glib::ustring heroStrategyTypeToString (const Type type);

	static Type heroStrategyTypeFromString (const Glib::ustring str);

	static HeroStrategy* copy (const HeroStrategy* r);

        static Type getRandom ();

        static bool compare (const HeroStrategy *l, const HeroStrategy *r);

    protected:

        Type d_type;

};

class HeroStrategy_None : public HeroStrategy
{
    public:
        HeroStrategy_None ();
	HeroStrategy_None (XML_Helper *helper);
	HeroStrategy_None (const HeroStrategy_None& orig);
        ~HeroStrategy_None ();

        Glib::ustring getDescription () const
          {return N_("Acts like any other stack");}

        bool save (XML_Helper* helper) const;

    private:

};

class HeroStrategy_Random: public HeroStrategy
{
    public:
        HeroStrategy_Random ();
        HeroStrategy_Random (guint32 turns);
	HeroStrategy_Random (const HeroStrategy_Random& orig);
	HeroStrategy_Random (XML_Helper *helper);
        ~HeroStrategy_Random ();

        Glib::ustring getDescription () const
          {return N_("Switches strategies every so often");}

        guint32 getTurns () const {return d_turns;}
        void setTurns (guint32 turns) {d_turns = turns;}

        bool save (XML_Helper* helper) const;

    private:
        guint32 d_turns;
};

class HeroStrategy_SimpleQuester: public HeroStrategy
{
    public:
        HeroStrategy_SimpleQuester ();
        HeroStrategy_SimpleQuester (Type t, guint32 num_helpers);
	HeroStrategy_SimpleQuester (XML_Helper *helper);
	HeroStrategy_SimpleQuester (const HeroStrategy_SimpleQuester& orig);
        virtual ~HeroStrategy_SimpleQuester ();

        Glib::ustring getDescription () const
          {return N_("Goes on quests with help");}

        Type getFallbackStrategy () const {return d_fallback_type;}
        void setFallbackStrategy (Type t) {d_fallback_type = t;}

        guint32 getNumHelpers () const {return d_num_helpers;}
        void setNumHelpers (guint32 n) {d_num_helpers = n;}

        bool save (XML_Helper* helper) const;


    private:
        Type d_fallback_type;
        guint32 d_num_helpers;
};

#endif
