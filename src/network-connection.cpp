// Copyright (C) 2008 Ole Laursen
// Copyright (C) 2008, 2014, 2015, 2017, 2021 Ben Asselstine
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

#include "network-common.h"
#include "network-connection.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <giomm.h>
#include <stdio.h>
#include "timing.h"
#include "File.h"
#include "defs.h"
#include "connection-manager.h"

void NetworkConnection::setup_connection()
{
  in = Gio::DataInputStream::create(conn->get_input_stream());
  out = Gio::DataOutputStream::create(conn->get_output_stream());
  source = 
    Gio::SocketSource::create
    (conn->property_socket(),
     Glib::IO_IN | Glib::IO_PRI | Glib::IO_ERR | Glib::IO_HUP | Glib::IO_NVAL);
  d_in_cb =
    source->connect(sigc::mem_fun(*this, &NetworkConnection::on_got_input));
  header_size = MESSAGE_SIZE_BYTES;
  header_left = header_size;
  memset (header, 0, sizeof (header));
  payload_size = 0;
  payload_left = 0;
  source->attach(Glib::MainContext::get_default());
}

void NetworkConnection::tear_down_connection(bool lockit)
{
  if (lockit)
    {
      d_in_cb.disconnect();
      d_stop = true;
      std::unique_lock<std::mutex> lock (mutex);
      cond_push.notify_one();

      //this doesn't seem to be needed, and enabling it is crashy.
      //cond_pop.notify_one();
      torn_down.emit();
    }
  else
    {
      d_in_cb.disconnect();
      d_stop = true;
      cond_push.notify_one();

      //this doesn't seem to be needed, and enabling it is crashy.
      //cond_pop.notify_one();
      torn_down.emit();
    }
}

bool NetworkConnection::on_got_input(Glib::IOCondition cond)
{
  connection_received_data.emit ();
  gssize len = -1;
  switch (cond)
    {
    case Glib::IO_IN:
    case Glib::IO_PRI:
      try
        {
      if (header_left > 0)
        len = on_header_received(in->fill (header_left));
      else if (payload_left > 0)
        len = on_payload_received(in->fill(payload_left));
        }
      catch (Gio::Error &ex)
        {
          len = -1;
        }
      if (len <= 0)
        {
          tear_down_connection();
          connection_lost.emit();
          return false;
        }
      break;
    case Glib::IO_ERR:
    case Glib::IO_HUP:
    case Glib::IO_NVAL:
      if (len <= 0)
        {
          tear_down_connection();
          connection_lost.emit();
          return false;
        }
      break;
    default:
      return false;
    }
  return true;
}

NetworkConnection::NetworkConnection(const Glib::RefPtr<Gio::SocketConnection> &c)
 : client (Gio::SocketClient::create()), payload(NULL), payload_left (0),
    payload_size (0), header {0}, header_left (0), header_size (0), d_host(""),
    d_port(0), d_stop(false), d_bail (false),
    d_cancellable(Gio::Cancellable::create())
{
  //okay, i've been asked to create a SERVER side network connection.
  client->set_protocol(Gio::SOCKET_PROTOCOL_TCP);
  if (c)
    {
      c->reference();
      conn = Gio::SocketConnection::create(c->property_socket());
      setup_connection();
    }
}

NetworkConnection::NetworkConnection()
 : client (Gio::SocketClient::create()), payload(NULL), payload_left (0),
    payload_size (0), header {0}, header_left (0), header_size (0), d_host(""),
    d_port(0), d_stop(false), d_bail (false),
    d_cancellable(Gio::Cancellable::create())
{
  client->set_protocol(Gio::SOCKET_PROTOCOL_TCP);
}

NetworkConnection::~NetworkConnection()
{
}

void NetworkConnection::on_connect_connected(Glib::RefPtr<Gio::AsyncResult> &result)
{
  d_connect_timer.disconnect();
  if (d_stop)
    {
      connection_failed.emit();
      return;
    }
  try
    {
      conn = client->connect_to_host_finish (result);
    }
  catch(const Glib::Exception &ex)
    {
      connection_failed.emit();
      return;
    }
  if (conn)
    {
      //okay, i've been asked to create a CLIENT side network connection.
      setup_connection();
      connected.emit();
    }
}

gssize NetworkConnection::on_header_received(gssize len)
{
  if (len <= 0)
    return len;

  in->read(header + (header_size - header_left), len);
  header_left -= len;
  if (header_left > 0)
    return len;

  guint32 val;
  memcpy(&val, header, header_size);
  payload_size = g_ntohl(val);
  payload_left = payload_size;
  payload = (char *) malloc (payload_size);
  return len;
}

gssize NetworkConnection::on_payload_received(gssize len)
{
  if (len <= 0)
    return len;
  in->read(payload + (payload_size - payload_left), len);
  payload_left -= len;

  payload_progress.emit (payload_size - payload_left, payload_size);
  if (payload_left > 0)
    return len;

  /*
   * ok, this next bit of code is definitely out of place.
   * the network connection object shouldn't have to treat certain
   * types of messages any differently, but it is.
   */
  int type = payload[1];
  bool keep_going;
  if (type == MESSAGE_TYPE_SENDING_MAP)
    {
      Glib::ustring file = "clientnetwork" + SAVE_EXT;
      Glib::ustring path = File::getSaveFile(file);

      FILE *fp = fopen (path.c_str(), "wb");
      fwrite (payload + MESSAGE_PREAMBLE_EXTRA_BYTES, 1,
              payload_size - MESSAGE_PREAMBLE_EXTRA_BYTES, fp);
      fclose (fp);
      keep_going = got_message.emit (type, path);
    }
  else
    keep_going = got_message.emit
      (type, Glib::ustring(payload + MESSAGE_PREAMBLE_EXTRA_BYTES,
                           payload_size - MESSAGE_PREAMBLE_EXTRA_BYTES));
  free (payload);
  payload = NULL;
  if (keep_going == false)
    {
      //time to go away
      return -1;
    }
  header_left = header_size; //set things up for the next header.
  payload_left = 0;
  payload_size = 0;
  return len;
}


void NetworkConnection::connectToHost(Glib::ustring host, int port)
{
  d_host = host;
  d_port = port;
  d_connect_timer = 
    Timing::instance().register_timer
    (sigc::mem_fun(this, &NetworkConnection::on_connect_timeout), 5000);
  client->connect_to_host_async 
    (host, port, d_cancellable,
     sigc::mem_fun(*this, &NetworkConnection::on_connect_connected));
}

void NetworkConnection::send(int type, const Glib::ustring &pay)
{
  queue_message (type, pay);
}

void NetworkConnection::sendFile(int type, const Glib::ustring &filename)
{
  queue_message (type, filename);
}

void NetworkConnection::sendFileMessage(int type, const Glib::ustring filename)
{
  FILE *fileptr = fopen (filename.c_str(), "r");
  if (fileptr == NULL)
    return;

  goffset file_size = File::get_size (filename);
  // write the preamble
  gchar buf[MESSAGE_HEADER_SIZE];
  guint32 l = g_htonl(MESSAGE_PREAMBLE_EXTRA_BYTES + file_size);
  memcpy(buf, &l, MESSAGE_SIZE_BYTES);
  buf[MESSAGE_SIZE_BYTES] = MESSAGE_PROTOCOL_VERSION;
  buf[MESSAGE_SIZE_BYTES + 1] = type;
  
  gsize bytessent = 0;
  bool wrote_all =  out->write_all ((const void*) buf, sizeof (buf), bytessent);
  if (wrote_all)
    {
      char *buffer = (char*) malloc (file_size);
      ssize_t bytesread = fread (buffer, 1, file_size, fileptr);
      fclose (fileptr);
      wrote_all = out->write_all (buffer, bytesread, bytessent);
      free (buffer);
    }
  File::erase(filename);
}

bool NetworkConnection::sendMessage(int type, const Glib::ustring &pay)
{
  // make the preamble
  guint32 l = g_htonl(MESSAGE_PREAMBLE_EXTRA_BYTES + pay.size());
  gchar *buf = (gchar*) malloc ((MESSAGE_HEADER_SIZE  + pay.size())* sizeof (gchar));
  memcpy(buf, &l, MESSAGE_SIZE_BYTES);
  buf[MESSAGE_SIZE_BYTES] = MESSAGE_PROTOCOL_VERSION;
  buf[MESSAGE_SIZE_BYTES + 1] = type;
  
  //concatenate the payload
  memcpy (&buf[MESSAGE_HEADER_SIZE], pay.c_str(), pay.size());
  gsize bytessent = 0;
  bool wrote_all = false;
  try
    {
      wrote_all =
        out->write_all (buf, (MESSAGE_HEADER_SIZE + pay.size()) *
                        sizeof (gchar), bytessent);
    }
  catch (Gio::Error &ex)
    {
      free (buf);
      tear_down_connection(false);
      connection_lost.emit();
      return false;
    }
  free (buf);
  return wrote_all;
}
  
bool NetworkConnection::on_connect_timeout()
{
  d_connect_timer.disconnect();
  d_stop = true;
  d_cancellable->cancel();
  connection_failed.emit();
  return Timing::STOP;
}

Glib::ustring NetworkConnection::get_peer_hostname()
{
  Glib::RefPtr<Gio::InetSocketAddress> iconn = Glib::wrap((GInetSocketAddress*)(conn->get_remote_address()->gobj()), true);
  Glib::ustring h = iconn->get_address()->to_string();
  size_t pos = h.rfind(':');
  if (pos == Glib::ustring::npos)
    return h;
  else
    return h.substr(pos + 1);
}

void NetworkConnection::disconnect()
{
  if (conn)
    {
      if (conn->get_socket()->is_closed() == false)
        conn->get_socket()->close();
    }
}

void NetworkConnection::queue_message(int type, const Glib::ustring &pay)
{
  std::unique_lock<std::mutex> lock (mutex);

  while(messages.size() >= 256)
    {
      if (d_stop)
        break;
      cond_pop.wait(lock);
      if (d_stop)
        break;
    }
  if (d_stop)
    return;
  struct Message m;
  m.type = type;
  m.payload = pay;
  messages.push(m);
  cond_push.notify_one();
}

void NetworkConnection::send_queued_messages()
{
  for(;;)
    {
        {
          if (d_stop)
            break;
          std::unique_lock<std::mutex> lock (mutex);
          while(messages.empty())
            {
              if (d_stop)
                break;
              cond_push.wait(lock);
              if (d_stop)
                break;
            }
          if (d_stop)
            break;
          struct Message m = messages.front();
          messages.pop();

          if (d_stop)
            break;
          if (m.type == MESSAGE_TYPE_SENDING_MAP)
            sendFileMessage (m.type, m.payload);
          else
            sendMessage (m.type, m.payload);
          if (d_stop)
            break;
          cond_pop.notify_one();
        }
    }
  queue_flushed.emit();
}
