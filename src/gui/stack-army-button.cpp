//  Copyright (C) 2011, 2014, 2015 Ben Asselstine
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

#include <gtkmm.h>

#include "stack-army-button.h"
#include "stack.h"
#include "army.h"

#include "input-helpers.h"
#include "builder-cache.h"
#include "army-info-tip.h"
#include "ucompose.hpp"
#include "ImageCache.h"
#include "File.h"
#include "playerlist.h"
#include "player.h"
#include "shield.h"

Glib::ustring StackArmyButton::get_file(Configuration::UiFormFactor factor)
{
  Glib::ustring file = "";
  switch (factor)
    {
    case Configuration::UI_FORM_FACTOR_DESKTOP:
      file = "stack-army-button-desktop.ui";
      break;
    case Configuration::UI_FORM_FACTOR_NETBOOK:
      file = "stack-army-button-netbook.ui";
      break;
    case Configuration::UI_FORM_FACTOR_LARGE_SCREEN:
      file = "stack-army-button-large-screen.ui";
      break;
    }
  return file;
}

StackArmyButton * StackArmyButton::create(guint32 factor)
{
  Glib::ustring file = 
    StackArmyButton::get_file (Configuration::UiFormFactor(factor));
  Glib::RefPtr<Gtk::Builder> xml = BuilderCache::get(file);

  StackArmyButton *box;
  xml->get_widget_derived("box", box);
  box->d_factor = factor;
  box->d_stack = NULL;
  box->d_army = NULL;
  box->d_circle_colour_id = 0;
  box->army_button->set_active(false);
  return box;
}

StackArmyButton::StackArmyButton(BaseObjectType* baseObject, const Glib::RefPtr<Gtk::Builder> &xml)
  : Gtk::Box(baseObject)
{
  army_info_tip = NULL;
  xml->get_widget("army_button", army_button);
  xml->get_widget("army_image", army_image);
  xml->get_widget("army_label", army_label);
  xml->get_widget("eventbox", eventbox);
  eventbox->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
  xml->get_widget("stack_button", stack_button);
  xml->get_widget("stack_image", stack_image);
  xml->get_widget("stack_button_container", stack_button_container);
  memset (army_conn, 0, sizeof (army_conn));
}

StackArmyButton::~StackArmyButton()
{
  if (army_info_tip)
    delete army_info_tip;
}

void StackArmyButton::update_stack_button(bool selected)
{
  if (d_stack)
    {
      stack_image->clear();
      if (selected)
        stack_image->property_pixbuf() = 
          stack_button->render_icon_pixbuf(Gtk::Stock::YES, Gtk::ICON_SIZE_MENU);
      else
        stack_image->property_pixbuf() = 
          stack_button->render_icon_pixbuf(Gtk::Stock::NO, Gtk::ICON_SIZE_MENU);
      stack_button->show();
    }
  else
    stack_button->hide();
}

bool StackArmyButton::on_army_button_event(GdkEventButton *e)
{
  MouseButtonEvent ev = to_input_event(e);
  if (ev.button == MouseButtonEvent::RIGHT_BUTTON && 
      ev.state == MouseButtonEvent::PRESSED) 
    {
      if (army_info_tip)
        delete army_info_tip;
      army_info_tip = new ArmyInfoTip(army_button, d_army);
      return true;
    }
  else if (ev.button == MouseButtonEvent::RIGHT_BUTTON && 
           ev.state == MouseButtonEvent::RELEASED) 
    {
      if (army_info_tip)
        {
          delete army_info_tip;
          army_info_tip = NULL;
        }
      return true;
    }

  return false;
}

void StackArmyButton::fill_buttons()
{
  fill_army_button();
  fill_stack_button();
}

void StackArmyButton::fill_army_button()
{
  Player *p = Playerlist::getActiveplayer();
  ImageCache *gc = ImageCache::getInstance();
  if (d_army)
    {
      bool greyed_out = false;
      Stack *active_stack = p->getActivestack();
      if (active_stack->getArmyById(d_army->getId()) == NULL)
        greyed_out = true;
      army_image->property_pixbuf() =
        gc->getCircledArmyPic(p->getArmyset(), d_army->getTypeId(),
                              p, d_army->getMedalBonuses(), greyed_out, 
                              !greyed_out ? p->getId() : d_circle_colour_id, 
                              true)->to_pixbuf();

      Pango::AttrList attrs;
      Pango::Attribute scale;
      switch (d_factor)
        {
        case Configuration::UI_FORM_FACTOR_DESKTOP:
          scale = Pango::Attribute::create_attr_scale(1.0);
          break;
        case Configuration::UI_FORM_FACTOR_NETBOOK:
          scale = Pango::Attribute::create_attr_scale(0.8);
          break;
        case Configuration::UI_FORM_FACTOR_LARGE_SCREEN:
          scale = Pango::Attribute::create_attr_scale(1.2);
          break;
        }
      attrs.insert(scale);
      army_label->set_attributes(attrs);
      army_label->set_label(String::ucompose("%1", d_army->getMoves()));
    }
  else
    {
      army_image->property_pixbuf() = 
        gc->getCircledArmyPic(p->getArmyset(), 0, p, NULL, false, 
                              Shield::NEUTRAL, false)->to_pixbuf();
      
      stack_image->clear();
      army_label->set_text("  ");
    }
}

void StackArmyButton::fill_stack_button()
{
  update_stack_button 
    (d_stack == Playerlist::getActiveplayer()->getActivestack());
}

void StackArmyButton::clear_signals()
{
  //clear the old signals
  stack_conn.disconnect();
  for (unsigned int i = 0; i < 3; i++)
    army_conn[i].disconnect();
}

void StackArmyButton::setup_signals()
{
  clear_signals();
  if (d_stack)
    stack_conn = stack_button->signal_clicked().connect
      (sigc::mem_fun(stack_clicked, &sigc::signal<void>::emit));
  if (d_army)
    {
      army_conn[0] = army_button->signal_toggled().connect
        (sigc::mem_fun(army_toggled, &sigc::signal<void>::emit));
      army_conn[1] = army_button->signal_button_press_event().connect
        (sigc::mem_fun(*this, &StackArmyButton::on_army_button_event), false);
      army_conn[2] = army_button->signal_button_release_event().connect
        (sigc::mem_fun(*this, &StackArmyButton::on_army_button_event), false);
    }
}

void StackArmyButton::reset()
{
  clear_signals();
  army_button->set_sensitive(true);
  stack_button->set_sensitive(true);
  draw(NULL, NULL, 0, false);
  army_button->set_sensitive(false);
  stack_button->set_sensitive(false);
}
    
void StackArmyButton::draw(Stack *s, Army *a, guint32 circle_colour_id, bool toggled)
{
  d_stack = s;
  d_army = a;
  d_circle_colour_id = circle_colour_id;
  army_button->set_sensitive(true);
  stack_button->set_sensitive(true);
  if (army_button->get_active() != toggled)
    army_button->set_active(toggled);
  fill_buttons();
  setup_signals();
}
