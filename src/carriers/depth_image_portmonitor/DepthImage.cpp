/*
 * Copyright (C) 2006-2018 Istituto Italiano di Tecnologia (IIT)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "DepthImage.h"

#include <algorithm>
#include <cstdio>
#include <cmath>

#include <yarp/sig/Image.h>

using namespace yarp::os;
using namespace yarp::sig;


bool DepthImageConverter::create(const yarp::os::Property& options)
{
    min = 0.2;
    max = 10.0;
    inMatrix = nullptr;
    outMatrix = nullptr;
    outImg.setPixelCode(VOCAB_PIXEL_MONO);
    return true;
}

void DepthImageConverter::destroy()
{
}

bool DepthImageConverter::setparam(const yarp::os::Property& params)
{
    return false;
}

bool DepthImageConverter::getparam(yarp::os::Property& params)
{
    return false;
}

bool DepthImageConverter::accept(yarp::os::Things& thing)
{
    Image* img = thing.cast_as<Image>();
    if(img == nullptr) {
        printf("DepthImageConverter: expected type FlexImage but got wrong data type!\n");
        return false;
    }

    if( img->getPixelCode() == VOCAB_PIXEL_MONO_FLOAT)
    {
        return true;
    }

    printf("DepthImageConverter: expected %s, got %s, not doing any conversion!\n", yarp::os::Vocab::decode(VOCAB_PIXEL_MONO_FLOAT).c_str(), yarp::os::Vocab::decode(img->getPixelCode()).c_str() );
    return false;
}

yarp::os::Things& DepthImageConverter::update(yarp::os::Things& thing)
{
    Image* img = thing.cast_as<Image>();
    inMatrix = reinterpret_cast<float **> (img->getRawImage());

    outImg.setPixelCode(VOCAB_PIXEL_MONO);
    outImg.setPixelSize(1);
    outImg.resize(img->width(), img->height());

    outImg.zero();
    float *inPixels = reinterpret_cast<float *> (img->getRawImage());
    unsigned char *pixels = outImg.getRawImage();
    for(size_t h=0; h<img->height(); h++)
    {
        for(size_t w=0; w<img->width(); w++)
        {
            float inVal = inPixels[w + (h * img->width())];
            if (inVal != inVal /* NaN */ || inVal < min || inVal > max) {
                pixels[w + (h * (img->width() ))] = 0;
            } else {
                int val = (int) (255.0 - (inVal * 255.0 / (max - min)));
                if(val >= 255)
                    val = 255;
                if(val <= 0)
                    val = 0;
                pixels[w + (h * (img->width() ))] = (char) val;
            }
        }
    }
    th.setPortWriter(&outImg);
    return th;
}

