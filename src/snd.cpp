// Copyright (C) 2006 Ulf Lorenz
// Copyright (C) 2006 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2014, 2015, 2020 Ben Asselstine
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

#include <config.h>

#include <glibmm.h>
#include <iostream>
#include <sigc++/functors/mem_fun.h>
#include "snd.h"
#include "File.h"
#include "Configuration.h"
#include "defs.h"
#include "xmlhelper.h"
#include "timing.h"
#include "rnd.h"
#include "ScenarioMedia.h"

#ifdef LW_SOUND
#include <gstreamermm.h>
#include <gstreamermm/playbin.h>
#endif

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

struct Snd::Impl
{
#ifdef LW_SOUND
  // currently playing background and foreground piece
  Glib::RefPtr<Gst::PlayBin> back;
  Glib::RefPtr<Gst::PlayBin> effect;
#endif
  int placeholder;
};

Snd* Snd::s_instance = 0;

Snd* Snd::getInstance()
{
    if (s_instance == 0)
        s_instance = new Snd();

    return s_instance;
}

void Snd::deleteInstance()
{
    if (s_instance == 0)
        return;

    delete s_instance;
    s_instance = 0;
}

Snd::Snd()
    :d_nloops(1), d_broken(false), d_background(false), impl(new Impl())
{
    debug("Snd constructor")

    XML_Helper helper(File::getMusicFile("music.xml"), std::ios::in);
    helper.registerTag("piece", sigc::mem_fun(this, &Snd::loadMusic));

    if (!helper.parseXML())
    {
        std::cerr<< _("Error loading music descriptions; disabling music.") << std::endl;
        d_broken = true;
	return;
    }
    helper.close();

#ifdef LW_SOUND
    impl->back = Gst::PlayBin::create();
    impl->effect = Gst::PlayBin::create();
    impl->effect->get_bus()->add_watch(sigc::bind(sigc::hide<0>(sigc::mem_fun(*this, &Snd::on_bus_message)), 0));
    impl->back->get_bus()->add_watch(sigc::bind(sigc::hide<0>(sigc::mem_fun(*this, &Snd::on_bus_message)), 1));
#endif
    debug("Music list contains " <<d_musicMap.size <<" entries.")
    debug("background list has " <<d_bgMap.size <<" entries.")
}

Snd::~Snd()
{
    debug("Snd destructor")
    halt(false);
    disableBackground();
    
    // remove all music pieces
    std::map<Glib::ustring, MusicItem*>::iterator it;
    for (it = d_musicMap.begin(); it != d_musicMap.end(); ++it)
        delete (*it).second;
}

bool Snd::isMusicEnabled()
{
    return Configuration::s_musicenable;
}

int Snd::getMusicVolume()
{
    return Configuration::s_musicvolume;
}

bool Snd::play(Glib::ustring piece, int nloops, bool fade)
{
  (void)piece;
  (void)nloops;
  (void)fade;
  debug("playing Music")
    if (d_broken || !Configuration::s_musicenable)
      return true;

  MusicItem *item = ScenarioMedia::getInstance()->getSoundEffect(piece);
  if (!item)
    item = d_musicMap[piece];
  // first, load the music piece
  if (item == NULL)
    return false;

#ifdef LW_SOUND
  d_nloops = nloops;
  impl->effect->set_state(Gst::STATE_NULL);
  impl->effect->property_uri() = 
    Glib::filename_to_uri(File::getMusicFile(item->file));
  impl->effect->property_video_sink() = Gst::FakeSink::create();
  impl->effect->property_audio_sink() = Gst::ElementFactory::create_element("autoaudiosink", "output");
  if (fade)
    {
      impl->effect->property_volume() = 0.0;
      Timing::instance().register_timer
        (sigc::bind(sigc::mem_fun(this, &Snd::on_effect_fade), 0.01), 100);
    }
  else
    impl->effect->property_volume() = (double)Configuration::s_musicvolume/128.0;
  impl->effect->set_state(Gst::STATE_PLAYING);

#endif
  return true;
}

bool Snd::on_bus_message(const Glib::RefPtr<Gst::Message> & msg, guint32 source)
{
  (void)msg;
  (void)source;
#ifdef LW_SOUND
  switch (msg->get_message_type())
    {
    case Gst::MESSAGE_EOS:
      if (source == 0)
        {
          if (d_nloops > 0)
            d_nloops--;
          if (d_nloops == 0)
            return TRUE;
          impl->effect->seek(Gst::FORMAT_TIME, Gst::SEEK_FLAG_FLUSH, 0);
        }
      else if (source == 1)
        {
          nextPiece();
        }
      break;
    default:
      break;
    }
#endif
  return true;
}
  
bool Snd::halt(bool fade)
{
  (void)fade;
  debug("stopping music")

#ifdef LW_SOUND
  if (fade == false)
    impl->effect->set_state(Gst::STATE_NULL);
  else
    Timing::instance().register_timer
      (sigc::bind(sigc::mem_fun(this, &Snd::on_effect_fade), -0.02), 100);
#endif
  return true;
}

bool Snd::on_effect_fade(double step)
{
  (void)step;
#ifdef LW_SOUND
  double volume = impl->effect->property_volume();
  double max = (double)Configuration::s_musicvolume/128.0;
  if (step < 0)
    {
      if (volume > std::abs(step))
        impl->effect->property_volume() = (volume + step);
      else
        impl->effect->property_volume() = 0.0;
    }
  else if (step > 0)
    {
      if (volume < max-step)
        impl->effect->property_volume() = (volume + step);
      else
        impl->effect->property_volume() = max;
    }
  if (step < 0 && impl->effect->property_volume() <= 0.0)
    {
      impl->effect->property_volume() = 0.0;
      return Timing::STOP;
    }
  if (step > 0 && impl->effect->property_volume() >= max)
    {
      impl->effect->property_volume() = max;
      return Timing::STOP;
    }
#endif
  return Timing::CONTINUE;
}

void Snd::enableBackground()
{
    debug("enabling background music")
    d_background = true;
    nextPiece();
}

void Snd::disableBackground()
{
    debug("disabling background music")
    d_background = false;

#ifdef LW_SOUND
  impl->back->set_state(Gst::STATE_NULL);
#endif
}

void Snd::nextPiece()
{
    debug("Snd::nextPiece")
    if (!d_background || !isMusicEnabled())
        return;

    std::vector<Glib::ustring> bgmap = d_bgMap;
    std::map<Glib::ustring, MusicItem*> map = d_musicMap;
    if (ScenarioMedia::getInstance()->getBackgroundMusic().empty() == false)
      {
        bgmap = ScenarioMedia::getInstance()->getBackgroundMusic();
        map = ScenarioMedia::getInstance()->getSounds();
      }
#ifdef LW_SOUND
    // select a random music piece from the list of background pieces
    while (!map.empty())
      {
        int i = Rnd::rand() % d_bgMap.size();
        if (!File::exists(File::getMusicFile(map[bgmap[i]]->file)))
            continue;
        impl->back->set_state(Gst::STATE_NULL);
        impl->back->property_uri() = 
          Glib::filename_to_uri(File::getMusicFile(map[bgmap[i]]->file));
        impl->back->property_video_sink() = Gst::FakeSink::create();
        impl->back->property_audio_sink() = Gst::ElementFactory::create_element("autoaudiosink", "output");
        impl->back->property_volume() = (double)Configuration::s_musicvolume/128.0;
        impl->back->set_state(Gst::STATE_PLAYING);
        break;
      }
#endif
}

bool Snd::loadMusic(Glib::ustring tag, XML_Helper* helper)
{
  if (tag != "piece")
    {
      std::cerr <<"Loading music: Wrong tag name\n";
      return false;
    }

  Glib::ustring tagname;
  MusicItem* item = new MusicItem();

  bool retval = true;
  retval &= helper->getData(tagname, "name");
  retval &= helper->getData(item->file, "filename");
  retval &= helper->getData(item->background, "background");
  retval &= helper->getData(item->alias, "alias");

  if (retval)
    {
      d_musicMap[tagname] = item;
      if (item->background)
        d_bgMap.push_back(tagname);
    }
  else
    delete item;

  return retval;
}
        
void Snd::updateVolume()
{
#ifdef LW_SOUND
  impl->effect->property_volume() = (double)Configuration::s_musicvolume/128.0;
  impl->back->property_volume() = (double)Configuration::s_musicvolume/128.0;
#endif
}

Glib::ustring Snd::getFile(Glib::ustring piece)
{
  MusicItem *item = d_musicMap[piece];
  if (!item)
    return "";
  return item->file;
}
