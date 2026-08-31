// Copyright (C) 2020, 2021 Ben Asselstine
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
#ifndef TAR_FILE_MASKED_IMAGE_H
#define TAR_FILE_MASKED_IMAGE_H

#include <gtkmm.h>
#include "vector.h"
#include "PixMask.h"

class PixMask;
class Tar_Helper;
class TarFile;
class XML_Helper;
class Player;

/**
 * A helper class to handle image files in tar files that have masks.
 *
 * A mask is a second half of an image that contains white with transparency
 * to be overlaid on the main image in the player's color.
 *
 * This encompasses files like army images, selectors, ships, planted
 * standards and more.
 */
class TarFileMaskedImage
{
public:

    enum MaskOrientation
      {
        // The mask is to the right of the image
        HORIZONTAL_MASK = 0,

        // The mask is below the image
        VERTICAL_MASK,
      };

  //! Default Constructor
  /**
   * @param o the orientation of the mask
   * @param d the general shape the backing image must have
   * we get the tar file from the load method later on.
   *
   */
  TarFileMaskedImage (MaskOrientation o, PixMask::DimensionType d);

  //! Copy Constructor
  TarFileMaskedImage (const TarFileMaskedImage &i);

  //! Destructor
  ~TarFileMaskedImage ();

  //! Return the orientation of the mask
  MaskOrientation getMaskOrientation () const {return orientation;}

  //! Return the number of images
  guint32 getNumberOfFrames () const {return frames.size ();}

  //! Return the basename of the image (archive member in tar file)
  Glib::ustring getName () const {return name;}

  //! Return the number of masks in the image file
  guint32 getNumMasks () const {return maskcount;}

  //! Get the whole image as loaded from the file
  PixMask *getBackingImage () {return image;}

  //! Return an image by index
  PixMask *getImage (guint32 i) const {return i < frames.size () ? frames[i][0] : NULL;}

  //! Return the first image
  PixMask *getImage () const {return frames.empty () ? NULL :frames[0][0];}

  //! Apply the mask onto the image in the player's color
  /**
   * @return a pointer to a new PixMask that must be deleted.
   */
  PixMask *applyMask (Player *p) const;
  PixMask *applyMask (std::vector<Gdk::RGBA> colors) const;

  //! Apply the mask onto the image at the given index in the player's color
  /**
   * @return a pointer to a new PixMask that must be deleted.
   */
  PixMask *applyMask (guint32 i, Player *p) const;
  PixMask *applyMask (guint32 i, std::vector<Gdk::RGBA> colors) const;

  //! Return all of the images
  std::vector<PixMask*> getImages () const
    { std::vector<PixMask*> o; for (auto f : frames) o.push_back (f.front ()); return o; }

  //! Return the dimensions that the images are scaled to
  Vector<int> getScaledImageDimensions () const {return scale_dimension;}

  //! Return the dimensions of the images in the backing image
  Vector<int> getImageDimensions () const  {return dimension;}

  //! Does the image in file F have the correct dimensions for this?
  bool checkDimension (Glib::ustring f);

  //! Set the basename of the image (archive member in the tar file)
  void setImageName (Glib::ustring n) {name = n;}

  //! Set the name of the archive member in the tar file that holds the image
  void setName (Glib::ustring n) {name = n;}

  //! Set the number of masks in the image file
  void setNumMasks (guint32 n) {maskcount = n;}

  //! Try to calculate the number of masks, return false if we can't.
  bool calculateNumberOfMasks (Glib::ustring file, bool &bad_dimension);

  //! Set the opened tar file
  void setTarFile (Tar_Helper *t) {tarfile = t;}

  //! Read the data tag from an opened xml file and put it in our name member
  void load (XML_Helper *helper, Glib::ustring name_tag,
             Glib::ustring mask_tag);

  //! Load an image from a tar file, with bname already provided
  /**
   * @param          t the unopened tar file
   * @param bname    the archive member containing the image
   * @return         true if something went wrong
   */
  bool load (TarFile *t, Glib::ustring bname);

  //! Load an image from the tar file, with bname already set by setName
  /**
   * @return         true if something went wrong
   */
  bool load ();

  //! Load an image from the tar file t, already provided bname
  /**
   * @param t        the opened tar file
   * @return         true if something went wrong
   */
  bool load (Tar_Helper *t);

  //! Load an image named bname from the tar file t
  /**
   * @param t        the opened tar file
   * @param bname    the archive member containing the image
   * @return         true if something went wrong
   */
  bool load (Tar_Helper *t, Glib::ustring bname);

  //! Load an image named bname from the tar file
  /**
   * @param          bname the archive member containing the image
   * @return         true if something went wrong
   */
  bool load (Glib::ustring bname);

  //! Load an image named filename from disk
  /**
   * @param filename the file holding the image
   * @return         true if something went wrong
   */
  bool loadFromFile (Glib::ustring filename);

  //! write the name and maskcount elements to an opened xml file
  bool save (XML_Helper *helper, Glib::ustring name_tag,
             Glib::ustring mask_tag);

  //! Process the backing image into a set of images and masks
  void instantiateImages (Vector<int> scale_to_dimension = Vector<int>(-1,-1));

  //! Destroy the images
  void uninstantiateImages ();

  //! Destroy the images and optionally clear the basename
  void clear (bool clear_name = true);

  //! Destroy just the backing image
  void dropBackingImage ();

  //! copy this to DEST in T
  bool copy (TarFile *t, TarFileMaskedImage *dest);

  void copyFrames (TarFileMaskedImage *dest);

  //! uninstantiate all images named NAME in IMAGES
  static void uninstantiate (Glib::ustring name, std::vector<TarFileMaskedImage*> images);
private:

  //! The orientation of the masked image
  MaskOrientation orientation;

  //! The opened tar file
  Tar_Helper *tarfile;

  //! The basename of the archive member holding the image
  Glib::ustring name;

  //! When we extract the file from the tar file, this is where it is
  Glib::ustring file_on_disk;

  //! What we want to scale the images to
  Vector<int> scale_dimension;

  //! The original dimensions of the images in the backing image
  Vector<int> dimension;

  //! The general shape that the backing images must be
  PixMask::DimensionType dimension_type;

  //! The backing image, the whole image as loaded from file_on_disk.
  PixMask *image;

  //! how many frames there are (columns).
  guint32 calculated_number_of_frames;

  //! how many masks the image has
  guint32 maskcount;

  //! The parts of the backing image cut up into images and masks
  /**
   * the first elemtn of the list is the image, and the rest are the masks.
   */
  std::vector<std::vector<PixMask*> > frames;

  //! Process a horizontally masked image
  void instantiateVertical  ();

  //! Process a vertically masked image
  void instantiateHorizontal ();

  //! Overlay the masks on the image in the given color.
  /**
   * @param frame  the image and its associated masks
   * @return a new pixmask that must be deleted.
   */
  PixMask* applyMask(std::vector<PixMask*> frame, std::vector<Gdk::RGBA> colors) const;

  //! Dice up image into a set of pixmasks.  the inner array is a column.
  std::vector<std::vector<PixMask*> > disassemble_grid (int rows, int cols);

  //! Chop up the image into COLS pieces of a single row
  std::vector<PixMask*> disassemble_row(int cols);
};

#endif
