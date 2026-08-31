// Copyright (C) 2002, 2003 Michael Bartl
// Copyright (C) 2002, 2003, 2004, 2005, 2006 Ulf Lorenz
// Copyright (C) 2003, 2004, 2005 Andrea Paternesi
// Copyright (C) 2011, 2012, 2014, 2021 Ben Asselstine
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

/** This class is intended to make xml handling easier. It offers
  * save and load functions. You can initialise it either with a filename or
  * a stream (to make dumping data also possible)
  *
  * Saving works like this:
  * Open a new tag with openTag(), save your data with saveData() and close the
  * tag with closeTag(). The output will look like:
  *
  * <map>
  *     <maptile>
  *         <d_x> 50 </d_x>
  *         <d_y> 10 </d_y>
  *         <d_type> 5 </d_type>
  *     </maptile>
  *     ...
  * </map>
  *
  * map and maptile are opened tags, the tags x, y and type are saved data.
  * ATTENTION: Always save data before you start opening up subtags!!!
  * i.e. if you want to save some data concerning the tag map, save it _before_
  * you open the first subtag (here maptile).
  *
  * The save code in LordsAWar follows a top-down hierarchy. As an example,
  * The player's stacklist saves its data first, then calls
  * the save functions of all the stacks. The stacks themselves call the save
  * functions of their armies when they have stored their own data. The topmost
  * object in the hierarchy is the GameScenario instance.
  * 
  * Loading:
  * First, you supply xml_helper with callback functions via registerTag.
  *
  * When you are ready, call parse. The parser will start until it finds a special
  * tag (not starting with d_). It will then parse the next tags, assuming that
  * these are data tags, until it finds the next special tag. When this is
  * reached, it stops and calls the callback for the former special tag. 
  * Sometimes (e.g. in the case of a ruin being created and a stack tag) this
  * callback will set up the callback for the next tag and demand some data.
  * This data has already been stored and will be supplied on a request of
  * getData(). That's all.
  *
  * Loading is also implemented in a hierarchical way. In the example above, the
  * stacklist constructor is initialised with the XML_Helper instance. Then it
  * registers an internal stacklist function as callback for the "stack" tags.
  * This function is started when the first stack tag is encountered and creates
  * a new Stack instance with the XML_Helper instance ...
  *
  * If this explanation was confusing, have a look at the loading and saving
  * functions. They should make the point somewhat clearer.
  */

#pragma once
#ifndef XML_HELPER_H
#define XML_HELPER_H

#include <gtkmm.h>
#include <string>
#include <iosfwd>
#include <fstream>
#include <list>
#include <map>
#include <sigc++/slot.h>
#include <libxml++/libxml++.h>
    
class XML_Helper;

typedef sigc::slot<bool, Glib::ustring, XML_Helper*> XML_Slot;

//! XML handling class.
class XML_Helper: public xmlpp::SaxParser
{
    public:

        static Glib::ustring xml_entity; // <?xml version=\"1.0\"?>

        /** The most common constructor reads or writes to a file
          * 
          * @param filename     the filename where data read from/written to
          * @param openmode     either std::ios::in for reading or out for writing
          */ 
        XML_Helper(Glib::ustring filename, std::ios::openmode mode);

        /** This constructor reads from a given input stream.
          * 
          * @param input        the input stream to read from
          */
        XML_Helper(std::istream* input);

        /** This constructor writes to a given output stream.
          * 
          * @param output       the output stream to write to
          */
        XML_Helper(std::ostream* output);
        ~XML_Helper();

        /** Call this at the very beginning of the saving procedure. It
          * initializes some items.
          *
          * @param version          the version number for the savegame
          * @return true on success, false on error
          */
        bool begin(Glib::ustring version);

        /** Opens a new subtag
          * 
          * @param name             the name of the subtag
          * @return true on success, false otherwise
          */
        bool openTag(Glib::ustring name);

        //! Closes the most recently opened tag
        bool closeTag();
        
        /** Save data
          * 
          * There exist various save functions for different types of data. The
          * data is enclosed in tags which are automatically prepended a d_ to
          * distinguish them from the subdividing tags.
          * 
          * @param identifier           the name for the data tag
          * @param value                the data
          * @return true on success, false otherwise
          */
        bool saveData(Glib::ustring identifier, const Glib::ustring value);
        bool saveData(Glib::ustring identifier, const int value);
        bool saveData(Glib::ustring identifier, const guint32 value);
        bool saveData(Glib::ustring identifier, const bool value);
        bool saveData(Glib::ustring identifier, const double value);
        /* amd64 fix, UL: still neccessary?*/
        bool saveData(Glib::ustring identifier, unsigned long int value);
	bool saveData(Glib::ustring identifier, const std::vector<Gdk::RGBA> value);
	bool saveData(Glib::ustring identifier, const Gdk::RGBA value);

        /** Closes the reading/writing stream.
          * @note As soon as you call this function, the object is dead with
          * all streams cut. It is just here to force saving of files as the
          * streams are also closed in the destructor.
          */
        bool close();


        /** Registers a new tag handler
          * 
          * Use this function to register new callbacks which are called when
          * certain tags are encountered.
          *
          * @param tag          the name of the tag associated with the callback
          * @param callback     a pointer to the callback function
          * @return true on success, false otherwise
          */
        bool registerTag(Glib::ustring tag, XML_Slot callback);

        /** Provides cached data
          * 
          * Use this function to get back the data. There are several functions
          * for several types of data.
          *
          * @param data         a reference where the data is returned
          * @param name         the name of the data tag
          * @return true false if the data was not found or of wrong type
          *
          * @note For string data you can also specify if the data should be
          * translated ro not.
          */
        bool getData(Glib::ustring& data, Glib::ustring name);
        bool getData(bool& data, Glib::ustring name);
        bool getData(int& data, Glib::ustring name);
        bool getData(guint32& data, Glib::ustring name);
        bool getData(double& data, Glib::ustring name);
	bool getData(std::vector<Gdk::RGBA> & data, Glib::ustring name);
	bool getData(Gdk::RGBA & data, Glib::ustring name);

        //! Returns the version number of the save file
        Glib::ustring getVersion() const {return d_version;}
        
        //! Use this function to start reading a file or stream
        bool parseXML();

        static Glib::ustring get_top_tag(Glib::ustring filename);
        static bool rewrite_version(Glib::ustring filename, Glib::ustring tag, Glib::ustring new_version);
        static guint32 flagsFromString(Glib::ustring flags, guint32 (*flagStrToNum)(Glib::ustring));
        

    protected:
        virtual void on_start_element(const Glib::ustring& name,
                                                                      const AttributeList& properties);
        virtual void on_end_element(const Glib::ustring& name);
        virtual void on_characters(const Glib::ustring& characters);
    private:
        /** Prepends a number of tags (depending on the number of opened tags)
          * to a line. Used for beautification.
          */
        inline void addTabs();

	bool lang_check(Glib::ustring lang);

        bool tag_open(Glib::ustring tag, Glib::ustring version, Glib::ustring lang);

        bool tag_close(Glib::ustring tag, Glib::ustring cdata = "");

        //streams, d_fout and d_fin are used when it comes to file
        //handling, d_in and d_out are used when actually reading or
        //writing data(either point to d_fout or d_fin or have a
        //separate stream)
        std::istringstream* d_inbuf;
        std::ostringstream* d_outbuf;

        std::ofstream* d_fout;    
        std::ifstream* d_fin;
        std::ostream* d_out;
        std::istream* d_in;

        std::list<Glib::ustring> d_tags;

        std::map<Glib::ustring, XML_Slot> d_callbacks;
        std::map<Glib::ustring, Glib::ustring> d_data;
        std::map<Glib::ustring, Glib::ustring> d_lang;
        
        Glib::ustring d_last_opened;
        Glib::ustring d_version;

        bool d_failed;
        Glib::ustring my_cdata;
        bool error;
};

//! A helper class for grabbing the version of a game file.
class VersionLoader 
{
public:
    VersionLoader(Glib::ustring filename, Glib::ustring tag, Glib::ustring &version, bool &broken)
      {
        std::ifstream in(filename.c_str());
        if (in)
          {
            d_tag = tag;
            XML_Helper helper(filename.c_str(), std::ios::in);
            helper.registerTag(tag, sigc::mem_fun(*this, &VersionLoader::load));
            bool retval = helper.parseXML();
            if (!retval)
              broken = true;
            version = d_version;
          }
      }
    bool load(Glib::ustring tag, XML_Helper* helper)
      {
        if (tag == d_tag)
          {
            d_version = helper->getVersion();
            return true;
          }
        return false;
      }

    Glib::ustring d_tag;
    Glib::ustring d_version;

};

#endif //XML_HELPER_H
