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

#include "transferFunction/Linear2DTransferFunction.h"
#include "Linear2DTransferFunction_ispc.h"

namespace ospray {
namespace tfn {

  void Linear2DTransferFunction::commit()
  {
    // Create the equivalent ISPC transfer function.
    if (ispcEquivalent == nullptr) createEquivalentISPC();

    // Commit to parent class
    TransferFunction::commit();

    // Retrieve the color and opacity values.
    colorW   = getParam1i("colorWidth",  0);
    colorH   = getParam1i("colorHeight", 0);
    opacityW = getParam1i("opacityWidth",  0);
    opacityH = getParam1i("opacityHeight", 0);
    // std::cout << "colorW " << colorW << std::endl;
    // std::cout << "colorH " << colorH << std::endl;
    // std::cout << "opacityW " << opacityW << std::endl;
    // std::cout << "opacityH " << opacityH << std::endl;
    colorValues   = getParamData("colors", nullptr);
    opacityValues = getParamData("opacities", nullptr);
    ispc::LTFN2D_setPreIntegration(ispcEquivalent,
				   getParam1i("preIntegration", 0));

    // Retrieve gradient range
    vec2f gradRange = getParam2f("gradRange", vec2f(0.0f, 1.0f));
    ispc::LTFN2D_setGradRange(ispcEquivalent,
			      (const ispc::vec2f &)gradRange);

    // Set the color values.
    if (colorValues) {
      ispc::LTFN2D_setColorValues(ispcEquivalent, 
				  colorValues->numItems, 
				  colorW, colorH,
				  (ispc::vec3f*)colorValues->data);
    }

    // Set the opacity values.
    if (opacityValues) {
      ispc::LTFN2D_setOpacityValues(ispcEquivalent, 
				    opacityValues->numItems, 
				    opacityW, opacityH,
				    (float *)opacityValues->data);
    }

    // Compute preingetration
    if (getParam1i("preIntegration", 0) && colorValues && opacityValues) 
    {
      ispc::LTFN2D_precomputePreIntegratedValues(ispcEquivalent);
    }

    // Set flag to query color using sample coordinate
    ispc::LTFN2D_setQueryByCoordinate(ispcEquivalent, true);
    
    // Notify listeners that the transfer function has changed.
    notifyListenersThatObjectGotChanged();
  }

  std::string Linear2DTransferFunction::toString() const
  {
    return "ospray::Linear2DTransferFunction";
  }

  void Linear2DTransferFunction::createEquivalentISPC()
  {
    // Debug output
    std::cout << "[ospray internal] Creating Linear2DTransferFunction" << std::endl;

    // The equivalent ISPC transfer function must not exist yet.
    exitOnCondition(ispcEquivalent != nullptr,
                    "attempt to overwrite an existing ISPC transfer function");

    // Create the equivalent ISPC transfer function.
    ispcEquivalent = ispc::LTFN2D_createInstance();

    // The object may not have been created.
    exitOnCondition(ispcEquivalent == nullptr,
                    "unable to create ISPC transfer function");
  }

  // A piecewise linear transfer function.
  OSP_REGISTER_TRANSFER_FUNCTION(Linear2DTransferFunction, piecewise_linear_2d);
}
} // ::ospray

