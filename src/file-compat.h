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

#pragma once
#ifndef FILE_COMPAT_H
#define FILE_COMPAT_H

#include <gtkmm.h>
#include <list>
#include <sigc++/trackable.h>

class UpgradeDetails;

//! A helper class for upgrading the schema of files.
class FileDetails
{
public:
  FileDetails(guint32 k, Glib::ustring f, Glib::ustring t, bool ta) 
    : type (k), file_extension (f), tag (t), tar (ta) {}
  guint32 type;
  Glib::ustring file_extension;
  Glib::ustring tag;
  bool tar;
};

//! An interface to provide backwards compatibility for files.
class FileCompat: public std::list<FileDetails>, public sigc::trackable
{
    public:

        enum Type {
          UNKNOWN = 0,
          CONFIGURATION,
          ITEMLIST,
          PROFILELIST,
          RECENTLYPLAYEDGAMELIST,
          GAMELIST,
          ARMYSET,
          TILESET,
          CITYSET,
          SHIELDSET,
          GAMESCENARIO
        };

        //! upgrade common files.
        void initialize();

        typedef sigc::slot<bool, Glib::ustring, Glib::ustring, Glib::ustring> Slot;
        void support_type (guint32 k, Glib::ustring f, Glib::ustring t, bool ta) 
          {push_back(FileDetails(k,f,t,ta));};
        void support_version(guint32 k, Glib::ustring from, Glib::ustring to, FileCompat::Slot slot);

        bool contains(FileCompat::Type type) const;
        FileCompat::Type getType(Glib::ustring filename) const;
        FileCompat::Type getTypeByFileInspection(Glib::ustring filename, bool &tar) const;

        bool isTarFile(FileCompat::Type type) const;

        Glib::ustring getTag(FileCompat::Type type) const;
        FileCompat::Type getTypeByFileExtension(Glib::ustring ext) const;
        Glib::ustring getFileExtension(FileCompat::Type type) const;
        std::list<Glib::ustring> getFileExtensions(FileCompat::Type type) const;
        bool get_tag_and_version_from_file(Glib::ustring filename, FileCompat::Type type, Glib::ustring &tag, Glib::ustring &version) const;

        bool upgrade(Glib::ustring filename, bool &same) const;
        bool upgrade(Glib::ustring filename, Glib::ustring old_version, Glib::ustring new_version, FileCompat::Type t, Glib::ustring tag) const;
        bool rewrite_with_updated_version(Glib::ustring filename, FileCompat::Type type, Glib::ustring tag, Glib::ustring version) const;
        bool upgradeGameScenario(Glib::ustring filename, Glib::ustring version, bool& upgraded_armyset, bool& upgraded_tileset, bool& upgraded_cityset, bool& upgraded_shieldset) const;

	// Static Methods
        static void support_backward_compatibility_for_common_files();
        static Glib::ustring typeToString(const FileCompat::Type type);
        static Glib::ustring typeToCode(const FileCompat::Type type);

        //! return the singleton instance of this class.
        static FileCompat * getInstance();

        //! Explicitly delete the singleton instance of this class.
        static void deleteInstance();

    protected:
        //! Default Constructor.
        FileCompat();

        //! Destructor.
        ~FileCompat() {};

        // helpers
        FileCompat::Type getTypeByXmlFileInspection(Glib::ustring filename) const;
        FileCompat::Type getTypeByTarFileInspection(Glib::ustring filename) const;

        bool upgradeGameScenarioWithXslt(Glib::ustring filename, Glib::ustring xsl_file, bool& armyset_upgraded, bool& tileset_upgraded, bool &cityset_upgraded, bool &shieldset_upgraded) const;

        bool can_upgrade_to(FileCompat::Type type, Glib::ustring version) const;
        bool get_upgrade_method(FileCompat::Type type, Glib::ustring version, Glib::ustring &next_version, FileCompat::Slot &slot) const;
        bool xsl_transform(Glib::ustring filename, Glib::ustring xsl_file) const;
        bool rewrite_with_xslt(Glib::ustring filename, FileCompat::Type type, Glib::ustring xsl_file) const;

    private:
	// DATA
        std::list<UpgradeDetails> versions[GAMESCENARIO + 1];


        //! A static pointer for the singleton instance.
        static FileCompat* s_instance;
};

//! A helper class for providing backwards compatibility in game files.
class UpgradeDetails
{
public:
  UpgradeDetails(Glib::ustring f, Glib::ustring t, FileCompat::Slot s) 
    : from_version (f), to_version (t), slot (s) {}
  Glib::ustring from_version;
  Glib::ustring to_version;
  FileCompat::Slot slot;
};


#endif // FILE_COMPAT_H

