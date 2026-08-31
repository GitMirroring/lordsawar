//  Copyright (C) 2007, 2008 Ole Laursen
//  Copyright (C) 2007, 2008, 2009, 2010, 2012, 2014, 2015, 2020 Ben Asselstine
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

#include <numeric>
#include <vector>
#include <gtkmm.h>

#include "fight-window.h"
#include "builder-cache.h"

#include "timing.h"
#include "File.h"
#include "player.h"
#include "army.h"
#include "ImageCache.h"
#include "Configuration.h"
#include "snd.h"
#include "GameMap.h"
#include "tileset.h"
#include "tilesetlist.h"
#include "Tile.h"
#include "SmallTile.h"
#include "font-size.h"

#define method(x) sigc::mem_fun(*this, &FightWindow::x)

bool FightWindow::s_quick_all = false;

FightWindow::FightWindow(Gtk::Window &parent, Fight &fight)
{
  Glib::RefPtr<Gtk::Builder> xml = BuilderCache::get("fight-window.ui");

  xml->get_widget("window", window);
  window->set_transient_for(parent);
  guint32 height = GameMap::getTileset()->getTileSize () * 3;
  guint32 width = height * (16.0/9.0);
  window->set_size_request (width, height);

  window->signal_key_release_event().connect_notify
    (method(on_key_release_event));

  xml->get_widget("decision_label", decision_label);

  Gtk::Box *attacker_close_vbox;
  Gtk::Box *defender_close_vbox;
  xml->get_widget("attacker_close_vbox", attacker_close_vbox);
  xml->get_widget("defender_close_vbox", defender_close_vbox);

  // extract attackers and defenders
  armies_type attackers, defenders;

  Fight::orderArmies (fight.getAttackers(), attackers);
  Fight::orderArmies (fight.getDefenders(), defenders);

  // add the armies
  std::vector<Gtk::Box *> close_hboxes;
  int close;
  std::map<guint32, guint32> initial_hps = fight.getInitialHPs();

  // ... attackers
  close = 0;
  for (armies_type::iterator i = attackers.begin(); i != attackers.end(); ++i)
    add_army(*i, initial_hps[(*i)->getId()], close_hboxes, attacker_close_vbox, 
             close++);

  close_hboxes.clear();

  // ... defenders
  close = 0;
  for (armies_type::iterator i = defenders.begin(); i != defenders.end(); ++i)
    add_army(*i, initial_hps[(*i)->getId()], close_hboxes, defender_close_vbox, 
             close++);

  // fill in shield pictures
  ImageCache *gc = ImageCache::getInstance();

  Gtk::Image *defender_shield_image;
  Player *p = defenders.front()->getOwner();
  xml->get_widget("defender_shield_image", defender_shield_image);
  defender_shield_image->property_pixbuf()=
    gc->getShieldPic(2, p, false,
                     FontSize::getInstance ()->get_height ())->to_pixbuf();

  Gtk::Image *attacker_shield_image;
  p = attackers.front()->getOwner();
  xml->get_widget("attacker_shield_image", attacker_shield_image);
  attacker_shield_image->property_pixbuf()=
    gc->getShieldPic(2, p, false,
                     FontSize::getInstance ()->get_height ())->to_pixbuf();

  actions = fight.getCourseOfEvents();
  d_quick = false;

  if (fight.getResult () == Fight::ATTACKER_WON)
    {
      Glib::ustring hero = fight.getStrongestLivingHeroName (attackers);
      if (hero.empty () == true)
        d_decision = _("Your armies have won the city!");
      else
        d_decision = String::ucompose (_("%1 has won the battle!"), hero);
    }
  else
    d_decision = _("You have lost!");

  fast_round_speed = Configuration::s_displayFightRoundDelayFast; //ms
  normal_round_speed = Configuration::s_displayFightRoundDelaySlow; //ms
  Snd::getInstance()->disableBackground();
  Snd::getInstance()->play("battle", -1, true);

}

FightWindow::~FightWindow()
{
  Snd::getInstance()->halt(true);
  Snd::getInstance()->enableBackground();
  delete window;
}

void FightWindow::run(bool *quick)
{
  round = 0;
  action_iterator = actions.begin();

  if (s_quick_all)
    Timing::instance().register_timer (method(do_round), fast_round_speed / 3);
  else
    Timing::instance().register_timer(method(do_round), 
                                      *quick == true ?
                                      fast_round_speed : normal_round_speed);

  window->show_all();
  main_loop = Glib::MainLoop::create();
  main_loop->run();
  if (quick && *quick == false)
    *quick = d_quick;
  if (s_quick_all)
    d_quick = true;
}

void FightWindow::add_army(Army *army, int initial_hp,
                           std::vector<Gtk::Box *> &hboxes, Gtk::Box *vbox, 
                           int current_no)
{
  Gtk::Box *army_box;
  Gtk::Image *army_image;
  Gtk::Image *water_image;

  Glib::RefPtr<Gtk::Builder> xml = BuilderCache::get("fighter.ui");

  xml->get_widget("army_box", army_box);
  xml->get_widget("army_image", army_image);
  xml->get_widget("water_image", water_image);

  // image
  guint32 fs = FontSize::getInstance()->get_height ();
  PixMask *armypic = ImageCache::getInstance()->getDialogArmyPic(army, fs);
  armypic_width = armypic->get_width ();
  armypic_height = armypic->get_height ();
  army_image->property_pixbuf() = armypic->to_pixbuf();
  int height = 3;
  SmallTile *water =
    Tilesetlist::getInstance()->getSmallTile(GameMap::getTileset()->getBaseName(), Tile::WATER);
  if (army->getStat(Army::SHIP, false) && water)
    {
      Cairo::RefPtr<Cairo::Surface> surf = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, armypic->get_width(), height);
      Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surf);
      Gdk::RGBA w = water->getColor();
      cr->set_source_rgb(w.get_red(), w.get_green(), w.get_blue());
      cr->rectangle(0, 0, armypic->get_width(), height);
      cr->paint();
      Glib::RefPtr<Gdk::Pixbuf> p =
        Gdk::Pixbuf::create(surf, 0, 0, armypic->get_width(), height);
      water_image->property_pixbuf() = p;
    }
  else
    {
      Glib::RefPtr<Gdk::Pixbuf> empty =
        Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, height, height);
      empty->fill(0x00000000);
      water_image->property_pixbuf() = empty;
    }

  // then add it to the right hbox
  int current_row = (current_no / max_cols);
  if (current_row >= int(hboxes.size()))
    {
      // add an hbox
      Gtk::Box *hbox = manage(new Gtk::Box (Gtk::ORIENTATION_HORIZONTAL));
      hbox->set_spacing(6);
      hboxes.push_back(hbox);

      Gtk::Box *a = manage(new Gtk::Box());
      a->add(*hbox);
      vbox->pack_start(*a, Gtk::PACK_SHRINK);
    }

  Gtk::Box *hbox = hboxes[current_row];
  army_box->get_parent()->remove(*army_box);
  Gtk::Box *box = new Gtk::Box(Gtk::ORIENTATION_VERTICAL );
  box->pack_start(*army_box, Gtk::PACK_SHRINK);
  army_box->reparent(*box);
  hbox->pack_start(*Gtk::manage(box), Gtk::PACK_SHRINK);

  // finally add an entry for later use
  ArmyItem item;
  item.army = army;
  item.hp = initial_hp;
  item.water_image = water_image;
  item.image = army_image;
  item.exploding = false;
  army_items.push_back(item);
}

bool FightWindow::do_round()
{
  ImageCache *gc = ImageCache::getInstance();
  PixMask *p = gc->getExplosionPic ()->copy ();
  PixMask::scale (p, armypic_width, armypic_height);
  Glib::RefPtr<Gdk::Pixbuf> expl = p->to_pixbuf();
  delete p;

  // first we clear out any explosions
  for (army_items_type::iterator i = army_items.begin(), end = army_items.end(); 
       i != end; ++i)
    {
      if (!i->exploding)
        continue;

      Glib::RefPtr<Gdk::Pixbuf> empty_pic
        = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, true, 8, armypic_width, armypic_height);
      empty_pic->fill(0x00000000);
      i->image->property_pixbuf() = empty_pic;
      i->exploding = false;
      return Timing::CONTINUE;
    }

  while (action_iterator != actions.end())
    {
      FightItem &f = *action_iterator;

      ++action_iterator;

      for (army_items_type::iterator i = army_items.begin(), 
           end = army_items.end(); i != end; ++i)
        if (i->army->getId() == f.id)
          {
            i->hp -= f.damage;
            if (i->hp < 0)
              i->hp = 0;
            double fraction = double(i->hp) / i->army->getStat(Army::HP);
            if (fraction == 0.0)
              {
                Glib::RefPtr<Gdk::Pixbuf> w = i->water_image->property_pixbuf ();
                w->fill(0x00000000);
                i->water_image->property_pixbuf () = w;
                Glib::RefPtr<Gdk::Pixbuf> a = i->image->property_pixbuf();
                //draw the explosion on the army
                expl->copy_area (0, 0, armypic_width, armypic_height, a, 0 , 0);
                i->image->property_pixbuf () = a;
                i->exploding = true;
              }

            break;
          }

      if (f.turn > round)
        {
          ++round;

          return Timing::CONTINUE;
        }
    }

  guint32 first_pause = 100000;
  guint32 second_pause = 2000000;

  while (g_main_context_iteration(NULL, FALSE)); //doEvents
  if (d_quick || s_quick_all)
    Glib::usleep (first_pause / 100);
  else
    Glib::usleep (first_pause);

  decision_label->set_text (d_decision);

  while (g_main_context_iteration(NULL, FALSE)); //doEvents
  if (d_quick || s_quick_all)
    Glib::usleep (second_pause / 100);
  else
    Glib::usleep (second_pause);


  window->hide();
  main_loop->quit();
    
  return Timing::STOP;
}

void FightWindow::on_key_release_event(GdkEventKey *e)
{
  if (e->keyval == GDK_KEY_exclam)
    {
      if (s_quick_all)
        {
          Timing::instance().register_timer (method(do_round),
                                             normal_round_speed);
          d_quick = false;
          s_quick_all = false;
        }
      else
        {
          d_quick = true;
          s_quick_all = true;
          Timing::instance().register_timer (method(do_round), 
                                             fast_round_speed / 3);
        }
    }
  else
    {
      Timing::instance().register_timer (method(do_round), fast_round_speed);
      d_quick = true;
    }
}
