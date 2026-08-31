//  Copyright (C) 2008, 2014, 2021, 2026 Ben Asselstine
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

#include <iostream>
#include <sstream>
#include <string.h>

#include "triumphs.h"
#include "player-list.h"

#include "xml-helper.h"

Glib::ustring Triumphs::d_tag = "triumphs";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<flush;}
#define debug(x)


Triumphs::Triumphs()
{
  memset(d_triumph, 0, sizeof(d_triumph));
}

Triumphs::Triumphs(XML_Helper* helper)
{
  for (unsigned int i = 0; i < 5; i++)
    {
      Glib::ustring tally;
      std::stringstream stally;
      guint32 val;
      switch (TriumphType(i))
	{
	case TALLY_HERO:
	  helper->get(tally, "hero");
	  break;
	case TALLY_NORMAL:
	  helper->get(tally, "normal");
	  break;
	case TALLY_SPECIAL:
	  helper->get(tally, "special");
	  break;
	case TALLY_SHIP:
	  helper->get(tally, "ship");
	  break;
	case TALLY_FLAG:
	  helper->get(tally, "flag");
	  break;
	}
      stally.str(tally);
      for (unsigned int j = 0; j < MAX_PLAYERS; j++)
	{
	  stally >> val;
	  d_triumph[j][i] = val;
	}
    }
}

Triumphs::Triumphs(const Triumphs& triumphs)
{
  for (guint32 i = 0; i < MAX_PLAYERS; i++)
    for (guint32 j = 0; j < 5; j++)
      d_triumph[i][j] = triumphs.d_triumph[i][j];
}

bool Triumphs::save(XML_Helper* helper) const
{
  bool retval = true;

  retval &= helper->open_tag(Triumphs::d_tag);
  for (unsigned int i = 0; i < 5; i++)
    {
      std::stringstream tally;
      for (unsigned int j = 0; j < MAX_PLAYERS; j++)
	tally << d_triumph[j][i] << " ";
      switch (TriumphType(i))
	{
	case TALLY_HERO:
	  retval &= helper->save("hero", tally.str());
	  break;
	case TALLY_NORMAL:
	  retval &= helper->save("normal", tally.str());
	  break;
	case TALLY_SPECIAL:
	  retval &= helper->save("special", tally.str());
	  break;
	case TALLY_SHIP:
	  retval &= helper->save("ship", tally.str());
	  break;
	case TALLY_FLAG:
	  retval &= helper->save("flag", tally.str());
	  break;
	}
    }

  retval &= helper->close_tag();

  return retval;
}

void Triumphs::tallyTriumph(Player *p, TriumphType type)
{
  //ignore monsters in a ruin who aren't owned by a player
  if (!p) 
    return;
  guint32 id = p->getId();
  //let's not tally neutrals
  if (p == Playerlist::getNeutral()) 
    return;
  //we (this player) have killed P's army. it was of type TYPE.
  d_triumph[id][type]++;
}
