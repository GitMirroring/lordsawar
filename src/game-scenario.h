// Copyright (C) 2000, 2001, 2002, 2003 Michael Bartl
// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2004, 2006 Andrea Paternesi
// Copyright (C) 2007, 2008, 2011, 2014, 2015, 2020, 2021 Ben Asselstine
// Copyright (C) 2007 Ole Laursen
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
#ifndef GAME_SCENARIO_H
#define GAME_SCENARIO_H

#include <list>
#include <sigc++/trackable.h>
#include "GameScenarioOptions.h"

#include "tarfile.h"
class XML_Helper;
class Tar_Helper;

//! A class to hold several scenario options.
/**
 * This class has two functions. On the one hand side, it holds some data
 * about the current scenario being played (such as the name), on the other
 * hand it has a kind of supervisor function. Loading and saving works in
 * a hierarchical way with superior objects (such as the playerlist) saving
 * their data and then telling inferior objects (such as players) to save
 * their data as well. GameScenario is kind of the root of the saving or
 * loading process. For more information about the saving procedure, have
 * a look at XML_Helper.
 */

class GameScenario: public GameScenarioOptions, public TarFile
{
    public:

	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag;
	static Glib::ustring d_top_tag;

        enum PlayMode
	  {
	    HOTSEAT = 0,
	    NETWORKED = 1
	  };

	static Glib::ustring playModeToString(const GameScenario::PlayMode mode);
	static GameScenario::PlayMode playModeFromString(const Glib::ustring str);

        //! Makes a copy of the GameScenario.
        GameScenario * copy () {return new GameScenario (*this);}

        /** Initializes an "empty" scenario
          *
          * @param name     the name of the scenario
          * @param comment  the comment for the scenario
          * @param turnmode the turnmode (see NextTurn for description)
          */
        GameScenario(Glib::ustring name, Glib::ustring comment,
		     GameScenario::PlayMode playmode = GameScenario::HOTSEAT);

        /** Load the game scenario using a specified save game
          *
          * @param savegame     the full name of the saved-game to load
          * @param broken       set to true if something goes wrong
          */
        GameScenario(Glib::ustring savegame, bool& broken);

        //! Copy constructor
        GameScenario (const GameScenario &g, bool unique);
    
        //! Destructor
        ~GameScenario();

        //! Returns the number of the current turn.
        unsigned int getRound() const {return s_round;}

	Glib::ustring getId() const {return d_id;};

	void setNewRandomId();

        void setUnique (bool unique) {d_unique = unique;}

        //! Returns the name of the scenario.
        Glib::ustring getName() const;

        //! Returns the comment for the scenario.
        Glib::ustring getComment() const;

        //! Returns the copyright for the scenario.
        Glib::ustring getCopyright() const {return d_copyright; };

        //! Returns the license of the scenario.
        Glib::ustring getLicense() const {return d_license;};

        //! Increments the turn number and does an autosave. Called by NextTurn
        //! via a signal.
        void nextRound();

        //! Sets the name of the scenario.
        void setName(Glib::ustring name) {d_name = name;}

        //! Sets the description of the scenario.
        void setComment(Glib::ustring comment) {d_comment = comment;}

        //! Sets the copyright of the scenario.
        void setCopyright(Glib::ustring copy) {d_copyright = copy;}

        //! Sets the license of the scenario.
        void setLicense(Glib::ustring license) {d_license = license;}

        /** Saves the game. See XML_Helper for further explanations.
          *
          * @param filename     the full name of the save game file
          * @return true if all went well, false otherwise
          */
        bool saveGame(Glib::ustring filename, Glib::ustring extension = SAVE_EXT) const;
        bool dump (Glib::ustring filename, Glib::ustring extension = SAVE_EXT) const;
        bool loadWithHelper(XML_Helper &helper);
        bool saveWithHelper(XML_Helper &helper) const;

	guint32 getPlayMode() const {return d_playmode;};
	void setPlayMode(GameScenario::PlayMode mode) {d_playmode = mode;};

	bool validate(std::list<Glib::ustring> &errors, std::list<Glib::ustring> &warnings);

        void clean_tmp_dir() const;

	void initialize(GameParameters g);

	static GameParameters loadGameParameters(Glib::ustring filename, bool &broken);

	static PlayMode loadPlayMode(Glib::ustring filename, bool &broken);

	static void loadDetails(Glib::ustring filename, bool &broken, guint32 &player_count, guint32 &city_count, Glib::ustring &name, Glib::ustring &comment, Glib::ustring &id);

        static Glib::ustring generate_guid();

        static void cleanup();

        static bool upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version);
        static void support_backward_compatibility();

	void inhibitAutosaveRemoval(bool i) {inhibit_autosave_removal = i;}

        //for progress bar
        static sigc::signal<void> load_tick;
        static sigc::signal<void> load_finish;

    private:
	  /** Callback function for loading a game. See XML_Helper for details.
	   *
	   * @param tag      the tag name
	   * @param helper   the helper for parsing the save game file
	   * @return true if all went well, false otherwise.
	   */
	  bool load(Glib::ustring tag, XML_Helper* helper);
	  void quickStartEvenlyDivided();
	  void quickStartAIHeadStart();
	  bool setupFog(bool hidden_map);
	  bool setupCities(GameParameters::QuickStartPolicy quick_start, GameParameters::BuildProductionMode build);
	  bool setupRewards(bool hidden_map, int difficulty);
	  bool setupMapRewards();
	  bool setupRuinOccupants();
	  bool setupRuinRewards(int difficulty);
	  bool setupItemRewards();
	  bool setupStacks(bool hidden_map);
	  void setupDiplomacy(bool diplomacy);
	  bool autoSave();

	  bool loadArmysets(Tar_Helper *t);
	  bool loadTilesets(Tar_Helper *t);
	  bool loadCitysets(Tar_Helper *t);
	  bool loadShieldsets(Tar_Helper *t);
        
          std::vector<Glib::ustring> getSetFiles () const;

	  // DATA
	  Glib::ustring d_name;
	  Glib::ustring d_comment;
	  Glib::ustring d_copyright;
	  Glib::ustring d_license;
	  guint32 d_playmode;
	  Glib::ustring d_id; //globally unique id identifying the scenario
	  bool inhibit_autosave_removal;
          Glib::ustring loaded_game_filename;
          bool d_unique; //whether we're the main singleton controller or not
};

#endif // GAME_SCENARIO_H

// End of file
