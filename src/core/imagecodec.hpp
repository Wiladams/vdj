#pragma once

#include "sampler.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

namespace alib
{
	class ImageCodec
	{
		virtual bool write(const char* filename, ISample2D<PixelRGBA>& pb, size_t w, size_t h) = 0;
	};

}
