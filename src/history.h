//  Copyright (C) 2007, 2008, 2011, 2014, 2015, 2026 Ben Asselstine
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
#ifndef HISTORY_H
#define HISTORY_H

#include <sigc++/trackable.h>

#include <glibmm.h>
class XML_Helper;

class Hero;
class City;
class Ruin;
class Item;
class Player;

//! A permanent record of an accomplishment during gameplay.
/** 
 * The purpose of the history classes is to keep track about what a 
 *  player has accomplished.  This list is retained for the duration of 
 *  the game.
 * 
 */
class History
{
    public:
	//! The xml tag of this object in a saved-game file.
	static Glib::ustring d_tag; 

	//! A History can be one of the following kinds.
        enum Type {
	  //! The player started a turn.
	  START_TURN = 1,
	  //! The player has searched a Ruin and found a sage.
	  FOUND_SAGE = 2,
	  //! The player has accrued a certain amount of gold in the treasury.
	  GOLD_TOTAL = 3,
	  //! A hero has emerged.
	  HERO_EMERGES = 4,
	  //! A City has been taken.
	  CITY_WON = 5,
	  //! A City has been razed.
	  CITY_RAZED = 6,
	  //! A Hero has inititiated a Quest.
	  HERO_QUEST_STARTED = 7,
	  //! A Hero has completed a Quest.
	  HERO_QUEST_COMPLETED = 8,
	  //! A Hero was killed in battle at a City.
	  HERO_KILLED_IN_CITY = 9,
	  //! A Hero was killed in battle in the field.
	  HERO_KILLED_IN_BATTLE = 10,
	  //! A Hero was killed searching a Ruin.
	  HERO_KILLED_SEARCHING = 11,
	  //! A Hero was involved in taking a City.
	  HERO_CITY_WON = 12,
	  //! The player has this score.
	  SCORE = 13,
	  //! The player has been utterly defeated.
	  PLAYER_VANQUISHED = 14,
	  //! The player has achieved peace with an opponent.
	  DIPLOMATIC_PEACE = 15,
	  //! The player has started a war with an opponent.
	  DIPLOMATIC_WAR = 16,
	  //! The player has been treacherous towards an opponent.
	  DIPLOMATIC_TREACHERY = 17,
	  //! A Hero finds some powerful allies.
	  HERO_FINDS_ALLIES = 18,
	  //! The player has finished a turn.
	  END_TURN = 19,
          //! The player has explored a ruin.
          HERO_RUIN_EXPLORED = 20,
          //! The player has been told of the location of a hidden ruin.
          HERO_REWARD_RUIN = 21,
          //! The player has used an item
          USE_ITEM = 22,
          //! The hero has had a quest expire
	  HERO_QUEST_EXPIRED = 23
        };
	static Glib::ustring historyTypeToString(const History::Type type);
	static History::Type historyTypeFromString(const Glib::ustring str);
                
	//! Default constructor.
        History(Type type);

	//! Loading from XML constructor.
	History (XML_Helper *helper);

	//! Destructor.
        virtual ~History() {};

        //! Returns debug information. Needs to be overwritten by derivatives
        virtual Glib::ustring dump() const = 0;

        /** 
	 * static load function (see XML_Helper)
         * 
         * Whenever a History item is loaded, this function is called. It
         * examines the stored History::Type and calls the constructor of 
	 * the appropriate History class.
         *
         * @param helper       The opened saved-game file to read from.
         */
	//! Load a History from an opened saved-game file.
        static History* handle_load(XML_Helper* helper);

        //! Copies a history into a new one.
        static History* copy(const History* a);

        //! Returns the id which identifies the type of History event.
        Type getType() const {return d_type;}
        
	bool save(XML_Helper* helper) const;
	bool saveContents(XML_Helper* helper) const;

    protected:
        virtual bool doSave(XML_Helper* helper) const = 0;
        Type d_type;
};

//-----------------------------------------------------------------------------

//! A permanent record of a player starting a turn.
class History_StartTurn : public History
{
    public:
	//! Default constructor.
        History_StartTurn();
	//! Copy constructor.
	History_StartTurn(const History_StartTurn &history);
	//! Load the historical event from an opened saved-game file.
        History_StartTurn(XML_Helper* helper);
	//! Destructor.
        ~History_StartTurn() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

    private:
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero searching a Ruin and finding a sage.
class History_FoundSage : public History
{
    public:
	//! Default constructor.
        History_FoundSage(Hero *hero);
	//! Copy constructor.
	History_FoundSage(const History_FoundSage &history);
	//! Load the historical event from an opened saved-game file.
        History_FoundSage(XML_Helper* helper);
	//! Destructor.
        ~History_FoundSage() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the name of the Hero who found the sage.
	Glib::ustring getHeroName() const {return d_hero;}
    
    private:
	//! The name of the Hero.
	Glib::ustring d_hero;
};

//-----------------------------------------------------------------------------

//! A permanent record of the amount of gold pieces a player has.
class History_GoldTotal : public History
{
    public:
	//! Default constructor.
        History_GoldTotal(int gold);
	//! Copy constructor.
	History_GoldTotal(const History_GoldTotal &history);
	//! Load the historical event from an opened saved-game file.
        History_GoldTotal(XML_Helper* helper);
	//! Destructor.
        ~History_GoldTotal() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the amount of gold associated with this event.
	int getGold() const {return d_gold;}
    
    private:
	//! The amount of gold pieces the player has at a point in time.
        int d_gold;
};

//-----------------------------------------------------------------------------

//! A permanent record of a new Hero emerging in a City.
class History_HeroEmerges : public History
{
    public:
	//! Default constructor.
        History_HeroEmerges(Hero *hero, City *city);
	//! Copy constructor.
	History_HeroEmerges(const History_HeroEmerges &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroEmerges(XML_Helper* helper);
	//! Destructor.
        ~History_HeroEmerges() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the name of the Hero who appeared.
	Glib::ustring getHeroName() const {return d_hero;}

	guint32 getHeroId() const {return d_hero_id;};

	//! Get the name of the City where the Hero has emerged.
	Glib::ustring getCityName() const {return d_city;}
    
    private:
	//! The name of the Hero who emerged.
	Glib::ustring d_hero;

	//! The id of the hero
	guint32 d_hero_id;

	//! The name of the City where the Hero emerged.
	Glib::ustring d_city;
};

//-----------------------------------------------------------------------------

//! A permanent record of an enemy city being defeated.
class History_CityWon : public History
{
    public:
	//! Default constructor.
        History_CityWon(City *city);
	//! Copy constructor.
	History_CityWon(const History_CityWon &history);
	//! Load the historical event from an opened saved-game file.
        History_CityWon(XML_Helper* helper);
	//! Destructor.
        ~History_CityWon() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the Id of the City object that was defeated.
	guint32 getCityId() const {return d_city;}
    
    private:
	//! The Id of the City object that was defeated.
	guint32 d_city;
};

//-----------------------------------------------------------------------------

//! A permanent record of an enemy city being defeated by a Hero.
class History_HeroCityWon: public History
{
    public:
	//! Default constructor.
        History_HeroCityWon(City *c, Hero *h);
	//! Copy constructor.
	History_HeroCityWon(const History_HeroCityWon &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroCityWon(XML_Helper* helper);
	//! Destructor.
        ~History_HeroCityWon() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the name of the Hero who conquered the City.
	Glib::ustring getHeroName() const {return d_hero;}

	//! Get the name of the City that was conquered.
	Glib::ustring getCityName() const {return d_city;}
    
    private:
	//! The name of the Hero who helped in conquering the City.
	Glib::ustring d_hero;

	//! The name of the City that was conquered.
	Glib::ustring d_city;
};

//-----------------------------------------------------------------------------

//! A permanent record of an enemy city being razed.
class History_CityRazed : public History
{
    public:
	//! Default constructor.
        History_CityRazed(City *c);
	//! Copy constructor.
	History_CityRazed(const History_CityRazed &history);
	//! Load the historical event from an opened saved-game file.
        History_CityRazed(XML_Helper* helper);
	//! Destructor.
        ~History_CityRazed() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the Id of the City object that was razed.
	guint32 getCityId() const {return d_city;}
    
    private:
	//! The Id of the City that was razed.
        guint32 d_city;
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero initiating a Quest.
class History_HeroQuestStarted : public History
{
    public:
	//! Default constructor.
        History_HeroQuestStarted(Hero *h);
	//! Copy constructor.
	History_HeroQuestStarted(const History_HeroQuestStarted &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroQuestStarted(XML_Helper* helper);
	//! Destructor.
        ~History_HeroQuestStarted() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the name of the Hero who started a Quest.
	Glib::ustring getHeroName() const {return d_hero;}
    
    private:
	//! The name of the Hero who started the Quest.
	Glib::ustring d_hero;
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero completing a Quest.
class History_HeroQuestCompleted: public History
{
    public:
	//! Default constructor.
        History_HeroQuestCompleted(Hero *h);
	//! Copy constructor.
	History_HeroQuestCompleted(const History_HeroQuestCompleted &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroQuestCompleted(XML_Helper* helper);
	//! Destructor.
        ~History_HeroQuestCompleted() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the name of the Hero who finished a Quest.
	Glib::ustring getHeroName() const {return d_hero;}
    
    private:
	//! The name of the Hero who completed the Quest.
	Glib::ustring d_hero;
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero killed in the defense or attack of a City.
class History_HeroKilledInCity : public History
{
    public:
	//! Default constructor.
        History_HeroKilledInCity(Hero *h, City *c);
	//! Copy constructor.
	History_HeroKilledInCity(const History_HeroKilledInCity &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroKilledInCity(XML_Helper* helper);
	//! Destructor.
        ~History_HeroKilledInCity() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the name of the Hero who died.
	Glib::ustring getHeroName() const {return d_hero;}

	//! Get the name of the City where the Hero died.
	Glib::ustring getCityName() const {return d_city;}
    
    private:
	//! Get the name of the Hero who was killed.
	Glib::ustring d_hero;

	//! Get the name of the City where the Hero was killed.
	Glib::ustring d_city;
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero killed in battle outside of a City.
class History_HeroKilledInBattle: public History
{
    public:
	//! Default constructor.
        History_HeroKilledInBattle(Hero *h);
	//! Copy constructor.
	History_HeroKilledInBattle(const History_HeroKilledInBattle &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroKilledInBattle(XML_Helper* helper);
	//! Destructor.
        ~History_HeroKilledInBattle() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the name of the Hero who died in battle outside of a City.
	Glib::ustring getHeroName() const {return d_hero;}
    
    private:
	//! The name of the Hero who died in battle outside of a City.
	Glib::ustring d_hero;
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero killed while searching a Ruin.
class History_HeroKilledSearching: public History
{
    public:
	//! Default constructor.
        History_HeroKilledSearching(Hero *h);
	//! Copy constructor.
	History_HeroKilledSearching(const History_HeroKilledSearching &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroKilledSearching(XML_Helper* helper);
	//! Destructor.
        ~History_HeroKilledSearching() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the name of the Hero who died while searching a Ruin.
	Glib::ustring getHeroName() const {return d_hero;}
    
    private:
	//! The name of the Hero who died while searching a Ruin.
	Glib::ustring d_hero;
};

//-----------------------------------------------------------------------------

//! A permanent record of the player's score.
class History_Score: public History
{
    public:
	//! Default constructor.
        History_Score(guint32 score);
	//! Copy constructor.
	History_Score(const History_Score &history);
	//! Load the historical event from an opened saved-game file.
        History_Score(XML_Helper* helper);
	//! Destructor.
        ~History_Score() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the player's score for this turn.
	guint32 getScore() const {return d_score;}
    
    private:
	//! The player's score.
        int d_score;
};

//-----------------------------------------------------------------------------

//! A permanent record of the player being utterly defeated.
class History_PlayerVanquished: public History
{
    public:
	//! Default constructor.
        History_PlayerVanquished();
	//! Copy constructor.
	History_PlayerVanquished(const History_PlayerVanquished &history);
	//! Load the historical event from an opened saved-game file.
        History_PlayerVanquished(XML_Helper* helper);
	//! Destructor.
        ~History_PlayerVanquished() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

};

//-----------------------------------------------------------------------------

//! A permanent record of the player making peace with an opponent.
class History_DiplomacyPeace : public History
{
    public:
	//! Default constructor.
        History_DiplomacyPeace(Player *p);
	//! Copy constructor.
	History_DiplomacyPeace(const History_DiplomacyPeace &history);
	//! Load the historical event from an opened saved-game file.
        History_DiplomacyPeace(XML_Helper* helper);
	//! Destructor.
        ~History_DiplomacyPeace() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the Id of the Player object we are at peace with.
	guint32 getOpponentId() const {return d_opponent_id;}
    
    private:
	//! The Id of the Player object we are at peace with.
	guint32 d_opponent_id;
};

//-----------------------------------------------------------------------------

//! A permanent record of the player going to war with an opponent.
class History_DiplomacyWar: public History
{
    public:
	//! Default constructor.
        History_DiplomacyWar(Player *p);
	//! Copy constructor.
	History_DiplomacyWar(const History_DiplomacyWar &history);
	//! Load the historical event from an opened saved-game file.
        History_DiplomacyWar(XML_Helper* helper);
	//! Destructor.
        ~History_DiplomacyWar() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the Id of the Player object we are at war with.
	guint32 getOpponentId() const {return d_opponent_id;}
    
    private:
	// The Id of the Player object we are at war with.
	guint32 d_opponent_id;
};

//-----------------------------------------------------------------------------

//! A permanent record of the player being treacherous to an opponent.
class History_DiplomacyTreachery: public History
{
    public:
	//! Default constructor.
        History_DiplomacyTreachery(Player *p);
	//! Copy constructor.
	History_DiplomacyTreachery(const History_DiplomacyTreachery &history);
	//! Load the historical event from an opened saved-game file.
        History_DiplomacyTreachery(XML_Helper* helper);
	//! Destructor.
        ~History_DiplomacyTreachery() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the Id of the Player object that we peformed treachery on.
	guint32 getOpponentId() const {return d_opponent_id;}
    
    private:
	//! The Id of the Player object that we peformed treachery on.
	guint32 d_opponent_id;
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero finding powerful allies.
class History_HeroFindsAllies : public History
{
    public:
	//! Default constructor.
        History_HeroFindsAllies(Hero *h);
	//! Copy constructor.
	History_HeroFindsAllies(const History_HeroFindsAllies &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroFindsAllies(XML_Helper* helper);
	//! Destructor.
        ~History_HeroFindsAllies() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the name of the Hero who found powerful allies.
	Glib::ustring getHeroName() const {return d_hero;}
    
    private:
	//! The name of the Hero who found powerful allies at a Ruin.
	Glib::ustring d_hero;
};

//-----------------------------------------------------------------------------
//! A permanent record of a player ending a turn.
class History_EndTurn : public History
{
    public:
	//! Default constructor.
	History_EndTurn();
	//! Copy constructor.
	History_EndTurn(const History_EndTurn &history);
	//! Load the historical event from an opened saved-game file.
	History_EndTurn(XML_Helper* helper);
	//! Destructor.
	~History_EndTurn() {};

	//! Return some debug information about this historical event.
	Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

    private:
};
//-----------------------------------------------------------------------------

//! A permanent record of a ruin being successfully searched by a Hero.
class History_HeroRuinExplored: public History
{
    public:
	//! Default constructor.
        History_HeroRuinExplored(Hero *h, Ruin *r);
	//! Copy constructor.
	History_HeroRuinExplored(const History_HeroRuinExplored &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroRuinExplored(XML_Helper* helper);
	//! Destructor.
        ~History_HeroRuinExplored() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the name of the Hero who searched the Ruin.
	Glib::ustring getHeroName() const {return d_hero;}

	//! Get the id of the Ruin that was searched.
	guint32 getRuinId() const {return d_ruin;}
    
    private:
	//! The name of the Hero who explored the Ruin.
	Glib::ustring d_hero;

	//! The id of the Ruin that was searched.
	guint32 d_ruin;
};


//-----------------------------------------------------------------------------

//! A permanent record of the location of a ruin being given to a Hero.
class History_HeroRewardRuin: public History
{
    public:
	//! Default constructor.
        History_HeroRewardRuin(Hero *h, Ruin *r);
	//! Copy constructor.
	History_HeroRewardRuin(const History_HeroRewardRuin&history);
	//! Load the historical event from an opened saved-game file.
        History_HeroRewardRuin(XML_Helper* helper);
	//! Destructor.
        ~History_HeroRewardRuin() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the name of the Hero who was given the location of the Ruin.
	Glib::ustring getHeroName() const {return d_hero;}

	//! Get the id of the Ruin that was exposed.
	guint32 getRuinId() const {return d_ruin;}
    
    private:
	//! The name of the Hero who was told the location of the Ruin.
	Glib::ustring d_hero;

	//! The id of the Ruin that was exposed.
	guint32 d_ruin;
};

//-----------------------------------------------------------------------------

//! A permanent record of the player using an item
class History_HeroUseItem: public History
{
    public:
	//! Default constructor.
        History_HeroUseItem(Hero *h, Item *i, Player *opponent, 
                            City *friendly_city, City *enemy_city, 
                            City *neutral_city, City *c);
	//! Copy constructor.
	History_HeroUseItem(const History_HeroUseItem &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroUseItem(XML_Helper* helper);
	//! Destructor.
        ~History_HeroUseItem() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

        //! Get the name of the hero that used the object.
        Glib::ustring getHeroName() const {return d_hero_name;}
        
        //! Get the name of the item that was used by the hero.
        Glib::ustring getItemName() const {return d_item_name;}

        //! Get the reported capabilities of the item.
        guint32 getItemBonus() const {return d_item_bonus;};

	//! Get the Id of the Player object that we used the item on.
	guint32 getOpponentId() const {return d_opponent_id;};
        guint32 getFriendlyCityId() const {return d_friendly_city_id;};
        guint32 getEnemyCityId() const {return d_enemy_city_id;};
        guint32 getNeutralCityId() const {return d_neutral_city_id;};
        guint32 getCityId() const {return d_city_id;};
    
    private:

        //! The name of the hero using an object.
        Glib::ustring d_hero_name;
        
        //! The name of the item that was used.
        Glib::ustring d_item_name;

        //! The kind of item.
        guint32 d_item_bonus;

	//! The Id of the Player object that we peformed treachery on.
        /**
         * Whether or not the item is used against the player is a function
         * of what kind of item it is.  As a result this field may sometimes 
         * be 0, but not used against the white player.
         */
	guint32 d_opponent_id;

        guint32 d_friendly_city_id;
        guint32 d_enemy_city_id;
        guint32 d_neutral_city_id;
        guint32 d_city_id;
};

//-----------------------------------------------------------------------------

//! A permanent record of a Hero having a quest expire.
class History_HeroQuestExpired: public History
{
    public:
	//! Default constructor.
        History_HeroQuestExpired(Hero *h);
	//! Copy constructor.
	History_HeroQuestExpired(const History_HeroQuestExpired &history);
	//! Load the historical event from an opened saved-game file.
        History_HeroQuestExpired(XML_Helper* helper);
	//! Destructor.
        ~History_HeroQuestExpired() {};

	//! Return some debug information about this historical event.
        Glib::ustring dump() const;

	//! Save the historical event to an opened saved-game file.
        virtual bool doSave(XML_Helper* helper) const;

	//! Get the name of the Hero who had a Quest expire.
	Glib::ustring getHeroName() const {return d_hero;}
    
    private:
	//! The name of the Hero who had a quest expire.
	Glib::ustring d_hero;
};

#endif
