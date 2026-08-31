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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#pragma once
#ifndef ARMYSET_UNDO_H
#define ARMYSET_UNDO_H

#include <gtkmm.h>
#include <sigc++/trackable.h>

#include "army-base.h"
#include "hero.h"
#include "defs.h"
#include "undo-action.h"
#include "undo-mgr.h"
class Armyset;

//! A record of an event in the armyset editor
/**
 * The purpose of these classes is to implement undo/redo in the armyset
 * editor.
 * The idea is to record changes to the model.
 */

class ArmySetUndoAction: public UndoAction
{
public:

    //! An ArmySet Undo Action can be one of the following kinds.
    enum Type
      {
        /** Modify description/copyright/license. */
        CHANGE_PROPERTIES = 1,
        /** A army, ship, bag, etc. image has been set */
        ADD_IMAGE = 2,
        /** An image has been cleared*/
        CLEAR_IMAGE = 3,
        /** The name of an army has been changed */
        NAME = 4,
        /** The white army images are copied down to other colors */
        MAKE_SAME = 5,
        /** An army has had its position changed in the set */
        REORDER = 6,
        /** A new army has been added to the set */
        ADD_ARMY = 7,
        /** An army has been deleted from the set */
        REMOVE_ARMY = 8,
        /** An army bonus has been modified (+1 str in hills, etc.) */
        BONUS = 9,
        /** An army's number of turns has been modified */
        TURNS = 10,
        /** An army's cost has been modified */
        COST = 11,
        /** An army's upkeep has been modified */
        UPKEEP = 12,
        /** An army's new cost has been modified */
        NEW_COST = 13,
        /** An army's stat has been modified (strength, exp, etc.) */
        STAT = 14,
        /** The army's ID has been modified */
        ID = 15,
        /** Whether or not it is awarded at ruins has been modified */
        RUIN_AWARD = 16,
        /** Whether or not it defends ruins has been modified */
        DEFENDS_RUIN = 17,
        /** The army's hero type has been modified */
        HERO = 18,
        /** The army's ability to fly has been modified */
        FLY = 19,
        /** The army's speed through forests has been modified */
        FASTER_IN_FORESTS = 20,
        /** The army's speed through marshland has been modified */
        FASTER_IN_MARSHES = 21,
        /** The army's speed through hills has been modified */
        FASTER_IN_HILLS = 22,
        /** The army's speed over mountains has been modified */
        FASTER_IN_MOUNTAINS = 23,
        /** The army selector has been modified */
        SELECTOR = 24,
        /** The army's experience points has been modified */
        EXP = 25,

      };

    //! Default constructor.
    ArmySetUndoAction (Type type, UndoAction::AggregateType aggregate =
                       UndoAction::AGGREGATE_NONE)
      :UndoAction (aggregate), m_type(type)
      {
      }

    ~ArmySetUndoAction ()
      {
      }

    Type get_type () const
      {
        return m_type;
      }

protected:

    Type m_type;
};

//-----------------------------------------------------------------------------

//! A record of the armyset's properties changing in the editor.
/**
 * The purpose of the ArmySetUndoAction_Properties class is to record
 * when an armyset's name, description, copyright, license, and tilesize have
 * changed.
 */
class ArmySetUndoAction_Properties: public ArmySetUndoAction
{
public:
    //! Make a new change properties action
    /**
     * Populate the properties action with the new name, description,
     * copyright, license text, and tile size.
     */
    ArmySetUndoAction_Properties (Glib::ustring n, Glib::ustring d,
                                  Glib::ustring c, Glib::ustring l,
                                  guint32 ts)
      :ArmySetUndoAction(ArmySetUndoAction::CHANGE_PROPERTIES), m_name (n),
      m_desc (d), m_copyright (c), m_license (l), m_tile_size (ts)
  {
  }

    //! Destroy a change properties action.
    ~ArmySetUndoAction_Properties ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Properties";
      }

    Glib::ustring get_name () const
      {
        return m_name;
      }

    Glib::ustring get_description () const
      {
        return m_desc;
      }

    Glib::ustring get_copyright () const
      {
        return m_copyright;
      }

    Glib::ustring get_license () const
      {
        return m_license;
      }

    guint32 get_tile_size () const
      {
        return m_tile_size;
      }

private:
    Glib::ustring m_name;
    Glib::ustring m_desc;
    Glib::ustring m_copyright;
    Glib::ustring m_license;
    guint32 m_tile_size;
};

//-----------------------------------------------------------------------------

//! A helper class for events that require saving the whole armyset

/**
 * Several actions require saving the whole tar file because it's the
 * easiest way to implement undo/redo.
 */
class ArmySetUndoAction_Save: public ArmySetUndoAction
{
public:
    ArmySetUndoAction_Save (Armyset *a, Type t)
      :ArmySetUndoAction (t)
      {
        m_armyset = new Armyset (*a);
        m_filename = File::get_tmp_file () + ARMYSET_EXT;
        a->save (m_filename, ARMYSET_EXT);
      }

    ~ArmySetUndoAction_Save ()
      {
        File::erase (m_filename);
        delete m_armyset;
      }

    Glib::ustring get_armyset_file_name () const
      {
        return m_filename;
      }

    Armyset *get_armyset () const
      {
        return m_armyset;
      }
private:
    Glib::ustring m_filename;
    Armyset *m_armyset;
};

//-----------------------------------------------------------------------------

//! A record of an armyset image being added or replaced
/**
 * The purpose of the ArmySetUndoAction_AddImage class is to record
 * when we select a new file.
 *
 * We take a copy of the whole armyset.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class ArmySetUndoAction_AddImage: public ArmySetUndoAction_Save
{
public:
    //! Make a new add-file action
    /**
     * Populate the add image action with the armyset.
     */
    ArmySetUndoAction_AddImage (Armyset *a)
      :ArmySetUndoAction_Save (a, ArmySetUndoAction::ADD_IMAGE)
      {
      }

    //! Destroy an add-image action, and delete the file.
    ~ArmySetUndoAction_AddImage ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add Image";
      }
};

//-----------------------------------------------------------------------------

//! A record of a armyset image being cleared
/**
 * The purpose of the ArmySetUndoAction_ClearImage class is to record
 * when we disassociate an image file with an army, ship, bag, etc.
 *
 * We take a copy of the whole armyset.  Our copy is a file on disk and
 * is deleted when this class is destroyed.
 */
class ArmySetUndoAction_ClearImage: public ArmySetUndoAction_Save
{
public:
    //! Make a new clear-file action
    /**
     * Populate the clear image action with the armyset.
     */
    ArmySetUndoAction_ClearImage (Armyset *a)
      :ArmySetUndoAction_Save (a, ArmySetUndoAction::CLEAR_IMAGE)
      {
      }

    //! Destroy an clear-image action, and delete the file.
    ~ArmySetUndoAction_ClearImage ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Clear Image";
      }
};

//-----------------------------------------------------------------------------

//! A helper class for events that require referencing the army's place
//in the set. this is not the id, and equates to the position in the treeview.

class ArmySetUndoAction_ArmyIndex: public ArmySetUndoAction
{
public:
    ArmySetUndoAction_ArmyIndex (Type t, guint32 i, bool agg = false)
      : ArmySetUndoAction (t, agg ? UndoAction::AGGREGATE_DELAY :
                           UndoAction::AGGREGATE_NONE), m_index (i)
        {
        }

    ~ArmySetUndoAction_ArmyIndex ()
      {
      }

    guint32 get_index () const
      {
        return m_index;
      }
private:
    guint32 m_index;
};
//-----------------------------------------------------------------------------

//! A record of an army's name being changed
/**
 * The purpose of the ArmySetUndoAction_Name class is to record
 * when we change the army's name.  This happens letter by letter.
 *
 */
class ArmySetUndoAction_Name: public ArmySetUndoAction_ArmyIndex, public UndoCursor
{
public:
    //! Make a new name action
    /**
     * Populate the action with the index of the army type whose name
     * we're modifying, the name, and the position of the cursor in the
     * entry.
     */
    ArmySetUndoAction_Name (guint32 i, Glib::ustring n, UndoMgr *u,
                            Gtk::Entry *e)
      : ArmySetUndoAction_ArmyIndex (NAME, i, true),
      UndoCursor (u->get_pos (e), e), m_name (n)
  {
  }
    //! Destroy a name action.
    ~ArmySetUndoAction_Name ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Name";
      }

    Glib::ustring get_name () const
      {
        return m_name;
      }

private:
    Glib::ustring m_name;
};

//-----------------------------------------------------------------------------

//! A record of the armyset's images changing in the editor en masse.
/**
 * The purpose of the ArmySetUndoAction_MakeSame class is to record
 * when a army's white images are copied down to the other colors.
 *
 * We take a copy of the whole armyset to get all of the images in one
 * go.  Our copy is a file on disk and is deleted when this class is
 * destroyed.
 */
class ArmySetUndoAction_MakeSame: public ArmySetUndoAction_Save
{
public:
    //! Make a new make same action
    ArmySetUndoAction_MakeSame (Armyset *a, bool make_same)
      :ArmySetUndoAction_Save(a, ArmySetUndoAction::MAKE_SAME),
      m_make_same (make_same)
      {
      }

    //! Destroy a white down action, and delete the file.
    ~ArmySetUndoAction_MakeSame ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Copy White Army";
      }

    guint32 get_make_same () const
      {
        return m_make_same;
      }
private:
    bool m_make_same;
};

//-----------------------------------------------------------------------------

//! A record of the armyset's order of army types changing
/**
 * The purpose of the ArmySetUndoAction_Reorder class is to record
 * when an army type moves up or down in the order of the set.
 *
 * We take a copy of the whole armyset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class ArmySetUndoAction_Reorder: public ArmySetUndoAction_Save
{
public:
    //! Make a new reorder action
    /**
     * Populate the reorder action with the armyset.
     */
    ArmySetUndoAction_Reorder (Armyset *a)
      :ArmySetUndoAction_Save(a, ArmySetUndoAction::REORDER)
      {
      }

    //! Destroy a reorder action, and delete the file.
    ~ArmySetUndoAction_Reorder ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Reorder";
      }
};

//-----------------------------------------------------------------------------

//! A record of a new army type being added to the set
/**
 * The purpose of the ArmySetUndoAction_AddArmy class is to record
 * when a new army type is added to the set - it is completely empty.
 *
 * We take a copy of the whole armyset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class ArmySetUndoAction_AddArmy: public ArmySetUndoAction_Save
{
public:
    //! Make a new add army action
    /**
     * Populate the add army action with the armyset.
     */
    ArmySetUndoAction_AddArmy (Armyset *a)
      :ArmySetUndoAction_Save(a, ArmySetUndoAction::ADD_ARMY)
      {
      }

    //! Destroy an add army action, and delete the file.
    ~ArmySetUndoAction_AddArmy ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Add Army";
      }
};

//-----------------------------------------------------------------------------

//! A record of an army type being deleted from the set
/**
 * The purpose of the ArmySetUndoAction_RemoveArmy class is to record
 * when an army type is removed to the set.
 *
 * We take a copy of the whole armyset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class ArmySetUndoAction_RemoveArmy: public ArmySetUndoAction_Save
{
public:
    //! Make a new remove army action
    /**
     * Populate the remove army action with the armyset.
     */
    ArmySetUndoAction_RemoveArmy (Armyset *a)
      :ArmySetUndoAction_Save(a, ArmySetUndoAction::REMOVE_ARMY)
      {
      }

    //! Destroy an remove army action, and delete the file.
    ~ArmySetUndoAction_RemoveArmy ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Remove Army";
      }
};

//-----------------------------------------------------------------------------

//! A record of an army type's bonus being modified
/**
 * The purpose of the ArmySetUndoAction_Bonus class is to record
 * when an army type's bonus is modified.
 * e.g. +1 in hills, +2 in city and so on.
 *
 */
class ArmySetUndoAction_Bonus : public ArmySetUndoAction_ArmyIndex
{
public:
    //! Make a new bonus action
    /**
     * Populate the bonus action with the type of bonus and the value.
     */
    ArmySetUndoAction_Bonus (guint32 i, ArmyBase::Bonus b, bool f)
      :ArmySetUndoAction_ArmyIndex (ArmySetUndoAction::BONUS, i),
      m_bonus (b), m_flag (f)
  {
  }

    //! Destroy a bonus action
    ~ArmySetUndoAction_Bonus ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return ArmyBase::bonusFlagToFriendlyName (m_bonus);
      }

    ArmyBase::Bonus get_bonus_type () const
      {
        return m_bonus;
      }

    bool get_flag () const
      {
        return m_flag;
      }
private:
    ArmyBase::Bonus m_bonus;
    bool m_flag;
};

//-----------------------------------------------------------------------------

//! A record of an army type's turns being modified
/**
 * The purpose of the ArmySetUndoAction_Turns class is to record
 * when an army type's turns is modified.   e.g. the number of turns it takes
 * to produce the army.
 */
class ArmySetUndoAction_Turns : public ArmySetUndoAction_ArmyIndex
{
public:
    //! Make a new turns action
    /**
     * Populate the turns action with the number of turns.
     */
    ArmySetUndoAction_Turns (guint32 i, guint32 t)
      :ArmySetUndoAction_ArmyIndex (ArmySetUndoAction::TURNS, i, true),
      m_turns (t)
  {
  }
    //! Destroy a turns action
    ~ArmySetUndoAction_Turns ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Turns";
      }

    guint32 get_turns () const
      {
        return m_turns;
      }
private:
    guint32 m_turns;
};

//-----------------------------------------------------------------------------

//! A record of an army type's cost being modified
/**
 * The purpose of the ArmySetUndoAction_Cost class is to record
 * when an army type's cost is modified.   e.g. the number of gold pieces
 * needed to instantiate this army type (e.g. one time production cost,
 * paid every time a new unit of this type is created.)
 */
class ArmySetUndoAction_Cost : public ArmySetUndoAction_ArmyIndex
{
public:
    //! Make a new cost action
    /**
     * Populate the cost action with the number of gold pieces.
     */
    ArmySetUndoAction_Cost (guint32 i, guint32 c)
      :ArmySetUndoAction_ArmyIndex (ArmySetUndoAction::COST, i, true),
      m_cost (c)
  {
  }

    //! Destroy a cost action
    ~ArmySetUndoAction_Cost ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Cost";
      }

    guint32 get_cost () const
      {
        return m_cost;
      }
private:
    guint32 m_cost;
};

//-----------------------------------------------------------------------------

//! A record of an army type's upkeep being modified
/**
 * The purpose of the ArmySetUndoAction_Upkeep class is to record
 * when an army type's upkeep is modified.   e.g. the number of gold pieces
 * needed to pay for this army type to keep going every turn after it is
 * instantiated.
 */
class ArmySetUndoAction_Upkeep : public ArmySetUndoAction_ArmyIndex
{
public:
    //! Make a new upkeep action
    /**
     * Populate the upkeep action with the number of gold pieces.
     */
    ArmySetUndoAction_Upkeep (guint32 i, guint32 u)
      :ArmySetUndoAction_ArmyIndex (ArmySetUndoAction::UPKEEP, i, true),
      m_upkeep (u)
  {
  }

    //! Destroy a upkeep action
    ~ArmySetUndoAction_Upkeep ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Upkeep";
      }

    guint32 get_upkeep ()
      {
        return m_upkeep;
      }
private:
    guint32 m_upkeep;
};

//-----------------------------------------------------------------------------

//! A record of an army type's new cost being modified
/**
 * The purpose of the ArmySetUndoAction_NewCost class is to record
 * when an army type's new cost is modified.   e.g. the number of gold pieces
 * needed to make a city produce this kind of army.  a one-time expense.
 */
class ArmySetUndoAction_NewCost : public ArmySetUndoAction_ArmyIndex
{
public:
    //! Make a new new-cost action
    /**
     * Populate the new-cost action with the number of gold pieces.
     */
    ArmySetUndoAction_NewCost (guint32 i, guint32 c)
      :ArmySetUndoAction_ArmyIndex (ArmySetUndoAction::NEW_COST, i,
                                    true), m_cost (c)
  {
  }

    //! Destroy a new-cost action
    ~ArmySetUndoAction_NewCost ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "New Cost";
      }

    guint32 get_new_cost () const
      {
        return m_cost;
      }
private:
    guint32 m_cost;
};

//-----------------------------------------------------------------------------

//! A record of an army type's stat being modified
/**
 * The purpose of the ArmySetUndoAction_Stat class is to record
 * when an army type's stat is modified.  e.g. strength, moves, sight.
 *
 */
class ArmySetUndoAction_Stat : public ArmySetUndoAction_ArmyIndex
{
public:
    //! Make a new stat action
    /**
     * Populate the stat action with the type of stat and the value.
     */
    ArmySetUndoAction_Stat (guint32 i, ArmyBase::Stat s, guint32 v)
      :ArmySetUndoAction_ArmyIndex (ArmySetUndoAction::STAT, i, true),
      m_stat(s), m_value(v)
  {
  }
    //! Destroy a stat action
    ~ArmySetUndoAction_Stat ()
      {
      }

    Glib::ustring get_action_name () const
      {
        switch (m_stat)
          {
          case ArmyBase::STRENGTH:
            return "Strength";

          case ArmyBase::MOVES:
            return "Moves";

          case ArmyBase::SIGHT:
            return "Sight";

          default:
            break;
          }
        return "";
      }

    ArmyBase::Stat get_stat_type () const
      {
        return m_stat;
      }

    guint32 get_value ()
      {
        return m_value;
      }
private:
    ArmyBase::Stat m_stat;
    guint32 m_value;
};

//-----------------------------------------------------------------------------

//! A record of an army type's id being modified
/**
 * The purpose of the ArmySetUndoAction_Id class is to record
 * when an army type's ID is modified.
 */
class ArmySetUndoAction_Id : public ArmySetUndoAction_ArmyIndex
{
public:
    //! Make a new id action
    /**
     * Populate the id action with the numeric id.
     */
    ArmySetUndoAction_Id (guint32 i, guint32 id)
      :ArmySetUndoAction_ArmyIndex (ArmySetUndoAction::ID, i, true),
      m_id (id)
  {
  }

    //! Destroy an id action
    ~ArmySetUndoAction_Id ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Id";
      }

    guint32 get_id () const
      {
        return m_id;
      }
private:
    guint32 m_id;
};

//-----------------------------------------------------------------------------

//! A record of an army type's ability to be awarded at a ruin being modified
/**
 * The purpose of the ArmySetUndoAction_RuinAward class is to record
 * when an army type is changed to make it a potential prize for searching a
 * ruin.  e.g. it can be one of the kinds of army types that show up as allies.
 */
class ArmySetUndoAction_RuinAward: public ArmySetUndoAction_ArmyIndex
{
public:
    //! Make a new ruin-award action
    /**
     * Populate the ruin-award action with the status of the army type.
     */
    ArmySetUndoAction_RuinAward (guint32 i, bool a)
      :ArmySetUndoAction_ArmyIndex (ArmySetUndoAction::RUIN_AWARD, i),
      m_award (a)
  {
  }

    //! Destroy a defends-ruin action
    ~ArmySetUndoAction_RuinAward ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Ruin Award";
      }

    bool get_award () const
      {
        return m_award;
      }
private:
    bool m_award;
};

//-----------------------------------------------------------------------------

//! A record of an army type's ability to defend a ruin being modified
/**
 * The purpose of the ArmySetUndoAction_DefendsRuins class is to record
 * when an army type is changed to make it defend a ruin or not. e.g. it is
 * a potential occupant/keeper of a ruin.
 *
 */
class ArmySetUndoAction_DefendsRuins: public ArmySetUndoAction_ArmyIndex
{
public:
    //! Make a new defends-ruins action
    /**
     * Populate the defends-ruin action with the status of the army type.
     */
    ArmySetUndoAction_DefendsRuins (guint32 i, bool d)
      :ArmySetUndoAction_ArmyIndex (ArmySetUndoAction::DEFENDS_RUIN, i),
      m_defend (d)
  {
  }
    //! Destroy a defends-ruin action
    ~ArmySetUndoAction_DefendsRuins ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Defends Ruins";
      }

    bool get_defend () const
      {
        return m_defend;
      }

private:
    bool m_defend;
};

//-----------------------------------------------------------------------------

//! A record of an army type's hero state being modified
/**
 * The purpose of the ArmySetUndoAction_Hero class is to record
 * when an army type's hero status is changed.  e.g female, not a hero.
 */
class ArmySetUndoAction_Hero: public ArmySetUndoAction_ArmyIndex
{
public:
    //! Make a new hero action
    /**
     * Populate the hero action with the status of the army type.
     */
    ArmySetUndoAction_Hero (guint32 i, Hero::Gender h)
      :ArmySetUndoAction_ArmyIndex (ArmySetUndoAction::HERO, i),
      m_hero(h)
  {
  }
    //! Destroy a hero action
    ~ArmySetUndoAction_Hero ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Hero";
      }

    Hero::Gender get_hero () const
      {
        return m_hero;
      }
private:
    Hero::Gender m_hero;
};

//-----------------------------------------------------------------------------

//! A helper class for faster properties changing in the editor.
class ArmySetUndoAction_Faster: public ArmySetUndoAction_ArmyIndex
{
public:
    //! Make a new faster action
    /**
     * Populate the action with the type of action, the
     * index of the army unit, and its movement bonus.
     */
    ArmySetUndoAction_Faster (Type t, guint32 i, guint32 b)
      : ArmySetUndoAction_ArmyIndex (t, i), m_bonus (b)
      {
      }

    //! Destroy a faster action.
    ~ArmySetUndoAction_Faster ()
      {
      }

    guint32 get_bonus () const
      {
        return m_bonus;
      }

private:
    guint32 m_bonus;
};

//-----------------------------------------------------------------------------

//! A record of the army's faster-in-forest property changing in the editor.
/**
 * The purpose of the ArmySetUndoAction_FasterInForest class is to record
 * when an army's speed in forests has been changed from normal to fast, or
 * vice versa.
 */
class ArmySetUndoAction_FasterInForest: public ArmySetUndoAction_Faster
{
public:
    //! Make a new faster in forest action
    /**
     * Populate the action with whether or not the army moves
     * faster in forest.
     */
    ArmySetUndoAction_FasterInForest (guint32 i, guint32 b)
      : ArmySetUndoAction_Faster (FASTER_IN_FORESTS, i, b)
      {
      }

    //! Destroy a faster in forests action.
    ~ArmySetUndoAction_FasterInForest ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Faster In Forests";
      }
};

//-----------------------------------------------------------------------------

//! A record of the army's faster-in-marshes property changing in the editor.
/**
 * The purpose of the ArmySetUndoAction_FasterInMarsh class is to record
 * when an army's speed in marshland has been changed from normal to fast, or
 * vice versa.
 */
class ArmySetUndoAction_FasterInMarsh: public ArmySetUndoAction_Faster
{
public:
    //! Make a new faster in marsh action
    /**
     * Populate the action with whether or not the army moves
     * faster in marshland.
     */
    ArmySetUndoAction_FasterInMarsh (guint32 i, guint32 b)
      : ArmySetUndoAction_Faster (FASTER_IN_MARSHES, i, b)
      {
      }

    //! Destroy a faster in marsh action.
    ~ArmySetUndoAction_FasterInMarsh ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Faster In Marshland";
      }
};

//-----------------------------------------------------------------------------

//! A record of the army's faster-in-hills property changing in the editor.
/**
 * The purpose of the ArmySetUndoAction_FasterInHills class is to record
 * when an army's speed in hills has been changed from normal to fast, or
 * vice versa.
 */
class ArmySetUndoAction_FasterInHills: public ArmySetUndoAction_Faster
{
public:
    //! Make a new faster in hills action
    /**
     * Populate the action with whether or not the army moves
     * faster in hills.
     */
    ArmySetUndoAction_FasterInHills (guint32 i, guint32 b)
      : ArmySetUndoAction_Faster (FASTER_IN_HILLS, i, b)
      {
      }

    //! Destroy a faster in hills action.
    ~ArmySetUndoAction_FasterInHills ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Faster In Hills";
      }
};

//-----------------------------------------------------------------------------

//! A record of the army's faster-in-mountains property changing in the editor.
/**
 * The purpose of the ArmySetUndoAction_FasterInMountains class is to record
 * when an army's speed in mountains has been changed from normal to fast, or
 * vice versa.
 */
class ArmySetUndoAction_FasterInMountains: public ArmySetUndoAction_Faster
{
public:
    //! Make a new faster in mountains action
    /**
     * Populate the action with whether or not the army moves
     * faster in mountains.
     */
    ArmySetUndoAction_FasterInMountains (guint32 i, guint32 b)
      : ArmySetUndoAction_Faster (FASTER_IN_MOUNTAINS, i, b)
      {
      }

    //! Destroy a faster in mountains action.
    ~ArmySetUndoAction_FasterInMountains ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Faster In Mountains";
      }
};

//-----------------------------------------------------------------------------

//! A record of an army type's ability to fly being modified
/**
 * The purpose of the ArmySetUndoAction_Fly class is to record
 * when an army type is changed to make it fly or not fly.
 *
 */
class ArmySetUndoAction_Fly : public ArmySetUndoAction_Faster
{
public:
    //! Make a new fly action
    /**
     * Populate the fly action with the flying status of the army type.
     */
    ArmySetUndoAction_Fly (guint32 i, guint32 b)
      :ArmySetUndoAction_Faster (ArmySetUndoAction::FLY, i, b)
      {
      }

    //! Destroy a fly action
    ~ArmySetUndoAction_Fly ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Fly";
      }
};

//-----------------------------------------------------------------------------

//! A record of the army unit selector images being changed
/**
 * The purpose of the ArmySetUndoAction_Selector class is to record
 * when the selector images are replaced or cleared.
 *
 * We take a copy of the whole armyset just to make it easy.  Our copy is a
 * file on disk and is deleted when this class is destroyed.
 */
class ArmySetUndoAction_Selector: public ArmySetUndoAction_Save
{
public:
    //! Make a new selector action
    /**
     * Populate the selector action with the armyset.
     */
    ArmySetUndoAction_Selector (Armyset *a)
      :ArmySetUndoAction_Save(a, ArmySetUndoAction::SELECTOR)
      {
      }

    //! Destroy a selector action, and delete the file.
    ~ArmySetUndoAction_Selector ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Selector";
      }
};

//-----------------------------------------------------------------------------

//! A record of an army type's experience points being modified
/**
 * The purpose of the ArmySetUndoAction_Exp class is to record
 * when an army type's XP is modified.   e.g. the number of points received
 * by killing this army type.
 */
class ArmySetUndoAction_Exp : public ArmySetUndoAction_ArmyIndex
{
public:
    //! Make a new exp action
    /**
     * Populate the exp action with the number of experience points.
     */
    ArmySetUndoAction_Exp (guint32 i, guint32 x)
      :ArmySetUndoAction_ArmyIndex (ArmySetUndoAction::EXP, i, true),
      m_exp (x)
  {
  }

    //! Destroy an exp action
    ~ArmySetUndoAction_Exp ()
      {
      }

    Glib::ustring get_action_name () const
      {
        return "Exp Points";
      }

    guint32 get_exp () const
      {
        return m_exp;
      }

private:
    guint32 m_exp;
};

#endif
