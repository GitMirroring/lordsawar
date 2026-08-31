//  Copyright (C) 2008, 2009, 2011, 2014, 2020 Ben Asselstine
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
#include <sigc++/functors/mem_fun.h>

#include "diplomacy-dialog.h"
#include "diplomacy-report-dialog.h"

#include "defs.h"
#include "ImageCache.h"
#include "playerlist.h"
#include "player.h"
#include "font-size.h"

#define method(x) sigc::mem_fun(*this, &DiplomacyDialog::x)

DiplomacyDialog::DiplomacyDialog(Gtk::Window &parent, Player *player)
 : LwDialog(parent, "diplomacy-dialog.ui")
{
  ImageCache *gc = ImageCache::getInstance();
  Playerlist *pl = Playerlist::getInstance();
  d_player = player;
  xml->get_widget("proposals_table", d_proposals_table);
  xml->get_widget("offers_table", d_offers_table);
  xml->get_widget("player_label", d_player_label);
  xml->get_widget("player_shield_image", d_player_shield_image);
  xml->get_widget("report_button", d_report_button);
    
  d_report_button->signal_clicked().connect (method(on_report_clicked));

  // put the shields across the top of the proposals table, minus our own
  guint32 i = 0;
  guint32 j = 0;
  for (unsigned int k = 0; k < MAX_PLAYERS; k++)
    {
      Player *p = pl->getPlayer(k);
      if (p == NULL)
	continue;
      if (pl->getNeutral() == p)
	continue;
      if (p == d_player)
	continue;
      Glib::RefPtr<Gdk::Pixbuf> pixbuf =
        gc->getShieldPic(2, p, false,
                         FontSize::getInstance ()->get_height ())->to_pixbuf();
      Gtk::Image *im = new Gtk::Image();
      im->property_pixbuf() = pixbuf;
      d_proposals_table->attach(*manage(im), i, 0, 1, 1);
      i++;
    }
  d_proposals_table->set_column_spacing (16);
    
  d_player_shield_image->property_pixbuf() =
    gc->getShieldPic(2, d_player, false,
                     FontSize::getInstance ()->get_height ())->to_pixbuf();

  d_player_label->set_text(d_player->getName());

  //fill in diplomatic state
  i = 0;
  j = 0;
  for (unsigned int k = 0; k < MAX_PLAYERS; k++)
    {
      Player *p = pl->getPlayer(k);
      if (p == NULL)
	continue;
      if (pl->getNeutral() == p)
	continue;
      if (p == d_player)
	continue;
      if (p->isDead())
	{
	  i++;
	  continue;
	}
      j = 0;
      Player::DiplomaticState state = d_player->getDiplomaticState (p);
      Glib::RefPtr<Gdk::Pixbuf> pixbuf=
        gc->getDiplomacyPic(1, state,
                            FontSize::getInstance ()->get_height ())->to_pixbuf();
      Gtk::Image *im = new Gtk::Image();
      im->property_pixbuf() = pixbuf;
      d_proposals_table->attach(*manage(im), i, j + 1, 1, 1);
      Player::DiplomaticProposal proposal = p->getDiplomaticProposal (d_player);
      if (proposal != Player::NO_PROPOSAL)
	{
	  j = 1;
	  Glib::RefPtr<Gdk::Pixbuf> pixbuf2;
	  switch (proposal)
	    {
	    case Player::PROPOSE_PEACE:
	      pixbuf2 =
                gc->getDiplomacyPic
                (1, Player::AT_PEACE,
                 FontSize::getInstance ()->get_height ())->to_pixbuf();
	      break;
	    case Player::PROPOSE_WAR_IN_FIELD:
	      pixbuf2 =
                gc->getDiplomacyPic
                (1, Player::AT_WAR_IN_FIELD,
                 FontSize::getInstance ()->get_height ())->to_pixbuf();
	      break;
	    case Player::PROPOSE_WAR:
	      pixbuf2 =
                gc->getDiplomacyPic
                (1, Player::AT_WAR,
                 FontSize::getInstance ()->get_height ())->to_pixbuf();
	      break;
	    default:
	      continue;
	    }

	  Gtk::Image *im2 = manage(new Gtk::Image());
	  im2->property_pixbuf() = pixbuf2;
	  d_proposals_table->attach(*manage(im2), i , j + 1, 1, 1);
	}
      i++;
    }


  // fill in the togglebuttons
  i = 0;
  j = 0;
  for (unsigned int k = 0; k < MAX_PLAYERS; k++)
    {
      Player *p = pl->getPlayer(k);
      if (p == NULL)
	continue;
      if (pl->getNeutral() == p)
	continue;
      if (p == d_player)
	continue;

      //show the peace radio buttons
      j = 0;
      Gtk::RadioButton *radio1= manage(new Gtk::RadioButton);
      Gtk::Image *im3 = new Gtk::Image();
      im3->property_pixbuf() = 
	gc->getDiplomacyPic(1, Player::AT_PEACE,
                            FontSize::getInstance ()->get_height ())->to_pixbuf();
      radio1->set_tooltip_text(_("Propose peace"));
      radio1->add(*manage(im3));
      radio1->set_mode(false);
      Gtk::RadioButtonGroup group = radio1->get_group();
      if (p->isDead())
	radio1->set_sensitive(false);
      else
	radio1->set_active (d_player->getDiplomaticProposal(p) == 
			    Player::PROPOSE_PEACE);
      radio1->signal_toggled().connect
        (sigc::bind(method(on_proposal_toggled), radio1, p, Player::PROPOSE_PEACE));
      d_offers_table->attach(*radio1, i, j ,1, 1);

      j = 1;
      Gtk::RadioButton *radio2= manage(new Gtk::RadioButton);
      Gtk::Image *im4 = new Gtk::Image();
      im4->property_pixbuf() = 
	gc->getDiplomacyPic(1, Player::AT_WAR_IN_FIELD,
                            FontSize::getInstance ()->get_height ())->to_pixbuf();
      radio2->set_tooltip_text(_("Propose war on armies not in cities"));
      radio2->add(*manage(im4));
      radio2->set_mode(false);
      radio2->set_group(group);
      if (p->isDead())
	radio2->set_sensitive(false);
      else
	radio2->set_active (d_player->getDiplomaticProposal(p) == 
			    Player::PROPOSE_WAR_IN_FIELD);
      radio2->signal_toggled().connect
        (sigc::bind(method(on_proposal_toggled), radio2, p, 
                    Player::PROPOSE_WAR_IN_FIELD));
      d_offers_table->attach(*radio2, i, j, 1 , 1);

      j = 2;
      Gtk::RadioButton *radio3= manage(new Gtk::RadioButton);
      Gtk::Image *im5 = new Gtk::Image();
      im5->property_pixbuf() = 
	gc->getDiplomacyPic(1, Player::AT_WAR,
                            FontSize::getInstance ()->get_height ())->to_pixbuf();
      radio3->set_tooltip_text(_("Propose war"));
      radio3->add(*manage(im5));
      radio3->set_mode(false);
      radio3->set_group(group);
      if (p->isDead())
	radio3->set_sensitive(false);
      else
	radio3->set_active (d_player->getDiplomaticProposal(p) == 
			    Player::PROPOSE_WAR);
      radio3->signal_toggled().connect
        (sigc::bind(method(on_proposal_toggled), radio3, p, Player::PROPOSE_WAR));
      d_offers_table->attach(*radio3, i, j, 1, 1);
      i++;
    }
}

void DiplomacyDialog::on_proposal_toggled (Gtk::ToggleButton *toggle, 
					   Player *player, 
					   Player::DiplomaticProposal proposal)
{
  if (toggle->get_active() == true)
    d_player->proposeDiplomacy (proposal, player);
}

void DiplomacyDialog::on_report_clicked()
{
  DiplomacyReportDialog d(*dialog, d_player);
  d.run_and_hide();
}
