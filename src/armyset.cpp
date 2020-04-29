//  Copyright (C) 2007, 2008, 2009, 2010, 2011, 2014, 2015, 2020 Ben Asselstine
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
#include <gtkmm.h>
#include "rectangle.h"
#include <sigc++/functors/mem_fun.h>

#include "armyset.h"
#include "File.h"
#include "shield.h"
#include "gui/image-helpers.h"
#include "armysetlist.h"
#include "armyprodbase.h"
#include "tarhelper.h"
#include "Configuration.h"
#include "file-compat.h"
#include "ucompose.hpp"
#include "xmlhelper.h"
#include "rnd.h"
#include "player.h"
#include "ImageCache.h"

Glib::ustring Armyset::d_tag = "armyset";
Glib::ustring Armyset::file_extension = ARMYSET_EXT;

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

#define DEFAULT_ARMY_TILE_SIZE 40
Armyset::Armyset(guint32 id, Glib::ustring name)
 : Set(ARMYSET_EXT, id, name, DEFAULT_ARMY_TILE_SIZE), d_bag(0)
{
  d_bag_name = "";
  d_stackship_name = "";
  d_standard_name = "";
  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      setLargeSelectorFilename (c, "");
      setSmallSelectorFilename (c, "");
    }

  clear_vectors ();
}

void Armyset::read_selector_name (XML_Helper *helper, Shield::Colour c, bool large)
{
  Glib::ustring name = "";
  Glib::ustring file = "";
  if (c == Shield::NEUTRAL)
    return;
  switch (c)
    {
    case Shield::WHITE: name = "white"; break;
    case Shield::GREEN: name = "green"; break;
    case Shield::YELLOW: name = "yellow"; break;
    case Shield::LIGHT_BLUE: name = "light_blue"; break;
    case Shield::ORANGE: name = "orange"; break;
    case Shield::DARK_BLUE: name = "dark_blue"; break;
    case Shield::RED: name = "red"; break;
    case Shield::BLACK: name = "black"; break;
    default: break;
    }
  if (large)
    name += "_large_selector";
  else
    name += "_small_selector";

  helper->getData(file, name);
  File::add_png_if_no_ext (file);

  if (large)
    {
      switch (c)
        {
        case Shield::WHITE: d_large_white_selector = file; break;
        case Shield::GREEN: d_large_green_selector = file; break;
        case Shield::YELLOW: d_large_yellow_selector = file; break;
        case Shield::LIGHT_BLUE: d_large_light_blue_selector = file; break;
        case Shield::ORANGE: d_large_orange_selector = file; break;
        case Shield::DARK_BLUE: d_large_dark_blue_selector = file; break;
        case Shield::RED: d_large_red_selector = file; break;
        case Shield::BLACK: d_large_black_selector = file; break;
        default: break;
        }
    }
  else
    {
      switch (c)
        {
        case Shield::WHITE: d_small_white_selector = file; break;
        case Shield::GREEN: d_small_green_selector = file; break;
        case Shield::YELLOW: d_small_yellow_selector = file; break;
        case Shield::LIGHT_BLUE: d_small_light_blue_selector = file; break;
        case Shield::ORANGE: d_small_orange_selector = file; break;
        case Shield::DARK_BLUE: d_small_dark_blue_selector = file; break;
        case Shield::RED: d_small_red_selector = file; break;
        case Shield::BLACK: d_small_black_selector = file; break;
        default: break;
        }
    }
}

Armyset::Armyset(XML_Helper *helper, Glib::ustring directory)
 : Set(ARMYSET_EXT, helper), d_bag(0)
{
  d_bag_name = "";
  d_stackship_name = "";
  d_standard_name = "";
  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      setLargeSelectorFilename (c, "");
      setSmallSelectorFilename (c, "");
    }

  clear_vectors ();
  setDirectory(directory);
  guint32 ts;
  helper->getData(ts, "tilesize");
  setTileSize(ts);
  helper->getData(d_stackship_name, "stackship");
  File::add_png_if_no_ext (d_stackship_name);
  helper->getData(d_standard_name, "plantedstandard");
  File::add_png_if_no_ext (d_standard_name);
  helper->getData(d_bag_name, "bag");
  File::add_png_if_no_ext (d_bag_name);

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    read_selector_name (helper, Shield::Colour(i), true);

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    read_selector_name (helper, Shield::Colour(i), false);

  helper->registerTag(ArmyProto::d_tag, 
		      sigc::mem_fun((*this), &Armyset::loadArmyProto));
}

Armyset::Armyset(const Armyset& a)
 : std::list<ArmyProto*>(), sigc::trackable(a), Set(a), d_bag(0)
{
  for (guint32 i = 0; i < a.d_ship.size (); i++)
    d_ship.push_back (a.d_ship[i]->copy ());

  for (guint32 i = 0; i < a.d_shipmask.size (); i++)
    d_shipmask.push_back (a.d_shipmask[i]->copy ());

  for (guint32 i = 0; i < a.d_ship.size (); i++)
    d_standard.push_back (a.d_standard[i]->copy ());

  for (guint32 i = 0; i < a.d_standard_mask.size (); i++)
    d_standard_mask.push_back (a.d_standard_mask[i]->copy ());

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      setSmallSelectorFilename (c, a.getSmallSelectorFilename (c));
    }

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      setLargeSelectorFilename (c, a.getLargeSelectorFilename (c));
    }
        
  clear_vectors ();

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      setNumberOfSelectorFrames (c, a.getNumberOfSelectorFrames (c));
    }

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      setNumberOfSmallSelectorFrames (c, a.getNumberOfSmallSelectorFrames (c));
    }

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      for (guint32 j = 0; j < getNumberOfSelectorFrames (c); j++)
        {
          if (a.getSelectorImage (c, j))
            setSelectorImage (c, j, a.getSelectorImage (c, j)->copy ());
          else
            setSelectorImage (c, j, NULL);
        }
    }

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      for (guint32 j = 0; j < getNumberOfSmallSelectorFrames (c); j++)
        {
          if (a.getSmallSelectorImage (c, j))
            setSmallSelectorImage (c, j, a.getSmallSelectorImage (c, j)->copy ());
          else
            setSmallSelectorImage (c, j, NULL);
        }
    }

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      for (guint32 j = 0; j < getNumberOfSelectorFrames (c); j++)
        {
          if (a.getSelectorMask (c, j))
            setSelectorMask (c, j, a.getSelectorMask (c, j)->copy ());
          else
            setSelectorMask (c, j, NULL);
        }
    }

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      for (guint32 j = 0; j < getNumberOfSmallSelectorFrames (c); j++)
        {
          if (a.getSmallSelectorMask (c, j))
            setSmallSelectorMask (c, j, a.getSmallSelectorMask (c, j)->copy ());
          else
            setSmallSelectorMask (c, j, NULL);
        }
    }

  if (a.d_bag)
    d_bag = a.d_bag->copy();

  d_standard_name = a.d_standard_name;
  d_stackship_name = a.d_stackship_name;
  d_bag_name = a.d_bag_name;

  for (const_iterator i = a.begin(); i != a.end(); i++)
    push_back(new ArmyProto(*(*i)));
}

Armyset::~Armyset()
{
  uninstantiateImages();
  for (iterator it = begin(); it != end(); it++)
    delete *it;
  clear();
  clean_tmp_dir();
}

bool Armyset::loadArmyProto(Glib::ustring tag, XML_Helper* helper)
{
    if (tag == ArmyProto::d_tag)
      {
        ArmyProto *a = new ArmyProto(helper);
        a->setArmyset(getId());
        push_back(a);
      }
    return true;
}

bool Armyset::save(Glib::ustring filename, Glib::ustring ext) const
{
  bool broken = false;
  Glib::ustring goodfilename = File::add_ext_if_necessary(filename, ext);

  Glib::ustring tmpfile = File::get_tmp_file();
  XML_Helper helper(tmpfile, std::ios::out);
  helper.begin(LORDSAWAR_ARMYSET_VERSION);
  broken = !save(&helper);
  helper.close();
  if (broken == true)
    return false;
  std::vector<Glib::ustring> extrafiles;
  return saveTar(tmpfile, tmpfile + ".tar", goodfilename, extrafiles);
}

void Armyset::write_selector_name (XML_Helper *helper, Shield::Colour c, bool large) const
{
  if (c == Shield::NEUTRAL)
    return;
  Glib::ustring name = "";
  switch (c)
    {
    case Shield::WHITE: name = "white"; break;
    case Shield::GREEN: name = "green"; break;
    case Shield::YELLOW: name = "yellow"; break;
    case Shield::LIGHT_BLUE: name = "light_blue"; break;
    case Shield::ORANGE: name = "orange"; break;
    case Shield::DARK_BLUE: name = "dark_blue"; break;
    case Shield::RED: name = "red"; break;
    case Shield::BLACK: name = "black"; break;
    default: break;
    }
  if (large)
    name += "_large_selector";
  else
    name += "_small_selector";
  Glib::ustring filename = "";
  if (large)
    {
      switch (c)
        {
        case Shield::WHITE: filename = d_large_white_selector; break;
        case Shield::GREEN: filename = d_large_green_selector; break;
        case Shield::YELLOW: filename = d_large_yellow_selector; break;
        case Shield::LIGHT_BLUE: filename = d_large_light_blue_selector; break;
        case Shield::ORANGE: filename = d_large_orange_selector; break;
        case Shield::DARK_BLUE: filename = d_large_dark_blue_selector; break;
        case Shield::RED: filename = d_large_red_selector; break;
        case Shield::BLACK: filename = d_large_black_selector; break;
        default: break;
        }
    }
  else
    {
      switch (c)
        {
        case Shield::WHITE: filename = d_small_white_selector; break;
        case Shield::GREEN: filename = d_small_green_selector; break;
        case Shield::YELLOW: filename = d_small_yellow_selector; break;
        case Shield::LIGHT_BLUE: filename = d_small_light_blue_selector; break;
        case Shield::ORANGE: filename = d_small_orange_selector; break;
        case Shield::DARK_BLUE: filename = d_small_dark_blue_selector; break;
        case Shield::RED: filename = d_small_red_selector; break;
        case Shield::BLACK: filename = d_small_black_selector; break;
        default: break;
        }
    }
  helper->saveData(name, filename);
}

bool Armyset::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(d_tag);

    retval &= Set::save(helper);
    retval &= helper->saveData("tilesize", getUnscaledTileSize());
    retval &= helper->saveData("stackship", d_stackship_name);
    retval &= helper->saveData("plantedstandard", d_standard_name);
    retval &= helper->saveData("bag", d_bag_name);

    for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
      write_selector_name (helper, Shield::Colour(i), true);

    for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
      write_selector_name (helper, Shield::Colour(i), false);

    for (const_iterator it = begin(); it != end(); it++)
      (*it)->save(helper);
    
    retval &= helper->closeTag();

    return retval;
}

ArmyProto * Armyset::lookupSimilarArmy(ArmyProto *army) const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getGender() == army->getGender() &&
	  (*it)->getStrength() == army->getStrength() &&
	  (*it)->getProduction() == army->getProduction() &&
	  (*it)->getArmyBonus() == army->getArmyBonus() &&
	  (*it)->getMoveBonus() == army->getMoveBonus() &&
	  (*it)->getMaxMoves() == army->getMaxMoves() &&
	  (*it)->getAwardable() == army->getAwardable() &&
	  (*it)->getDefendsRuins() == army->getDefendsRuins())
	return *it;
    }
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getGender() == army->getGender() &&
	  (*it)->getStrength() == army->getStrength() &&
	  (*it)->getProduction() == army->getProduction() &&
	  (*it)->getArmyBonus() == army->getArmyBonus() &&
	  (*it)->getMoveBonus() == army->getMoveBonus() &&
	  (*it)->getMaxMoves() == army->getMaxMoves())
	return *it;
    }
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getGender() == army->getGender() &&
	  (*it)->getStrength() == army->getStrength() &&
	  (*it)->getProduction() == army->getProduction() &&
	  (*it)->getMaxMoves() == army->getMaxMoves())
	return *it;
    }
  return NULL;
}

ArmyProto * Armyset::lookupArmyByGender(Hero::Gender gender) const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getGender() == gender)
	return *it;
    }
  return  NULL;
}
ArmyProto * Armyset::lookupArmyByStrengthAndTurns(guint32 str, guint32 turns) const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      if (str && turns)
	{
	  if ((*it)->getStrength() == str && (*it)->getProduction() == turns)
	    return *it;
	}
      else if (str && !turns)
	{
	  if ((*it)->getStrength() == str)
	    return *it;
	}
      else if (turns && !str)
	{
	  if ((*it)->getProduction() == turns)
	    return *it;
	}
    }
  return NULL;
}

ArmyProto * Armyset::lookupArmyByName(Glib::ustring name) const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getName().uppercase() == name.uppercase())
	return *it;
    }
  return NULL;
}
	
ArmyProto * Armyset::lookupArmyByType(guint32 army_type_id) const
{
  for (const_iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getId() == army_type_id)
	return *it;
    }
  return NULL;
}
	
bool Armyset::validateHero()
{
  bool found = false;
  //do we have a hero?
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->isHero() == true)
        {
          found = true;
          break;
        }
    }
  if (!found)
    return false;
  return true;
}

bool Armyset::validatePurchasables()
{
  bool found = false;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getNewProductionCost() > 0 )
	{
	  found = true;
	  break;
	}
    }
  if (!found)
    return false;
  return true;
}

bool Armyset::validateRuinDefenders()
{
  bool found = false;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getDefendsRuins() == true)
	{
	  found = true;
	  break;
	}
    }
  if (!found)
    return false;
  return true;
}

bool Armyset::validateAwardables()
{
  bool found = false;
  for (iterator it = begin(); it != end(); it++)
    {
      if ((*it)->getAwardable() == true)
	{
	  found = true;
	  break;
	}
    }
  if (!found)
    return false;
  return true;
}
bool Armyset::validateShip()
{
  if (getShipImageName() == "")
    return false;
  return true;
}

bool Armyset::validateBag()
{
  if (getBagImageName() == "")
    return false;
  return true;
}

bool Armyset::validateStandard()
{
  if (getStandardImageName() == "")
    return false;
  return true;
}

bool Armyset::validateArmyUnitImage(ArmyProto *army, Shield::Colour &c)
{
  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    if (army->getImageName(Shield::Colour(i)) == "")
      {
	c = Shield::Colour(i);
	return false;
      }
  return true;
}
bool Armyset::validateArmyUnitImages()
{
  Shield::Colour c;
  for (iterator it = begin(); it != end(); it++)
    {
      if (validateArmyUnitImage(*it, c) == false)
	return false;
    }
  return true;
}

bool Armyset::validateArmyUnitName(ArmyProto *army)
{
  if (army->getName() == "")
    return false;
  return true;
}
bool Armyset::validateArmyUnitNames()
{
  for (iterator it = begin(); it != end(); it++)
    {
      if (validateArmyUnitName(*it) == false)
	return false;
    }
  return true;
}
bool Armyset::validateArmyTypeIds()
{
  std::list<guint32> ids = std::list<guint32>();
  for (iterator it = begin(); it != end(); it++)
    {
      if (std::find(ids.begin(), ids.end(), (*it)->getId()) == ids.end())
        ids.push_back((*it)->getId());
      else
        return false;
    }
  return true;
}
bool Armyset::validate()
{
  if (String::utrim (getName ()) == "")
    return false;

  bool valid = true;
  valid = validateHero();
  if (!valid)
    return false;
  valid = validatePurchasables();
  if (!valid)
    return false;
  //do we have any units that defend ruins?
  valid = validateRuinDefenders();
  if (!valid)
    return false;
  //do we have any units that can be awarded?
  valid = validateAwardables();
  if (!valid)
    return false;
  //is the stackship set?
  valid = validateShip();
  if (!valid)
    return false;
  //is the standard set?
  valid = validateStandard();
  if (!valid)
    return false;
  //is the bag set?
  valid = validateBag();
  if (!valid)
    return false;
  //is there an image set for each army unit?
  valid = validateArmyUnitImages();
  if (!valid)
    return false;
  //is there a name set for each army unit?
  valid = validateArmyUnitNames();
  if (!valid)
    return false;
  //unique Ids per army unit?
  valid = validateArmyTypeIds();
  if (!valid)
    return false;

  return valid;
}

//! Helper class for making a new Armyset object from an armyset file.
class ArmysetLoader
{
public:
    ArmysetLoader(Glib::ustring filename, bool &broken, bool &unsupported)
      {
        unsupported_version = false;
	armyset = NULL;
	dir = File::get_dirname(filename);
        file = File::get_basename(filename);
	if (File::nameEndsWith(filename, Armyset::file_extension) == false)
	  filename += Armyset::file_extension;
        Tar_Helper t(filename, std::ios::in, broken);
        if (broken)
          return;
        Glib::ustring lwafilename = 
          t.getFirstFile(Armyset::file_extension, broken);
        if (broken)
          return;
	XML_Helper helper(lwafilename, std::ios::in);
	helper.registerTag(Armyset::d_tag, sigc::mem_fun((*this), &ArmysetLoader::load));
	if (!helper.parseXML())
	  {
            unsupported = unsupported_version;
            std::cerr << String::ucompose(_("Error!  can't load armyset `%1'."), filename) << std::endl;
	    if (armyset != NULL)
	      delete armyset;
	    armyset = NULL;
	  }
        helper.close();
        File::erase(lwafilename);
        t.Close();
      };
    bool load(Glib::ustring tag, XML_Helper* helper)
      {
	if (tag == Armyset::d_tag)
	  {
            if (helper->getVersion() == LORDSAWAR_ARMYSET_VERSION)
              {
                armyset = new Armyset(helper, dir);
                armyset->setBaseName(file);
                return true;
              }
            else
              {
                unsupported_version = true;
                return false;
              }
	  }
	return false;
      };
    Glib::ustring dir;
    Glib::ustring file;
    Armyset *armyset;
    bool unsupported_version;
};

Armyset *Armyset::create(Glib::ustring filename, bool &unsupported_version)
{
  bool broken = false;
  ArmysetLoader d(filename, broken, unsupported_version);
  if (broken)
    return NULL;
  return d.armyset;
}

void Armyset::instantiateImages(bool scale, bool &broken)
{
  uninstantiateImages();
  broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return;

  for (iterator it = begin(); it != end(); ++it)
    (*it)->instantiateImages(getUnscaledTileSize(), &t, scale, broken);

  Glib::ustring ship_filename = "";
  Glib::ustring flag_filename = "";
  Glib::ustring bag_filename = "";
  if (getShipImageName().empty() == false && !broken)
    ship_filename = t.getFile(getShipImageName(), broken);
  if (getStandardImageName().empty() == false && !broken)
    flag_filename = t.getFile(getStandardImageName(), broken);
  if (getBagImageName().empty() == false && !broken)
    bag_filename = t.getFile(getBagImageName(), broken);

  if (!broken)
    {

      if (ship_filename.empty() == false)
        loadShipPic(ship_filename, scale, broken);
      if (flag_filename.empty() == false)
        loadStandardPic(flag_filename, scale, broken);
      if (bag_filename.empty() == false)
        loadBagPic(bag_filename, broken);
    }

  if (ship_filename.empty() == false)
    File::erase(ship_filename);
  if (flag_filename.empty() == false)
    File::erase(flag_filename);
  if (bag_filename.empty() == false)
    File::erase(bag_filename);
      
  bool ret = loadSelectorPics (&t);
  if (ret == false)
    broken = false;
  t.Close();
}
      
bool Armyset::loadSelectorPics (Tar_Helper *t)
{
  bool broken = false;
  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      Glib::ustring filename = "";
      if (getLargeSelectorFilename (c) != "")
        filename = t->getFile (getLargeSelectorFilename (c), broken);
      if (filename.empty () == false)
        {
          std::vector<PixMask* > images;
          std::vector<PixMask* > masks;
          bool success =
            SelectorPixMaskCacheItem::loadSelectorImages (filename, 
                                                          getUnscaledTileSize(), 
                                                          images, masks, true);
          if (success)
            {
              setNumberOfSelectorFrames(c, images.size());
              for (unsigned int j = 0; j < images.size(); j++)
                {
                  setSelectorImage(c, j, images[j]);
                  setSelectorMask(c, j, masks[j]);
                }
            }
          File::erase(filename);
        }
      if (!broken)
        {
          filename = "";
          if (getSmallSelectorFilename (c) != "")
            filename = t->getFile (getSmallSelectorFilename (c), broken);
          if (filename.empty () == false)
            {
              std::vector<PixMask* > images;
              std::vector<PixMask* > masks;
              bool success =
                SelectorPixMaskCacheItem::loadSelectorImages
                (filename, getUnscaledTileSize(), images, masks, true);
              if (success)
                {
                  setNumberOfSmallSelectorFrames(c, images.size());
                  for (unsigned int j = 0; j < images.size(); j++)
                    {
                      setSmallSelectorImage(c, j, images[j]);
                      setSmallSelectorMask(c, j, masks[j]);
                    }
                }
              File::erase(filename);
            }
        }
    }
  return broken;
}

void Armyset::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();

  for (guint32 i = 0; i < d_standard.size(); i++)
    delete d_standard[i];
  d_standard.clear ();

  for (guint32 i = 0; i < d_standard_mask.size(); i++)
    delete d_standard_mask[i];
  d_standard_mask.clear ();

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      for (guint32 j = 0; j < getNumberOfSelectorFrames (c); j++)
        {
          PixMask *p = getSelectorImage (c, j);
          if (p)
            delete p;
        }
    }

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      for (guint32 j = 0; j < getNumberOfSelectorFrames (c); j++)
        {
          PixMask *p = getSelectorMask (c, j);
          if (p)
            delete p;
        }
    }

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      for (guint32 j = 0; j < getNumberOfSmallSelectorFrames (c); j++)
        {
          PixMask *p = getSmallSelectorImage (c, j);
          if (p)
            delete p;
        }
    }

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      for (guint32 j = 0; j < getNumberOfSmallSelectorFrames (c); j++)
        {
          PixMask *p = getSmallSelectorMask (c, j);
          if (p)
            delete p;
        }
    }
  clear_vectors ();

  if (d_bag)
    delete d_bag;

  for (guint32 i = 0; i < d_ship.size(); i++)
    delete d_ship[i];
  d_ship.clear ();

  for (guint32 i = 0; i < d_shipmask.size(); i++)
    delete d_shipmask[i];
  d_shipmask.clear ();

  d_bag = NULL;
}

void Armyset::loadShipPic(Glib::ustring image_filename, bool scale,
                          bool &broken)
{
  PixMask *p = PixMask::create (image_filename, broken);
  if (broken)
    return;
  std::vector<PixMask*> pics =
    disassemble_row(p->to_pixbuf (), MAX_PLAYERS, true);
  std::vector<PixMask*> masks =
    disassemble_row(p->to_pixbuf (), MAX_PLAYERS, false);
  delete p;
  if (scale)
    {
      int s = getUnscaledTileSize();
      for (guint32 i = 0; i < pics.size (); i++)
        PixMask::scale(pics[i], s, s);
      for (guint32 i = 0; i < masks.size (); i++)
        PixMask::scale(masks[i], s, s);
    }
  setShipImages (pics);
  setShipMasks (masks);
}

void Armyset::loadBagPic(Glib::ustring image_filename, bool &broken)
{
  if (image_filename.empty() == true)
    {
      broken = true;
      return;
    }
  if (!broken)
    {
      PixMask *p = PixMask::create(image_filename, broken);
      if (p && !broken)
        {
          int s = getTileSize();
          PixMask::scale (p, s, s);
          setBagPic(p);
        }
    }
}

void Armyset::loadStandardPic(Glib::ustring image_filename, bool scale,
                              bool &broken)
{
  PixMask *p = PixMask::create (image_filename, broken);
  if (broken)
    return;
  std::vector<PixMask*> pics =
    disassemble_row(p->to_pixbuf (), MAX_PLAYERS, true);
  std::vector<PixMask*> masks =
    disassemble_row(p->to_pixbuf (), MAX_PLAYERS, false);
  delete p;
  if (scale)
    {
      int s = getUnscaledTileSize();
      for (guint32 i = 0; i < pics.size (); i++)
        PixMask::scale(pics[i], s, s);
      for (guint32 i = 0; i < masks.size (); i++)
        PixMask::scale(masks[i], s, s);
    }
  setStandardPics (pics);
  setStandardMasks (masks);
}

void Armyset::switchArmysetForRuinKeeper(Army *army, const Armyset *armyset)
{
  //do our best to change the armyset for the given ruin keeper.
 
  //go find an equivalent type in the new armyset.
  Armyset *old_armyset
    = Armysetlist::getInstance()->get(army->getOwner()->getArmyset());
  ArmyProto *old_armyproto = old_armyset->lookupArmyByType(army->getTypeId());
  if (old_armyproto == NULL)
    return;
  const ArmyProto *new_armyproto = armyset->lookupArmyByType(army->getTypeId());

  //try looking at the same id first
  if (new_armyproto != NULL && 
      old_armyproto->getName() == new_armyproto->getName() &&
      old_armyproto->getDefendsRuins() == new_armyproto->getDefendsRuins())
    {
      army->morph(new_armyproto);
      return;
    }

  //try finding an army by the same name
  new_armyproto = armyset->lookupArmyByName(old_armyproto->getName());
  if (new_armyproto != NULL &&
      old_armyproto->getDefendsRuins() == new_armyproto->getDefendsRuins())
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any ruin keeper will do.
  new_armyproto = armyset->getRandomRuinKeeper();
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }
}

void Armyset::switchArmyset(ArmyProdBase *army, const Armyset *armyset)
{
  //do our best to change the armyset for the given armyprodbase.

  //go find an equivalent type in the new armyset.
  Armyset *old_armyset
    = Armysetlist::getInstance()->get(army->getArmyset());
  ArmyProto *old_armyproto = old_armyset->lookupArmyByType(army->getTypeId());
  if (old_armyproto == NULL)
    return;
  ArmyProto *new_armyproto = armyset->lookupArmyByType(army->getTypeId());

  //try looking at the same id first
  if (new_armyproto != NULL && 
      old_armyproto->getName() == new_armyproto->getName())
    {
      army->morph(new_armyproto);
      return;
    }

  //try finding an army by the same name
  new_armyproto = armyset->lookupArmyByName(old_armyproto->getName());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army with similar characteristics will do.
  new_armyproto = armyset->lookupSimilarArmy(old_armyproto);
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army with the same strength and turns will do.
  new_armyproto = 
    armyset->lookupArmyByStrengthAndTurns(old_armyproto->getStrength(),
					  old_armyproto->getProduction());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army with the same strength will do.
  new_armyproto = 
    armyset->lookupArmyByStrengthAndTurns(old_armyproto->getStrength(), 0);
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army with the same turns will do.
  new_armyproto = 
    armyset->lookupArmyByStrengthAndTurns(0, old_armyproto->getProduction());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army will do.
  new_armyproto = armyset->lookupArmyByGender(old_armyproto->getGender());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }
}

void Armyset::switchArmyset(Army *army, const Armyset *armyset)
{
  //do our best to change the armyset for the given army.

  //go find an equivalent type in the new armyset.
  Armyset *old_armyset
    = Armysetlist::getInstance()->get(army->getOwner()->getArmyset());
  ArmyProto *old_armyproto = old_armyset->lookupArmyByType(army->getTypeId());
  if (!old_armyproto)
    return;
  ArmyProto *new_armyproto = armyset->lookupArmyByType(army->getTypeId());

  //try looking at the same id first
  if (new_armyproto != NULL && 
      old_armyproto->getId() == new_armyproto->getId())
    {
      army->morph(new_armyproto);
      return;
    }

  //try finding an army by the same name
  new_armyproto = armyset->lookupArmyByName(old_armyproto->getName());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, an army with the same gender (heroes).
  if (army->isHero() == true)
    {
      new_armyproto = armyset->lookupArmyByGender(old_armyproto->getGender());
      if (new_armyproto != NULL)
	{
	  army->morph(new_armyproto);
	  return;
	}
    }

  //failing that, any army with similar characteristics will do.
  new_armyproto = armyset->lookupSimilarArmy(old_armyproto);
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army with the same strength and turns will do.
  new_armyproto = 
    armyset->lookupArmyByStrengthAndTurns(old_armyproto->getStrength(),
					  old_armyproto->getProduction());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army with the same strength will do.
  new_armyproto = 
    armyset->lookupArmyByStrengthAndTurns(old_armyproto->getStrength(), 0);
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army with the same turns will do.
  new_armyproto = 
    armyset->lookupArmyByStrengthAndTurns(0, old_armyproto->getProduction());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

  //failing that, any army will do.
  new_armyproto = armyset->lookupArmyByGender(old_armyproto->getGender());
  if (new_armyproto != NULL)
    {
      army->morph(new_armyproto);
      return;
    }

}

const ArmyProto * Armyset::getRandomRuinKeeper() const
{
  // list all the army types that can be a sentinel.
  std::vector<const ArmyProto*> occupants;
  for (const_iterator i = begin(); i != end(); i++)
    {
      const ArmyProto *a = *i;
      if (a->getDefendsRuins())
	occupants.push_back(a);
    }
            
  if (!occupants.empty())
    return occupants[Rnd::rand() % occupants.size()];

  return NULL;
}

const ArmyProto *Armyset::getRandomAwardableAlly() const
{
  // list all the army types that can be given out as a reward.
  std::vector<const ArmyProto*> allies;
  for (const_iterator i = begin(); i != end(); i++)
    {
      const ArmyProto *a = *i;
      if (a->getAwardable() == true)
	allies.push_back(a);
    }
            
  if (!allies.empty())
    return allies[Rnd::rand() % allies.size()];

  return NULL;
}

void Armyset::reload(bool &broken)
{
  broken = false;
  bool unsupported = false;
  ArmysetLoader d(getConfigurationFile(), broken, unsupported);
  if (!broken && d.armyset && d.armyset->validate())
    {
      uninstantiateImages();
      for (iterator it = begin(); it != end(); it++)
        delete *it;
      clear();
      for (iterator it = d.armyset->begin(); it != d.armyset->end(); it++)
        push_back(new ArmyProto(*(*it)));
      *this = *d.armyset;
      instantiateImages(true, broken);
    }
}

bool Armyset::calculate_preferred_tile_size(guint32 &ts) const
{
  guint32 tilesize = 0;
  std::map<guint32, guint32> sizecounts;

  if (d_ship.empty () == false)
    sizecounts[d_ship[0]->get_unscaled_width()]++;
  if (d_standard.empty () == false)
    sizecounts[d_standard[0]->get_unscaled_width()]++;
  if (d_bag)
    sizecounts[d_bag->get_unscaled_width()]++;
  for (const_iterator it = begin(); it != end(); it++)
    {
      ArmyProto *a = (*it);
      if (a->getImage(Shield::NEUTRAL) != NULL)
        sizecounts[a->getImage(Shield::NEUTRAL)->get_unscaled_width()]++;
    }

  guint32 maxcount = 0;
  for (std::map<guint32, guint32>::iterator it = sizecounts.begin(); 
       it != sizecounts.end(); it++)
    {
      if ((*it).second > maxcount)
        {
          maxcount = (*it).second;
          tilesize = (*it).first;
        }
    }
  bool ret = true;
  if (tilesize == 0)
    {
      ts = DEFAULT_ARMY_TILE_SIZE;
      ret = false;
    }
  else
    ts = tilesize;
  return ret;
}

bool Armyset::upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version)
{
  return FileCompat::getInstance()->upgrade(filename, old_version, new_version,
                                            FileCompat::ARMYSET, d_tag);
}

void Armyset::support_backward_compatibility()
{
  FileCompat::getInstance()->support_type(FileCompat::ARMYSET, file_extension, 
                                          d_tag, true);
  FileCompat::getInstance()->support_version
    (FileCompat::ARMYSET, "0.2.1", "0.3.0",
     sigc::ptr_fun(&Armyset::upgrade));
}

Armyset * Armyset::copy(const Armyset *armyset)
{
  if (!armyset)
    return NULL;
  return new Armyset(*armyset);
}

guint32 Armyset::getMaxId() const
{
  guint32 max = 0;
  for (const_iterator i = begin(); i != end(); i++)
    if ((*i)->getId() > max)
      max = (*i)->getId();
  return max;
}

bool weakest_quickest (const ArmyProto* first, const ArmyProto* second)
{
  int ffly = first->getMoveBonus() == Tile::isFlying();
  int sfly = second->getMoveBonus() == Tile::isFlying();
  int f = (first->getStrength() * 100) + (first->getProduction() * 101) + (ffly * 1000);
  int s = (second->getStrength() * 100) + (second->getProduction() * 101) + (sfly * 1000);
  if (f < s)
    return true;
  return false;
}

ArmyProto *Armyset::lookupWeakestQuickestArmy() const
{
  Armyset *a = new Armyset(*this);
  a->sort(weakest_quickest);
  guint32 type_id = (*(a->begin()))->getId();
  ArmyProto *p = Armysetlist::getInstance()->getArmy(getId(), type_id);
  delete a;
  return p;
}

void Armyset::clearStandardImage (bool clear_name)
{
  if (clear_name)
    setStandardImageName ("");

  std::vector<PixMask *>pics = getStandardPics ();
  for (auto  p : pics)
    {
      if (p)
        delete p;
    }
  pics.clear ();
  setStandardPics (pics);

  pics = getStandardMasks ();
  for (auto p : pics)
    {
      if (p)
        delete p;
    }
  pics.clear ();
  setStandardMasks (pics);
}

void Armyset::clearBagImage (bool clear_name)
{
  if (clear_name)
    setBagImageName ("");

  PixMask *p = getBagPic ();
  if (p)
    delete p;
  setBagPic (NULL);
}

void Armyset::clearShipImage (bool clear_name)
{
  if (clear_name)
    setShipImageName ("");

  std::vector<PixMask *>pics = getShipPics ();
  for (auto  p : pics)
    {
      if (p)
        delete p;
    }
  pics.clear ();
  setShipImages (pics);

  pics = getShipMasks ();
  for (auto p : pics)
    {
      if (p)
        delete p;
    }
  pics.clear ();
  setShipMasks (pics);
}

bool Armyset::instantiateBagImage ()
{
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getBagImageName();
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        {
          clearBagImage (false);
          loadBagPic(filename, broken);
        }
    }
  return broken;
}

bool Armyset::instantiateStandardImage ()
{
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getStandardImageName();
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        {
          clearStandardImage (false);
          loadStandardPic(filename, false, broken);
        }
    }
  return broken;
}

bool Armyset::instantiateShipImage ()
{
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getShipImageName();
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        {
          clearShipImage (false);
          loadShipPic(filename, false, broken);
        }
    }
  return broken;
}

guint32 Armyset::get_default_tile_size ()
{
  Armyset *a = new Armyset (1, "");
  guint32 ts = a->getUnscaledTileSize ();
  delete a;
  return ts;
}

void Armyset::uninstantiateSameNamedImages (Glib::ustring name)
{
  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      Shield::Colour c = Shield::Colour (i);
      if (getLargeSelectorFilename (c) == name)
        clearLargeSelectorImage (c);
      if (getSmallSelectorFilename (c) == name)
        clearSmallSelectorImage (c);
    }
  if (getBagImageName() == name)
    clearBagImage ();
  if (getStandardImageName() == name)
    clearStandardImage ();
  if (getShipImageName() == name)
    clearShipImage ();
  for (iterator i = begin (); i != end (); i++)
    {
      for (guint32 cc = Shield::WHITE; cc <= Shield::NEUTRAL; cc++)
        {
          Shield::Colour c = Shield::Colour (cc);
          if ((*i)->getImageName (c) == name)
            (*i)->clearImage (c);
        }
    }
}

void Armyset::clearSmallSelectorImage (Shield::Colour c, bool clear_name)
{
  if (clear_name)
    setSmallSelectorFilename (c, "");

  for (unsigned int i = 0; i < getNumberOfSmallSelectorFrames(c); i++)
    {
      PixMask *p = getSmallSelectorImage (c, i);
      if (p)
        delete p;
      setSmallSelectorImage (c, i, NULL);
      p = getSmallSelectorMask (c, i);
      if (p)
        delete p;
      setSmallSelectorMask (c, i, NULL);
    }
}

void Armyset::clearLargeSelectorImage (Shield::Colour c, bool clear_name)
{
  if (clear_name)
    setLargeSelectorFilename (c, "");

  for (unsigned int i = 0; i < getNumberOfSelectorFrames(c); i++)
    {
      PixMask *p = getSelectorImage (c, i);
      if (p)
        delete p;
      setSelectorImage (c, i, NULL);
      p = getSelectorMask (c, i);
      if (p)
        delete p;
      setSelectorMask (c, i, NULL);
    }
}

bool Armyset::instantiateSmallSelectorImages()
{
  for (int i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    if (instantiateSmallSelectorImages(Shield::Colour (i)) == false)
      return false;
  return true;
}

bool Armyset::instantiateSmallSelectorImages(Shield::Colour c)
{
  clearSmallSelectorImage (c, false);
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getSmallSelectorFilename(c);
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        {
          std::vector<PixMask* > images, masks;
          bool success =
            SelectorPixMaskCacheItem::loadSelectorImages
            (filename, getUnscaledTileSize(), images, masks, false);
          if (success)
            {
              setNumberOfSmallSelectorFrames(c, images.size());
              for (unsigned int i = 0; i < images.size(); i++)
                {
                  setSmallSelectorImage(c, i, images[i]);
                  setSmallSelectorMask(c, i, masks[i]);
                }
            }
          else
            broken = true;
        }
    }
  return broken;
}

bool Armyset::instantiateLargeSelectorImages()
{
  for (int i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    if (instantiateLargeSelectorImages(Shield::Colour (i)) == false)
      return false;
  return true;
}

bool Armyset::instantiateLargeSelectorImages(Shield::Colour c)
{
  clearLargeSelectorImage (c, false);
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  Glib::ustring imgname = getLargeSelectorFilename(c);
  if (imgname.empty() == false)
    {
      Glib::ustring filename = t.getFile(imgname, broken);
      if (!broken)
        {
          std::vector<PixMask* > images, masks;
          bool success =
            SelectorPixMaskCacheItem::loadSelectorImages
            (filename, getUnscaledTileSize(), images, masks, false);
          if (success)
            {
              setNumberOfSelectorFrames(c, images.size());
              for (unsigned int i = 0; i < images.size(); i++)
                {
                  setSelectorImage(c, i, images[i]);
                  setSelectorMask(c, i, masks[i]);
                }
            }
          else
            broken = true;
        }
    }
  return broken;
}

PixMask *Armyset::getSelectorImage(Shield::Colour c, guint32 i) const
{
  switch (c)
    {
    case Shield::WHITE: return white_selector[i];
    case Shield::GREEN: return green_selector[i];
    case Shield::YELLOW: return yellow_selector[i];
    case Shield::LIGHT_BLUE: return light_blue_selector[i];
    case Shield::ORANGE: return orange_selector[i];
    case Shield::DARK_BLUE: return dark_blue_selector[i];
    case Shield::RED: return red_selector[i];
    case Shield::BLACK: return black_selector[i];
    default: return NULL;
    }
  return NULL;
}

PixMask *Armyset::getSelectorMask(Shield::Colour c, guint32 i) const
{
  switch (c)
    {
    case Shield::WHITE: return white_selectormask[i];
    case Shield::GREEN: return green_selectormask[i];
    case Shield::YELLOW: return yellow_selectormask[i];
    case Shield::LIGHT_BLUE: return light_blue_selectormask[i];
    case Shield::ORANGE: return orange_selectormask[i];
    case Shield::DARK_BLUE: return dark_blue_selectormask[i];
    case Shield::RED: return red_selectormask[i];
    case Shield::BLACK: return black_selectormask[i];
    default: return NULL;
    }
  return NULL;
}

PixMask *Armyset::getSmallSelectorImage(Shield::Colour c, guint32 i) const
{
  switch (c)
    {
    case Shield::WHITE: return white_smallselector[i];
    case Shield::GREEN: return green_smallselector[i];
    case Shield::YELLOW: return yellow_smallselector[i];
    case Shield::LIGHT_BLUE: return light_blue_smallselector[i];
    case Shield::ORANGE: return orange_smallselector[i];
    case Shield::DARK_BLUE: return dark_blue_smallselector[i];
    case Shield::RED: return red_smallselector[i];
    case Shield::BLACK: return black_smallselector[i];
    default: return NULL;
    }
  return NULL;
}

PixMask *Armyset::getSmallSelectorMask(Shield::Colour c, guint32 i) const
{
  switch (c)
    {
    case Shield::WHITE: return white_smallselectormask[i];
    case Shield::GREEN: return green_smallselectormask[i];
    case Shield::YELLOW: return yellow_smallselectormask[i];
    case Shield::LIGHT_BLUE: return light_blue_smallselectormask[i];
    case Shield::ORANGE: return orange_smallselectormask[i];
    case Shield::DARK_BLUE: return dark_blue_smallselectormask[i];
    case Shield::RED: return red_smallselectormask[i];
    case Shield::BLACK: return black_smallselectormask[i];
    default: return NULL;
    }
  return NULL;
}

void Armyset::setSmallSelectorImage(Shield::Colour c, guint32 i, PixMask *p)
{
  switch (c)
    {
    case Shield::WHITE: white_smallselector[i] = p; break;
    case Shield::GREEN: green_smallselector[i] = p; break;
    case Shield::YELLOW: yellow_smallselector[i] = p; break;
    case Shield::LIGHT_BLUE: light_blue_smallselector[i] = p; break;
    case Shield::ORANGE: orange_smallselector[i] = p; break;
    case Shield::DARK_BLUE: dark_blue_smallselector[i] = p; break;
    case Shield::RED: red_smallselector[i] = p; break;
    case Shield::BLACK: black_smallselector[i] = p; break;
    default: return;
    }
  return;
}

void Armyset::setSmallSelectorMask(Shield::Colour c, guint32 i, PixMask *p)
{
  switch (c)
    {
    case Shield::WHITE: white_smallselectormask[i] = p; break;
    case Shield::GREEN: green_smallselectormask[i] = p; break;
    case Shield::YELLOW: yellow_smallselectormask[i] = p; break;
    case Shield::LIGHT_BLUE: light_blue_smallselectormask[i] = p; break;
    case Shield::ORANGE: orange_smallselectormask[i] = p; break;
    case Shield::DARK_BLUE: dark_blue_smallselectormask[i] = p; break;
    case Shield::RED: red_smallselectormask[i] = p; break;
    case Shield::BLACK: black_smallselectormask[i] = p; break;
    default: return;
    }
  return;
}

void Armyset::setSelectorImage(Shield::Colour c, guint32 i, PixMask *p)
{
  switch (c)
    {
    case Shield::WHITE: white_selector[i] = p; break;
    case Shield::GREEN: green_selector[i] = p; break;
    case Shield::YELLOW: yellow_selector[i] = p; break;
    case Shield::LIGHT_BLUE: light_blue_selector[i] = p; break;
    case Shield::ORANGE: orange_selector[i] = p; break;
    case Shield::DARK_BLUE: dark_blue_selector[i] = p; break;
    case Shield::RED: red_selector[i] = p; break;
    case Shield::BLACK: black_selector[i] = p; break;
    default: return;
    }
  return;
}

void Armyset::setSelectorMask(Shield::Colour c, guint32 i, PixMask *p)
{
  switch (c)
    {
    case Shield::WHITE: white_selectormask[i] = p; break;
    case Shield::GREEN: green_selectormask[i] = p; break;
    case Shield::YELLOW: yellow_selectormask[i] = p; break;
    case Shield::LIGHT_BLUE: light_blue_selectormask[i] = p; break;
    case Shield::ORANGE: orange_selectormask[i] = p; break;
    case Shield::DARK_BLUE: dark_blue_selectormask[i] = p; break;
    case Shield::RED: red_selectormask[i] = p; break;
    case Shield::BLACK: black_selectormask[i] = p; break;
    default: return;
    }
  return;
}

guint32 Armyset::getNumberOfSelectorFrames(Shield::Colour c) const
{
  switch (c)
    {
    case Shield::WHITE: return number_of_white_selector_frames;
    case Shield::GREEN: return number_of_green_selector_frames;
    case Shield::YELLOW: return number_of_yellow_selector_frames;
    case Shield::LIGHT_BLUE: return number_of_light_blue_selector_frames;
    case Shield::ORANGE: return number_of_orange_selector_frames;
    case Shield::DARK_BLUE: return number_of_dark_blue_selector_frames;
    case Shield::RED: return number_of_red_selector_frames;
    case Shield::BLACK: return number_of_black_selector_frames;
    default: return 0;
    }
  return 0;
}

guint32 Armyset::getNumberOfSmallSelectorFrames(Shield::Colour c) const
{
  switch (c)
    {
    case Shield::WHITE: return number_of_white_small_selector_frames;
    case Shield::GREEN: return number_of_green_small_selector_frames;
    case Shield::YELLOW: return number_of_yellow_small_selector_frames;
    case Shield::LIGHT_BLUE: return number_of_light_blue_small_selector_frames;
    case Shield::ORANGE: return number_of_orange_small_selector_frames;
    case Shield::DARK_BLUE: return number_of_dark_blue_small_selector_frames;
    case Shield::RED: return number_of_red_small_selector_frames;
    case Shield::BLACK: return number_of_black_small_selector_frames;
    default: return 0;
    }
  return 0;
}

void Armyset::setNumberOfSelectorFrames (Shield::Colour c, guint32 num)
{
  switch (c)
    {
    case Shield::WHITE:
      white_selector.reserve (num);
      white_selectormask.reserve (num);
      number_of_white_selector_frames = num;
      break;
    case Shield::GREEN:
      green_selector.reserve (num);
      green_selectormask.reserve (num);
      number_of_green_selector_frames = num;
      break;
    case Shield::YELLOW:
      yellow_selector.reserve (num);
      yellow_selectormask.reserve (num);
      number_of_yellow_selector_frames = num;
      break;
    case Shield::LIGHT_BLUE:
      light_blue_selector.reserve (num);
      light_blue_selectormask.reserve (num);
      number_of_light_blue_selector_frames = num;
      break;
    case Shield::ORANGE:
      orange_selector.reserve (num);
      orange_selectormask.reserve (num);
      number_of_orange_selector_frames = num;
      break;
    case Shield::DARK_BLUE:
      dark_blue_selector.reserve (num);
      dark_blue_selectormask.reserve (num);
      number_of_dark_blue_selector_frames = num;
      break;
    case Shield::RED:
      red_selector.reserve (num);
      red_selectormask.reserve (num);
      number_of_red_selector_frames = num;
      break;
    case Shield::BLACK:
      black_selector.reserve (num);
      black_selectormask.reserve (num);
      number_of_black_selector_frames = num;
      break;
    default: return;
    }
  return;
}

void Armyset::setNumberOfSmallSelectorFrames (Shield::Colour c, guint32 num)
{
  switch (c)
    {
    case Shield::WHITE:
      white_smallselector.reserve (num);
      white_smallselectormask.reserve (num);
      number_of_white_small_selector_frames = num;
      break;
    case Shield::GREEN:
      green_smallselector.reserve (num);
      green_smallselectormask.reserve (num);
      number_of_green_small_selector_frames = num;
      break;
    case Shield::YELLOW:
      yellow_smallselector.reserve (num);
      yellow_smallselectormask.reserve (num);
      number_of_yellow_small_selector_frames = num;
      break;
    case Shield::LIGHT_BLUE:
      light_blue_smallselector.reserve (num);
      light_blue_smallselectormask.reserve (num);
      number_of_light_blue_small_selector_frames = num;
      break;
    case Shield::ORANGE:
      orange_smallselector.reserve (num);
      orange_smallselectormask.reserve (num);
      number_of_orange_small_selector_frames = num;
      break;
    case Shield::DARK_BLUE:
      dark_blue_smallselector.reserve (num);
      dark_blue_smallselectormask.reserve (num);
      number_of_dark_blue_small_selector_frames = num;
      break;
    case Shield::RED:
      red_smallselector.reserve (num);
      red_smallselectormask.reserve (num);
      number_of_red_small_selector_frames = num;
      break;
    case Shield::BLACK:
      black_smallselector.reserve (num);
      black_smallselectormask.reserve (num);
      number_of_black_small_selector_frames = num;
      break;
    default: return;
    }
  return;
}

Glib::ustring Armyset::getLargeSelectorFilename(Shield::Colour c) const
{
  switch (c)
    {
    case Shield::WHITE: return d_large_white_selector;
    case Shield::GREEN: return d_large_green_selector;
    case Shield::YELLOW: return d_large_yellow_selector;
    case Shield::LIGHT_BLUE: return d_large_light_blue_selector;
    case Shield::ORANGE: return d_large_orange_selector;
    case Shield::DARK_BLUE: return d_large_dark_blue_selector;
    case Shield::RED: return d_large_red_selector;
    case Shield::BLACK: return d_large_black_selector;
    default: return "";
    }
  return "";
}

Glib::ustring Armyset::getSmallSelectorFilename(Shield::Colour c) const
{
  switch (c)
    {
    case Shield::WHITE: return d_small_white_selector;
    case Shield::GREEN: return d_small_green_selector;
    case Shield::YELLOW: return d_small_yellow_selector;
    case Shield::LIGHT_BLUE: return d_small_light_blue_selector;
    case Shield::ORANGE: return d_small_orange_selector;
    case Shield::DARK_BLUE: return d_small_dark_blue_selector;
    case Shield::RED: return d_small_red_selector;
    case Shield::BLACK: return d_small_black_selector;
    default: return "";
    }
  return "";
}

void Armyset::setLargeSelectorFilename(Shield::Colour c, Glib::ustring f)
{
  switch (c)
    {
    case Shield::WHITE: d_large_white_selector = f; break;
    case Shield::GREEN: d_large_green_selector = f; break;
    case Shield::YELLOW: d_large_yellow_selector = f; break;
    case Shield::LIGHT_BLUE: d_large_light_blue_selector = f; break;
    case Shield::ORANGE: d_large_orange_selector = f; break;
    case Shield::DARK_BLUE: d_large_dark_blue_selector = f; break;
    case Shield::RED: d_large_red_selector = f; break;
    case Shield::BLACK: d_large_black_selector = f; break;
    default: return;
    }
  return;
}

void Armyset::setSmallSelectorFilename(Shield::Colour c, Glib::ustring f)
{
  switch (c)
    {
    case Shield::WHITE: d_small_white_selector = f; break;
    case Shield::GREEN: d_small_green_selector = f; break;
    case Shield::YELLOW: d_small_yellow_selector = f; break;
    case Shield::LIGHT_BLUE: d_small_light_blue_selector = f; break;
    case Shield::ORANGE: d_small_orange_selector = f; break;
    case Shield::DARK_BLUE: d_small_dark_blue_selector = f; break;
    case Shield::RED: d_small_red_selector = f; break;
    case Shield::BLACK: d_small_black_selector = f; break;
    default: return;
    }
  return;
}

void Armyset::clear_vectors()
{
  number_of_white_selector_frames = 0;
  number_of_green_selector_frames = 0;
  number_of_yellow_selector_frames = 0;
  number_of_light_blue_selector_frames = 0;
  number_of_orange_selector_frames = 0;
  number_of_dark_blue_selector_frames = 0;
  number_of_red_selector_frames = 0;
  number_of_black_selector_frames = 0;
  number_of_white_small_selector_frames = 0;
  number_of_green_small_selector_frames = 0;
  number_of_yellow_small_selector_frames = 0;
  number_of_light_blue_small_selector_frames = 0;
  number_of_orange_small_selector_frames = 0;
  number_of_dark_blue_small_selector_frames = 0;
  number_of_red_small_selector_frames = 0;
  number_of_black_small_selector_frames = 0;
  white_selector.clear ();
  green_selector.clear ();
  yellow_selector.clear ();
  light_blue_selector.clear ();
  orange_selector.clear ();
  dark_blue_selector.clear ();
  red_selector.clear ();
  black_selector.clear ();
  white_selectormask.clear ();
  green_selectormask.clear ();
  yellow_selectormask.clear ();
  light_blue_selectormask.clear ();
  orange_selectormask.clear ();
  dark_blue_selectormask.clear ();
  red_selectormask.clear ();
  black_selectormask.clear ();
  white_smallselector.clear ();
  green_smallselector.clear ();
  yellow_smallselector.clear ();
  light_blue_smallselector.clear ();
  orange_smallselector.clear ();
  dark_blue_smallselector.clear ();
  red_smallselector.clear ();
  black_smallselector.clear ();
  white_smallselectormask.clear ();
  green_smallselectormask.clear ();
  yellow_smallselectormask.clear ();
  light_blue_smallselectormask.clear ();
  orange_smallselectormask.clear ();
  dark_blue_smallselectormask.clear ();
  red_smallselectormask.clear ();
  black_smallselectormask.clear ();
}
