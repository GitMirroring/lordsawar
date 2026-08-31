//  Copyright (C) 2007, 2008, 2014, 2015, 2017, 2020, 2021 Ben Asselstine
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

#include "rewardlist.h"
#include "reward.h"
#include "xmlhelper.h"
#include "rnd.h"

Glib::ustring Rewardlist::d_tag = "rewardlist";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Rewardlist* Rewardlist::s_instance = 0;

Rewardlist* Rewardlist::getInstance()
{
    if (s_instance == 0)
        s_instance = new Rewardlist();

    return s_instance;
}

Rewardlist* Rewardlist::getInstance(XML_Helper* helper)
{
    if (s_instance)
        deleteInstance();

    s_instance = new Rewardlist(helper);
    return s_instance;
}

void Rewardlist::deleteInstance()
{
    if (s_instance)
        delete s_instance;

    s_instance = 0;
}

void Rewardlist::deleteReward(const Reward* s)
{
  for (const_iterator it = this->begin(); it != this->end(); ++it)
    if ((*it) == s)
      {
        this->flRemove(s);
        return;
      }
}

Rewardlist::Rewardlist()
{
}

Rewardlist::Rewardlist (const Rewardlist &r)
 : std::list<Reward*> (), sigc::trackable (r)
{
  for (auto reward : r)
    push_back (Reward::copy (reward));
}

Rewardlist::~Rewardlist()
{
  flClear();
}

Rewardlist::Rewardlist(Rewardlist *rewardlist)
{
  for (iterator it = rewardlist->begin(); it != rewardlist->end(); ++it)
    {
      switch ((*it)->getType())
        {
        case Reward::GOLD:
          push_back(new Reward_Gold(*dynamic_cast<Reward_Gold*>(*it)));
          break;
        case Reward::ALLIES:
          push_back(new Reward_Allies(*dynamic_cast<Reward_Allies*>(*it)));
          break;
        case Reward::ITEM:
          push_back(new Reward_Item(*dynamic_cast<Reward_Item*>(*it)));
          break;
        case Reward::RUIN:
          push_back(new Reward_Ruin(*dynamic_cast<Reward_Ruin*>(*it)));
          break;
        case Reward::MAP:
          push_back(new Reward_Map(*dynamic_cast<Reward_Map*>(*it)));
          break;
        }
    }
}

Rewardlist::Rewardlist(XML_Helper* helper)
{
  helper->registerTag(Reward::d_tag, sigc::mem_fun((*this), &Rewardlist::load));
  load(Rewardlist::d_tag, helper);
}

void Rewardlist::flClear()
{
  for (iterator it = begin(); it != end(); ++it)
    delete (*it);

  clear();
}

Rewardlist::iterator Rewardlist::flErase(iterator object)
{
  delete (*object);
  return erase(object);
}

bool Rewardlist::flRemove(const Reward* object)
{
  debug("removing reward with id " << object->getId() << endl);
  const_iterator rewardit = find(begin(), end(), object);
  if (rewardit != end())
    {
      delete object;
      erase(rewardit);
      return true;
    }
  return false;
}

bool Rewardlist::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->openTag(Rewardlist::d_tag);

  //save rewards
  for (const_iterator it = begin(); it != end(); ++it)
    {
      if ((*it)->getType() == Reward::GOLD)
        static_cast<Reward_Gold*>(*it)->save(helper);
      else if ((*it)->getType() == Reward::ALLIES)
        static_cast<Reward_Allies*>(*it)->save(helper);
      else if ((*it)->getType() == Reward::ITEM)
        static_cast<Reward_Item*>(*it)->save(helper);
      else if ((*it)->getType() == Reward::RUIN)
        static_cast<Reward_Ruin*>(*it)->save(helper);
      else if ((*it)->getType() == Reward::MAP)
        static_cast<Reward_Map*>(*it)->save(helper);
    }

  retval &= helper->closeTag();

  return retval;
}

bool Rewardlist::load(Glib::ustring tag, XML_Helper* helper)
{
  if (tag == Reward::d_tag)
    {
      Reward *s = Reward::handle_load(helper);
      push_back(s);
      return true;
    }

    return false;
}

Reward *Rewardlist::pop (Reward::Type type)
{
  Rewardlist::iterator iter;
  std::vector<Reward*> rewards;
  for (iter = begin(); iter != end(); ++iter)
    {
      if ((*iter)->getType() == type)
        rewards.push_back(*iter);
    }
  if (rewards.size())
    {
      Reward *newReward = rewards[Rnd::rand() % rewards.size()];
      remove(newReward);
      return newReward;
    }
  else
    return NULL;
}

void Rewardlist::reset (Rewardlist *r)
{
  delete s_instance;
  s_instance = r;
}
// End of file
