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
#ifndef ARMYSET_EDITOR_ACTIONS_H
#define ARMYSET_EDITOR_ACTIONS_H

#include <gtkmm.h>
#include <sigc++/trackable.h>

#include "armybase.h"
#include "hero.h"
#include "defs.h"
#include "undo-action.h"
class Armyset;

//! A record of an event in the armyset editor
/** 
 * The purpose of these classes is to implement undo/redo in the armyset
 * editor.
 * The idea is to record changes to the model.
 */

class ArmySetEditorAction: public UndoAction
{
    public:

	//! An ArmySet Editor Action can be one of the following kinds.
        enum Type {
	        /** Modify description/copyright/license. */
                CHANGE_PROPERTIES = 1,
                /** A army, ship, bag, etc. image has been set */
                ADD_IMAGE = 2,
                /** An image has been cleared*/
                CLEAR_IMAGE = 3,
                /** The name of an army has been changed */
                NAME = 4,
                /** The description of an army has been changed */
                DESCRIPTION = 5,
                /** The white army images are copied down to other colours */
                COPY_WHITE_DOWN = 6,
                /** An army has had its position changed in the set */
                REORDER = 7,
                /** A new army has been added to the set */
                ADD_ARMY = 8,
                /** An army has been deleted from the set */
                REMOVE_ARMY = 9,
                /** An army bonus has been modified (+1 str in hills, etc.) */
                BONUS = 10,
                /** An army's number of turns has been modified */
                TURNS = 11,
                /** An army's cost has been modified */
                COST = 12,
                /** An army's upkeep has been modified */
                UPKEEP = 13,
                /** An army's new cost has been modified */
                NEW_COST = 14,
                /** An army's stat has been modified (strength, exp, etc.) */
                STAT = 15,
                /** The army's ID has been modified */
                ID = 16,
                /** Whether or not it is awarded at ruins has been modified */
                RUIN_AWARD = 17,
                /** Whether or not it defends ruins has been modified */
                DEFENDS_RUIN = 18,
                /** The army's hero type has been modified */
                HERO = 19,
                /** The army's ability to fly has been modified */
                FLY = 20,
                /** The army's speed through forests has been modified */
                FASTER_IN_FORESTS = 21,
                /** The army's speed through marshland has been modified */
                FASTER_IN_MARSHES = 22,
                /** The army's speed through hills has been modified */
                FASTER_IN_HILLS = 23,
                /** The army's speed over mountains has been modified */
                FASTER_IN_MOUNTAINS = 24,
                /** The army selector has been modified */
                SELECTOR = 25,
                /** The army's experience points has been modified */
                EXP = 26,

        };

	//! Default constructor.
        ArmySetEditorAction(Type type, UndoAction::AggregateType aggregate = UndoAction::AGGREGATE_NONE);

        Type getType() const {return d_type;}

    protected:

        Type d_type;
};

//-----------------------------------------------------------------------------

//! A record of the armyset's properties changing in the editor.
/**
 * The purpose of the ArmySetEditorAction_Properties class is to record
 * when an armyset's name, description, copyright, license, and tilesize have
 * changed.
 */
class ArmySetEditorAction_Properties: public ArmySetEditorAction
{
    public:
	//! Make a new change properties action
	/**
         * Populate the properties action with the new name, description,
         * copyright, license text, and tile size.
         */
        ArmySetEditorAction_Properties (Glib::ustring n, Glib::ustring d, Glib::ustring c, Glib::ustring l, guint32 ts);
	//! Destroy a change properties action.
        ~ArmySetEditorAction_Properties () {};

        Glib::ustring getActionName () const {return _("Properties");}

        Glib::ustring getName () {return d_name;}
        Glib::ustring getDescription () {return d_desc;}
        Glib::ustring getCopyright () {return d_copyright;}
        Glib::ustring getLicense () {return d_license;}
        guint32 getTileSize () {return d_tile_size;}

    private:
        Glib::ustring d_name;
        Glib::ustring d_desc;
        Glib::ustring d_copyright;
        Glib::ustring d_license;
        guint32 d_tile_size;
};

//-----------------------------------------------------------------------------

//! A helper class for events that require saving the whole armyset

/**
 * Several actions require saving the whole tar file because it's the
 * easiest way to implement undo/redo.
 */
class ArmySetEditorAction_Save: public ArmySetEditorAction
{
    public:
        ArmySetEditorAction_Save (Armyset *s, Type t);
        ~ArmySetEditorAction_Save ();

        Glib::ustring getArmysetFilename () const {return d_filename;}
        Armyset *getArmyset () const {return d_armyset;}
    private:
        Glib::ustring d_filename;
        Armyset *d_armyset;
};

//-----------------------------------------------------------------------------

//! A record of an armyset image being added or replaced
/**
 * The purpose of the ArmySetEditorAction_AddImage class is to record
 * when we select a new file.
 *
 * We take a copy of the whole armyset.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class ArmySetEditorAction_AddImage: public ArmySetEditorAction_Save
{
    public:
	//! Make a new add-file action
	/**
         * Populate the add image action with the armyset.
         */
        ArmySetEditorAction_AddImage (Armyset *a)
          :ArmySetEditorAction_Save (a, ArmySetEditorAction::ADD_IMAGE) {};
	//! Destroy an add-image action, and delete the file.
        ~ArmySetEditorAction_AddImage () {};

        Glib::ustring getActionName () const {return _("Add Image");}
};

//-----------------------------------------------------------------------------

//! A record of a armyset image being cleared
/**
 * The purpose of the ArmySetEditorAction_ClearImage class is to record
 * when we disassociate an image file with an army, ship, bag, etc.
 *
 * We take a copy of the whole armyset.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class ArmySetEditorAction_ClearImage: public ArmySetEditorAction_Save
{
    public:
	//! Make a new clear-file action
	/**
         * Populate the clear image action with the armyset.
         */
        ArmySetEditorAction_ClearImage (Armyset *a)
          :ArmySetEditorAction_Save (a, ArmySetEditorAction::CLEAR_IMAGE) {};
	//! Destroy an clear-image action, and delete the file.
        ~ArmySetEditorAction_ClearImage () {};

        Glib::ustring getActionName () const {return _("Clear Image");}
};

//-----------------------------------------------------------------------------

//! A helper class for events that require referencing the army's place
//in the set. this is not the id, and equates to the position in the treeview.

class ArmySetEditorAction_ArmyIndex: public ArmySetEditorAction
{
    public:
        ArmySetEditorAction_ArmyIndex (Type t, guint32 i, bool agg = false)
          : ArmySetEditorAction (t, agg ? UndoAction::AGGREGATE_DELAY : UndoAction::AGGREGATE_NONE), d_index (i) {}
        ~ArmySetEditorAction_ArmyIndex () {}

        guint32 getIndex () {return d_index;}
    private:
        guint32 d_index;
};
//-----------------------------------------------------------------------------

//! A record of an army's name being changed
/**
 * The purpose of the ArmySetEditorAction_Name class is to record
 * when we change the army's name.  This happens letter by letter.
 *
 */
class ArmySetEditorAction_Name: public ArmySetEditorAction_ArmyIndex
{
    public:
	//! Make a new name action
	/**
         * Populate the action with the index of the army type whose name
         * we're modifying, the name, and the position of the cursor in the
         * entry.
         */
        ArmySetEditorAction_Name (guint32 i, Glib::ustring n, int p)
          : ArmySetEditorAction_ArmyIndex (NAME, i, true), d_name (n),
         d_cursor_pos (p) {};
	//! Destroy a name action.
        ~ArmySetEditorAction_Name () {};

        Glib::ustring getActionName () const {return _("Name");}
        Glib::ustring getName () {return d_name;}
        int getCursorPosition () {return d_cursor_pos;}

    private:
        Glib::ustring d_name;
        int d_cursor_pos;
};

//-----------------------------------------------------------------------------

//! A record of an army's description being changed
/**
 * The purpose of the ArmySetEditorAction_Description class is to record
 * when we change the army's description.  This happens letter by letter.
 *
 */
class ArmySetEditorAction_Description: public ArmySetEditorAction_ArmyIndex
{
    public:
	//! Make a new description action
	/**
         * Populate the action with the description of the army type.
         */
        ArmySetEditorAction_Description (guint32 i, Glib::ustring d)
          : ArmySetEditorAction_ArmyIndex (DESCRIPTION, i, true),
          d_description (d) {};
	//! Destroy a description action.
        ~ArmySetEditorAction_Description () {}

        Glib::ustring getActionName () const {return _("Description");}

        Glib::ustring getDescription () const {return d_description;}

    private:
        Glib::ustring d_description;
};

//-----------------------------------------------------------------------------

//! A record of the armyset's images changing in the editor en masse.
/**
 * The purpose of the ArmySetEditorAction_WhiteDown class is to record
 * when a army's white images are copied down to the other colours.
 *
 * We take a copy of the whole armyset to get all of the images in one
 * go.  Our copy is a file on disk and is deleted when this class is 
 * destroyed.
 */
class ArmySetEditorAction_WhiteDown: public ArmySetEditorAction_Save
{
    public:
	//! Make a new copy white down action
	/**
         * Populate the white down action with the armyset.
         */
        ArmySetEditorAction_WhiteDown (Armyset *a)
          :ArmySetEditorAction_Save(a, ArmySetEditorAction::COPY_WHITE_DOWN){};
	//! Destroy a white down action, and delete the file.
        ~ArmySetEditorAction_WhiteDown () {};

        Glib::ustring getActionName () const {return _("Copy White Army");}
};

//-----------------------------------------------------------------------------

//! A record of the armyset's order of army types changing
/**
 * The purpose of the ArmySetEditorAction_Reorder class is to record
 * when an army type moves up or down in the order of the set.
 *
 * We take a copy of the whole armyset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class ArmySetEditorAction_Reorder: public ArmySetEditorAction_Save
{
    public:
	//! Make a new reorder action
	/**
         * Populate the reorder action with the armyset.
         */
        ArmySetEditorAction_Reorder (Armyset *a)
          :ArmySetEditorAction_Save(a, ArmySetEditorAction::REORDER) {};
	//! Destroy a reorder action, and delete the file.
        ~ArmySetEditorAction_Reorder () {};

        Glib::ustring getActionName () const {return _("Reorder");}
};

//-----------------------------------------------------------------------------

//! A record of a new army type being added to the set
/**
 * The purpose of the ArmySetEditorAction_AddArmy class is to record
 * when a new army type is added to the set - it is completely empty.
 *
 * We take a copy of the whole armyset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class ArmySetEditorAction_AddArmy: public ArmySetEditorAction_Save
{
    public:
	//! Make a new add army action
	/**
         * Populate the add army action with the armyset.
         */
        ArmySetEditorAction_AddArmy (Armyset *a)
          :ArmySetEditorAction_Save(a, ArmySetEditorAction::ADD_ARMY) {};
	//! Destroy an add army action, and delete the file.
        ~ArmySetEditorAction_AddArmy () {};

        Glib::ustring getActionName () const {return _("Add Army");}
};

//-----------------------------------------------------------------------------

//! A record of an army type being deleted from the set
/**
 * The purpose of the ArmySetEditorAction_RemoveArmy class is to record
 * when an army type is removed to the set.
 *
 * We take a copy of the whole armyset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class ArmySetEditorAction_RemoveArmy: public ArmySetEditorAction_Save
{
    public:
	//! Make a new remove army action
	/**
         * Populate the remove army action with the armyset.
         */
        ArmySetEditorAction_RemoveArmy (Armyset *a)
          :ArmySetEditorAction_Save(a, ArmySetEditorAction::REMOVE_ARMY) {};
	//! Destroy an remove army action, and delete the file.
        ~ArmySetEditorAction_RemoveArmy () {};

        Glib::ustring getActionName () const {return _("Remove Army");}
};

//-----------------------------------------------------------------------------

//! A record of an army type's bonus being modified
/**
 * The purpose of the ArmySetEditorAction_Bonus class is to record
 * when an army type's bonus is modified. 
 * e.g. +1 in hills, +2 in city and so on.
 *
 */
class ArmySetEditorAction_Bonus : public ArmySetEditorAction_ArmyIndex
{
    public:
	//! Make a new bonus action 
	/**
         * Populate the bonus action with the type of bonus and the value.
         */
        ArmySetEditorAction_Bonus (guint32 i, ArmyBase::Bonus b, bool f)
          :ArmySetEditorAction_ArmyIndex (ArmySetEditorAction::BONUS, i),
          d_bonus (b), d_flag (f) {};
	//! Destroy a bonus action
        ~ArmySetEditorAction_Bonus () {};

        Glib::ustring getActionName () const
          {return ArmyBase::bonusFlagToFriendlyName (d_bonus);}

        ArmyBase::Bonus getBonusType () {return d_bonus;}
        bool getFlag () {return d_flag;}
    private:
        ArmyBase::Bonus d_bonus;
        bool d_flag;
};

//-----------------------------------------------------------------------------

//! A record of an army type's turns being modified
/**
 * The purpose of the ArmySetEditorAction_Turns class is to record
 * when an army type's turns is modified.   e.g. the number of turns it takes
 * to produce the army.
 */
class ArmySetEditorAction_Turns : public ArmySetEditorAction_ArmyIndex
{
    public:
	//! Make a new turns action 
	/**
         * Populate the turns action with the number of turns.
         */
        ArmySetEditorAction_Turns (guint32 i, guint32 t)
          :ArmySetEditorAction_ArmyIndex (ArmySetEditorAction::TURNS, i, true),
          d_turns (t) {}
	//! Destroy a turns action
        ~ArmySetEditorAction_Turns () {}

        Glib::ustring getActionName () const {return _("Turns");}

        guint32 getTurns () {return d_turns;}
    private:
        guint32 d_turns;
};

//-----------------------------------------------------------------------------

//! A record of an army type's cost being modified
/**
 * The purpose of the ArmySetEditorAction_Cost class is to record
 * when an army type's cost is modified.   e.g. the number of gold pieces
 * needed to instantiate this army type (e.g. one time production cost,
 * paid every time a new unit of this type is created.)
 */
class ArmySetEditorAction_Cost : public ArmySetEditorAction_ArmyIndex
{
    public:
	//! Make a new cost action 
	/**
         * Populate the cost action with the number of gold pieces.
         */
        ArmySetEditorAction_Cost (guint32 i, guint32 c)
          :ArmySetEditorAction_ArmyIndex (ArmySetEditorAction::COST, i, true),
          d_cost (c) {}
	//! Destroy a cost action
        ~ArmySetEditorAction_Cost () {}

        Glib::ustring getActionName () const {return _("Cost");}

        guint32 getCost () {return d_cost;}
    private:
        guint32 d_cost;
};

//-----------------------------------------------------------------------------

//! A record of an army type's upkeep being modified
/**
 * The purpose of the ArmySetEditorAction_Upkeep class is to record
 * when an army type's upkeep is modified.   e.g. the number of gold pieces
 * needed to pay for this army type to keep going every turn after it is
 * instantiated.
 */
class ArmySetEditorAction_Upkeep : public ArmySetEditorAction_ArmyIndex
{
    public:
	//! Make a new upkeep action 
	/**
         * Populate the upkeep action with the number of gold pieces.
         */
        ArmySetEditorAction_Upkeep (guint32 i, guint32 u)
          :ArmySetEditorAction_ArmyIndex (ArmySetEditorAction::UPKEEP, i, true),
          d_upkeep (u) {}
	//! Destroy a upkeep action
        ~ArmySetEditorAction_Upkeep () {}

        Glib::ustring getActionName () const {return _("Upkeep");}

        guint32 getUpkeep () {return d_upkeep;}
    private:
        guint32 d_upkeep;
};

//-----------------------------------------------------------------------------

//! A record of an army type's new cost being modified
/**
 * The purpose of the ArmySetEditorAction_NewCost class is to record
 * when an army type's new cost is modified.   e.g. the number of gold pieces
 * needed to make a city produce this kind of army.  a one-time expense.
 */
class ArmySetEditorAction_NewCost : public ArmySetEditorAction_ArmyIndex
{
    public:
	//! Make a new new-cost action 
	/**
         * Populate the new-cost action with the number of gold pieces.
         */
        ArmySetEditorAction_NewCost (guint32 i, guint32 c)
          :ArmySetEditorAction_ArmyIndex (ArmySetEditorAction::NEW_COST, i,
                                          true), d_cost (c) {}
	//! Destroy a new-cost action
        ~ArmySetEditorAction_NewCost () {}

        Glib::ustring getActionName () const {return _("New Cost");}

        guint32 getNewCost () {return d_cost;}
    private:
        guint32 d_cost;
};

//-----------------------------------------------------------------------------

//! A record of an army type's stat being modified
/**
 * The purpose of the ArmySetEditorAction_Stat class is to record
 * when an army type's stat is modified.  e.g. strength, moves, sight.
 *
 */
class ArmySetEditorAction_Stat : public ArmySetEditorAction_ArmyIndex
{
    public:
	//! Make a new stat action 
	/**
         * Populate the stat action with the type of stat and the value.
         */
        ArmySetEditorAction_Stat (guint32 i, ArmyBase::Stat s, guint32 v)
          :ArmySetEditorAction_ArmyIndex (ArmySetEditorAction::STAT, i, true),
          d_stat(s), d_value(v) {};
	//! Destroy a stat action
        ~ArmySetEditorAction_Stat () {};

        Glib::ustring getActionName () const
          {
            switch (d_stat)
              {
              case ArmyBase::STRENGTH:
                return _("Strength");
              case ArmyBase::MOVES:
                return _("Moves");
              case ArmyBase::SIGHT:
                return _("Sight");
              default:
                break;
              }
            return "";
          }

        ArmyBase::Stat getStatType () {return d_stat;}
        guint32 getValue () {return d_value;}
    private:
        ArmyBase::Stat d_stat;
        guint32 d_value;
};

//-----------------------------------------------------------------------------

//! A record of an army type's id being modified
/**
 * The purpose of the ArmySetEditorAction_Id class is to record
 * when an army type's ID is modified.
 */
class ArmySetEditorAction_Id : public ArmySetEditorAction_ArmyIndex
{
    public:
	//! Make a new id action 
	/**
         * Populate the id action with the numeric id.
         */
        ArmySetEditorAction_Id (guint32 i, guint32 id)
          :ArmySetEditorAction_ArmyIndex (ArmySetEditorAction::ID, i, true),
          d_id (id) {}
	//! Destroy an id action
        ~ArmySetEditorAction_Id () {}

        Glib::ustring getActionName () const {return _("Id");}

        guint32 getId () {return d_id;}
    private:
        guint32 d_id;
};

//-----------------------------------------------------------------------------

//! A record of an army type's ability to be awarded at a ruin being modified
/**
 * The purpose of the ArmySetEditorAction_RuinAward class is to record
 * when an army type is changed to make it a potential prize for searching a
 * ruin.  e.g. it can be one of the kinds of army types that show up as allies.
 */
class ArmySetEditorAction_RuinAward: public ArmySetEditorAction_ArmyIndex
{
    public:
	//! Make a new ruin-award action 
	/**
         * Populate the ruin-award action with the status of the army type.
         */
        ArmySetEditorAction_RuinAward (guint32 i, bool a)
          :ArmySetEditorAction_ArmyIndex (ArmySetEditorAction::RUIN_AWARD, i),
          d_award (a) {};
	//! Destroy a defends-ruin action
        ~ArmySetEditorAction_RuinAward () {};

        Glib::ustring getActionName () const {return _("Ruin Award");}

        bool getAward () {return d_award;}
    private:
        bool d_award;
};

//-----------------------------------------------------------------------------

//! A record of an army type's ability to defend a ruin being modified
/**
 * The purpose of the ArmySetEditorAction_DefendsRuins class is to record
 * when an army type is changed to make it defend a ruin or not. e.g. it is
 * a potential occupant/keeper of a ruin.
 *
 */
class ArmySetEditorAction_DefendsRuins: public ArmySetEditorAction_ArmyIndex
{
    public:
	//! Make a new defends-ruins  action 
	/**
         * Populate the defends-ruin action with the status of the army type.
         */
        ArmySetEditorAction_DefendsRuins (guint32 i, bool d)
          :ArmySetEditorAction_ArmyIndex (ArmySetEditorAction::DEFENDS_RUIN, i),
          d_defend (d) {};
	//! Destroy a defends-ruin action
        ~ArmySetEditorAction_DefendsRuins () {};

        Glib::ustring getActionName () const {return _("Defends Ruins");}

        bool getDefend () {return d_defend;}
    private:
        bool d_defend;
};

//-----------------------------------------------------------------------------

//! A record of an army type's hero state being modified
/**
 * The purpose of the ArmySetEditorAction_Hero class is to record
 * when an army type's hero status is changed.  e.g female, not a hero.
 */
class ArmySetEditorAction_Hero: public ArmySetEditorAction_ArmyIndex
{
    public:
	//! Make a new hero action 
	/**
         * Populate the hero action with the status of the army type.
         */
        ArmySetEditorAction_Hero (guint32 i, Hero::Gender h)
          :ArmySetEditorAction_ArmyIndex (ArmySetEditorAction::HERO, i),
          d_hero(h) {};
	//! Destroy a hero action
        ~ArmySetEditorAction_Hero () {};

        Glib::ustring getActionName () const {return _("Hero");}

        Hero::Gender getHero () {return d_hero;}
    private:
        Hero::Gender d_hero;
};

//-----------------------------------------------------------------------------

//! A helper class for faster properties changing in the editor.
class ArmySetEditorAction_Faster: public ArmySetEditorAction_ArmyIndex
{
    public:
	//! Make a new faster action
	/**
         * Populate the action with the type of action, the
         * index of the army unit, and its movement bonus.
         */
        ArmySetEditorAction_Faster (Type t, guint32 i, guint32 b)
          : ArmySetEditorAction_ArmyIndex (t, i), d_bonus (b) {};
	//! Destroy a faster action.
        ~ArmySetEditorAction_Faster () {};

        guint32 getBonus () {return d_bonus;}

    private:
        guint32 d_bonus;
};

//-----------------------------------------------------------------------------

//! A record of the army's faster-in-forest property changing in the editor.
/**
 * The purpose of the ArmySetEditorAction_FasterInForest class is to record
 * when an army's speed in forests has been changed from normal to fast, or
 * vice versa.
 */
class ArmySetEditorAction_FasterInForest: public ArmySetEditorAction_Faster
{
    public:
	//! Make a new faster in forest action
	/**
         * Populate the action with whether or not the army moves
         * faster in forest.
         */
        ArmySetEditorAction_FasterInForest (guint32 i, guint32 b)
          : ArmySetEditorAction_Faster (FASTER_IN_FORESTS, i, b) {};
	//! Destroy a faster in forests action.
        ~ArmySetEditorAction_FasterInForest() {};
        Glib::ustring getActionName () const {return _("Faster In Forests");}
};

//-----------------------------------------------------------------------------

//! A record of the army's faster-in-marshes property changing in the editor.
/**
 * The purpose of the ArmySetEditorAction_FasterInMarsh class is to record
 * when an army's speed in marshland has been changed from normal to fast, or
 * vice versa.
 */
class ArmySetEditorAction_FasterInMarsh: public ArmySetEditorAction_Faster
{
    public:
	//! Make a new faster in marsh action
	/**
         * Populate the action with whether or not the army moves
         * faster in marshland.
         */
        ArmySetEditorAction_FasterInMarsh (guint32 i, guint32 b)
          : ArmySetEditorAction_Faster (FASTER_IN_MARSHES, i, b) {};
	//! Destroy a faster in marsh action.
        ~ArmySetEditorAction_FasterInMarsh () {};
        Glib::ustring getActionName () const {return _("Faster In Marshland");}
};

//-----------------------------------------------------------------------------

//! A record of the army's faster-in-hills property changing in the editor.
/**
 * The purpose of the ArmySetEditorAction_FasterInHills class is to record
 * when an army's speed in hills has been changed from normal to fast, or
 * vice versa.
 */
class ArmySetEditorAction_FasterInHills: public ArmySetEditorAction_Faster
{
    public:
	//! Make a new faster in hills action
	/**
         * Populate the action with whether or not the army moves
         * faster in hills.
         */
        ArmySetEditorAction_FasterInHills (guint32 i, guint32 b)
          : ArmySetEditorAction_Faster (FASTER_IN_HILLS, i, b) {};
	//! Destroy a faster in hills action.
        ~ArmySetEditorAction_FasterInHills () {};
        Glib::ustring getActionName () const {return _("Faster In Hills");}
};

//-----------------------------------------------------------------------------

//! A record of the army's faster-in-mountains property changing in the editor.
/**
 * The purpose of the ArmySetEditorAction_FasterInMountains class is to record
 * when an army's speed in mountains has been changed from normal to fast, or
 * vice versa.
 */
class ArmySetEditorAction_FasterInMountains: public ArmySetEditorAction_Faster
{
    public:
	//! Make a new faster in mountains action
	/**
         * Populate the action with whether or not the army moves
         * faster in mountains.
         */
        ArmySetEditorAction_FasterInMountains (guint32 i, guint32 b)
          : ArmySetEditorAction_Faster (FASTER_IN_MOUNTAINS, i, b) {};
	//! Destroy a faster in mountains action.
        ~ArmySetEditorAction_FasterInMountains () {};
        Glib::ustring getActionName () const {return _("Faster In Mountains");}
};

//-----------------------------------------------------------------------------

//! A record of an army type's ability to fly being modified
/**
 * The purpose of the ArmySetEditorAction_Fly class is to record
 * when an army type is changed to make it fly or not fly.
 *
 */
class ArmySetEditorAction_Fly : public ArmySetEditorAction_Faster
{
    public:
	//! Make a new fly action 
	/**
         * Populate the fly action with the flying status of the army type.
         */
        ArmySetEditorAction_Fly (guint32 i, guint32 b)
          :ArmySetEditorAction_Faster (ArmySetEditorAction::FLY, i, b) {}
	//! Destroy a fly action
        ~ArmySetEditorAction_Fly () {};
        Glib::ustring getActionName () const {return _("Fly");}
};

//-----------------------------------------------------------------------------

//! A record of the army unit selector images being changed
/**
 * The purpose of the ArmySetEditorAction_Selector class is to record
 * when the selector images are replaced or cleared.
 *
 * We take a copy of the whole armyset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class ArmySetEditorAction_Selector: public ArmySetEditorAction_Save
{
    public:
	//! Make a new selector action
	/**
         * Populate the selector action with the armyset.
         */
        ArmySetEditorAction_Selector (Armyset *a)
          :ArmySetEditorAction_Save(a, ArmySetEditorAction::SELECTOR) {};
	//! Destroy a selector action, and delete the file.
        ~ArmySetEditorAction_Selector () {};
        Glib::ustring getActionName () const {return _("Selector");}
};

//-----------------------------------------------------------------------------

//! A record of an army type's experience points being modified
/**
 * The purpose of the ArmySetEditorAction_Exp class is to record
 * when an army type's XP is modified.   e.g. the number of points received
 * by killing this army type.
 */
class ArmySetEditorAction_Exp : public ArmySetEditorAction_ArmyIndex
{
    public:
	//! Make a new exp action 
	/**
         * Populate the exp action with the number of experience points.
         */
        ArmySetEditorAction_Exp (guint32 i, guint32 x)
          :ArmySetEditorAction_ArmyIndex (ArmySetEditorAction::EXP, i, true),
          d_exp (x) {}
	//! Destroy an exp action
        ~ArmySetEditorAction_Exp () {}

        Glib::ustring getActionName () const {return _("Exp Points");}

        guint32 getExp () {return d_exp;}
    private:
        guint32 d_exp;
};

#endif //ARMYSET_EDITOR_ACTIONS_H
