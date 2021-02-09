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
      if (dynamic_cast<UndoCursor*>(action))
        {
          UndoCursor *c = dynamic_cast<UndoCursor*>(action);
          unwound_pos[c->getObject ()] = c->getPos ();
        }
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
      if (dynamic_cast<UndoCursor*>(action))
        {
          UndoCursor *c = dynamic_cast<UndoCursor*>(action);
          unwound_pos[c->getObject ()] = c->getPos ();
        }
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

void UndoMgr::connect (Gtk::Entry *entry)
{
  connections.push_back
    (entry->property_cursor_position ().signal_changed ().connect
     (sigc::bind (sigc::mem_fun (this, &UndoMgr::updateEntry), entry)));
}

void UndoMgr::connect (Gtk::TextView *textview)
{
  connections.push_back
    (textview->get_buffer ()->property_cursor_position ().signal_changed ().connect
     (sigc::bind (sigc::mem_fun (this, &UndoMgr::updateTextView), textview)));
}

void UndoMgr::addCursor (Gtk::Entry *entry)
{
  entries[entry] = std::pair<int, int>(-1, -1);
}

void UndoMgr::addCursor (Gtk::TextView *textview)
{
  textviews[textview] = std::pair<int, int>(-1, -1);
}

void UndoMgr::updateEntry(Gtk::Entry *entry)
{
  auto it = entries.find (entry);
  if (it == entries.end ())
    return;
  std::pair<int,int> p = (*it).second;

  p.second = p.first;
  p.first = entry->get_position ();
  entries[entry] = p;
}

void UndoMgr::updateTextView(Gtk::TextView *textview)
{
  auto it = textviews.find (textview);
  if (it == textviews.end ())
    return;
  std::pair<int,int> p = (*it).second;

  p.second = p.first;
  p.first = textview->get_buffer ()->property_cursor_position ().get_value ();
  textviews[textview] = p;
}

void UndoMgr::disconnect_signals ()
{
  for (auto c : connections)
    c.disconnect ();
  connections.clear ();
}

void UndoMgr::connect_signals ()
{
  for (auto l : entries)
    connect (l.first);
  for (auto l : textviews)
    connect (l.first);
}

int UndoMgr::getPos (Gtk::Entry *e)
{
  auto it = entries.find (e);
  if (it == entries.end ())
    return 0;
  std::pair<int,int> p = (*it).second;
  int pos = 0;
  if (p.second != -1)
    pos = p.second;
  else if (p.first != -1)
    pos = p.first;
  return pos;
}

int UndoMgr::getPos (Gtk::TextView *t)
{
  auto it = textviews.find (t);
  if (it == textviews.end ())
    return 0;
  std::pair<int,int> p = (*it).second;
  int pos = 0;
  if (p.second != -1)
    pos = p.second;
  else if (p.first != -1)
    pos = p.first;
  return pos;
}

void UndoMgr::setPos (Gtk::Entry *e)
{
  auto it = unwound_pos.find (e);
  if (it == unwound_pos.end ())
    return;
  e->set_position (unwound_pos[e]);
  updateEntry (e);
  updateEntry (e);
}

void UndoMgr::setPos (Gtk::TextView *t)
{
  auto it = unwound_pos.find (t);
  if (it == unwound_pos.end ())
    return;
  int p = unwound_pos[t];
  t->get_buffer ()->place_cursor (t->get_buffer ()->get_iter_at_offset (p));
  updateTextView (t);
  updateTextView (t);
}

void UndoMgr::setCursors ()
{
  for (auto l : unwound_pos)
    {
      if (dynamic_cast<Gtk::Entry*>(l.first))
        setPos (dynamic_cast<Gtk::Entry*>(l.first));
      else if (dynamic_cast<Gtk::TextView*>(l.first))
        setPos (dynamic_cast<Gtk::TextView*>(l.first));
    }
}
