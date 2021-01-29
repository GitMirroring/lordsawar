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

#include <sstream>
#include <sigc++/functors/mem_fun.h>

#include "undo-mgr.h"
#include "ucompose.hpp"
#include "defs.h"

const int UndoMgr::LIMIT = 100;
const double UndoMgr::DELAY = 0.6;

UndoMgr::~UndoMgr ()
{
  clear ();
}

void UndoMgr::undo ()
{
  std::list<UndoAction*> redo_group;
  std::list<UndoAction*> group = popGroup (&undos);
  for (auto action : group)
    {
      UndoAction *redo = execute_signal.emit (action);
      redo->setTime (action->getTime ());
      redo_group.push_back (redo);
    }

  if (countUndoBlocks (redos, true) > d_limit)
    popLastGroup (&redos, true);
  for (auto action: redo_group)
    redos.push_front (action);

  for (auto action : group)
    delete action;
}

void UndoMgr::redo ()
{
  std::list<UndoAction*> undo_group;
  std::list<UndoAction*> group = popGroup (&redos, true);
  for (auto action : group)
    {
      UndoAction *undo = execute_signal.emit (action);
      undo->setTime (action->getTime ());
      undo_group.push_back (undo);
    }

  if (countUndoBlocks (undos) > d_limit)
    popLastGroup (&undos);
  for (auto action: undo_group)
    undos.push_front (action);

  for (auto action : group)
    delete action;
}


void UndoMgr::clear ()
{
  for (auto a : redos)
    delete a;
  redos.clear ();
  for (auto a : undos)
    delete a;
  undos.clear ();
}

void UndoMgr::add (UndoAction *action)
{
  undos.push_front (action);
  if (countUndoBlocks (undos) > d_limit)
    popLastGroup (&undos);
}

std::list<UndoAction*> UndoMgr::popLastGroup (std::list<UndoAction*> *list, bool bottom)
{
  std::list<UndoAction*> group;
  std::vector<UndoAction*> blanks;
  UndoAction *prev = NULL;
  for (auto it = list->rbegin (); it != list->rend (); it++)
    {
      UndoAction *l = *it;
      l->setTime (list->back ()->getTime ());
      if (isGrouped (prev, l, bottom))
        {
          group.push_back (l);
          if (l->getActionName () == "")
            blanks.push_back (l);
        }
      else
        break;
      prev = l;
    }
  for (auto g : group)
    g->setTime (group.front ()->getTime ());

  for (guint32 i = 0; i < group.size (); i++)
    list->pop_back ();

  if (blanks.size () > 1)
    {
      int i = 0;
      for (auto b : blanks)
        {
          if (i > 0)
            {
              group.remove (b);
              delete b;
            }
          i++;
        }
    }
  return group;
}

bool UndoMgr::isGrouped (UndoAction *l, UndoAction *r, bool bottom)
{
  if (!l)
    return true;
  if (l->getAggregate () != r->getAggregate ())
    return false;

  if (l == NULL && r->getAggregate () == UndoAction::AGGREGATE_NONE)
    return true;
  if (l->getAggregate () == UndoAction::AGGREGATE_NONE && r == NULL)
    return true;
  if (l->getAggregate () == UndoAction::AGGREGATE_NONE &&
      r->getAggregate () == UndoAction::AGGREGATE_NONE)
    return false;

  Glib::ustring leftaction = l->getActionName ();
  Glib::ustring rightaction = r->getActionName ();

  if (l->getAggregate () == UndoAction::AGGREGATE_BLANK)
    {
      if (bottom)
        {
          if (rightaction == "")
            rightaction = leftaction;
        }
      else
        {
          if (leftaction == "")
            leftaction = rightaction;
        }
    }

  if (leftaction != rightaction)
    return false;

  if (l->getAggregate () == UndoAction::AGGREGATE_DELAY)
    {
      double diff = r->getTime ().as_double () - l->getTime ().as_double ();
      if (diff > 0 && diff > d_delay)
        return false;
      if (diff < 0 && diff < -d_delay)
        return false;
    }
  return true;
}

std::list<UndoAction*> UndoMgr::popGroup (std::list<UndoAction*> *list, bool bottom)
{
  std::list<UndoAction*> group;
  UndoAction *prev = NULL;
  std::list<UndoAction*> blanks;
  for (auto l : *list)
    {
      if (isGrouped (prev, l, bottom))
        {
          group.push_back (l);
          if (l->getActionName () == "")
            blanks.push_back (l);
        }
      else
        break;
      prev = l;
    }
  for (auto g : group)
    g->setTime (group.front ()->getTime ());
  for (guint32 i = 0; i < group.size (); i++)
    list->pop_front ();
  if (blanks.size () > 1)
    {
      int i = 0;
      for (auto b : blanks)
        {
          if (i > 0)
            {
              group.remove (b);
              delete b;
            }
          i++;
        }
    }
  return group;
}

Glib::ustring UndoMgr::getActionName (std::list<UndoAction*> list)
{
  for (auto l : list)
    {
      if (l->getActionName () == "")
        continue;
      return l->getActionName ();
    }
  return "";
}

guint32 UndoMgr::countUndoBlocks (std::list<UndoAction*> list, bool bottom)
{
  guint32 count = 0;
  UndoAction *prev = NULL;
  for (auto u : list)
    {
      if (u->getAggregate () == UndoAction::AGGREGATE_BLANK ||
          u->getAggregate () == UndoAction::AGGREGATE_DELAY)
        {
          if (isGrouped (prev, u, bottom) == false)
            count++;
        }
      else
        count++;
      prev = u;
    }
  return count;
}

void UndoMgr::updateMenuItems (Gtk::MenuItem *undo, Gtk::MenuItem *redo)
{
  redo->set_sensitive (redoEmpty () == false);
  undo->set_sensitive (undoEmpty () == false);
  if (redo->get_sensitive () == false)
    redo->set_label (_("Redo"));
  if (undo->get_sensitive () == false)
    undo->set_label (_("Undo"));
  if (undoEmpty () == false)
    undo->set_label (String::ucompose (_("Undo %1"), getUndoName ()));
  if (redoEmpty () == false)
    redo->set_label (String::ucompose (_("Redo %1"), getRedoName ()));
}
