//  Copyright (C) 2006 Ulf Lorenz
//  Copyright (C) 2006 Andrea Paternesi
//  Copyright (C) 2006, 2007, 2008, 2014, 2015, 2020, 2026 Ben Asselstine
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

#include <config.h>

#include <glibmm.h>
#include <iostream>
#include <sigc++/functors/mem_fun.h>
#include "snd.h"
#include "file.h"
#include "configuration.h"
#include "defs.h"
#include "xml-helper.h"
#include "rnd.h"
#include "scenario-media.h"

#include <gst/gst.h>

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

struct Impl
{
  GstElement *back;
  GstElement *effect;
  Snd *snd;
};
struct Impl *impl;

int nloops;

gboolean on_bus_message (GstBus *bus, GstMessage *message, gpointer user_data)
{
  (void)bus;
  int source = GPOINTER_TO_INT (user_data);
  switch (GST_MESSAGE_TYPE (message))
    {
    case GST_MESSAGE_EOS:
      if (source == 0)
        {
          if (nloops > 0)
            nloops--;
          if (nloops == 0)
            return TRUE;
          gst_element_seek
            (impl->effect,
             1.0,
             GST_FORMAT_TIME,
             GST_SEEK_FLAG_FLUSH,
             GST_SEEK_TYPE_SET, 0,
             GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE);
        }
      else if (source == 1)
        {
          impl->snd->nextPiece ();
        }
      break;
    default:
      break;
    }
  return true;
}

Snd* Snd::s_instance = 0;

Snd* Snd::instance ()
{
  if (s_instance == 0)
    s_instance = new Snd ();

  return s_instance;
}

void Snd::deleteInstance ()
{
  if (s_instance == 0)
    return;

  delete s_instance;
  s_instance = 0;
}

Snd::Snd ()
        :d_broken (false), d_background (false)
{
    impl = new Impl ();
    impl->snd = this;
    nloops = 1;
    debug("Snd constructor")

    XML_Helper helper (File::getMusicFile ("music.xml"), std::ios::in);
    helper.register_tag ("piece", sigc::mem_fun(*this, &Snd::loadMusic));

    if (!helper.parse_XML ())
      {
        std::cerr<<
          _("Error loading music descriptions; disabling music.") << std::endl;
        d_broken = true;
        return;
      }
    helper.close ();

    impl->back = gst_element_factory_make ("playbin", NULL);
    impl->effect = gst_element_factory_make ("playbin", NULL);
    GstBus *effect_bus = gst_element_get_bus (impl->effect);
    GstBus *back_bus = gst_element_get_bus (impl->back);
    gst_bus_add_watch (effect_bus, (GstBusFunc) on_bus_message,
                       GINT_TO_POINTER(0));
    gst_bus_add_watch (back_bus, (GstBusFunc) on_bus_message,
                       GINT_TO_POINTER(1));

    debug("Music list contains " << d_musicMap.size <<" entries.")
    debug("background list has " << d_bgMap.size <<" entries.")
}

Snd::~Snd ()
{
  debug("Snd destructor")
    halt (false);
  disableBackground ();

  // remove all music pieces
  std::map<Glib::ustring, MusicItem*>::iterator it;
  for (it = d_musicMap.begin (); it != d_musicMap.end (); ++it)
    delete (*it).second;

  gst_element_set_state (impl->effect, GST_STATE_NULL);
  gst_element_set_state (impl->back, GST_STATE_NULL);

  gst_object_unref (impl->effect);
  gst_object_unref (impl->back);

  impl->effect = NULL;
  impl->back   = NULL;
}

bool Snd::isMusicEnabled ()
{
  return Configuration::s_musicenable;
}

int Snd::getMusicVolume ()
{
  return Configuration::s_musicvolume;
}

typedef struct
{
  GstElement *effect;
  double step;
} FadeData;


gboolean on_effect_fade_callback (gpointer user_data)
{
  gdouble max = (double)Configuration::s_musicvolume / 128.0;

  FadeData *fade = (FadeData*)user_data;

  gdouble volume = 0.0;
  g_object_get (fade->effect, "volume", &volume, NULL);

  if (fade->step < 0)
    {
      if (volume > std::abs (fade->step))
        g_object_set (fade->effect, "volume", volume + fade->step, NULL);
      else
        g_object_set (fade->effect, "volume", 0, NULL);
    }
  else if (fade->step > 0)
    {
      if (volume < max - fade->step)
        g_object_set (fade->effect, "volume", volume + fade->step, NULL);
      else
        g_object_set (fade->effect, "volume", max, NULL);
    }
  if (fade->step < 0 && volume <= 0.0)
    {
      g_object_set (fade->effect, "volume", 0, NULL);
      g_free (fade);
      return FALSE;
    }
  if (fade->step > 0 && volume >= max)
    {
      g_object_set (fade->effect, "volume", max, NULL);
      g_free (fade);
      return FALSE;
    }

  return TRUE;
}

bool Snd::play (Glib::ustring piece, int loops, bool fade)
{
  if (d_broken || !Configuration::s_musicenable)
    return true;

  MusicItem *item = ScenarioMedia::instance()->getSoundEffect (piece);
  if (!item)
    item = d_musicMap[piece];
  printf ("item is %p\n", item);
  // first, load the music piece
  if (item == NULL)
    return false;

  nloops = loops;

  std::string uri = Glib::filename_to_uri (File::getMusicFile (item->file));

  GstElement *video_sink = gst_element_factory_make ("fakesink", NULL);
  GstElement *audio_sink = gst_element_factory_make ("autoaudiosink", "output");

  g_object_set (impl->effect,
                "uri", uri.c_str (),
                "video-sink", video_sink,
                "audio-sink", audio_sink,
                NULL);

  if (fade)
    {
      g_object_set (impl->effect, "volume", 0.0, NULL);

      FadeData *f = g_new (FadeData, 1);
      f->effect = impl->effect;
      f->step = 0.01;
      g_timeout_add (10, on_effect_fade_callback, f);
    }
  else
    {
      double volume = (double)Configuration::s_musicvolume / 128.0;
      g_object_set (impl->effect, "volume", volume, NULL);
    }

  gst_element_set_state (impl->effect, GST_STATE_PLAYING);

  gst_object_unref (video_sink);
  gst_object_unref (audio_sink);

  return true;
}

  
bool Snd::halt (bool fade)
{
  (void)fade;
  debug("stopping music")

  if (fade == false)
    gst_element_set_state (impl->effect, GST_STATE_NULL);
  else
    {
      FadeData *f = g_new (FadeData, 1);
      f->effect = impl->effect;
      f->step = -0.02;
      g_timeout_add (100, on_effect_fade_callback, f);
    }
  return true;
}

void Snd::enableBackground ()
{
    debug("enabling background music")
    d_background = true;
    nextPiece ();
}

void Snd::disableBackground ()
{
    debug("disabling background music")
    d_background = false;

    gst_element_set_state (impl->back, GST_STATE_NULL);
}

void Snd::nextPiece ()
{
    debug("Snd::nextPiece")
    if (!d_background || !isMusicEnabled ())
        return;

    std::vector<Glib::ustring> bgmap = d_bgMap;
    std::map<Glib::ustring, MusicItem*> map = d_musicMap;
    if (ScenarioMedia::instance ()->getBackgroundMusic().empty () == false)
      {
        bgmap = ScenarioMedia::instance ()->getBackgroundMusic ();
        map = ScenarioMedia::instance ()->getSounds ();
      }

    // select a random music piece from the list of background pieces
    while (!map.empty ())
      {
        int i = Rnd::rand () % d_bgMap.size ();
        if (!File::exists (File::getMusicFile (map[bgmap[i]]->file)))
          continue;

        std::string uri =
          Glib::filename_to_uri (File::getMusicFile (map[bgmap[i]]->file));

        GstElement *video_sink = gst_element_factory_make ("fakesink", NULL);
        GstElement *audio_sink = gst_element_factory_make ("autoaudiosink",
                                                           "output");

        double volume = (double)Configuration::s_musicvolume / 128.0;

        g_object_set (impl->back,
                      "uri", uri.c_str (),
                      "video-sink", video_sink,
                      "audio-sink", audio_sink,
                      "volume", volume,
                      NULL);

        gst_object_unref (video_sink);
        gst_object_unref (audio_sink);

        gst_element_set_state (impl->back, GST_STATE_PLAYING);
        break;
      }
}

bool Snd::loadMusic (Glib::ustring tag, XML_Helper* helper)
{
  if (tag != "piece")
    {
      std::cerr << "Loading music: Wrong tag name" << std::endl;
      return false;
    }

  Glib::ustring tagname;
  MusicItem* item = new MusicItem ();

  bool retval = true;
  retval &= helper->get (tagname, "name");
  retval &= helper->get (item->file, "filename");
  retval &= helper->get (item->background, "background");
  retval &= helper->get (item->alias, "alias");

  if (retval)
    {
      d_musicMap[tagname] = item;
      if (item->background)
        d_bgMap.push_back (tagname);
    }
  else
    delete item;

  return retval;
}
        
void Snd::updateVolume ()
{
  gdouble max = (double)Configuration::s_musicvolume / 128.0;
  g_object_set (impl->effect, "volume", max, NULL);
  g_object_set (impl->back, "volume", max, NULL);
}

Glib::ustring Snd::getFile (Glib::ustring piece)
{
  MusicItem *item = d_musicMap[piece];
  if (!item)
    return "";
  return item->file;
}
