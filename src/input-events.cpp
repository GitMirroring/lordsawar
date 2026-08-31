//  Copyright (C) 2026 Ben Asselstine
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

#include "input-events.h"
#include <cmath>

struct MouseButtonEvent to_input_event (int n, double x, double y, bool press)
{
  struct MouseButtonEvent ev {};
  ev.pos = Vector<int>(std::round (x), std::round (y));
  ev.state = press ?
    MouseButtonEvent::State::PRESSED :
    MouseButtonEvent::State::RELEASED;
  switch (n)
    {
    case 1:
      ev.button = MouseButtonEvent::LEFT_BUTTON;
      break;
    case 2:
      ev.button = MouseButtonEvent::MIDDLE_BUTTON;
      break;
    case 3:
      ev.button = MouseButtonEvent::RIGHT_BUTTON;
      break;
    }
  return ev;
}
