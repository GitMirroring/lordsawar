//  Copyright (C) 2007-2009, 2011, 2012, 2014-2017, 2020 Ben Asselstine
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

#include "report-dialog.h"
#include "bar-chart.h"
#include "ucompose.hpp"
#include "defs.h"
#include "GameMap.h"
#include "playerlist.h"
#include "citylist.h"
#include "city.h"
#include "action.h"
#include "ImageCache.h"
#include "armysetlist.h"
#include "shield.h"
#include "armyprodbase.h"
#include "font-size.h"

#define method(x) sigc::mem_fun(*this, &ReportDialog::x)

ReportDialog::ReportDialog(Gtk::Window &parent, Player *player, ReportType type)
 : LwDialog(parent, "report-dialog.ui")
{
  d_player = player;
  xml->get_widget("map_image", map_image);
  citymap = new CityMap();
  citymap->map_changed.connect (method(on_city_map_changed));
  armymap = new ArmyMap();
  armymap->map_changed.connect (method(on_army_map_changed));
  City *c = d_player->getFirstCity();
  vectormap = new VectorMap(c, VectorMap::SHOW_ALL_VECTORING, false);
  vectormap->map_changed.connect (method(on_vector_map_changed));

  xml->get_widget("army_label", army_label);
  xml->get_widget("city_label", city_label);
  xml->get_widget("gold_label", gold_label);
  xml->get_widget("production_label", production_label);
  xml->get_widget("winning_label", winning_label);


  xml->get_widget("report_notebook", report_notebook);
  report_notebook->set_current_page(type);
  switch_conn = report_notebook->signal_switch_page().connect
    (sigc::hide<0>(method(on_switch_page)));

  armies_list = Gtk::ListStore::create(armies_columns);
  xml->get_widget("treeview", armies_treeview);
  armies_treeview->set_model(armies_list);
  armies_treeview->append_column("", armies_columns.image);
  armies_treeview->append_column("", armies_columns.desc);

  //loop through the action list looking for production actions
  std::list<Action*> actions = player->getReportableActions();
  guint32 total = 0;
  std::list<Action*>::const_iterator it;
  for (it = actions.begin(); it != actions.end(); ++it)
    {
      if ((*it)->getType() == Action::PRODUCE_UNIT ||
	  (*it)->getType() == Action::PRODUCE_VECTORED_UNIT)
	total++;
    addProduction(*it);
    }
    armies_treeview->get_selection()->signal_changed().connect
      (method(on_army_selected));

  Glib::ustring s;
  s = String::ucompose(ngettext("You produced %1 army this turn!",
				"You produced %1 armies this turn!",
		  		total), total);
  production_label->set_text(s);

  xml->get_widget("army_alignment", army_alignment);
  xml->get_widget("city_alignment", city_alignment);
  xml->get_widget("gold_alignment", gold_alignment);
  xml->get_widget("winning_alignment", winning_alignment);
  updateArmyChart();
  updateGoldChart();
  updateCityChart();
  updateWinningChart();
  fill_in_info();
}

ReportDialog::~ReportDialog()
{
  switch_conn.disconnect();
  delete vectormap;
  delete armymap;
  delete citymap;
  report_notebook->property_show_tabs () = false;
}

void ReportDialog::run()
{
  citymap->resize();
  citymap->draw();
  vectormap->resize();
  vectormap->draw();
  armymap->resize();
  armymap->draw();

  dialog->show_all();
  dialog->run();
}

void ReportDialog::on_army_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  if (report_notebook->get_current_page() == ARMY)
    {
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
        Gdk::Pixbuf::create(map, 0, 0, 
                            armymap->get_width(), armymap->get_height());
      map_image->property_pixbuf() = pixbuf;
    }
}

void ReportDialog::on_city_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  if (report_notebook->get_current_page() == CITY ||
      report_notebook->get_current_page() == GOLD ||
      report_notebook->get_current_page() == WINNING)
    {
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
        Gdk::Pixbuf::create(map, 0, 0, 
                            citymap->get_width(), citymap->get_height());
      map_image->property_pixbuf() = pixbuf;
    }
}

void ReportDialog::on_vector_map_changed(Cairo::RefPtr<Cairo::Surface> map)
{
  if (report_notebook->get_current_page() == PRODUCTION)
    {
      Glib::RefPtr<Gdk::Pixbuf> pixbuf = 
        Gdk::Pixbuf::create(map, 0, 0, 
                            vectormap->get_width(), vectormap->get_height());
      map_image->property_pixbuf() = pixbuf;
    }
}

void ReportDialog::on_switch_page(guint number)
{
  switch (number)
    {
    case ARMY:
      map_image->property_pixbuf() =
        Gdk::Pixbuf::create(armymap->get_surface(), 0, 0, 
                            armymap->get_width(), citymap->get_height());
      break;
    case CITY: 
    case GOLD: 
    case WINNING:
      map_image->property_pixbuf() =
        Gdk::Pixbuf::create(citymap->get_surface(), 0, 0,
                            citymap->get_width(), citymap->get_height());
      break;
    case PRODUCTION:
      map_image->property_pixbuf() =
        Gdk::Pixbuf::create(vectormap->get_surface(), 0, 0,
                            vectormap->get_width(), vectormap->get_height());
      break;
    }
  fill_in_info();
}

void ReportDialog::fill_in_info()
{
  switch (report_notebook->get_current_page())
    {
    case ARMY:
      dialog->set_title(_("Army Report"));
      break;
    case CITY: 
      dialog->set_title(_("City Report"));
      break;
    case GOLD: 
      dialog->set_title(_("Gold Report"));
      break;
    case PRODUCTION:
      dialog->set_title(_("Production Report"));
      break;
    case WINNING:
      dialog->set_title(_("Winning Report"));
      break;
    }
}

void ReportDialog::updateArmyChart()
{
  std::list<guint32> bars;
  std::list<Gdk::RGBA> colors;
  Gdk::RGBA color;
  Glib::ustring s;
  guint32 total;
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      Player *p = Playerlist::getInstance()->getPlayer(i);
      if (p == NULL)
	continue;
      if (p == Playerlist::getInstance()->getNeutral())
	continue;
      total = 0;
      total = p->countArmies();
      bars.push_back(total);
      color = p->getColor();
      colors.push_back(color);
      if (p == d_player)
        army_label->set_text
          (String::ucompose(ngettext("You have %1 army!", "You have %1 armies!",
                                     total), total));
    }

  army_chart = new BarChart(bars, colors, 0);
  army_chart->set_vexpand (true);
  army_alignment->add(*manage(army_chart));
}

void ReportDialog::updateCityChart()
{
  std::list<guint32> bars;
  std::list<Gdk::RGBA> colors;
  Gdk::RGBA color;
  Glib::ustring s;
  guint32 total;
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      Player *p = Playerlist::getInstance()->getPlayer(i);
      if (p == NULL)
	continue;
      if (p == Playerlist::getInstance()->getNeutral())
	continue;
      total = Citylist::getInstance()->countCities(p);

      bars.push_back(total);
      color = p->getColor();
      colors.push_back(color);
      if (p == d_player)
        city_label->set_text
          (String::ucompose(ngettext("You have %1 city!", "You have %1 cities!",
                                     total), total));

    }
  city_chart = new BarChart(bars, colors, Citylist::getInstance()->size());
  city_chart->set_vexpand (true);
  city_alignment->add(*manage(city_chart));
}

void ReportDialog::updateGoldChart()
{
  std::list<guint32> bars;
  std::list<Gdk::RGBA> colors;
  Gdk::RGBA color;
  Glib::ustring s;
  guint32 total;
  bars.clear();
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      Player *p = Playerlist::getInstance()->getPlayer(i);
      if (p == NULL)
	continue;
      if (p == Playerlist::getInstance()->getNeutral())
	continue;
      total = p->getGold();
      bars.push_back(total);
      color = p->getColor();
      colors.push_back(color);
      if (p == d_player)
        gold_label->set_text
          (String::ucompose(ngettext("You have %1 gold piece!",
                                     "You have %1 gold pieces!", total), total));
    }
  gold_chart = new BarChart(bars, colors, 0);
  gold_chart->set_vexpand (true);
  gold_alignment->add(*manage(gold_chart));
}

Glib::ustring ReportDialog::get_rank_string(int rank)
{
  if (rank == 0)
      return _("first"); 
  else if (rank == 1)
    return _("second"); 
  else if (rank == 2)
    return _("third"); 
  else if (rank == 3) 
    return _("fourth"); 
  else if (rank == 4)
    return _("fifth");
  else if (rank == 5)
    return _("sixth"); 
  else if (rank == 6)
    return _("seventh"); 
  else if (rank == 7)
    return _("eighth");
  else
    return _("unknown");
}

Glib::ustring ReportDialog::calculateRank(std::list<guint32> scores, guint32 score)
{
  guint32 rank = 0;
  for (std::list<guint32>::iterator it = scores.begin();
       it != scores.end(); ++it)
    {
      if (score < *it)
	rank++;
    }
  Glib::ustring s = String::ucompose("%1", get_rank_string(rank));
  return s;
}

void ReportDialog::updateWinningChart()
{
  std::list<guint32> bars;
  std::list<Gdk::RGBA> colors;
  Gdk::RGBA color;
  Glib::ustring s;
  guint32 score;
  for (unsigned int i = 0; i < MAX_PLAYERS; i++)
    {
      Player *p = Playerlist::getInstance()->getPlayer(i);
      if (p == NULL)
	continue;
      if (p == Playerlist::getInstance()->getNeutral())
	continue;
      score = p->getScore();
      bars.push_back(score);
      color = p->getColor();
      colors.push_back(color);
    }
  s = String::ucompose(_("You are coming %1"), calculateRank(bars, d_player->getScore()));
  winning_label->set_text(s);
  winning_chart = new BarChart(bars, colors, 100);
  winning_chart->set_vexpand (true);
  winning_alignment->add(*manage(winning_chart));
}

void ReportDialog::addProduction(const Action *action)
{
  ImageCache *gc = ImageCache::getInstance();
  Player *p = d_player;

  int army_type = 0;
  guint32 city_id = 0;

  Glib::ustring s = "";
  if (action->getType() == Action::PRODUCE_UNIT)
    {
      const Action_Produce *act;
      act = dynamic_cast<const Action_Produce*>(action);
      army_type = act->getArmy()->getTypeId();
      for (Citylist::iterator cit = Citylist::getInstance()->begin();
           cit != Citylist::getInstance()->end(); ++cit)
	if ((*cit)->getId() == act->getCityId())
	  {
	    s += (*cit)->getName();
	    break;
	  }
      if (act->getVectored())
	s += "...";
      city_id = act->getCityId();
    }
  else if (action->getType() == Action::PRODUCE_VECTORED_UNIT)
    {
      const Action_ProduceVectored *act = 
        dynamic_cast<const Action_ProduceVectored*>(action);
      army_type = act->getArmy()->getTypeId();
      Vector<int> pos = act->getDestination();
      City *c = GameMap::getCity(pos);
      s+="...";
      if (c)
	s += c->getName();
      else
	s += _("Standard");
      city_id = GameMap::getCity(act->getOrigination())->getId();
    }
  else if (action->getType() == Action::CITY_DESTITUTE)
    {
      const Action_CityTooPoorToProduce *act;
      act = dynamic_cast<const Action_CityTooPoorToProduce*>(action);
      army_type = act->getArmyType();
      City *c = Citylist::getInstance()->getById(act->getCityId());
      s = String::ucompose(_("%1 stops production!"), c->getName());
      city_id = act->getCityId();
    }
  Gtk::TreeIter i = armies_list->append();
  (*i)[armies_columns.city_id] = city_id;
  (*i)[armies_columns.image] = 
    gc->getCircledArmyPic(p->getArmyset(), army_type, p, NULL, false,
                          Shield::NEUTRAL, true,
                          FontSize::getInstance ()->get_height ())->to_pixbuf();
  (*i)[armies_columns.desc] = s;
}

void ReportDialog::on_army_selected()
{
    Gtk::TreeIter i = armies_treeview->get_selection()->get_selected();
    if (i)
    {
	City *c = 
	  Citylist::getInstance()->getById((*i)[armies_columns.city_id]);
	if (c)
	  vectormap->setCity(c);
    }
}
