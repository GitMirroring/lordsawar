// Copyright (C) 2004 John Farrell
// Copyright (C) 2004, 2005, 2020 Ulf Lorenz
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

#pragma once
#ifndef AICITYINFO_H
#define AICITYINFO_H

#include "city.h"
class Threatlist;
class Threat;

//! Artificial intelligence helper class for relating threats to a city.
/** Class which contains some threat-related information about a city. It is
  * used by the smart AI.
  * 
  * There are three important values:
  * - danger is a rough estimate of the strength of the stacks that are close
  *   to the city
  * - reinforcements is an indicator of the strength of the troops that have
  *   been assigned to protect the city
  * - the Threatlist contains a list of all threats (usually stacks) that
  *   endanger the city
  *
  *   See ai_smart.h for a comment about the smart AI.
  */
class AICityInfo
{
    public:

        //! Constructor.
        AICityInfo(City *c);

        //! Copy Constructor.
        AICityInfo(const AICityInfo &i);

        ~AICityInfo();

        //! record this threat as threatening this city
        void addThreat(float dangerFromThisThreat, Threat *threat);

        //! return the total danger to this city
        float getDanger() const { return d_danger; }

        //! return the total reinforcements allocated to this city
        float getReinforcements() const { return d_reinforcements; }

        //! advise that reinforcements have been sent to the city
        void addReinforcements(float reinforcements) { d_reinforcements += reinforcements; }

        //! return the threats to this city
        Threatlist *getThreats() const { return d_threats; }

        //! Returns the location of the city
        Vector<int> getPos() const { return d_city->getPos(); }
        
        //! Get the number of armies in the city
        int getDefenderCount() const { return d_num_defenders; }
    private:
        float d_danger;
        float d_reinforcements;
        Threatlist *d_threats;
        City *d_city;
        int d_num_defenders;
};

#endif // AICITYINFO_H

// End of file
