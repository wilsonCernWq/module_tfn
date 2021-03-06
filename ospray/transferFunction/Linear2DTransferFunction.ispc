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
			     void *uniform _volume,
			     const varying vec3f coordinate)
{
  // ====================================================================== //
  // Check coordinate value
  // ====================================================================== //
  
  // Return (0,0,0) for NaN values.
  if (isnan(coordinate)) { return make_vec3f(0.0f); }

  // ====================================================================== //
  // Check function length
  // ====================================================================== //
  
  // Cast to the actual TransferFunction subtype.
  const uniform Linear2DTransferFunction *uniform self
    = (const uniform Linear2DTransferFunction *uniform) _self;

  // No color values may be available.
  if (self->colorValueCount == 0) {  return make_vec3f(1.0f); }

  // ====================================================================== //
  // Compute
  // ====================================================================== //
  
  // Cast to the Volume type.
  uniform Volume *uniform volume
    = (uniform Volume *uniform) _volume;

  // Sample value
  varying float v = volume->sample(volume, coordinate);
  varying float g = length(volume->computeGradient(volume, coordinate));
  
  // Clamp the value to the lower bound of the value range.
  if (v <= self->super.valueRange.x) {
    if (self->colorValueH == 1) { return self->colorValues[0]; }
    v = self->super.valueRange.x;
  }

  // Clamp the value to the upper bound of the value range.
  if (v >= self->super.valueRange.y) {
    if (self->colorValueH == 1) {
      return self->colorValues[self->colorValueCount - 1];
    }
    v = self->super.valueRange.y;
  }

  // Clamp the value to the lower bound of the gradient range.
  if (g <= self->gradRange.x) { g = self->gradRange.x; }
  
  // Clamp the value to the upper bound of the gradient range.
  if (g >= self->gradRange.y) { g = self->gradRange.y; }
  
  // Map the value into the range [0.0, 1.0].
  v = (v - self->super.valueRange.x)
    / (self->super.valueRange.y - self->super.valueRange.x)
    * (self->colorValueW - 1.0f);

  g = (g - self->gradRange.x)
    / (self->gradRange.y - self->gradRange.x)
    * (self->colorValueH - 1.0f);

  // Compute the color index and fractional offset.
  const varying int   i_x = floor(v);
  const varying float r_x = v - i_x;
  const varying int   i_y = floor(g);
  const varying float r_y = g - i_y;

  // The interpolated color.
  const varying int idx_ll =
    i_y * self->colorValueW + i_x;
  const varying int idx_lr =
    i_y * self->colorValueW + min(i_x + 1, self->colorValueW - 1);
  const varying int idx_ul =
    min(i_y + 1, self->colorValueH - 1) * self->colorValueW + i_x;
  const varying int idx_ur =
    min(i_y + 1, self->colorValueH - 1) * self->colorValueW + min(i_x + 1, self->colorValueW - 1);
  return
    ((1.f - r_y) * ((1.0f - r_x) * self->colorValues[idx_ll] + r_x * self->colorValues[idx_lr]) +
     (r_y      ) * ((1.0f - r_x) * self->colorValues[idx_ul] + r_x * self->colorValues[idx_ur]));
}

inline varying float
LTFN2D_getOpacityForCoordinate(const void *uniform _self,
			       void *uniform _volume,
			       const varying vec3f coordinate)
{
  // ====================================================================== //
  // Check coordinate value
  // ====================================================================== //
  
  // Return (0,0,0) for NaN values.
  if (isnan(coordinate)) { return 0.0f; }

  // ====================================================================== //
  // Check function length
  // ====================================================================== //
  
  // Cast to the actual TransferFunction subtype.
  const uniform Linear2DTransferFunction *uniform self
    = (const uniform Linear2DTransferFunction *uniform) _self;

  // No opacity values may be available.
  if (self->opacityValueCount == 0) {  return 1.0f; }

  // ====================================================================== //
  // Compute
  // ====================================================================== //
  
  // Cast to the Volume type.
  uniform Volume *uniform volume
    = (uniform Volume *uniform) _volume;

  // Sample value
  varying float v = volume->sample(volume, coordinate);
  varying float g = length(volume->computeGradient(volume, coordinate));

  // Clamp the value to the lower bound of the value range.
  if (v <= self->super.valueRange.x) {
    if (self->opacityValueH == 1) { return self->opacityValues[0]; }
    v = self->super.valueRange.x;
  }

  // Clamp the value to the upper bound of the value range.
  if (v >= self->super.valueRange.y) {
    if (self->opacityValueH == 1) {
      return self->opacityValues[self->opacityValueCount - 1];
    }
    v = self->super.valueRange.y;
  }

  // Clamp the value to the lower bound of the gradient range.
  if (g <= self->gradRange.x) { g = self->gradRange.x; }
  
  // Clamp the value to the upper bound of the gradient range.
  if (g >= self->gradRange.y) { g = self->gradRange.y; }
  
  // Map the value into the range [0.0, 1.0].
  v = (v - self->super.valueRange.x)
    / (self->super.valueRange.y - self->super.valueRange.x)
    * (self->opacityValueW - 1.0f);

  g = (g - self->gradRange.x)
    / (self->gradRange.y - self->gradRange.x)
    * (self->opacityValueH - 1.0f);

  // Compute the opacity index and fractional offset.
  const varying int   i_x = floor(v);
  const varying float r_x = v - i_x;
  const varying int   i_y = floor(g);
  const varying float r_y = g - i_y;
  
  // The interpolated opacity.
  const varying int idx_ll =
    i_y * self->opacityValueW + i_x;
  const varying int idx_lr =
    i_y * self->opacityValueW + min(i_x + 1, self->opacityValueW - 1);
  const varying int idx_ul =
    min(i_y + 1, self->opacityValueH - 1) * self->opacityValueW + i_x;
  const varying int idx_ur =
    min(i_y + 1, self->opacityValueH - 1) * self->opacityValueW + min(i_x + 1, self->opacityValueW - 1);
  const float ret =
    ((1.f - r_y) * ((1.0f - r_x) * self->opacityValues[idx_ll] + r_x * self->opacityValues[idx_lr]) +
     (r_y      ) * ((1.0f - r_x) * self->opacityValues[idx_ul] + r_x * self->opacityValues[idx_ur]));
  return ret;
}

//-----------------------------------------------------------------------------------

inline varying vec3f
LTFN2D_getColorForValue(const void *uniform _self,
			varying float value)
{
  // Return (0,0,0) for NaN values.
  if (isnan(value))
    return make_vec3f(0.0f);

  // Cast to the actual TransferFunction subtype.
  const Linear2DTransferFunction *uniform self
    = (const Linear2DTransferFunction *uniform) _self;

  // No color values may be available.
  if (self->colorValueCount == 0)
    return make_vec3f(1.0f);

  // Clamp the value to the lower bound of the value range.
  if (value <= self->super.valueRange.x)
    return self->colorValues[0];

  // Clamp the value to the upper bound of the value range.
  if (value >= self->super.valueRange.y)
    return self->colorValues[self->colorValueW - 1];

  // Map the value into the range [0.0, 1.0].
  value
    = (value - self->super.valueRange.x)
    / (self->super.valueRange.y - self->super.valueRange.x)
    * (self->colorValueW - 1.0f);

  // Compute the color index and fractional offset.
  int   index     = floor(value);
  float remainder = value - index;

  // The interpolated color.
  return
    ((1.0f - remainder) * self->colorValues[index]
     + remainder        * self->colorValues[min(index + 1, self->colorValueW - 1)]);
}

inline varying float
LTFN2D_getOpacityForValue(const void *uniform _self,
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
    return self->opacityValues[self->opacityValueW - 1];

  // Map the value into the range [0.0, numValues).
  const float remapped
    = (value - self->super.valueRange.x)
    / (self->super.valueRange.y - self->super.valueRange.x)
    * (self->opacityValueW - 1.0f);

  // Compute the opacity index and fractional offset.
  int index = floor(remapped);
  float remainder = remapped - index;

  // The interpolated opacity.
  float ret =
    (1.0f - remainder) * self->opacityValues[index]
    + remainder * self->opacityValues[min(index + 1, self->opacityValueW - 1)];
  return ret;
}


inline varying float
LTFN2D_getIntegratedOpacityForValue(const void *uniform _self,
				    varying float value1, varying float value2)
{
  // disable this function
  return 1.0f;

  // Cast to the actual TransferFunction subtype.
  const Linear2DTransferFunction *uniform self
    = (const Linear2DTransferFunction *uniform) _self;
  if (!self->super.preIntegration || !self->super.preIntegrationComputed)
    return LTFN2D_getOpacityForValue(_self, value2);
  if (value1 > value2)
  {
   float tmpVal = value1;
   value1 = value2;
   value2 = tmpVal;
  }
  // Return 0 for NaN values.
  if (isnan(value1) || isnan(value2)) return 0.0f;

  // No opacity values may be available.
  if (self->opacityValueCount == 0)
    return 1.0f;

  // Clamp the value to the lower bound of the value range.
  if (value2 <= self->super.valueRange.x)
    value2 = self->super.valueRange.x;

  // Clamp the value to the lower bound of the value range.
  if (value1 <= self->super.valueRange.x)
    value1 = self->super.valueRange.x;

  // Clamp the value to the upper bound of the value range.
  if (value2 >= self->super.valueRange.y)
    value2 = self->super.valueRange.y;

  // Clamp the value to the upper bound of the value range.
  if (value1 >= self->super.valueRange.y)
    value1 = self->super.valueRange.y;

  // Map the value into the range [0.0, numValues).
  const float remapped1
    = (value1 - self->super.valueRange.x)
    / (self->super.valueRange.y - self->super.valueRange.x)
    * (self->opacityValueCount - 1.0f);

  const float remapped2
    = (value2 - self->super.valueRange.x)
    / (self->super.valueRange.y - self->super.valueRange.x)
    * (self->opacityValueCount - 1.0f);

  // Compute the opacity index and fractional offset.
  int index1 = floor(remapped1);
  float remainder1 = remapped1 - index1;

  // The interpolated opacity.
  float interpval1 =
    (1.0f - remainder1) * self->opacityValues[index1]
    + remainder1 * self->opacityValues[min(index1 + 1, self->opacityValueCount - 1)];

  // Compute the opacity index and fractional offset.
  int index2 = floor(remapped2);
  float remainder2 = remapped2 - index2;

  // The interpolated opacity.
  float interpval2 =
    (1.0f - remainder2) * self->opacityValues[index2]
    + remainder2 * self->opacityValues[min(index2 + 1, self->opacityValueCount - 1)];

  float ret = 0.f;

  // ret += interpval2 + interpval1;
  // int numAdded = 0;
  // for (int i=index1+1;i<index2 && i < self->opacityValueCount;i++)
  // {
  //   ret += self->opacityValues[i];
  //   numAdded++;
  // }
  // ret = ret / (2.f+numAdded);

  if (index2 - index1 > 1)
    ret += self->opacityPITable[(index1+1)+(index2-1)*self->opacityValueCount];
  return  (ret + interpval1 + interpval2)/(index2-index1+1);
}

inline varying vec3f
LTFN2D_getIntegratedColorForValue(const void *uniform _self,
				  varying float value1, varying float value2)
{
  // disable this function
  return make_vec3f(1.0f);
      
  // Cast to the actual TransferFunction subtype.
  const Linear2DTransferFunction *uniform self
    = (const Linear2DTransferFunction *uniform) _self;

  if (!self->super.preIntegration || !self->super.preIntegrationComputed)
    return LTFN2D_getColorForValue(_self, value2);
  //TODO: use lookup table, but for now I don't notice much of a slowdown
  // Return (0,0,0) for NaN values.
  if (isnan(value1) || isnan(value2))
    return make_vec3f(0.0f);

  // No color values may be available.
  if (self->colorValueCount == 0)
    return make_vec3f(1.0f);

   if (value1 > value2)
   {
     float tmpVal = value1;
     value1 = value2;
     value2 = tmpVal;
   }

  // Clamp the value to the upper bound of the value range.
  if (value1 > self->super.valueRange.y)
    value1 = self->super.valueRange.y;

  // Clamp the value to the upper bound of the value range.
  if (value2 > self->super.valueRange.y)
    value2 = self->super.valueRange.y;

  // Clamp the value to the upper bound of the value range.
  if (value1 < self->super.valueRange.x)
    value1 = self->super.valueRange.x;

  // Clamp the value to the upper bound of the value range.
  if (value2 < self->super.valueRange.x)
    value2 = self->super.valueRange.x;

  // Map the value into the range [0.0, 1.0].
  value1
    = (value1 - self->super.valueRange.x)
    / (self->super.valueRange.y - self->super.valueRange.x)
    * (self->colorValueCount - 1.0f);

  // Compute the color index and fractional offset.
  int   index1     = floor(value1);
  float remainder1 = value1 - index1;
  // The interpolated opacity.
  int oindex1 = min((value1)/((float)(self->colorValueCount-1))*((float)(self->opacityValueCount-1)),self->opacityValueCount-1);
  float opacity1 =
    (1.0f - remainder1) * self->opacityValues[oindex1]
    + remainder1 * self->opacityValues[min(oindex1 + 1, self->opacityValueCount - 1)];

  // The interpolated color.
   vec3f interpColor1 = ((1.0f - remainder1) * self->colorValues[index1]
     + remainder1        * self->colorValues[min(index1 + 1, self->colorValueCount - 1)]);

  // Map the value into the range [0.0, 1.0].
  value2
    = (value2 - self->super.valueRange.x)
    / (self->super.valueRange.y - self->super.valueRange.x)
    * (self->colorValueCount - 1.0f);

  // Compute the color index and fractional offset.
  int   index2     = floor(value2);
  float remainder2 = value2 - index2;
  int oindex2 = min((value2)/((float)(self->colorValueCount-1))*((float)(self->opacityValueCount-1)),self->opacityValueCount-1);
  float opacity2 =
    (1.0f - remainder1) * self->opacityValues[oindex2]
    + remainder1 * self->opacityValues[min(oindex2 + 1, self->opacityValueCount - 1)];

  // The interpolated color.
   vec3f interpColor2 = ((1.0f - remainder2) * self->colorValues[index2]
     + remainder2        * self->colorValues[min(index2 + 1, self->colorValueCount - 1)]);

  vec3f ret = make_vec3f(0.f);

  float dev = max((index2-index1+1.f),2.f);
  return (ret + (interpColor1*opacity1 + interpColor2*opacity2)*2.f/(opacity1+opacity2+1e-5f))/dev;
}

uniform vec2f
LTFN2D_getMinMaxOpacityInRange(void *uniform _self,
			       const uniform vec2f &range)
{
  uniform Linear2DTransferFunction *uniform self = (uniform Linear2DTransferFunction *uniform) _self;
  const uniform int   maxDim = PRECOMPUTED_OPACITY_SUBRANGE_COUNT - 1;
  const uniform float denom  = self->super.valueRange.y - self->super.valueRange.x;
  const uniform int   i      = floor(clamp((range.x - self->super.valueRange.x) / denom) * maxDim);
  const uniform int   j      = ceil(clamp((range.y - self->super.valueRange.x) / denom) * maxDim);
  return self->minMaxOpacityInRange[min(i,maxDim)][min(j,maxDim)];
}

varying float
LTFN2D_getMaxOpacityInRange(void *uniform _self,
			    const varying vec2f &range)
{
  uniform Linear2DTransferFunction *uniform self = (uniform Linear2DTransferFunction *uniform) _self;
  const uniform int maxDim = PRECOMPUTED_OPACITY_SUBRANGE_COUNT - 1;
  const uniform float denom = self->super.valueRange.y - self->super.valueRange.x;
  const varying int i = floor(clamp((range.x - self->super.valueRange.x) / denom) * maxDim);
  const varying int j = ceil(clamp((range.y - self->super.valueRange.x) / denom) * maxDim);
  return self->minMaxOpacityInRange[min(i,maxDim)][min(j,maxDim)].y;
}

void LTFN2D_precomputeMinMaxOpacityRanges(void *uniform _self)
{
  uniform Linear2DTransferFunction *uniform self
    = (uniform Linear2DTransferFunction *uniform) _self;

  // Compute the diagonal.
  for (uniform int i=0; i < PRECOMPUTED_OPACITY_SUBRANGE_COUNT; i++) {

    // Figure out the range of values in the array we are going to compare.
    const uniform int checkRangeLow
      = self->opacityValueW * (((float) i) / PRECOMPUTED_OPACITY_SUBRANGE_COUNT);
    const uniform int checkRangeHigh
      = self->opacityValueW * (((float)i + 1) / PRECOMPUTED_OPACITY_SUBRANGE_COUNT);

    uniform vec2f range = make_vec2f(self->opacityValues[checkRangeLow]);
    
    for (uniform int opacityIDX = checkRangeLow + 1; opacityIDX < checkRangeHigh; opacityIDX++)
    {
      for (uniform int id = 0; id < self->opacityValueH; id++) {
	range = make_vec2f(min(range.x, self->opacityValues[opacityIDX + id * self->opacityValueW]),
			   max(range.y, self->opacityValues[opacityIDX + id * self->opacityValueW]));
      }
    }

    self->minMaxOpacityInRange[i][i] = range;
  }

  // Fill out each column from the diagonal up.
  for (uniform int i=0; i < PRECOMPUTED_OPACITY_SUBRANGE_COUNT; i++) {
    for (uniform int j = i + 1; j < PRECOMPUTED_OPACITY_SUBRANGE_COUNT; j++) {

      // Figure out the range of values in the array we are going to compare.
      const uniform int checkRangeLow
	= self->opacityValueW * (((float) i) / PRECOMPUTED_OPACITY_SUBRANGE_COUNT);
      const uniform int checkRangeHigh
	= self->opacityValueW * (((float) j + 1) / PRECOMPUTED_OPACITY_SUBRANGE_COUNT);

      uniform vec2f range = self->minMaxOpacityInRange[i][i];
      for (uniform int opacityIDX = checkRangeLow + 1; opacityIDX < checkRangeHigh; opacityIDX++) {
	for (uniform int id = 0; id < self->opacityValueH; id++) {
	  range = make_vec2f(min(range.x, self->opacityValues[opacityIDX + id * self->opacityValueW]),
			     max(range.y, self->opacityValues[opacityIDX + id * self->opacityValueW]));
	}
      }

      self->minMaxOpacityInRange[i][j] = range;

    }

  }

}

void LTFN2D_precomputePreIntegratedOpacityValues(void* uniform _self)
{
  // Cast to the actual TransferFunction subtype.
  Linear2DTransferFunction *uniform self
    = (Linear2DTransferFunction *uniform) _self;

  if (self->opacityPITable)
    delete[] self->opacityPITable;
  self->opacityPITable = uniform new uniform float[self->opacityValueCount*self->opacityValueCount];

  for(int i=0;i<self->opacityValueCount;i++)
  {
    for(int j=0;j<self->opacityValueCount;j++)
    {
      if (j < i)
        continue;
      float val = 0.f;
      for (int k = i; k <= j; k++)
        val += self->opacityValues[k];
      self->opacityPITable[j*self->opacityValueCount+i] = val;
    }
  }
}

void LTFN2D_precomputePreIntegratedColorValues(void* uniform _self)
{
  // Cast to the actual TransferFunction subtype.
  Linear2DTransferFunction *uniform self
    = (Linear2DTransferFunction *uniform) _self;

  if (self->colorPITable)
    delete[] self->colorPITable;
  self->colorPITable = uniform new uniform vec3f[self->colorValueCount*self->colorValueCount];

  for(int i=0;i<self->colorValueCount;i++)
  {
    for(int j=0;j<self->colorValueCount;j++)
    {
      if (j < i)
        continue;
      vec3f val = make_vec3f(0.f);
      float opacityAccum = 0.f;
      for (int k = i; k <= j; k++)
      {
        const float interp = ((float)k)/(float)self->colorValueCount;
        float opacity = 1.f;
        if (self->opacityValueCount > 0)
        {
          opacity = self->opacityValues[interp*self->opacityValueCount];
        }
        val = val + self->colorValues[k]*opacity;
        opacityAccum += opacity;
      }
      if (opacityAccum > 0.f)
        val = val/opacityAccum;
      self->colorPITable[j*self->colorValueCount+i] = val;
    }
  }
}

//-----------------------------------------------------------------------------------

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
  LTFN2D_precomputePreIntegratedColorValues(_self);
  LTFN2D_precomputePreIntegratedOpacityValues(_self);
  self->super.preIntegrationComputed = true;
}

//-----------------------------------------------------------------------------------

export void *uniform LTFN2D_createInstance()
{
  // The transfer function.
  Linear2DTransferFunction *uniform self = uniform new uniform Linear2DTransferFunction;

  self->super.getColorForCoordinate = LTFN2D_getColorForCoordinate;
  self->super.getOpacityForCoordinate = LTFN2D_getOpacityForCoordinate;
  
  // Function to get the interpolated color for a given value.
  self->super.getColorForValue = LTFN2D_getColorForValue;

  // // Function to get the interpolated opacity for a given value.
  self->super.getOpacityForValue = LTFN2D_getOpacityForValue;

  // // Function to get the interpolated color for a given value.
  self->super.getIntegratedColorForValue = LTFN2D_getIntegratedColorForValue;

  // // Function to get the interpolated opacity for a given value.
  self->super.getIntegratedOpacityForValue = LTFN2D_getIntegratedOpacityForValue;

  // Virtual function to look up the maximum opacity based on an input range.
  self->super.getMaxOpacityInRange = LTFN2D_getMaxOpacityInRange;

  // Virtual function to look up the min/max opacity based on an input range.
  self->super.getMinMaxOpacityInRange = LTFN2D_getMinMaxOpacityInRange;

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

// ======================================================================== //
// Exported FUnctions
// ======================================================================== //

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

  // Precompute the min / max opacity ranges.
  LTFN2D_precomputeMinMaxOpacityRanges(_self);

  self->super.preIntegrationComputed = false;
}

//-----------------------------------------------------------------------------------

export void LTFN2D_setQueryByCoordinate(void *uniform _self) 
{
  // Cast to the actual type.
  Linear2DTransferFunction *uniform self
    = (Linear2DTransferFunction *uniform) _self;

  // Set the transfer function value range.
  self->super.queryByCoordinate = true;
}

export void LTFN2D_setGradRange(void *uniform _self, 
				const uniform vec2f &grad) 
{
  // Cast to the actual type.
  Linear2DTransferFunction *uniform self
    = (Linear2DTransferFunction *uniform) _self;

  // Set the transfer function value range.
  self->gradRange = grad;
}
