// Copyright (C) 2000, 2001, 2003 Michael Bartl
// Copyright (C) 2001, 2003, 2004, 2005 Ulf Lorenz
// Copyright (C) 2007, 2008, 2009, 2010, 2014, 2020, 2021 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "xmlhelper.h"
#include "templelist.h"
#include "playerlist.h"
#include "stack.h"
#include "citysetlist.h"
#include "cityset.h"
#include "GameMap.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Glib::ustring Templelist::d_tag = "templelist";
Templelist* Templelist::s_instance=0;

Templelist* Templelist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Templelist();

    return s_instance;
}

Templelist* Templelist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Templelist(helper);
    return s_instance;
}

void Templelist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

Templelist::Templelist()
{
}

Templelist::~Templelist()
{
  for (iterator it = begin(); it != end(); ++it)
    delete *it;
  d_object.clear();
  d_id.clear();
}

Templelist::Templelist (const Templelist &t, bool sync_ids)
 : LocationList<Temple*> (), sigc::trackable (t)
{
  for (auto temple : t)
    add (new Temple (*temple, sync_ids));
}

Templelist::Templelist(XML_Helper* helper)
{
  helper->registerTag(Temple::d_tag, sigc::mem_fun(this, &Templelist::load));
}

bool Templelist::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(Templelist::d_tag);

    for (const_iterator it = begin(); it != end();++it)
        retval &= (*it)->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

bool Templelist::load(Glib::ustring tag, XML_Helper* helper)
{
    if (tag != Temple::d_tag)    
    //what has happened?
        return false;
    
    guint32 width = GameMap::getCityset()->getTempleTileWidth();
    add(new Temple(helper, width));

    return true;
}

static bool isFogged(void *t)
{
  return ((Temple*)t)->isVisible(Playerlist::getViewingplayer()) == false;
}

Temple * Templelist::getNearestVisibleTemple(const Vector<int>& pos) const
{
  std::list<bool (*)(void *)> filters;
  filters.push_back(isFogged);
  return getNearestObject(pos, &filters);
}

Temple* Templelist::getNearestVisibleAndUsefulTemple(Stack *s, 
						     double percent_can_be_blessed) const
{
  Vector<int> pos = s->getPos();
  int diff = -1;
  const_iterator diffit;

  for (const_iterator it = begin(); it != end(); ++it)
    {
      Temple *t= *it;
      if (isFogged(t))
	continue;

      double num_needed = (double)s->size() * (percent_can_be_blessed / 100.0);
      double num_not_blessed = 
        (double)s->size() - s->countArmiesBlessedAtTemple(t->getId());
      if (num_not_blessed < num_needed)
	continue;

      Vector<int> p = t->getPos();
      int delta = abs(p.x - pos.x);
      if (delta < abs(p.y - pos.y))
	delta = abs(p.y - pos.y);

      if ((diff > delta) || (diff == -1))
	{
	  diff = delta;
	  diffit = it;
	}
    }

  if (diff == -1) return 0;
  return (*diffit);

}

Temple* Templelist::getNearestVisibleAndUsefulTemple(Stack *stack, 
						     double percent_can_be_blessed, 
						     int dist) const
{
  Vector<int> pos = stack->getPos();
  Temple *t = getNearestVisibleAndUsefulTemple
    (stack, percent_can_be_blessed);
  if (!t)
    return NULL;
  if (t->getPos().x <= pos.x + dist && t->getPos().x >= pos.x - dist &&
      t->getPos().y <= pos.y + dist && t->getPos().y >= pos.y - dist)
    return t;
  return NULL;
}

Temple* Templelist::getNearestVisibleTemple(const Vector<int>& pos, int dist) const
{
  Temple *t = getNearestVisibleTemple(pos);
  if (!t)
    return NULL;
  if (t->getPos().x <= pos.x + dist && t->getPos().x >= pos.x - dist &&
      t->getPos().y <= pos.y + dist && t->getPos().y >= pos.y - dist)
    return t;
  return NULL;
}

guint32 Templelist::countUnamedTemples () const
{
  guint32 count = 0;
  for (const_iterator it = begin (); it != end (); ++it)
    if ((*it)->getName () == DEFAULT_TEMPLE_NAME)
      count++;
  return count;
}

void Templelist::reset (Templelist *t)
{
  delete s_instance;
  s_instance = t;
}
