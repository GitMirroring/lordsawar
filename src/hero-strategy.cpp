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
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
//  02110-1301, USA.

#include <sstream>
#include <vector>
#include <sigc++/functors/mem_fun.h>

#include "rnd.h"
#include "hero-strategy.h"

Glib::ustring HeroStrategy::d_tag = "herostrategy";

HeroStrategy::HeroStrategy (Type type)
 : d_type (type)
{
}

HeroStrategy::HeroStrategy (XML_Helper *helper)
{
  Glib::ustring type_str;
  helper->getData (type_str, "type");
}

HeroStrategy::HeroStrategy (const HeroStrategy& orig)
 : d_type (orig.d_type)
{
}

HeroStrategy* HeroStrategy::handle_load (XML_Helper* helper)
{
    guint32 t;
    Glib::ustring type_str;
    helper->getData (type_str, "type");
    t = heroStrategyTypeFromString (type_str);
    switch (t)
      {
      case HeroStrategy::NONE:
        return (new HeroStrategy_None (helper));
      case HeroStrategy::RANDOM:
        return (new HeroStrategy_Random (helper));
      case HeroStrategy::SIMPLE_QUESTER:
        return (new HeroStrategy_SimpleQuester (helper));
      }
    return 0;
}

HeroStrategy_None::HeroStrategy_None ()
 :HeroStrategy (HeroStrategy::NONE)
{
}

HeroStrategy_None::HeroStrategy_None (XML_Helper* helper)
 : HeroStrategy (helper)
{ 
  d_type = HeroStrategy::NONE;
}

HeroStrategy_None::HeroStrategy_None (const HeroStrategy_None & orig)
 : HeroStrategy (orig)
{
}

bool HeroStrategy_None::save (XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag (HeroStrategy::d_tag);
  Glib::ustring type_str =
    heroStrategyTypeToString (HeroStrategy::Type (d_type));
  retval &= helper->saveData ("type", type_str);
  retval &= helper->closeTag ();
  return retval;
}

HeroStrategy_None::~HeroStrategy_None ()
{
}

HeroStrategy_Random::HeroStrategy_Random ()
 : HeroStrategy (HeroStrategy::RANDOM), d_turns (1)
{
}

HeroStrategy_Random::HeroStrategy_Random (guint32 t)
 : HeroStrategy (HeroStrategy::RANDOM), d_turns (t)
{
}

HeroStrategy_Random::HeroStrategy_Random (XML_Helper* helper)
 : HeroStrategy (helper)
{
  d_type = HeroStrategy::RANDOM;
  helper->getData (d_turns, "turns");
}

HeroStrategy_Random::HeroStrategy_Random (const HeroStrategy_Random& orig)
 : HeroStrategy (orig), d_turns (orig.d_turns)
{
}

bool HeroStrategy_Random::save (XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag (HeroStrategy::d_tag);
  Glib::ustring type_str =
    heroStrategyTypeToString (HeroStrategy::Type (d_type));
  retval &= helper->saveData ("type", type_str);
  retval &= helper->saveData ("turns", d_turns);
  retval &= helper->closeTag ();
  return retval;
}

HeroStrategy_Random::~HeroStrategy_Random ()
{
}

HeroStrategy::Type HeroStrategy::getRandom ()
{
  std::vector<Type> types;
  for (guint32 i = NONE; i <= SIMPLE_QUESTER; i++)
    {
      if (i == guint32 (HeroStrategy::RANDOM))
        continue;
      types.push_back (HeroStrategy::Type (i));
    }
  int idx = Rnd::rand () % types.size ();
  return types[idx];
}

HeroStrategy_SimpleQuester::HeroStrategy_SimpleQuester ()
 : HeroStrategy (HeroStrategy::SIMPLE_QUESTER),
    d_fallback_type (HeroStrategy::NONE), d_num_helpers (0)
{
}

HeroStrategy_SimpleQuester::HeroStrategy_SimpleQuester (Type t, guint32 num)
 : HeroStrategy (HeroStrategy::SIMPLE_QUESTER), d_fallback_type (t),
    d_num_helpers (num)
{
}

HeroStrategy_SimpleQuester::HeroStrategy_SimpleQuester (XML_Helper* helper)
 : HeroStrategy (helper)
{
  d_type = HeroStrategy::SIMPLE_QUESTER;
  Glib::ustring type_str;
  helper->getData (type_str, "fallback_type");
  d_fallback_type = heroStrategyTypeFromString (type_str);
  helper->getData (d_num_helpers, "num_helpers");
}

HeroStrategy_SimpleQuester::HeroStrategy_SimpleQuester (const HeroStrategy_SimpleQuester& orig)
 : HeroStrategy (orig), d_fallback_type (orig.d_fallback_type), 
    d_num_helpers (orig.d_num_helpers)
{
}

bool HeroStrategy_SimpleQuester::save (XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag (HeroStrategy::d_tag);
  Glib::ustring type_str =
    heroStrategyTypeToString (HeroStrategy::Type (d_type));
  retval &= helper->saveData ("type", type_str);
  type_str = heroStrategyTypeToString (HeroStrategy::Type (d_fallback_type));
  retval &= helper->saveData ("fallback_type", type_str);
  retval &= helper->saveData ("num_helpers", d_num_helpers);
  retval &= helper->closeTag ();
  return retval;
}

HeroStrategy_SimpleQuester::~HeroStrategy_SimpleQuester ()
{
}

Glib::ustring HeroStrategy::heroStrategyTypeToString (const HeroStrategy::Type type)
{
  switch (type)
    {
      case HeroStrategy::NONE: return "HeroStrategy::NONE";
      case HeroStrategy::RANDOM: return "HeroStrategy::RANDOM";
      case HeroStrategy::SIMPLE_QUESTER: return "HeroStrategy::SIMPLE_QUESTER";
    }
  return "HeroStrategy::NONE";
}

HeroStrategy::Type HeroStrategy::heroStrategyTypeFromString (const Glib::ustring str)
{
  if (str.size() > 0 && isdigit (str.c_str ()[0]))
    return HeroStrategy::Type (atoi (str.c_str ()));
  if (str == "HeroStrategy::NONE") return HeroStrategy::NONE;
  else if (str == "HeroStrategy::RANDOM") return HeroStrategy::RANDOM;
  else if (str == "HeroStrategy::SIMPLE_QUESTER") return HeroStrategy::SIMPLE_QUESTER;
  return HeroStrategy::NONE;
}

HeroStrategy* HeroStrategy::copy (const HeroStrategy* r)
{
  switch(r->getType ())
    {
    case HeroStrategy::NONE:
      return (new HeroStrategy_None
              (*dynamic_cast<const HeroStrategy_None*>(r)));
    case HeroStrategy::RANDOM:
      return (new HeroStrategy_Random
              (*dynamic_cast<const HeroStrategy_Random*>(r)));
    case HeroStrategy::SIMPLE_QUESTER:
      return (new HeroStrategy_SimpleQuester
              (*dynamic_cast<const HeroStrategy_SimpleQuester*>(r)));
    }
  return 0;
}

bool HeroStrategy::compare (const HeroStrategy *l, const HeroStrategy *r)
{
  if (l && !r)
    return false;
  if (!l && r)
    return false;
  if (!l && !r)
    return true;
  if (l->getType () != r->getType ())
    return false;
  switch(r->getType ())
    {
    case HeroStrategy::NONE:
      break;
    case HeroStrategy::RANDOM:
        {
          const HeroStrategy_Random *left =
            (dynamic_cast<const HeroStrategy_Random*>(l));
          const HeroStrategy_Random *right =
            (dynamic_cast<const HeroStrategy_Random*>(r));
          if (left->getTurns () != right->getTurns ())
            return false;
        }
      break;
    case HeroStrategy::SIMPLE_QUESTER:
        {
          const HeroStrategy_SimpleQuester *left =
            (dynamic_cast<const HeroStrategy_SimpleQuester*>(l));
          const HeroStrategy_SimpleQuester *right =
            (dynamic_cast<const HeroStrategy_SimpleQuester*>(r));
          if (left->getNumHelpers () != right->getNumHelpers ())
            return false;
          if (left->getFallbackStrategy () != right->getFallbackStrategy ())
            return false;
        }
      break;
    }
  return true;
}
