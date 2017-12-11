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

#pragma once

// ospray
#include "common/Data.h"
#include "volume/Volume.h"
#include "transferFunction/TransferFunction.h"

namespace ospray {
namespace tfn {

  /*! \brief A concrete implementation of the TransferFunction class for
    piecewise linear transfer functions.
  */
  struct OSPRAY_SDK_INTERFACE Linear2DTransferFunction : public TransferFunction
  {
    Linear2DTransferFunction() = default;
    virtual ~Linear2DTransferFunction() override = default;
    virtual void commit() override;
    virtual std::string toString() const override;
  private:
    size_t colorW = 0; // width of the color map (scalar axis)
    size_t colorH = 0; // height of the color map (gradient axis)
    size_t opacityW = 0;
    size_t opacityH = 0;
    //! Data 2D array that stores the color map.
    Ref<Data> colorValues;
    //! Data 2D array that stores the opacity map.
    Ref<Data> opacityValues;
    //! Create the equivalent ISPC transfer function.
    void createEquivalentISPC();
  };
};
} // ::ospray

