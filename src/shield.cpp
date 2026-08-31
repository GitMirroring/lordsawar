//  Copyright (C) 2008, 2009, 2010, 2011, 2014, 2015, 2020, 2021,
//  2026 Ben Asselstine
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

#include <fstream>
#include <sstream>
#include "shield.h"
#include "xml-helper.h"
#include "ucompose.hpp"
#include "shield-set.h"
#include "tar-helper.h"
#include "tar-file-masked-image.h"

Glib::ustring Shield::d_tag = "shield";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
#define debug(x)

Shield::Shield(XML_Helper* helper)
{
  guint32 shield;
  helper->get(shield, "owner");
  d_owner = Shield::Color (shield);
  helper->get(d_colors, "color");
}

Shield::Shield(const Shield& s)
: std::list<ShieldStyle*>(), Tartan(s), sigc::trackable(s), d_owner(s.d_owner),
    d_colors(s.d_colors)
{
  for (const_iterator it = s.begin(); it != s.end(); ++it)
    push_back(new ShieldStyle(*(*it)));
}

Shield::Shield(Shield::Color shield, std::vector<Gdk::RGBA> colors)
:Tartan()
{
  d_owner = shield;
  d_colors = colors;
}

Shield::~Shield()
{
  for (iterator it = begin(); it != end(); ++it)
      delete *it;
}

std::vector<Gdk::RGBA> Shield::get_default_colors(int shield)
{
  Gdk::RGBA c;
  switch (shield % MAX_PLAYERS)
    {
    case Shield::WHITE: c.set_rgba(252.0/255.0,252.0/255.0,252.0/255.0); break;
    case Shield::GREEN: c.set_rgba(80.0/255.0, 195.0/255.0, 28.0/255.0); break;
    case Shield::YELLOW: c.set_rgba(252.0/255.0,236.0/255.0,32.0/255.0); break;
    //case Shield::DARK_BLUE: c.set_rgba(0,252.0/255.0,252.0/255.0); break;
    case Shield::DARK_BLUE: c.set_rgba(22.0/255.0,92.0/255.0, 252.0/255.0); break;
    case Shield::ORANGE: c.set_rgba(252.0/255.0,160.0/255.0,0);break;
    case Shield::LIGHT_BLUE: 
		      c.set_rgba(44.0/255.0,184.0/255.0,252.0/255.0); break;
    case Shield::RED: c.set_rgba(196.0/255.0, 28.0/255.0, 0); break;
    case Shield::BLACK: c.set_rgba(0,0,0); break;
    }
    
    std::vector<Gdk::RGBA> l;
    l.push_back (c);
    return l;
}

std::vector<Gdk::RGBA> Shield::get_default_colors_for_neutral()
{
  Gdk::RGBA color;
  color.set_rgba(204.0/255.0,204.0/255.0,204.0/255.0);
  std::vector<Gdk::RGBA> l;
  l.push_back (color);
  return l;
}

Glib::ustring Shield::colorToString(const Shield::Color c)
{
  switch (c)
    {
    case Shield::WHITE: return "Shield::WHITE";
    case Shield::GREEN: return "Shield::GREEN";
    case Shield::YELLOW: return "Shield::YELLOW";
    case Shield::LIGHT_BLUE: return "Shield::LIGHT_BLUE";
    case Shield::RED: return "Shield::RED";
    case Shield::DARK_BLUE: return "Shield::DARK_BLUE";
    case Shield::ORANGE: return "Shield::ORANGE";
    case Shield::BLACK: return "Shield::BLACK";
    case Shield::NEUTRAL: return "Shield::NEUTRAL";
    }
  return "Shield::NEUTRAL";
}

Glib::ustring Shield::colorToFriendlyName (const Shield::Color c)
{
  switch (c)
    {
    case Shield::WHITE: return _("White");
    case Shield::GREEN: return _("Green");
    case Shield::YELLOW: return _("Yellow");
    case Shield::LIGHT_BLUE: return _("Light Blue");
    case Shield::RED: return _("Red");
    case Shield::DARK_BLUE: return _("Dark Blue");
    case Shield::ORANGE: return _("Orange");
    case Shield::BLACK: return _("Black");
    case Shield::NEUTRAL: return _("Neutral");
    }
  return _("Neutral");
}

Shield::Color Shield::colorFromFriendlyName (const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Shield::Color (atoi(str.c_str()));

  if (str == _("White"))
    return Shield::WHITE;
  else if (str == _("Green"))
    return Shield::GREEN;
  else if (str == _("Yellow"))
    return Shield::YELLOW;
  else if (str == _("Light Blue"))
    return Shield::LIGHT_BLUE;
  else if (str == _("Red"))
    return Shield::RED;
  else if (str == _("Dark Blue"))
    return Shield::DARK_BLUE;
  else if (str == _("Orange"))
    return Shield::ORANGE;
  else if (str == _("Black"))
    return Shield::BLACK;
  else if (str == _("Neutral"))
    return Shield::NEUTRAL;
  return Shield::WHITE;
}


bool Shield::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->open_tag(d_tag);
  guint32 owner = d_owner;
  retval &= helper->save("owner", owner);
  retval &= helper->save("color", d_colors);
  for (const_iterator it = begin(); it != end(); ++it)
    (*it)->save(helper);
  retval &= saveTartan(helper);
  retval &= helper->close_tag();
  return retval;
}
	
ShieldStyle *Shield::getFirstShieldstyle(ShieldStyle::Type type)
{
  for (iterator i = begin(); i != end(); ++i)
    {
      if (ShieldStyle::Type((*i)->getType()) == type)
	return *i;
    }
  return NULL;
}
    
guint32 Shield::get_next_shield(guint32 color)
{
  if (color == Shield::NEUTRAL)
    {
      color = Shield::WHITE;
      return color;
    }
  color++;
  return color;
}

Shield::Color Shield::colorFromString(const Glib::ustring str)
{
  if (str.size() > 0 && isdigit(str.c_str()[0]))
    return Shield::Color (atoi(str.c_str()));
  if (str == "Shield::WHITE")
    return Shield::WHITE;
  else if (str == "Shield::GREEN")
    return Shield::GREEN;
  else if (str == "Shield::YELLOW")
    return Shield::YELLOW;
  else if (str == "Shield::LIGHT_BLUE")
    return Shield::LIGHT_BLUE;
  else if (str == "Shield::RED")
    return Shield::RED;
  else if (str == "Shield::DARK_BLUE")
    return Shield::DARK_BLUE;
  else if (str == "Shield::ORANGE")
    return Shield::ORANGE;
  else if (str == "Shield::BLACK")
    return Shield::BLACK;
  else if (str == "Shield::NEUTRAL")
    return Shield::NEUTRAL;
  return Shield::WHITE;
}

void Shield::calculate_progress_width (guint32 iwidth, PixMask *left, PixMask *center, PixMask *right, guint32 &width, guint32 &centers, bool &include_right)
{
  //calculate the width, ugh.
  for (width = left->get_width(); width < iwidth - right->get_width();
       width += center->get_width())
    centers++;
  width += right->get_width();
  include_right = true;
  for (guint32 j = 0; j < centers; j++)
    {
      if (width > iwidth)
        {
          width -= center->get_width();
          if (centers)
            centers--;
        }
      else
        break;
    }
  if (width > iwidth)
    {
      width -= right->get_width();
      include_right = false;
    }
}

PixMask *Shield::get_progress_bar_completed (Shieldset *shieldset,
                                             Shield::Color shield,
                                             guint32 width)
{
  Shield *ss = shieldset->lookupShieldByColor (shield);

  TarFileMaskedImage *mim = ss->getTartanMaskedImage (Tartan::LEFT);
  PixMask *left = mim->applyMask (shieldset, shield);

  mim = ss->getTartanMaskedImage (Tartan::CENTER);
  PixMask *center = mim->applyMask (shieldset, shield);

  mim = ss->getTartanMaskedImage (Tartan::RIGHT);
  PixMask *right = mim->applyMask (shieldset, shield);

  //okay, so we have our left, right and center images, now we need to
  //concatenate them together

  guint32 w = 0, num_centers = 0;
  bool include_right = false;
  calculate_progress_width (width, left, center, right, w, num_centers,
                            include_right);
  //okay w is the actual width of the image, which is the same or less than
  //the width we asked for.  it has NUM_CENTERS center pieces and it may or may
  //not have an ending piece (if we can fit it.)
  //we always have the leftmost piece though because we have to show something.
  Glib::RefPtr<Gdk::Pixbuf> pixbuf =
    Gdk::Pixbuf::create (Gdk::Colorspace::RGB, true, 8, w, left->get_height ());
  pixbuf->fill (0x00000000);
  PixMask *tartan = PixMask::create (pixbuf);

  //blit the left image
  guint32 l = 0;
  left->blit (tartan->get_pixmap (), l, 0);
  l += left->get_width ();

  //blit the center images
  for (guint32 j = 0; j < num_centers; j++)
    {
      center->blit (tartan->get_pixmap (), l, 0);
      l += center->get_width ();
    }

  //blit the right image
  if (include_right)
    right->blit (tartan->get_pixmap (), l, 0);
  delete left;
  delete center;
  delete right;
  return tartan;
}

PixMask *Shield::get_progress_bar_uncompleted (Shieldset *shieldset,
                                               Shield::Color shield,
                                               guint32 width)
{
  Shield *ss = shieldset->lookupShieldByColor (shield);
  //okay, here's where we fashion the new image.
  //we take the leftmost tartan image for this player
  //and then we repeat the center tartan image a bunch of times
  //and then finally we cap it off with the rightmost tartan image
  //the images are all masked in the player's color.

  //these empty tartan pictures are the same as the regular tartan pictures
  //except they're not colored in the player's color.

  TarFileMaskedImage *mim = ss->getTartanMaskedImage (Tartan::LEFT);
  PixMask *left = mim->getImage ()->copy ();

  mim = ss->getTartanMaskedImage (Tartan::CENTER);
  PixMask *center = mim->getImage ()->copy ();

  mim = ss->getTartanMaskedImage (Tartan::RIGHT);
  PixMask *right = mim->getImage ()->copy ();

  //okay, so we have our left, right and center images, now we need to
  //concatenate them together

  guint32 w = 0, num_centers = 0;
  bool include_right = false;
  calculate_progress_width (width, left, center, right, w, num_centers,
                            include_right);
  //okay w is the actual width of the image, which is the same or less than
  //the width we asked for.  it has NUM_CENTERS center pieces and it may or may
  //not have an ending piece (if we can fit it.)
  //we always have the leftmost piece though because we have to show something.
  Glib::RefPtr<Gdk::Pixbuf> pixbuf =
    Gdk::Pixbuf::create (Gdk::Colorspace::RGB, true, 8, w, left->get_height ());
  pixbuf->fill (0x00000000);
  PixMask *tartan = PixMask::create (pixbuf);

  //blit the left image
  guint32 l = 0;
  left->blit (tartan->get_pixmap (), l, 0);
  l += left->get_width ();

  //blit the center images
  for (guint32 j = 0; j < num_centers; j++)
    {
      center->blit (tartan->get_pixmap (), l, 0);
      l += center->get_width ();
    }

  //blit the right image
  if (include_right)
    right->blit (tartan->get_pixmap (), l, 0);
  delete left;
  delete center;
  delete right;
  return tartan;
}
