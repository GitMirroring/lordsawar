//  Copyright (C) 2007, 2008, 2009, 2011, 2012, 2014, 2015, 2016, 2017, 2020,
//  2021, 2026 Ben Asselstine
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
//  Foundation, Inc., 31 Milk Street #960789, Boston, MA 02196, USA.

#include <gtkmm.h>
#ifndef REPORT_DIALOG_H
#define REPORT_DIALOG_H
#include "lw-dialog-base.h"
#include "bar-chart.h"
#include "army-map.h"
#include "vector-map.h"
#include "city-map.h"
#include "lw-column.h"

class ReportProductionRow: public Glib::Object
{
public:
    guint32 m_city_id;
    PixMask *m_image;
    Glib::ustring m_desc;

    static Glib::RefPtr<ReportProductionRow> create (guint32 city,
                                                     PixMask *image,
                                                     Glib::ustring desc)
      {
        return
          Glib::make_refptr_for_instance<ReportProductionRow>
          (new ReportProductionRow (city, image, desc));
      }

protected:
    ReportProductionRow (guint32 city, PixMask *image, Glib::ustring desc)
      : m_city_id (city), m_image (image), m_desc (desc)
      {
      }
};

class ReportDialog: public LwDialogBase
{
public:
    static std::string get_resource_name ()
      {
        return "report.ui";
      }

    enum ReportType
      {
        ARMY,
        CITY,
        GOLD,
        PRODUCTION,
        WINNING
      };

    ~ReportDialog ()
      {
        delete m_vectormap;
        delete m_armymap;
        delete m_citymap;
        m_armymap_changed.disconnect ();
        m_citymap_changed.disconnect ();
        m_vectormap_changed.disconnect ();
        m_switch_page.disconnect ();
      }

    ReportDialog (BaseObjectType* o,
                  const Glib::RefPtr<Gtk::Builder>& xml)
      : LwDialogBase (o, xml)
      {
        m_close_button = load <Gtk::Button> ("close_button");
        m_map_drawing_area = load <Gtk::DrawingArea> ("map_drawing_area");
        m_army_label = load <Gtk::Label> ("army_label");
        m_city_label = load <Gtk::Label> ("city_label");
        m_gold_label = load <Gtk::Label> ("gold_label");
        m_production_label = load <Gtk::Label> ("production_label");
        m_winning_label = load <Gtk::Label> ("winning_label");
        m_report_notebook = load <Gtk::Notebook> ("report_notebook");
        m_army_alignment = load <Gtk::Box> ("army_alignment");
        m_city_alignment = load <Gtk::Box> ("city_alignment");
        m_gold_alignment = load <Gtk::Box> ("gold_alignment");
        m_winning_alignment = load <Gtk::Box> ("winning_alignment");
        m_treeview = load <Gtk::ColumnView> ("treeview");
      }

    void setup (ReportType report_type)
      {
        set_response (m_close_button, Gtk::ResponseType::ACCEPT);

        fill_army_tab ();
        fill_city_tab ();
        fill_gold_tab ();
        fill_production_tab ();
        fill_winning_tab ();

        m_switch_page = m_report_notebook->signal_switch_page ().connect
          ([this] (Gtk::Widget *, guint number)
           {
             switch (number)
               {
               case ARMY:
                 m_armymap->draw ();
                 break;

               case CITY:
               case GOLD:
               case WINNING:
                 m_citymap->draw ();
                 break;

               case PRODUCTION:
                 m_vectormap->draw ();
                 break;
               }

             switch (number)
               {
               case ARMY:
                 set_title (_("Army Report"));
                 break;

               case CITY:
                 set_title (_("City Report"));
                 break;

               case GOLD:
                 set_title (_("Gold Report"));
                 break;

               case PRODUCTION:
                 set_title (_("Production Report"));
                 break;

               case WINNING:
                 set_title (_("Winning Report"));
               }
           });
        // this is how we're forcing a redraw on the tab we want
        m_report_notebook->set_current_page (!report_type);
        m_report_notebook->set_current_page (report_type);
      }

    void fill_army_tab ()
      {
        m_armymap = new ArmyMap ();
        m_armymap->resize ();
        m_armymap_changed = m_armymap->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });

        auto motion = Gtk::EventControllerMotion::create ();
        motion->signal_motion ().connect
          ([this](double x, double y)
           {
             auto c = m_armymap->get_cursor (x, y);
             static ImageCache::CursorType prev_cursor = ImageCache::SHIP;
             if (c != prev_cursor)
               {
                 auto hotspot = ImageCache::get_hotspot (c);
                 auto im = ImageCache::instance ()->getCursorPic (c);
                 auto cursor = Gdk::Cursor::create (im->to_texture (),
                                                    hotspot.x, hotspot.y);
                 m_map_drawing_area->set_cursor (cursor);
               }
             prev_cursor = c;
           });
        m_map_drawing_area->add_controller (motion);

        std::list<guint32> bars;
        std::list<Gdk::RGBA> colors;
        for (unsigned int i = 0; i < MAX_PLAYERS; i++)
          {
            Player *p = Playerlist::instance ()->get (i);
            if (p == NULL ||
                p == Playerlist::instance ()->getNeutral ())
              continue;
            guint32 total = p->countArmies ();
            bars.push_back (total);
            Gdk::RGBA color =
              GameMap::getShieldset ()->getColor (p->get_shield ());
            colors.push_back (color);
            if (p == Playerlist::getActiveplayer ())
              m_army_label->set_text
                (String::ucompose
                 (ngettext ("You have %1 army!", "You have %1 armies!",
                            total), total));
          }

        m_army_chart = Gtk::make_managed<BarChart> (bars, colors, 0);
        m_army_chart->set_vexpand (true);
        m_army_alignment->append (*m_army_chart);
      }

    void fill_city_tab ()
      {
        m_citymap = new CityMap ();
        m_citymap->resize ();
        m_citymap_changed = m_citymap->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });
        std::list<guint32> bars;
        std::list<Gdk::RGBA> colors;
        for (unsigned int i = 0; i < MAX_PLAYERS; i++)
          {
            Player *p = Playerlist::instance ()->get (i);
            if (p == NULL ||
                p == Playerlist::instance ()->getNeutral ())
              continue;
            guint32 total = Citylist::instance ()->countCities (p);
            bars.push_back (total);
            Gdk::RGBA color =
              GameMap::getShieldset ()->getColor (p->get_shield ());
            colors.push_back (color);
            if (p == Playerlist::getActiveplayer ())
              m_city_label->set_text
                (String::ucompose
                 (ngettext ("You have %1 city!", "You have %1 cities!",
                            total), total));
          }

        m_city_chart = Gtk::make_managed<BarChart> (bars, colors, 0);
        m_city_chart->set_vexpand (true);
        m_city_alignment->append (*m_city_chart);
      }

    void fill_gold_tab ()
      {
        std::list<guint32> bars;
        std::list<Gdk::RGBA> colors;
        bars.clear ();
        for (unsigned int i = 0; i < MAX_PLAYERS; i++)
          {
            Player *p = Playerlist::instance ()->get (i);
            if (p == NULL ||
                p == Playerlist::instance ()->getNeutral ())
              continue;
            guint32 total = p->getGold ();
            bars.push_back (total);
            Gdk::RGBA color =
              GameMap::getShieldset ()->getColor (p->get_shield ());
            colors.push_back (color);
            if (p == Playerlist::getActiveplayer ())
              m_gold_label->set_text
                (String::ucompose
                 (ngettext ("You have %1 gold piece!",
                            "You have %1 gold pieces!", total), total));
          }
        m_gold_chart = Gtk::make_managed<BarChart> (bars, colors, 0);
        m_gold_chart->set_vexpand (true);
        m_gold_alignment->append (*m_gold_chart);
      }

    void fill_production_tab ()
      {
        m_store = Gio::ListStore<ReportProductionRow>::create ();
        m_selection_model = Gtk::SingleSelection::create (m_store);
        m_treeview->set_model (m_selection_model);

        setup_army_image_column ();
        setup_description_column ();

        City *c = Playerlist::getActiveplayer ()->getFirstCity ();
        m_vectormap = new VectorMap (c, VectorMap::SHOW_ALL_VECTORING, false);
        m_vectormap->resize ();
        m_vectormap_changed = m_vectormap->map_changed.connect
          ([this] (Cairo::RefPtr<Cairo::Surface> map)
           {
             cairo_surface_to_drawing_area (map, m_map_drawing_area);
           });

        guint32 total = 0;
        for (auto act : Playerlist::getActiveplayer ()->getReportableActions ())
          {
            if (act->getType () == Action::PRODUCE_UNIT ||
                act->getType () == Action::PRODUCE_VECTORED_UNIT)
              total++;
            add_production_record (act);
          }

        m_selection_model->signal_selection_changed ().connect
          ([this] (guint, guint)
           {
             auto obj = m_selection_model->get_selected_item ();
             auto item = std::dynamic_pointer_cast<ReportProductionRow>(obj);
             City *city = Citylist::instance ()->getById (item->m_city_id);
             if (city)
               m_vectormap->setCity (city);
           });

        Glib::ustring s =
          String::ucompose (ngettext ("You produced %1 army this turn!",
                                      "You produced %1 armies this turn!",
                                      total), total);
        m_production_label->set_text (s);
      }

    void add_production_record (const Action *action)
      {
        auto p = Playerlist::getActiveplayer ();

        int army_type = 0;
        guint32 city_id = 0;
        Glib::ustring s = "";

        switch (action->getType ())
          {
          case Action::PRODUCE_UNIT:
              {
                const Action_Produce *act =
                  dynamic_cast<const Action_Produce*>(action);
                army_type = act->getArmy ()->getTypeId ();
                for (auto cit : *Citylist::instance ())
                  if (cit->getId () == act->getCityId ())
                    {
                      s += cit->getName ();
                      break;
                    }
                if (act->getVectored ())
                  s += "...";
                city_id = act->getCityId ();
              }
            break;

          case Action::PRODUCE_VECTORED_UNIT:
              {
                const Action_ProduceVectored *act =
                  dynamic_cast<const Action_ProduceVectored*>(action);
                army_type = act->getArmy ()->getTypeId ();
                Vector<int> pos = act->getDestination ();
                City *c = GameMap::getCity (pos);
                s += "...";
                if (c)
                  s += c->getName ();
                else
                  s += _("Standard");
                city_id = GameMap::getCity (act->getOrigination ())->getId ();
              }
            break;

          case Action::CITY_DESTITUTE:
              {
                const Action_CityTooPoorToProduce *act =
                  dynamic_cast<const Action_CityTooPoorToProduce*>(action);
                army_type = act->getArmyType ();
                City *c = Citylist::instance ()->getById (act->getCityId ());
                s = String::ucompose (_("%1 stops production!"), c->getName ());
                city_id = act->getCityId ();
              }
            break;

          default:
            break;
          }

        auto im =
          ImageCache::instance ()->getCircledArmyPic
          (p->getArmyset (), army_type, p->get_shield (), NULL, false,
           Shield::NEUTRAL, true, Lw::get_dark ());
        m_store->append (ReportProductionRow::create (city_id, im, s));
      }

    void setup_army_image_column ()
      {
        LwColumn::setup_picture_column<ReportProductionRow>
          (m_treeview, LW_BUTTON_SIZE, LW_BUTTON_SIZE, "army_image", "",
           [] (const auto& row)
           {
             return row->m_image->to_texture ();
           });
      }

    void setup_description_column ()
      {
        LwColumn::setup_text_column<ReportProductionRow>
          (m_treeview, "desc_label", true, Gtk::Justification::LEFT,
           "",
           [] (const auto& row)
           {
             return row->m_desc;
           });
      }

    void fill_winning_tab ()
      {
        std::list<guint32> bars;
        std::list<Gdk::RGBA> colors;
        for (unsigned int i = 0; i < MAX_PLAYERS; i++)
          {
            Player *p = Playerlist::instance ()->get (i);
            if (p == NULL ||
                p == Playerlist::instance ()->getNeutral ())
              continue;
            guint32 score = p->getScore ();
            bars.push_back (score);
            Gdk::RGBA color =
              GameMap::getShieldset ()->getColor (p->get_shield ());
            colors.push_back (color);
          }
        auto p = Playerlist::getActiveplayer ();
        Glib::ustring s =
          String::ucompose (_("You are coming %1"),
                            calculate_rank (bars, p->getScore ()));
        m_winning_label->set_text (s);
        m_winning_chart = new BarChart (bars, colors, 100);
        m_winning_chart->set_vexpand (true);
        m_winning_alignment->append (*m_winning_chart);
      }

    static Glib::ustring calculate_rank (std::list<guint32> scores, guint32 score)
      {
        guint32 rank = 0;
        for (auto sscore : scores)
          {
            if (score < sscore)
              rank++;
          }
        return String::ucompose ("%1", get_rank (rank));
      }
private:
    Gtk::Button *m_close_button;
    Gtk::DrawingArea *m_map_drawing_area;
    Gtk::Notebook *m_report_notebook;
    Gtk::Label *m_army_label;
    Gtk::Label *m_city_label;
    Gtk::Label *m_gold_label;
    Gtk::Label *m_production_label;
    Gtk::Label *m_winning_label;
    Gtk::Box *m_army_alignment;
    Gtk::Box *m_city_alignment;
    Gtk::Box *m_gold_alignment;
    Gtk::Box *m_winning_alignment;
    Gtk::ColumnView *m_treeview;

    BarChart *m_army_chart;
    BarChart *m_city_chart;
    BarChart *m_gold_chart;
    BarChart *m_winning_chart;
    VectorMap* m_vectormap;
    ArmyMap* m_armymap;
    CityMap* m_citymap;

    sigc::connection m_armymap_changed;
    sigc::connection m_citymap_changed;
    sigc::connection m_vectormap_changed;
    sigc::connection m_switch_page;

    Glib::RefPtr<Gtk::SingleSelection> m_selection_model;
    Glib::RefPtr<Gio::ListStore<ReportProductionRow>> m_store;

    static Glib::ustring get_rank (int rank)
      {
        Glib::ustring s = "";
        switch (rank)
          {
          case 0:
            s = _("first");
            break;

          case 1:
            s = _("second");
            break;

          case 2:
            s = _("third");
            break;

          case 3:
            s = _("fourth");
            break;

          case 4:
            s = _("fifth");
            break;

          case 5:
            s = _("sixth");
            break;

          case 6:
            s = _("seventh");
            break;

          case 7:
            s = _("eighth");
            break;

          default:
            s = "unknown";
            break;
          }
        return s;
      }

};
#endif
