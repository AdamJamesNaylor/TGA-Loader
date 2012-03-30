#include <memory.h>

#include "TGAImage.h"

namespace Rise
{
	TGAImage::TGAImage()
	{ 
		_image = _palette = NULL;
		_header.Width = _header.Height = _header.BPP = 0;
		_size = 0;
	}

 
	TGAImage::~TGAImage()
	{
		if (_image)
		{
			delete[] _image;
			_image = NULL;
		}
 
		if (_palette)
		{
			delete[] _palette;
			_palette = NULL;
		}
	}
 
 
	LoadResult TGAImage::Load(byte *data, unsigned long size)
	{
		LoadResult result;

		// Clear out any existing image and palette
		if (_image)
		{
			delete[] _image;
			_image = NULL;
		}
 
		if (_palette)
		{
			delete[] _palette;
			_palette = NULL;
		}
  
		// Process the header
		result = FillHeader(data);
 
		if (result != RISE_OK)
			return result;
 
		switch (_header.ImageType)
		{
			case RGB:
			{
				// Check filesize against header values
				if ((_size + TGAHeader::SIZE + _header.IdentSize) > size)
					return RISE_ERR_BAD_FORMAT;
  
				// Load image data
				result = LoadRawData(data);
				if (result != RISE_OK)
					return result;
 
				BGRtoRGB(); // Convert to RGB
				break;
			}
			case Grayscale:
			{
				break;
			}
			case RLE_RGB:
			{
				// Load image data
				result = LoadTgaRLEData(data);
				if(result != RISE_OK)
					return result;
 
				BGRtoRGB(); // Convert to RGB
				break;
			}
 
			default:
			return RISE_ERR_UNSUPPORTED;
		}
 
		// Check flip bit
		if ((data[17] & 0x10)) 
			FlipImg();
 
		return RISE_OK;
	}

	LoadResult TGAImage::FillHeader(byte *data) // Examine the header and populate our class attributes
	{
		short x1, y1, x2, y2;
 
		_header.IdentSize = data[0];

		//double check the TARGA type is supported
		if (data[1] != 0)    // 0 (RGB) and 1 (Indexed) are the only types we know about
			return RISE_ERR_UNSUPPORTED;
 
		if (data[2] == 2) _header.ImageType = RGB;
		else if	(data[2] == 3) _header.ImageType = Grayscale;
		else if	(data[2] == 10) _header.ImageType = RLE_RGB;
		else return RISE_ERR_UNSUPPORTED;

		// Get image window and produce width & height values
		memcpy(&x1, &data[8],2);
		memcpy(&y1, &data[10],2);
		memcpy(&x2, &data[12],2);
		memcpy(&y2, &data[14],2);
 
		_header.Width = x2 - x1;
		_header.Height = y2 - y1;
 
		if (_header.Width < 1 || _header.Height < 1)
			return RISE_ERR_BAD_FORMAT;
 
		_header.BPP = data[16];
 
		// Check flip / interleave byte
		if (data[17] > 32) // Interleaved data
			return RISE_ERR_BAD_FORMAT;
 
		// Calculate image size
		_size = (_header.Width * _header.Height * (_header.BPP / 8));
 
		return RISE_OK;
	}

	LoadResult TGAImage::LoadRawData(byte *data) // Load uncompressed image data
	{
		short iOffset;
 
		if (_image) // Clear old data if present
			delete[] _image;
 
		_image = new byte[_size];
 
		if (_image == NULL)
			return RISE_ERR_MEM_FAIL;
 
		iOffset = _header.IdentSize + TGAHeader::SIZE; // Add header to ident field size
 
		memcpy(_image, &data[iOffset], _size);
 
		return RISE_OK;
	}
 
	LoadResult TGAImage::LoadTgaRLEData(byte *data) // Load RLE compressed image data
	{
		short iOffset, pixelSize;
		byte *pCur;
		unsigned long pixel = 0;
		byte bLength, bLoop;
 
		// Calculate offset to image data
		iOffset = data[0] + TGAHeader::SIZE;
 
		// Get pixel size in bytes
		pixelSize = _header.BPP / 8;
 
		// Set our pointer to the beginning of the image data
		pCur = &data[iOffset];
 
		// Allocate space for the image data
		if (_image != NULL)
			delete[] _image;
 
		_image = new byte[_size];
 
		if (_image == NULL)
			return RISE_ERR_MEM_FAIL;
 
		// Decode
		while (pixel < _size) 
		{
			if (*pCur & 0x80) // Run length chunk (High bit = 1)
			{
				bLength = *pCur - 127;	// Get run length
				pCur++;					// Move to pixel data  
 
				// Repeat the next pixel bLength times
				for (bLoop = 0; bLoop != bLength; ++bLoop, pixel += pixelSize)
					memcpy(&_image[pixel], pCur, pixelSize);
 
				pCur += pixelSize; // Move to the next descriptor chunk
			}
			else // Raw chunk (*pCur & 0x80) == 0
			{
				bLength=*pCur+1; // Get run length
				pCur++;          // Move to pixel data
 
				// Write the next bLength pixels directly
				for (bLoop = 0; bLoop != bLength; ++bLoop, pixel += pixelSize)
					memcpy(&_image[pixel], pCur, pixelSize);
			}
		}
 
		return RISE_OK;
	}

	void TGAImage::BGRtoRGB() // Convert BGR to RGB (or back again)
	{
		unsigned long Index, pixelCount;
		byte *grinder;
		byte temp;
		short pixelSize;
 
		// Set ptr to start of image
		grinder = _image;
 
		// Calc number of pixels
		pixelCount = _header.Width * _header.Height;
 
		// Get pixel size in bytes
		pixelSize = _header.BPP / 8;
 
		for (Index = 0; Index != pixelCount; ++Index)  // For each pixel
		{
			temp=*grinder;				// Remember blue val
			*grinder=*(grinder + 2);	// Swap red value into first position
			*(grinder + 2) = temp;		// Write back blue to last position
 
			grinder += pixelSize;		// Jump to next pixel
		}
 
	}

	void TGAImage::FlipImg() // Flips the image vertically (Why store images upside down?)
	{
		byte temp;
		byte *pLine1, *pLine2;
		int iLineLen, iIndex;
 
		iLineLen = _header.Width * (_header.BPP / 8);
		pLine1 = _image;
		pLine2 = &_image[iLineLen * (_header.Height - 1)];

		for (; pLine1 < pLine2; pLine2 -= (iLineLen * 2))
		{
			for(iIndex = 0; iIndex != iLineLen; ++pLine1, ++pLine2, ++iIndex)
			{
				temp = *pLine1;
				*pLine1 = *pLine2;
				*pLine2 = temp;       
			}
		} 
 
	}
}
