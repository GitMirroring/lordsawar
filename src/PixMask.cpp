// Copyright (C) 2009, 2010, 2011, 2014, 2015 Ben Asselstine
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

#include <iostream>
#include "defs.h"
#include "PixMask.h"
#include <string.h>
#include <cairomm/cairomm.h>
#include <gdkmm.h>
#include "ucompose.hpp"


PixMask::PixMask(Glib::RefPtr<Gdk::Pixbuf> pixbuf)
     : width(0), height(0), unscaled_width(0), unscaled_height(0)
{
  pixmap = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, pixbuf->get_width(), pixbuf->get_height());
  gc = Cairo::Context::create(pixmap);
  Gdk::Cairo::set_source_pixbuf(gc, pixbuf, 0, 0);
  gc->paint();
  mask = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, pixbuf->get_width(), pixbuf->get_height());
  unscaled_width = pixbuf->get_width();
  unscaled_height = pixbuf->get_height();
  width = unscaled_width;
  height = unscaled_height;
}

PixMask::~PixMask()
{
  pixmap.clear();
  mask.clear();
  gc.clear();
}

PixMask::PixMask(Cairo::RefPtr<Cairo::Surface> p, Cairo::RefPtr<Cairo::Surface> m)
    : width(0), height(0)
{
  gc = Cairo::Context::create(p);
  double x1, x2, y1, y2;
  gc->get_clip_extents (x1, y1, x2, y2);
  width = x2 - x1;
  height = y2 - y1;

  pixmap = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, width, height);
  if (p)
    {
      gc = Cairo::Context::create(pixmap);
      gc->rectangle(0, 0, width, height);
      gc->clip();
      gc->save();
      gc->set_source (p, 0, 0);
      gc->rectangle (0, 0, width, height);
      gc->clip();
      gc->paint();
      gc->restore();
    }

  mask = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, width, height);
  if (m)
    {
      Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create(mask);
      context->rectangle(0, 0, width, height);
      context->clip();
      context->save();
      context->set_source (m, 0, 0);
      context->rectangle (0, 0, width, height);
      context->clip();
      context->paint();
      context->restore();
    }
  unscaled_width = width;
  unscaled_height = height;
}

PixMask::PixMask(const PixMask&p)
{
  width = p.width;
  height = p.height;
  unscaled_width = p.unscaled_width;
  unscaled_height = p.unscaled_height;
  pixmap = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, width, height);
  if (p.pixmap)
    {
      gc = Cairo::Context::create(pixmap);
      gc->rectangle(0, 0, width, height);
      gc->clip();
      gc->save();
      gc->set_source (p.pixmap, 0, 0);
      gc->rectangle (0, 0, width, height);
      gc->clip();
      gc->paint();
      gc->restore();
    }

  mask = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, width, height);
  if (p.mask)
    {
      Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create(mask);
      context->rectangle(0, 0, width, height);
      context->clip();
      context->save();
      context->set_source (p.mask, 0, 0);
      context->rectangle (0, 0, width, height);
      context->clip();
      context->paint();
      context->restore();
    }
}

PixMask::PixMask(Glib::ustring filename, bool &broken)
     : width(0), height(0)
{
  if (Gtk::Main::instance() == NULL)
    {
      broken = true;
      return;
    }
  Glib::RefPtr<Gdk::Pixbuf> pixbuf;
  try
    {
      pixbuf = Gdk::Pixbuf::create_from_file(filename);
    }
  catch (const Glib::Exception &ex)
    {
      std::cerr << String::ucompose(_("Could not load image file `%1'."), filename) << std::endl;
      broken = true;
      return;
    }
  pixmap = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, pixbuf->get_width(), pixbuf->get_height());
  gc = Cairo::Context::create(pixmap);
  Gdk::Cairo::set_source_pixbuf(gc, pixbuf, 0, 0);
  gc->paint();
  mask = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, pixbuf->get_width(), pixbuf->get_height());
  width = pixbuf->get_width();
  height = pixbuf->get_height();
  unscaled_width = width;
  unscaled_height = height;
}

PixMask* PixMask::create(Glib::ustring filename, bool &broken)
{
  return new PixMask(filename, broken);
}

PixMask* PixMask::create(Glib::RefPtr<Gdk::Pixbuf> pixbuf)
{
  return new PixMask(pixbuf);
}

PixMask* PixMask::create(Cairo::RefPtr<Cairo::Surface> pixmap, Cairo::RefPtr<Cairo::Surface> mask)
{
  return new PixMask(pixmap, mask);
}

PixMask* PixMask::copy()
{
  return new PixMask(*this);
}

void PixMask::blit_centered(Cairo::RefPtr<Cairo::Surface> dest, Vector<int> pos)
{
  blit (dest, pos.x - (width/2), pos.y - (height/2));
  return;
}

void PixMask::blit(Cairo::RefPtr<Cairo::Surface> dest, Vector<int> pos)
{
  blit (dest, pos.x, pos.y);
  return;
}

void PixMask::blit(Cairo::RefPtr<Cairo::Surface> dest, int dest_x, int dest_y)
{
  //Here we are the map tile, blitting ourselves to the buffer where other 
  //map tiles live.
  Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create(dest);
  context->set_source (pixmap, dest_x, dest_y);
  context->paint();
}
     
void PixMask::blit(Rectangle src, Cairo::RefPtr<Cairo::Surface> p, Vector<int> dest)
{
  Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create(p);
  // Select the clipping rectangle
  context->rectangle(dest.x, dest.y, src.w, src.h);

  context->clip();
  context->save();
  context->set_source (pixmap, dest.x-src.x, dest.y-src.y);
  context->rectangle (0, 0, src.w, src.h);
  context->clip();
  context->paint();
  context->restore();
}

void PixMask::blit(Vector<int> tile, int ts, Cairo::RefPtr<Cairo::Surface> p, Vector<int> dest)
{
  Vector<int> src = tile * ts;
  blit (Rectangle(src.x, src.y, ts, ts), p, dest);
}

void PixMask::scale(PixMask*& p, int xsize, int ysize, Gdk::InterpType interp)
{
  PixMask *scaled = p->scale(xsize, ysize, interp);
  delete p;
  p = scaled;
  p->set_unscaled_width(p->get_unscaled_width());
  p->set_unscaled_height(p->get_unscaled_height());
  return;
}

PixMask * PixMask::scale(int xsize, int ysize, Gdk::InterpType interp)
{
  Glib::RefPtr<Gdk::Pixbuf> pixbuf = to_pixbuf();
  PixMask *pix = PixMask::create(pixbuf->scale_simple(xsize, ysize, interp));
  pix->set_unscaled_width(get_unscaled_width());
  pix->set_unscaled_height(get_unscaled_height());
  pixbuf.reset();
  return pix;
}

Glib::RefPtr<Gdk::Pixbuf> PixMask::to_pixbuf()
{
  Glib::RefPtr<Gdk::Pixbuf> buf = Gdk::Pixbuf::create(pixmap, 0, 0, width, height);
  Glib::RefPtr<Gdk::Pixbuf> alphabuf = buf->add_alpha(true, 255, 87, 204);
  return alphabuf;
}
      
void PixMask::draw_pixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, int src_x, int src_y, int dest_x, int dest_y, int w, int h)
{

  Cairo::RefPtr<Cairo::Context> context = Cairo::Context::create(pixmap);
  // Select the clipping rectangle
  context->rectangle(dest_x, dest_y, w, h);

  context->clip();
  context->save();
  PixMask *p = create(pixbuf);
  context->set_source (p->get_pixmap(), src_x, src_y);
  context->rectangle (src_x, src_y, w, h);
  context->clip();
  context->paint();
  context->restore();
  delete p;
}

int PixMask::get_depth()
{
    return 32;
}
     
Vector<int> PixMask::get_dim() const
{
  return Vector<int>(width, height);
}
