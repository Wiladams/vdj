#pragma once

#include "imagecodec.hpp"

namespace alib
{
	// Write out a PPM (binary) 24-bit file
	class PPMBinary : public ImageCodec
	{
	public:
		virtual bool write(const char* filename, ISample2D<PixelRGBA>& pb, size_t awidth, size_t aheight)
		{
			FILE* fp;
			fopen_s(&fp, filename, "wb");

			if (!fp)
				return false;

			// write out the image header
			fprintf(fp, "P6\n%d %d\n255\n", (int)awidth, (int)aheight);

			// write the individual pixel values in binary form
			alib::PixelRGBA pix;
			uint8_t pixdata[3];

			for (size_t row = 0; row < aheight; row++)
			{
				double v = alib::Map(row, 0, aheight - 1, 0.0, 1.0);

				for (size_t col = 0; col < awidth; col++)
				{
					double u = alib::Map(col, 0, awidth - 1, 0.0, 1.0);
					auto pix = pb.getValue(u, v);
					
					// Assign to array to ensure the order is
					// correct within the file
					pixdata[0] = pix.r;
					pixdata[1] = pix.g;
					pixdata[2] = pix.b;

					fwrite(pixdata, 3, 1, fp);
				}
			}

			fclose(fp);

			return true;
		}
	};
}
