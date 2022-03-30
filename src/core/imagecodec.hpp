#pragma once

#include "pixeltypes.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>



namespace alib
{
	class ImageCodec
	{
		virtual bool write(const char* filename, PixelView& pb) = 0;
	};

	// Write out a PPM (binary) 24-bit file
	class PPMBinary : public ImageCodec
	{
	public:
		virtual bool write(const char* filename, PixelView& pb)
		{
			FILE* fp;
			fopen_s(&fp, filename, "wb");

			if (!fp)
				return false;

			// write out the image header
			fprintf(fp, "P6\n%d %d\n255\n", (int)pb.width(), (int)pb.height());

			// write the individual pixel values in binary form
			alib::PixelRGBA pix;
			uint8_t pixdata[3];

			for (size_t row = 0; row < pb.height(); row++)
			{
				for (size_t col = 0; col < pb.width(); col++)
				{
					pix = pb.At<alib::PixelRGBA>(col, row);

					// Assign to array to ensure the order is
					// correct within the file
					pixdata[0] = pix.red();
					pixdata[1] = pix.green();
					pixdata[2] = pix.blue();

					fwrite(pixdata, 3, 1, fp);
				}
			}

			fclose(fp);

			return true;
		}
	};
}
