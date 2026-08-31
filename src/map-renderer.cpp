// Copyright (C) 2003 Michael Bartl
// Copyright (C) 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2005 Andrea Paternesi
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2012, 2014, 2015, 2020,
// 2021 Ben Asselstine
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

#include "MapRenderer.h"
#include "army.h"
#include "GameMap.h"
#include "player.h"
#include "FogMap.h"
#include "ImageCache.h"
#include "playerlist.h"
#include "File.h"
#include "tileset.h"

MapRenderer::MapRenderer(Cairo::RefPtr<Cairo::Surface> surface)
 : d_surface (surface), gc (Cairo::Context::create(surface))
{
}
 
void MapRenderer::render(int x, int y, int tileStartX, int tileStartY,
			 int columns, int rows)
{
  return render(x, y, tileStartX, tileStartY, columns, rows, d_surface, gc);
}

void MapRenderer::render_tile(Vector<int> draw, Vector<int> tile,
			      Cairo::RefPtr<Cairo::Surface> surface)
{
  Player *p = Playerlist::getActiveplayer();
  if (p->getFogMap()->isCompletelyObscuredFogTile(tile) == true)
    return;

  // get correct tile
  Maptile *mtile = GameMap::getInstance()->getTile(tile);

  TileStyle *style = mtile->getTileStyle(GameMap::getTileset ());
  bool use_default_pic = false;
  if (style == NULL)
    {
      printf ("style for tile %d at col=%d,row=%d is null\n",
              mtile->getType(), tile.x, tile.y);
      use_default_pic = true;
    }
  else
    {
      if (style->getImage() == NULL)
	{
	  printf ("pic for style %d for tile %d at %d,%d is null\n",
		  style->getType(), mtile->getType(), tile.x, tile.y);
          use_default_pic = true;
	}
    }
		
  if (use_default_pic == true)
    {
      guint32 type = TileStyle::OTHER;
      if (style)
        type = style->getType();
      int tilesize = GameMap::getInstance()->getTileSize();
      PixMask *img = 
        ImageCache::getInstance()->getDefaultTileStylePic(type,
                                                             tilesize);
      if (img)
        img->blit(surface, draw.x, draw.y);
    }
  else
    style->getImage()->blit(surface, draw.x, draw.y);
}

void MapRenderer::render(int x, int y, int tileStartX, int tileStartY,
			 int columns, int rows, Cairo::RefPtr<Cairo::Surface> surface, Cairo::RefPtr<Cairo::Context> context)
{
  int width = GameMap::getWidth();
  int height = GameMap::getHeight();
  int tilesize = GameMap::getInstance()->getTileSize();
  int drawY = y;

  for (int tileY = tileStartY; tileY < (tileStartY + rows); tileY++)
    {
      int drawX = x;
      for (int tileX = tileStartX; tileX < (tileStartX + columns); tileX++)
        {
          // first check if we're out of the map bounds
          if (tileX >= width || tileY >= height) 
            {
              context->set_source_rgba(FOG_COLOR.get_red(), FOG_COLOR.get_blue(), FOG_COLOR.get_green(), FOG_COLOR.get_alpha());
              context->rectangle(drawX, drawY, tilesize, tilesize);
              context->fill();
            }
          else
            render_tile(Vector<int>(drawX,drawY), Vector<int>(tileX,tileY),
                        surface);

          drawX += tilesize;
        }
      drawY += tilesize;
    }

}
// End of file
