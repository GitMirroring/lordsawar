//  Copyright (C) 2011, 2014 Ben Asselstine
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

//#include <iostream>
#include <fstream>
#include <sstream>
#include "hosted-game.h"
#include "advertised-game.h"
#include "xmlhelper.h"
#include "profile.h"


Glib::ustring HostedGame::d_tag = "hostedgame";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

HostedGame::HostedGame(AdvertisedGame *advertised_game)
  : d_advertised_game (advertised_game), d_pid (0), unresponsive (false)
{
  //watch out, not copying here.
}

HostedGame::HostedGame(XML_Helper *helper)
{
  unresponsive = false;
  helper->getData(d_pid, "pid");
  helper->registerTag(AdvertisedGame::d_tag_name, 
		      sigc::mem_fun(*this, &HostedGame::loadAdvertisedGame));
}

HostedGame::~HostedGame()
{
  delete d_advertised_game;
}

bool HostedGame::save(XML_Helper* helper) const
{
  bool retval = true;
  retval &= helper->openTag(d_tag);
  retval &= helper->saveData("pid", d_pid);
  retval &= d_advertised_game->saveEntry(helper);
  retval &= helper->closeTag();
  return retval;
}

bool HostedGame::loadAdvertisedGame(Glib::ustring tag, XML_Helper *helper)
{
  if (tag == AdvertisedGame::d_tag_name)
    {
      d_advertised_game = new AdvertisedGame(helper);
      return true;
    }
  return false;
}

void HostedGame::ping()
{
  getAdvertisedGame()->pinged.connect
    (sigc::mem_fun(*this, &HostedGame::on_pinged));
  getAdvertisedGame()->ping();
}

void HostedGame::on_pinged(bool success)
{
  if (!success)
    cannot_ping_game.emit(this);
}
