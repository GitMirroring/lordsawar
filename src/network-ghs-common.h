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
#ifndef NETWORK_GHS_COMMON_H
#define NETWORK_GHS_COMMON_H

#include <config.h>

enum GhsMessageType 
{
  GHS_MESSAGE_HOST_NEW_GAME = 1,  //sent from client (lw host)
  GHS_MESSAGE_AWAITING_MAP = 2, //from server
  GHS_MESSAGE_UNHOST_GAME = 3, //from client (lw host)
  GHS_MESSAGE_GAME_UNHOSTED = 4, //from server
  GHS_MESSAGE_COULD_NOT_HOST_GAME = 5, //from server
  GHS_MESSAGE_COULD_NOT_UNHOST_GAME = 6, //from server
  GHS_MESSAGE_REQUEST_RELOAD = 7, //from client
  GHS_MESSAGE_RELOADED = 8, //from server
  GHS_MESSAGE_COULD_NOT_RELOAD = 9, //from server
  GHS_MESSAGE_GAME_LIST = 10, //from server
  GHS_MESSAGE_COULD_NOT_GET_GAME_LIST = 11, //from server
  GHS_MESSAGE_REQUEST_GAME_LIST = 12, //from client (lw host or client)
  GHS_MESSAGE_SENDING_MAP = 13, //from client
  GHS_MESSAGE_GAME_HOSTED = 14, //from server
  GHS_MESSAGE_COULD_NOT_READ_MAP = 15, //from server
  GHS_MESSAGE_COULD_NOT_START_GAME = 16, //from server
  GHS_MESSAGE_REQUEST_TERMINATION = 17 //from client
};

#endif
