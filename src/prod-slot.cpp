//  Copyright (C) 2008, 2015 Ben Asselstine
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

#include "prodslot.h"

#include "armyprodbase.h"
#include "xmlhelper.h"

Glib::ustring ProdSlot::d_tag = "slot";

//#define debug(x) {std::cerr<<__FILE__<<": "<<__LINE__<<": "<<x<<std::endl<<std::flush;}
//#define debug(x)

ProdSlot::ProdSlot()
{
  d_armyprodbase = NULL;
}

ProdSlot::ProdSlot(const ProdSlot& object)
{
  if (object.d_armyprodbase != NULL)
    d_armyprodbase = new ArmyProdBase(*object.d_armyprodbase);
  else
    d_armyprodbase = NULL;
}

ProdSlot::ProdSlot(XML_Helper* helper)
{
  d_armyprodbase = NULL;
  helper->registerTag(ArmyProdBase::d_tag, 
		      sigc::mem_fun(this, &ProdSlot::load));

}

bool ProdSlot::load(Glib::ustring tag, XML_Helper *helper)
{
  if (tag == ArmyProdBase::d_tag)
    {
      d_armyprodbase = new ArmyProdBase(helper);
      return true;
    }
  return false;
}

ProdSlot::~ProdSlot()
{
  if (d_armyprodbase)
    {
      delete d_armyprodbase;
      d_armyprodbase = NULL;
    }
}

bool ProdSlot::save(XML_Helper *helper) const
{
  bool retval = true;
  retval &= helper->openTag(ProdSlot::d_tag);
  if (d_armyprodbase)
    retval &= d_armyprodbase->save(helper);
  retval &= helper->closeTag();
  return retval;
}
    
void ProdSlot::clear()
{
  if (d_armyprodbase) 
    delete d_armyprodbase; 
  d_armyprodbase = NULL;
}
