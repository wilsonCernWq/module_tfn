#pragma once

#include <memory>
#include <vector>
#include <array>
#include <ospcommon/vec.h>
#include <GLFW/glfw3.h>
#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#endif

#include "ospray/ospray_cpp/TransferFunction.h"
#include "common/sg/transferFunction/TransferFunction.h"
#include "tfn_reader/tfn_lib.h"
#include "widgets/Imgui3dExport.h"

namespace ospray {
  namespace tfn_widget {
    class OSPRAY_IMGUI3D_INTERFACE TransferFunctionWidget
    {
    public:

      TransferFunctionWidget(std::shared_ptr<sg::TransferFunction> tfn);
      ~TransferFunctionWidget();
      TransferFunctionWidget(const TransferFunctionWidget &t);
      TransferFunctionWidget& operator=(const TransferFunctionWidget &t);
      /* Draw the transfer function editor widget, returns true if the
       * transfer function changed
       */
      void drawUi();
      /* Render the transfer function to a 1D texture that can
       * be applied to volume data
       */
      void render();
      // Load the transfer function in the file passed and set it active
      void load(const ospcommon::FileName &fileName);
      // Save the current transfer function out to the file
      void save(const ospcommon::FileName &fileName) const;

      /* struct Line */
      /* { */
      /* 	// A line is made up of points sorted by x, its coordinates are */
      /* 	// on the range [0, 1] */
      /* 	std::vector<ospcommon::vec2f> line; */
      /* 	int color; */

      /* 	// TODO: Constructor that takes an existing line */
      /* 	// Construct a new diagonal line: [(0, 0), (1, 1)] */
      /* 	Line(); */
      /* 	/\* Move a point on the line from start to end, if the line is */
      /* 	 * not split at 'start' it will be split then moved */
      /* 	 * TODO: Should we have some cap on the number of points? We should */
      /* 	 * also track if you're actively dragging a point so we don't recreate */
      /* 	 * points if you move the mouse too fast */
      /* 	 *\/ */
      /* 	void movePoint(const float &startX, const ospcommon::vec2f &end); */
      /* 	// Remove a point from the line, merging the two segments on either side */
      /* 	void removePoint(const float &x); */
      /* }; */

    private:

      // This defines the control point class
      struct ColorPoint {
	float p; // location of the control point [0, 1]
	float r, g, b;
	ColorPoint() = default;
        ColorPoint(const float cp, const float cr, const float cg, const float cb):
	  p(cp), r(cr), g(cg), b(cb){}
        ColorPoint(const ColorPoint& c) : p(c.p), r(c.r), g(c.g), b(c.b){}
	// This function gives Hex color for ImGui
	unsigned long GetHex() {
	  return (0xff << 24) +
	    ((static_cast<uint8_t>(b * 255.f) & 0xff) << 16) +
	    ((static_cast<uint8_t>(g * 255.f) & 0xff) << 8) +
	    ((static_cast<uint8_t>(r * 255.f) & 0xff));
	}
      };
      struct OpacityPoint {
	OpacityPoint() = default;
        OpacityPoint(const float cp, const float ca) : p(cp), a(ca){}
        OpacityPoint(const OpacityPoint& c) : p(c.p), a(c.a){}
	float p; // location of the control point [0, 1]
	float a;
      };

      // The indices of the transfer function color presets available
      enum ColorMap {
	JET,
	ICE_FIRE,
	COOL_WARM,
	BLUE_RED,
	GRAYSCALE,
      };

    private:

      // The color control point list
      std::vector<ColorPoint>   tfn_c;
      // The opacity control point list
      std::vector<OpacityPoint> tfn_o;

      // interpolate trasnfer function
      int tfn_w = 256;
      int tfn_h = 1;
      
      // The scenegraph transfer function being manipulated by this widget
      std::shared_ptr<sg::TransferFunction>     tfn_sg;
      // The list of avaliable transfer functions, both built-in and loaded
      std::vector<tfn_reader::TransferFunction> tfn_readers;
      // The selected transfer function being shown
      int  tfn_selection;
      // Mode to delete control points
      bool tfn_delete = 0;
      
      /* // Lines for RGBA transfer function controls */
      /* std::array<Line, 4> rgbaLines; */
      /* // The line currently being edited */
      /* int activeLine; */
      /* // If we're customizing the transfer function's RGB values */
      /* bool customizing; */

      // Track if the function changed and must be re-uploaded.
      // We start by marking it changed to upload the initial palette
      bool   tfn_changed;
      // The 2d palette texture on the GPU for displaying the color map in the UI.
      GLuint tfn_palette;

      // The filename input text buffer
      std::vector<char> text_buffer;


      /* // Select the provided color map specified by tfcnSelection. useOpacity */
      /* // indicates if the transfer function's opacity values should be used if available */
      /* // overwriting the user's set opacity data. This is done when loading from a file */
      /* // to show the loaded tfcn, but not when switching from the preset picker. */
      /* void setColorMap(const bool useOpacity); */
      /* // Load up the preset color maps */
      /* void loadColorMapPresets(); */
      void LoadDefaultMap();
    };
  };
}// ::ospray

