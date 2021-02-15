//  Copyright (C) 2008, 2009, 2014, 2017, 2020 Ben Asselstine
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
#ifndef CREATE_SCENARIO_RANDOMIZE_H
#define CREATE_SCENARIO_RANDOMIZE_H

#include <fstream>
#include <vector>
#include <gtkmm.h>
#include "shield.h"
#include "namelist.h"
#include <sigc++/signal.h>

class Signpost;
class Army;
class Player;
class Reward;
class Keeper;

//! Generates random values for various map buildings.
class CreateScenarioRandomize
{
    public:
        CreateScenarioRandomize();
        CreateScenarioRandomize(const CreateScenarioRandomize &r);
        virtual ~CreateScenarioRandomize() {};

	/** take a random city name
	 */
	Glib::ustring popRandomCityName();

	/* give a random city name
	 */
	void pushRandomCityName(Glib::ustring name);
	Glib::ustring popRandomRuinName();
	void pushRandomRuinName(Glib::ustring name);
	Glib::ustring popRandomTempleName();
	void pushRandomTempleName(Glib::ustring name);
	Glib::ustring popRandomSignpost();
	void pushRandomSignpost(Glib::ustring name);
	guint32 getRandomCityIncome(bool capital = false);
	bool randomSignpostsEmpty() {return d_signposts->empty();}
	Glib::ustring getDynamicSignpost(Signpost *signpost);
	int getNumSignposts() {return d_signposts->size();}
	Keeper * getRandomRuinKeeper(Vector<int> pos);


	//! Based on the difficulty, get how much gold each player should
	//start with.
	void getBaseGold (int difficulty, int *base_gold);

	int adjustBaseGold (int base_gold);

        sigc::signal<std::list<Glib::ustring> > signal_collect_city_names ()
          {return collect_city_names;}
        sigc::signal<std::list<Glib::ustring> > signal_collect_temple_names ()
          {return collect_temple_names;}
        sigc::signal<std::list<Glib::ustring> > signal_collect_ruin_names ()
          {return collect_ruin_names;}
        sigc::signal<std::list<Glib::ustring> > signal_collect_signposts ()
          {return collect_signposts;}
	//! Get the default player name for the given shield color.
	static Glib::ustring getPlayerName(Shield::Color id);

        void cleanup();
    protected:
        //the namelists
	NameList *d_citynames;
	NameList *d_signposts;
	NameList *d_templenames;
	NameList *d_ruinnames;

    private:

        sigc::signal<std::list<Glib::ustring> > collect_city_names;
        sigc::signal<std::list<Glib::ustring> > collect_temple_names;
        sigc::signal<std::list<Glib::ustring> > collect_ruin_names;
        sigc::signal<std::list<Glib::ustring> > collect_signposts;
	Glib::ustring popRandomListName(std::vector<Glib::ustring>& list);
        Glib::ustring get_direction(int xdir, int ydir);
	bool loadNames(std::vector<Glib::ustring>& list, std::ifstream& file);
};

#endif  //CREATE_SCENARIO_RANDOMIZE_H
