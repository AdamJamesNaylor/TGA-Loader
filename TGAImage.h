//*******************************************************************************
//
// /Framework/IO/Console.h
//
// Copyright 2011 COPYRIGHT_HOLDER
// 
// LICENCE
//
// Author: Adam J. Naylor
//
//*******************************************************************************
 
 
#include "../../Library/DataTypes.h" //just for byte etc...

namespace Rise
{
	//--------------------------------------------------------------------------------
	///
	/// Currently used by TGAImage::Load(), I will probably refactor this away.
	///
	//--------------------------------------------------------------------------------
	enum LoadResult
	{
		RISE_OK,
		RISE_ERR_NO_FILE,
		RISE_ERR_MEM_FAIL,
		RISE_ERR_BAD_FORMAT,
		RISE_ERR_UNSUPPORTED
	};

	//--------------------------------------------------------------------------------
	///
	/// The types of TARGA images supported by the FrameWork
	/// http://en.wikipedia.org/wiki/Truevision_TGA#Header
	///
	//--------------------------------------------------------------------------------
	enum TGAImageType
	{
		NoImage			= 0,	//no image data is present
		//UNSUPPORTED	= 1,	//uncompressed, color-mapped image
		RGB				= 2,	//uncompressed, true-color image
		Grayscale		= 3,	//uncompressed, black-and-white image
		//UNSUPPORTED	= 9,	//run-length encoded, color-mapped Image
		RLE_RGB			= 10	//run-length encoded, true-color image and
		//UNSUPPORTED	= 11	//run-length encoded, black-and-white Image
	};

	/// The structure used to pull the header data from a TGA file.
	/// http://en.wikipedia.org/wiki/Truevision_TGA#Header
	struct TGAHeader
	{
		static const int	SIZE = 18;			//TGA headers are 18 bytes long

		byte				IdentSize;			//The size of the Identity section, used to offset to the image data.
		bool				HasColourMap;		//If this file uses a colour map.
		TGAImageType		ImageType;
		unsigned short		ColourMapOrigin;	//First entry index (2 bytes): offset into the color map table
		unsigned short		ColourMapLength;	//Color map length (2 bytes): number of entries
		byte				ColourMapEntrySize; //Color map entry size (1 byte): number of bits per pixel (16,24,32)
		unsigned short		XOrigin, YOrigin;	//absolute coordinate of lower-left corner for displays where origin is at the lower left
		unsigned short		Width, Height;		//in pixels
		byte				BPP;				//bits per pixel, size of a pixel in bits
		byte				Attributes;			//bits 3-0 give the alpha channel depth, bits 5-4 give direction
	};

	//--------------------------------------------------------------------------------
	///
	/// Class used for loading and storing TGA image files.
	/// http://gpwiki.org/index.php/LoadTGACpp
	///
	//--------------------------------------------------------------------------------
	class TGAImage
	{
	public:
	/// Constructors/Destructors ///

							TGAImage();

							~TGAImage();

	/// Constructors/Destructors ///

		//--------------------------------------------------------------------------------
		///
		/// Loads the image contained within the supplied pointer.
		///
		/// @param data
		///  The memory containing the image data. 
		/// @param size
		///  How big the file is.
		/// @return
		///   TODO this needs to change.
		///
		//--------------------------------------------------------------------------------
		LoadResult			Load(byte *data, unsigned long size);

	/// Properties ///

		int					Get_BPP()		{ return _header.BPP; }
		int					Get_Width()		{ return _header.Width; }
		int					Get_Height()	{ return _header.Height; }
		byte *				Get_Image()		{ return _image; }
		byte *				Get_Palette()	{ return _palette; }
 
	private:
		LoadResult			FillHeader(byte *data);
		LoadResult			LoadRawData(byte *data);
		LoadResult			LoadTgaRLEData(byte *data);
		void				BGRtoRGB();
		void				FlipImg();

		TGAHeader			_header;
		unsigned long		_size;
		byte *				_image;
		byte *				_palette;
	};
}

