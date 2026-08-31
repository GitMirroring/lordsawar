//  Copyright (C) 2007, 2008, 2010, 2011, 2014, 2015, 2020 Ben Asselstine
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

#include <sigc++/functors/mem_fun.h>

#include "tilestyleset.h"

#include "xmlhelper.h"
#include "gui/image-helpers.h"
#include "File.h"
#include "tileset.h"
#include "tarhelper.h"

Glib::ustring TileStyleSet::d_tag = "tilestyleset";

#include <iostream>

        
TileStyleSet::TileStyleSet()
{
}

TileStyleSet::TileStyleSet(const TileStyleSet &t)
  : sigc::trackable(t), std::vector<TileStyle*>(), d_name(t.d_name)
{
  for (TileStyleSet::const_iterator i = t.begin(); i != t.end(); ++i)
    push_back(new TileStyle(*(*i)));
}
        
bool TileStyleSet::validate_image(Glib::ustring filename)
{
  return image_width_is_multiple_of_image_height (filename);
}

TileStyleSet::TileStyleSet(Glib::ustring file, guint32 tilesize, bool &success, TileStyle::Type type)
{
  success = validate_image(file);
  if (success == false)
    return;
    
  guint32 width;
  guint32 height;
  bool broken = false;
  get_image_width_and_height (file, width, height, broken);
  if (!broken)
    {
      d_name = File::get_basename(file, true);
      guint32 num_tilestyles = width / height;
      for (guint32 i = 0; i < num_tilestyles; i++)
        push_back(new TileStyle(0, type));
      loadImages(tilesize, file, true, broken);
      if (!broken)
        success = true;
      else
        success = false;
    }
  else
    success = false;
}

TileStyleSet::TileStyleSet(XML_Helper *helper)
{
  helper->getData(d_name, "name"); 
  File::add_png_if_no_ext (d_name);
}

TileStyleSet::~TileStyleSet()
{
  for (unsigned int i=0; i < size(); i++)
    delete (*this)[i];
  clear();
}

bool TileStyleSet::save(XML_Helper *helper) const
{
  bool retval = true;

  retval &= helper->openTag(TileStyleSet::d_tag);
  retval &= helper->saveData("name", d_name);
  for (TileStyleSet::const_iterator i = begin(); i != end(); ++i)
    retval &= (*i)->save(helper);
  retval &= helper->closeTag();

  return retval;
}

void TileStyleSet::getUniqueTileStyleTypes(std::list<TileStyle::Type> &types) const
{
  for (TileStyleSet::const_iterator i = begin(); i != end(); ++i)
    if (find (types.begin(), types.end(), (*i)->getType()) == types.end())
      types.push_back((*i)->getType());
}

bool TileStyleSet::validate() const
{
  if (getName().empty() == true)
    return false;
  return true;
}

void TileStyleSet::uninstantiateImages()
{
  for (unsigned int i = 0; i < size(); i++)
    (*this)[i]->uninstantiateImage();
}

void TileStyleSet::loadImages(int tilesize, Glib::ustring filename,
                              bool scale, bool &broken)
{
  if (filename.empty() == false && !broken)
    {
      std::vector<PixMask *> styles = disassemble_row(filename, size(), broken);
      if (!broken)
        {
          for (unsigned int i = 0; i < size(); i++)
            {
              if (scale)
                PixMask::scale(styles[i], tilesize, tilesize);
              (*this)[i]->setImage(styles[i]);
            }
        }
    }
}

bool TileStyleSet::instantiateImages (Tileset *set)
{
  uninstantiateImages ();
  bool broken = false;
  Tar_Helper t(set->getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getName();
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        {
          std::vector<PixMask *> styles =
            disassemble_row(filename, size(), broken);
          if (!broken)
            {
              for (unsigned int i = 0; i < size(); i++)
                (*this)[i]->setImage(styles[i]);
            }
        }
    }
  return broken;
}
// End of file
