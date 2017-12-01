// ======================================================================== //
// Copyright 2009-2017 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "transferFunction/Linear2DTransferFunction.ih"

inline varying vec3f
LTFN2D_getColorForCoordinate(const void *uniform _self,
			     varying vec3f coordinate)
{

  // Return (0,0,0) for NaN values.
  if (isnan(coordinate)) { return make_vec3f(0.0f); }

  return make_vec3f(0.0f);

  // Cast to the actual TransferFunction subtype.
  const uniform Linear2DTransferFunction *uniform self
    = (const uniform Linear2DTransferFunction *uniform) _self;
  
  // No color values may be available.
  if (self->colorValueCount == 0) {  return make_vec3f(1.0f); }

  // Sample value
  varying float value = self->volume->sample(self->volume, coordinate);
  varying float grad  = self->volume->computeGradient(self->volume, coordinate);

  // Clamp the value to the lower bound of the value range.
  if (value <= self->super.valueRange.x)
    return self->colorValues[0];

  // Clamp the value to the upper bound of the value range.
  if (value >= self->super.valueRange.y)
    return self->colorValues[self->colorValueCount - 1];

  // // Map the value into the range [0.0, 1.0].
  // value
  //   = (value - self->super.valueRange.x)
  //   / (self->super.valueRange.y - self->super.valueRange.x)
  //   * (self->colorValueCount - 1.0f);

  // // Compute the color index and fractional offset.
  // int   index     = floor(value);
  // float remainder = value - index;

  // // The interpolated color.
  // return
  //   ((1.0f - remainder) * self->colorValues[index]
  //    + remainder        * self->colorValues[min(index + 1, self->colorValueCount - 1)]);
}

inline varying float
Linear2DTransferFunction_getOpacityForValue(const void *uniform _self,
					    varying float value)
{
  // Return 0 for NaN values.
  if (isnan(value)) return 0.0f;

  // Cast to the actual TransferFunction subtype.
  const Linear2DTransferFunction *uniform self
    = (const Linear2DTransferFunction *uniform) _self;

  // No opacity values may be available.
  if (self->opacityValueCount == 0)
    return 1.0f;

  // Clamp the value to the lower bound of the value range.
  if (value <= self->super.valueRange.x)
    return self->opacityValues[0];

  // Clamp the value to the upper bound of the value range.
  if (value >= self->super.valueRange.y)
    return self->opacityValues[self->opacityValueCount - 1];

  // Map the value into the range [0.0, numValues).
  const float remapped
    = (value - self->super.valueRange.x)
    / (self->super.valueRange.y - self->super.valueRange.x)
    * (self->opacityValueCount - 1.0f);

  // Compute the opacity index and fractional offset.
  int index = floor(remapped);
  float remainder = remapped - index;

  // The interpolated opacity.
  float ret =
    (1.0f - remainder) * self->opacityValues[index]
    + remainder * self->opacityValues[min(index + 1, self->opacityValueCount - 1)];
  return ret;
}

export void *uniform LTFN2D_createInstance()
{
  // The transfer function.
  Linear2DTransferFunction *uniform self = 
    uniform new uniform Linear2DTransferFunction;

  // Function to get the interpolated color for a given value.
  self->super.getColorForCoordinate = LTFN2D_getColorForCoordinate;

  // Function to get the interpolated opacity for a given value.
  self->super.getOpacityForValue = Linear2DTransferFunction_getOpacityForValue;

  // Transfer function colors and count.
  self->colorValues = NULL;
  self->colorValueCount = 0;
  self->colorPITable = NULL;

  // Transfer function opacity values and count.
  self->opacityValues = NULL;
  self->opacityValueCount = 0;
  self->opacityPITable = NULL;

  // The default transfer function value range.
  self->super.valueRange = make_vec2f(0.0f, 1.0f);

  // Freshly baked transfer function.
  return self;
}

void Linear2DTransferFunction_precomputePreIntegratedOpacityValues(void* uniform _self)
{
  return;
}

void Linear2DTransferFunction_precomputePreIntegratedColorValues(void* uniform _self)
{
  return;
}

export void LTFN2D_setColorValues(void *uniform _self,
				  const uniform size_t &count,
				  const uniform size_t &width,
				  const uniform size_t &height,
				  vec3f *uniform source)
{
  // Cast to the actual TransferFunction subtype.
  uniform Linear2DTransferFunction *uniform self
    = (uniform Linear2DTransferFunction *uniform) _self;

  // Free memory for any existing color values.
  if (self->colorValues != NULL)
    delete[] self->colorValues;

  // Allocate memory for the incoming color values.
  self->colorValueCount = count;
  self->colorValueW = width;
  self->colorValueH = height;
  self->colorValues = uniform new uniform vec3f[count];

  // Copy the color values into the transfer function.
  foreach_tiled (i = 0 ... count) {
    self->colorValues[i] = source[i];
  }

  self->super.preIntegrationComputed = false;
}

export void LTFN2D_setOpacityValues(void *uniform _self,
				    const uniform size_t &count,
				    const uniform size_t &width,
				    const uniform size_t &height,
				    float *uniform source)
{
  // Cast to the actual TransferFunction subtype.
  uniform Linear2DTransferFunction *uniform self
    = (uniform Linear2DTransferFunction *uniform) _self;

  self->opacityValues = source;
  self->opacityValueCount = count;
  self->opacityValueW = width;
  self->opacityValueH = height;

  self->super.preIntegrationComputed = false;
}

export void LTFN2D_setPreIntegration(void *uniform _self,
				    const uniform bool& value)
{
  // Cast to the actual TransferFunction subtype.
  Linear2DTransferFunction *uniform self
    = (Linear2DTransferFunction *uniform) _self;
  self->super.preIntegration = value;
}

export void LTFN2D_precomputePreIntegratedValues(void *uniform _self)
{
    // Cast to the actual TransferFunction subtype.
  Linear2DTransferFunction *uniform self
    = (Linear2DTransferFunction *uniform) _self;
  Linear2DTransferFunction_precomputePreIntegratedColorValues(_self);
  Linear2DTransferFunction_precomputePreIntegratedOpacityValues(_self);
  self->super.preIntegrationComputed = true;
}

export void LTFN2D_setVolume(void *uniform _self, 
			     void* uniform volume,
			     const uniform float step) 
{
  // Cast to the actual TransferFunction subtype.
  uniform Linear2DTransferFunction *uniform self
    = (uniform Linear2DTransferFunction *uniform) _self;

  // Set the transfer function value range.
  self->volume = (uniform Volume* uniform)volume;
  self->gradStep = step;
}

export void LTFN2D_setQueryByCoordinate(void *uniform _self, 
					const uniform int flag) 
{
  // Cast to the actual type.
  uniform TransferFunction *uniform self = 
    (uniform TransferFunction *uniform) _self;

  // Set the transfer function value range.
  self->queryByCoordinate = flag;
}
