//  Copyright (C) 2017, 2021 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "game-actionlist.h"
#include "network-action.h"
#include "xmlhelper.h"

Glib::ustring GameActionlist::d_tag = "turnlist";
//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

GameActionlist* GameActionlist::s_instance = 0;

GameActionlist* GameActionlist::getInstance()
{
  if (s_instance == NULL)
    s_instance = new GameActionlist();

  return s_instance;
}

GameActionlist* GameActionlist::getInstance(XML_Helper* helper)
{
  if (s_instance)
    deleteInstance();

  s_instance = new GameActionlist(helper);
  return s_instance;
}

void GameActionlist::deleteInstance()
{
  if (s_instance)
    delete s_instance;

  s_instance = NULL;
}

GameActionlist::GameActionlist()
{
}

GameActionlist::GameActionlist (const GameActionlist &g)
 : std::list<TurnActionlist*> (), sigc::trackable (g)
{
  for (auto turnactionlist: g)
    push_back (new TurnActionlist (*turnactionlist));
}

GameActionlist::~GameActionlist()
{
  for (GameActionlist::iterator it = begin(); it != end(); ++it)
    delete *it;
  clear();
}

GameActionlist::GameActionlist(XML_Helper* helper)
{
  helper->registerTag(TurnActionlist::d_tag, sigc::mem_fun(this, &GameActionlist::load));
}

bool GameActionlist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(GameActionlist::d_tag);

    for (GameActionlist::const_iterator it = begin(); it != end(); ++it)
      retval &= (*it)->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool GameActionlist::load(Glib::ustring tag, XML_Helper* helper)
{
  if (tag == TurnActionlist::d_tag)
    {
      TurnActionlist *t = new TurnActionlist(helper);
      push_back(t);
      return true;
    }

    return false;
}

void GameActionlist::add(TurnActionlist *t)
{
  push_back(t);
}

void GameActionlist::reset (GameActionlist *g)
{
  delete s_instance;
  s_instance = g;
}
