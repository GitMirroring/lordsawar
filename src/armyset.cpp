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
#include "TarFileMaskedImage.h"

Glib::ustring Armyset::d_tag = "armyset";
Glib::ustring Armyset::file_extension = ARMYSET_EXT;

#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

#define DEFAULT_ARMY_TILE_SIZE 40
Armyset::Armyset(guint32 id, Glib::ustring name)
 : Set(ARMYSET_EXT, id, name, DEFAULT_ARMY_TILE_SIZE), d_bag(0)
{
  d_bag_name = "";
  d_stackship = new TarFileMaskedImage ();
  d_standard = new TarFileMaskedImage ();
  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      d_selector[0][i] = new TarFileMaskedImage ();
      d_selector[1][i] = new TarFileMaskedImage ();
    }

}

void Armyset::read_selector_name (XML_Helper *helper, Shield::Colour c, bool large)
{
  Glib::ustring name = "";
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
  d_selector[large ? 1 : 0][c]->load_name (helper, name);

}

Armyset::Armyset(XML_Helper *helper, Glib::ustring directory)
 : Set(ARMYSET_EXT, helper), d_bag(0)
{
  d_bag_name = "";
  d_stackship = new TarFileMaskedImage ();
  d_standard = new TarFileMaskedImage ();
  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      d_selector[0][i] = new TarFileMaskedImage ();
      d_selector[1][i] = new TarFileMaskedImage ();
    }

  setDirectory(directory);
  guint32 ts;
  helper->getData(ts, "tilesize");
  setTileSize(ts);
  d_stackship->load_name (helper, "stackship");
  d_standard->load_name (helper, "plantedstandard");
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
  d_stackship = new TarFileMaskedImage (*a.d_stackship);
  d_standard = new TarFileMaskedImage (*a.d_standard);

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      d_selector[0][i] = new TarFileMaskedImage (*a.d_selector[0][i]);
      d_selector[1][i] = new TarFileMaskedImage (*a.d_selector[1][i]);
    }

  if (a.d_bag)
    d_bag = a.d_bag->copy();

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
  helper->saveData(name, d_selector[large ? 1 : 0][c]->getName ());
}

bool Armyset::save(XML_Helper* helper) const
{
    bool retval = true;

    retval &= helper->openTag(d_tag);

    retval &= Set::save(helper);
    retval &= helper->saveData("tilesize", getUnscaledTileSize());
    retval &= helper->saveData("stackship", d_stackship->getName ());
    retval &= helper->saveData("plantedstandard", d_standard->getName ());
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
  return d_stackship->getName () == "" ? false : true;
}

bool Armyset::validateBag()
{
  return getBagImageName () == "" ? false : true;
}

bool Armyset::validateStandard()
{
  return d_standard->getName () == "" ? false : true;
}

bool Armyset::validateArmyUnitImage(ArmyProto *army, Shield::Colour &c)
{
  for (unsigned int i = Shield::WHITE; i <= Shield::NEUTRAL; i++)
    if (army->getMaskedImage(Shield::Colour(i))->getName () == "")
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

  Vector<int> scale_dim = Vector<int>(-1,-1);
  if (scale)
    scale_dim = Vector<int>(getUnscaledTileSize(),getUnscaledTileSize ());

  broken = d_stackship->load (&t);
  if (broken)
    return;
  d_stackship->instantiateImages (scale_dim);

  broken = d_standard->load (&t);
  if (broken)
    return;
  d_standard->instantiateImages (scale_dim);

  Glib::ustring bag_filename = "";
  if (getBagImageName().empty() == false && !broken)
    bag_filename = t.getFile(getBagImageName(), broken);

  if (!broken)
    {
      if (bag_filename.empty() == false)
        loadBagPic(bag_filename, broken);
    }

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
      if (d_selector[0][i]->getName ().empty () == false)
        {
          broken = d_selector[0][i]->load (t);
          if (broken)
            break;
          d_selector[0][i]->instantiateImages ();
        }

      if (d_selector[1][i]->getName ().empty () == false)
        {
          broken = d_selector[1][i]->load (t);
          if (broken)
            break;
          d_selector[1][i]->instantiateImages ();
        }
    }
  return broken;
}

void Armyset::uninstantiateImages()
{
  for (iterator it = begin(); it != end(); it++)
    (*it)->uninstantiateImages();

  d_stackship->uninstantiateImages ();
  d_standard->uninstantiateImages ();

  for (guint32 i = Shield::WHITE; i < Shield::NEUTRAL; i++)
    {
      d_selector[0][i]->uninstantiateImages ();
      d_selector[1][i]->uninstantiateImages ();
    }

  if (d_bag)
    delete d_bag;

  d_bag = NULL;
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

  if (d_stackship->getName ().empty () == false)
    sizecounts[d_stackship->getImage(0)->get_unscaled_width()]++;
  if (d_standard->getName ().empty () == false)
    sizecounts[d_standard->getImage(0)->get_unscaled_width()]++;
  if (d_bag)
    sizecounts[d_bag->get_unscaled_width()]++;
  for (const_iterator it = begin(); it != end(); it++)
    {
      ArmyProto *a = (*it);
      if (a->getMaskedImage(Shield::NEUTRAL)->getImage () != NULL)
        sizecounts[a->getMaskedImage(Shield::NEUTRAL)->getImage ()->get_unscaled_width()]++;
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

void Armyset::clearBagImage (bool clear_name)
{
  if (clear_name)
    setBagImageName ("");

  PixMask *p = getBagPic ();
  if (p)
    delete p;
  setBagPic (NULL);
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
  broken = d_standard->load (&t);
  if (!broken)
    d_standard->instantiateImages ();
  t.Close ();
  return broken;
}

bool Armyset::instantiateShipImage ()
{
  bool broken = false;
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  broken = d_stackship->load (&t);
  if (!broken)
    d_stackship->instantiateImages ();
  t.Close ();
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
      if (getSelector(true, c)->getName () == name)
        getSelector(true, c)->clear ();
      if (getSelector(false, c)->getName () == name)
        getSelector(false, c)->clear ();
    }

  if (getBagImageName() == name)
    clearBagImage ();
  if (d_standard->getName () == name)
    d_standard->clear ();
  if (d_stackship->getName () == name)
    d_stackship->clear ();

  for (iterator i = begin (); i != end (); i++)
    {
      for (guint32 cc = Shield::WHITE; cc <= Shield::NEUTRAL; cc++)
        {
          Shield::Colour c = Shield::Colour (cc);
          if ((*i)->getMaskedImage(c)->getName () == name)
            (*i)->getMaskedImage (c)-> clear ();
        }
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
  bool broken = false;
  d_selector[0][c]->clear (false);
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  broken = d_selector[0][c]->load (&t);
  if (broken)
    return broken;
  d_selector[0][c]->instantiateImages ();
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
  bool broken = false;
  d_selector[1][c]->clear (false);
  Tar_Helper t(getConfigurationFile(), std::ios::in, broken);
  if (broken)
    return broken;
  broken = d_selector[1][c]->load (&t);
  if (broken)
    return broken;
  d_selector[1][c]->instantiateImages ();
  return broken;
}

TarFileMaskedImage *Armyset::getSelector (bool large, Shield::Colour c) const
{
  return d_selector[large ? 1 : 0][c];
}
