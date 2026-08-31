//  Copyright (C) 2011, 2014, 2015 Ben Asselstine
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

#pragma once
#ifndef NETWORK_GLS_COMMON_H
#define NETWORK_GLS_COMMON_H

#include <config.h>

enum GlsMessageType 
{
  GLS_MESSAGE_ADVERTISE_GAME = 1, //from client (lw host or client)
  GLS_MESSAGE_UNADVERTISE_GAME = 2, //from client (lw host or client)
  GLS_MESSAGE_REQUEST_GAME_LIST = 3, //from client (lw host or client)
  GLS_MESSAGE_GAME_LIST = 4, //from server
  GLS_MESSAGE_COULD_NOT_ADVERTISE_GAME = 5, //from server
  GLS_MESSAGE_COULD_NOT_UNADVERTISE_GAME = 6, //from server
  GLS_MESSAGE_GAME_ADVERTISED = 7, //from server
  GLS_MESSAGE_GAME_UNADVERTISED = 8, //from server
  GLS_MESSAGE_COULD_NOT_GET_GAME_LIST = 9, //from server
  GLS_MESSAGE_REQUEST_RELOAD = 10, //from client
  GLS_MESSAGE_RELOADED = 11, //from server
  GLS_MESSAGE_COULD_NOT_RELOAD = 12, //from server
  GLS_MESSAGE_REQUEST_TERMINATION = 13 //from client
};

#endif
