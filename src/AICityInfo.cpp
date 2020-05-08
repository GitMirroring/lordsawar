// Copyright (C) 2004 John Farrell
// Copyright (C) 2020 Ben Asselstine
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

#include "AICityInfo.h"
#include "Threatlist.h"
#include "Threat.h"
#include "city.h"

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::flush<<std::endl;}
#define debug(x)

AICityInfo::AICityInfo(City *c)
    :d_danger(0), d_reinforcements(0), d_city(c)
{
    d_threats = new Threatlist();
    d_num_defenders = d_city->countDefenders();
}

AICityInfo::AICityInfo (const AICityInfo &i)
 : d_danger (i.d_danger), d_reinforcements (i.d_reinforcements),
    d_threats (new Threatlist()), d_city (i.d_city),
    d_num_defenders (i.d_num_defenders)
{
  for (auto t : *i.d_threats)
    d_threats->push_back (new Threat (*t));
}

AICityInfo::~AICityInfo()
{
  delete d_threats;
}

void AICityInfo::addThreat(float dangerFromThisThreat, Threat *threat)
{
  this->d_danger += dangerFromThisThreat;
  this->d_threats->push_back (new Threat (*threat));
}
