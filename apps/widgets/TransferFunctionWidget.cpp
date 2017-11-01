#include <cmath>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include <imgui.h>
#include <imconfig.h>

#include "ospcommon/math.h"
#include "ospray/ospray_cpp/Data.h"
#include "ospray/ospray_cpp/TransferFunction.h"
#include "common/sg/common/Data.h"

#include "tfn_reader/tfn_lib.h"
#include "TransferFunctionWidget.h"

using namespace ospcommon;

template<typename T>
int find_idx(const T& A, float p, int l = -1, int r = -1)
{
  l = l == -1 ? 0 : l;
  r = r == -1 ? A.size()-1 : r;
  int m = (r + l) / 2;
  if (A[l].p > p) { return l; }
  else if (A[r].p <= p) { return r+1; }
  else if ((m == l) || (m == r)) { return m + 1; }
  else {
    if (A[m].p <= p) { return find_idx(A, p, m, r); }
    else { return find_idx(A, p, l, m); }
  }
}

float lerp(const float& l, const float& r,
	   const float& pl, const float& pr, const float& p)
{
  const float dl = std::abs(pr - pl) > 0.0001f ? (p - pl) / (pr - pl) : 0.f;
  const float dr = 1.f - dl;
  return l * dr + r * dl;
}

void ospray::tfn_widget::TransferFunctionWidget::LoadDefaultMap()
{
  tfn_c_list.emplace_back(4);
  tfn_c_list.back()[0] = ColorPoint(0.0f, 0.f, 0.f, 1.f);
  tfn_c_list.back()[1] = ColorPoint(0.3f, 0.f, 1.f, 1.f);
  tfn_c_list.back()[2] = ColorPoint(0.6f, 1.f, 1.f, 0.f);
  tfn_c_list.back()[3] = ColorPoint(1.0f, 1.f, 0.f, 0.f);
  tfn_o_list.emplace_back(5);
  tfn_o_list.back()[0] = OpacityPoint(0.00f, 0.00f);
  tfn_o_list.back()[1] = OpacityPoint(0.25f, 0.25f);
  tfn_o_list.back()[2] = OpacityPoint(0.50f, 0.50f);
  tfn_o_list.back()[3] = OpacityPoint(0.75f, 0.75f);
  tfn_o_list.back()[4] = OpacityPoint(1.00f, 1.00f);
  tfn_editable.emplace_back(true);
};

void ospray::tfn_widget::TransferFunctionWidget::SetTFNSelection(int selection) {
  tfn_selection = selection;
  tfn_c = &(tfn_c_list[selection]);
  tfn_o = &(tfn_o_list[selection]);
  tfn_edit = tfn_editable[selection];
}

ospray::tfn_widget::TransferFunctionWidget::~TransferFunctionWidget()
{
  if (tfn_palette) { glDeleteTextures(1, &tfn_palette); }
}

ospray::tfn_widget::TransferFunctionWidget::TransferFunctionWidget
(std::shared_ptr<sg::TransferFunction> tfn) :
  tfn_sg(tfn),
  tfn_selection(JET),
  tfn_changed(true),
  tfn_palette(0),
  tfn_text_buffer(512, '\0')
{
  LoadDefaultMap();
  SetTFNSelection(tfn_selection);
}

ospray::tfn_widget::TransferFunctionWidget::TransferFunctionWidget
(const TransferFunctionWidget &t) :
  tfn_c_list(t.tfn_c_list),
  tfn_o_list(t.tfn_o_list),
  tfn_sg(t.tfn_sg),
  tfn_readers(t.tfn_readers),
  tfn_selection(t.tfn_selection),
  tfn_changed(true),
  tfn_palette(0),
  tfn_text_buffer(512, '\0')
{
  SetTFNSelection(tfn_selection);
}

ospray::tfn_widget::TransferFunctionWidget&
ospray::tfn_widget::TransferFunctionWidget::operator=
(const ospray::tfn_widget::TransferFunctionWidget &t)
{
  if (this == &t) { return *this; }
  tfn_c_list = t.tfn_c_list;
  tfn_o_list = t.tfn_o_list;
  tfn_sg      = t.tfn_sg;
  tfn_readers = t.tfn_readers;
  tfn_selection = t.tfn_selection;
  tfn_changed = true;
  tfn_palette = 0;
  return *this;
}

void ospray::tfn_widget::TransferFunctionWidget::drawUi()
{
  if (!ImGui::Begin("Transfer Function Widget")) { ImGui::End(); return; }
  ImGui::Text("1D Transfer Function");
  ImGui::Text("Left click and drag to move control points\n"
	      "Left double click on empty area to add control points\n"
	      "Right double click on control points to remove it\n"
	      "Left click on color preview can open the color editor\n");
  // radio paremeters
  ImGui::Separator();
  ImGui::InputText("Transfer Function Filename",
		   tfn_text_buffer.data(), tfn_text_buffer.size() - 1);
  if (ImGui::Button("Save")) { save(tfn_text_buffer.data()); }
  ImGui::SameLine();
  if (ImGui::Button("Load")) { load(tfn_text_buffer.data()); }
  //------------ Transfer Function -------------------
  // style
  // only God and me know what do they do ...
  ImDrawList *draw_list = ImGui::GetWindowDrawList();
  float canvas_x = ImGui::GetCursorScreenPos().x;
  float canvas_y = ImGui::GetCursorScreenPos().y;
  float canvas_avail_x = ImGui::GetContentRegionAvail().x;
  float canvas_avail_y = ImGui::GetContentRegionAvail().y;    
  const float mouse_x = ImGui::GetMousePos().x;
  const float mouse_y = ImGui::GetMousePos().y;
  const float scroll_x = ImGui::GetScrollX();
  const float scroll_y = ImGui::GetScrollY();
  const float margin = 10.f;
  const float width  = canvas_avail_x - 2.f * margin;
  const float height = 60.f;
  const float color_len   = 9.f;
  const float opacity_len = 7.f;
  // draw preview texture
  ImGui::SetCursorScreenPos(ImVec2(canvas_x + margin, canvas_y));
  ImGui::Image(reinterpret_cast<void*>(tfn_palette), ImVec2(width, height));
  ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
  for (int i = 0; i < tfn_o->size()-1; ++i)
  {
    std::vector<ImVec2> polyline;
    polyline.emplace_back(canvas_x + margin + (*tfn_o)[i].p * width,
			  canvas_y + height);
    polyline.emplace_back(canvas_x + margin + (*tfn_o)[i].p * width,
			  canvas_y + height - (*tfn_o)[i].a * height);
    polyline.emplace_back(canvas_x + margin + (*tfn_o)[i+1].p * width + 1,
			  canvas_y + height - (*tfn_o)[i+1].a * height);
    polyline.emplace_back(canvas_x + margin + (*tfn_o)[i+1].p * width + 1,
			  canvas_y + height);
    draw_list->AddConvexPolyFilled(polyline.data(), polyline.size(), 0xFFD8D8D8, true);
  }
  canvas_y       += height + margin;
  canvas_avail_y -= height + margin;
  // draw color control points
  ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
  if (tfn_edit)
  {
    // draw circle background
    draw_list->AddRectFilled(ImVec2(canvas_x + margin, canvas_y - margin),
			     ImVec2(canvas_x + margin + width,
				    canvas_y - margin + 2.5 * color_len), 0xFF474646);    
    // draw circles
    for (int i = 0; i < tfn_c->size(); ++i)
    {
      const ImVec2 pos(canvas_x + width * (*tfn_c)[i].p + margin, canvas_y);
      ImGui::SetCursorScreenPos(ImVec2(pos.x - color_len, pos.y));
      ImGui::InvisibleButton(("square-"+std::to_string(i)).c_str(),
			     ImVec2(2.f * color_len, 2.f * color_len));
      ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
      // white background
      draw_list->AddTriangleFilled(ImVec2(pos.x - 0.5f * color_len, pos.y),
				   ImVec2(pos.x + 0.5f * color_len, pos.y),
				   ImVec2(pos.x, pos.y - color_len),
				   0xFFD8D8D8);
      draw_list->AddCircleFilled(ImVec2(pos.x, pos.y + 0.5f * color_len), color_len,
				 0xFFD8D8D8);
      // dark highlight
      draw_list->AddCircleFilled(ImVec2(pos.x, pos.y + 0.5f * color_len), 0.5f*color_len,
				 ImGui::IsItemHovered() ? 0xFF051C33 : 0xFFBCBCBC);
      // setup interactions
      // delete color point
      if (ImGui::IsMouseDoubleClicked(1) && ImGui::IsItemHovered()) {
	if (i > 0 && i < tfn_c->size()-1) {
	  tfn_c->erase(tfn_c->begin() + i);
	  tfn_changed = true;
	}	
      }
      if (ImGui::IsItemActive())
      {
	ImVec2 delta = ImGui::GetIO().MouseDelta;	  
	if (i > 0 && i < tfn_c->size()-1) {
	  (*tfn_c)[i].p += delta.x/width;
	  (*tfn_c)[i].p = clamp((*tfn_c)[i].p, (*tfn_c)[i-1].p, (*tfn_c)[i+1].p);
	}	
      	tfn_changed = true;
      }
      // draw picker
      ImGui::SetCursorScreenPos(ImVec2(pos.x - color_len, pos.y + 1.5f * color_len));
      ImVec4 picked_color = ImColor((*tfn_c)[i].r, (*tfn_c)[i].g, (*tfn_c)[i].b, 1.f);
      if (ImGui::ColorEdit4(("ColorPicker"+std::to_string(i)).c_str(),
      			    (float*)&picked_color,
			    ImGuiColorEditFlags_NoAlpha |
			    ImGuiColorEditFlags_NoInputs |
			    ImGuiColorEditFlags_NoLabel |
			    ImGuiColorEditFlags_AlphaPreview |
			    ImGuiColorEditFlags_NoOptions))
      {
      	(*tfn_c)[i].r = picked_color.x;
      	(*tfn_c)[i].g = picked_color.y;
      	(*tfn_c)[i].b = picked_color.z;
      	tfn_changed = true;
      }
    }
  }    
  // draw opacity control points
  ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
  {
    // draw circles
    for (int i = 0; i < tfn_o->size(); ++i)
    {
      const ImVec2 pos(canvas_x + width  * (*tfn_o)[i].p + margin,
		       canvas_y - height * (*tfn_o)[i].a - margin);		
      ImGui::SetCursorScreenPos(ImVec2(pos.x - opacity_len, pos.y - opacity_len));
      ImGui::InvisibleButton(("button-"+std::to_string(i)).c_str(),
			     ImVec2(2.f * opacity_len, 2.f * opacity_len));
      ImGui::SetCursorScreenPos(ImVec2(canvas_x, canvas_y));
      // dark bounding box
      draw_list->AddCircleFilled(pos,        opacity_len, 0xFF565656);
      // white background
      draw_list->AddCircleFilled(pos, 0.8f * opacity_len, 0xFFD8D8D8);
      // highlight
      draw_list->AddCircleFilled(pos, 0.6f * opacity_len,
				 ImGui::IsItemHovered() ? 0xFF051c33 : 0xFFD8D8D8);
      // setup interaction
      // delete opacity point
      if (ImGui::IsMouseDoubleClicked(1) && ImGui::IsItemHovered()) {
	if (i > 0 && i < tfn_o->size()-1) {
	  tfn_o->erase(tfn_o->begin() + i);
	  tfn_changed = true;	  
	}
      }
      if (ImGui::IsItemActive())
      {
	ImVec2 delta = ImGui::GetIO().MouseDelta;	  
	(*tfn_o)[i].a -= delta.y/height;
	(*tfn_o)[i].a = clamp((*tfn_o)[i].a, 0.0f, 1.0f);
	if (i > 0 && i < tfn_o->size()-1) {
	  (*tfn_o)[i].p += delta.x/width;
	  (*tfn_o)[i].p = clamp((*tfn_o)[i].p, (*tfn_o)[i-1].p, (*tfn_o)[i+1].p);
	}
	tfn_changed = true;
      }
    }
  }
  // draw background interaction
  ImGui::SetCursorScreenPos(ImVec2(canvas_x + margin, canvas_y - margin));
  ImGui::InvisibleButton("tfn_palette", ImVec2(width, 2.5 * color_len));
  // add color point
  if (tfn_edit && ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
  {
    const float p = clamp((mouse_x - canvas_x - margin - scroll_x) / (float) width,
			  0.f, 1.f);
    const int ir = find_idx(*tfn_c, p);
    const int il = ir - 1;
    const float pr = (*tfn_c)[ir].p;
    const float pl = (*tfn_c)[il].p;
    const float r = lerp((*tfn_c)[il].r, (*tfn_c)[ir].r, pl, pr, p);
    const float g = lerp((*tfn_c)[il].g, (*tfn_c)[ir].g, pl, pr, p);
    const float b = lerp((*tfn_c)[il].b, (*tfn_c)[ir].b, pl, pr, p);
    ColorPoint pt; pt.p = p, pt.r = r; pt.g = g; pt.b = b;
    tfn_c->insert(tfn_c->begin() + ir, pt);      
    tfn_changed = true;
    printf("[GUI] add opacity point at %f with value = (%f, %f, %f)\n", p, r, g, b);
  }	      
  // draw background interaction
  ImGui::SetCursorScreenPos(ImVec2(canvas_x + margin, canvas_y - height - margin));
  ImGui::InvisibleButton("tfn_palette", ImVec2(width,height));
  // add opacity point
  if (ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered())
  {
    const float x = clamp((mouse_x - canvas_x - margin - scroll_x) / (float) width,
			  0.f, 1.f);
    const float y = clamp(-(mouse_y - canvas_y + margin - scroll_y) / (float) height,
			  0.f, 1.f);
    const int idx = find_idx(*tfn_o, x);
    OpacityPoint pt; pt.p = x, pt.a = y;
    tfn_o->insert(tfn_o->begin()+idx, pt);
    tfn_changed = true;
    printf("[GUI] add opacity point at %f with value = %f\n", x, y);
  }
  // update cursors
  canvas_y       += 4.f * color_len + margin;
  canvas_avail_y -= 4.f * color_len + margin;
  //------------ Transfer Function -------------------
  ImGui::End();
  
  // if (ImGui::Begin("Transfer Function (Qi's version)")){
  //   ImGui::Text("Left click and drag to add/move points\nRight click to remove\n");
  //   ImGui::InputText("filename", textBuffer.data(), textBuffer.size() - 1);

  //   if (ImGui::Button("Save")){
  //     save(textBuffer.data());
  //   }
  //   ImGui::SameLine();
  //   if (ImGui::Button("Load")){
  //     load(textBuffer.data());
  //   }

  //   std::vector<const char*> colorMaps(transferFunctions.size(), nullptr);
  //   std::transform(transferFunctions.begin(), transferFunctions.end(), colorMaps.begin(),
  // 		   [](const tfn_reader::TransferFunction &t) { return t.name.c_str(); });
  //   if (ImGui::Combo("ColorMap", &tfcnSelection, colorMaps.data(), colorMaps.size())) {
  //     setColorMap(false);
  //   }

  //   ImGui::Checkbox("Customize", &customizing);
  //   if (customizing) {
  //     ImGui::SameLine();
  //     ImGui::RadioButton("Red", &activeLine, 0); ImGui::SameLine();
  //     ImGui::SameLine();
  //     ImGui::RadioButton("Green", &activeLine, 1); ImGui::SameLine();
  //     ImGui::SameLine();
  //     ImGui::RadioButton("Blue", &activeLine, 2); ImGui::SameLine();
  //     ImGui::SameLine();
  //     ImGui::RadioButton("Alpha", &activeLine, 3);
  //   } else {
  //     activeLine = 3;
  //   }

  //   vec2f canvasPos(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
  //   vec2f canvasSize(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
  //   // Force some min size of the editor
  //   if (canvasSize.x < 50.f){
  //     canvasSize.x = 50.f;
  //   }
  //   if (canvasSize.y < 50.f){
  //     canvasSize.y = 50.f;
  //   }

  //   if (paletteTex){
  //     ImGui::Image(reinterpret_cast<void*>(paletteTex), ImVec2(canvasSize.x, 16));
  //     canvasPos.y += 20;
  //     canvasSize.y -= 20;
  //   }

  //   ImDrawList *draw_list = ImGui::GetWindowDrawList();
  //   draw_list->AddRect(canvasPos, canvasPos + canvasSize, ImColor(255, 255, 255));

  //   const vec2f viewScale(canvasSize.x, -canvasSize.y + 10);
  //   const vec2f viewOffset(canvasPos.x, canvasPos.y + canvasSize.y - 10);

  //   ImGui::InvisibleButton("canvas", canvasSize);
  //   if (ImGui::IsItemHovered()){
  //     vec2f mousePos = vec2f(ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
  //     mousePos = (mousePos - viewOffset) / viewScale;
  //     // Need to somehow find which line of RGBA the mouse is closest too
  //     if (ImGui::GetIO().MouseDown[0]){
  // 	rgbaLines[activeLine].movePoint(mousePos.x, mousePos);
  // 	fcnChanged = true;
  //     } else if (ImGui::IsMouseClicked(1)){
  // 	rgbaLines[activeLine].removePoint(mousePos.x);
  // 	fcnChanged = true;
  //     }
  //   }
  //   draw_list->PushClipRect(canvasPos, canvasPos + canvasSize);

  //   if (customizing) {
  //     for (int i = 0; i < static_cast<int>(rgbaLines.size()); ++i){
  // 	if (i == activeLine){
  // 	  continue;
  // 	}
  // 	for (size_t j = 0; j < rgbaLines[i].line.size() - 1; ++j){
  // 	  const vec2f &a = rgbaLines[i].line[j];
  // 	  const vec2f &b = rgbaLines[i].line[j + 1];
  // 	  draw_list->AddLine(viewOffset + viewScale * a, viewOffset + viewScale * b,
  // 			     rgbaLines[i].color, 2.0f);
  // 	}
  //     }
  //   }
  //   // Draw the active line on top
  //   for (size_t j = 0; j < rgbaLines[activeLine].line.size() - 1; ++j){
  //     const vec2f &a = rgbaLines[activeLine].line[j];
  //     const vec2f &b = rgbaLines[activeLine].line[j + 1];
  //     draw_list->AddLine(viewOffset + viewScale * a, viewOffset + viewScale * b,
  // 			 rgbaLines[activeLine].color, 2.0f);
  //     draw_list->AddCircleFilled(viewOffset + viewScale * a, 4.f, rgbaLines[activeLine].color);
  //     // If we're last draw the list Circle as well
  //     if (j == rgbaLines[activeLine].line.size() - 2) {
  // 	draw_list->AddCircleFilled(viewOffset + viewScale * b, 4.f, rgbaLines[activeLine].color);
  //     }
  //   }
  //   draw_list->PopClipRect();
  // }
  // ImGui::End();
}

void RenderTFNTexture(GLuint& tex, int width, int height)
{
  GLint prevBinding = 0;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevBinding);
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
	       0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if (prevBinding) {
    glBindTexture(GL_TEXTURE_2D, prevBinding);
  }
}

void ospray::tfn_widget::TransferFunctionWidget::render()
{
  int num_samples = tfn_sg->child("numSamples").valueAs<int>();
  bool paltte_size_changed = (tfn_w != num_samples);
  tfn_w = num_samples;
  tfn_h = 1; // TODO: right now we onlu support 1D TFN
  
  // Upload to GL if the transfer function has changed
  if (!tfn_palette) {
    RenderTFNTexture(tfn_palette, tfn_w, tfn_h);
    paltte_size_changed = false;
  }

  // Update texture color
  if (tfn_changed)
  {
    // Backup old states
    GLint prevBinding = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &prevBinding);

    // Sample the palette then upload the data
    std::vector<uint8_t> palette(tfn_w * tfn_h * 4, 0);
    auto colors = sg::createNode("colors", "DataVector3f")->nodeAs<sg::DataVector3f>();
    auto alpha  = sg::createNode("alpha", "DataVector2f")->nodeAs<sg::DataVector2f>();
    colors->v.resize(tfn_w);
    alpha->v.resize(tfn_w);
    const float step = 1.0f / (float)tfn_w;
    for (int i = 0; i < tfn_w; ++i)
    {
      const float p = clamp(i * step, 0.0f, 1.0f);
      int ir, il; float pr, pl;
      // color
      ir = find_idx(*tfn_c, p);
      il = ir - 1;
      pr = (*tfn_c)[ir].p;
      pl = (*tfn_c)[il].p;
      const float r = lerp((*tfn_c)[il].r, (*tfn_c)[ir].r, pl, pr, p);
      const float g = lerp((*tfn_c)[il].g, (*tfn_c)[ir].g, pl, pr, p);
      const float b = lerp((*tfn_c)[il].b, (*tfn_c)[ir].b, pl, pr, p);
      colors->v[i][0] = r;
      colors->v[i][1] = g;
      colors->v[i][2] = b;
      // opacity
      ir = find_idx(*tfn_o, p);
      il = ir - 1;
      pr = (*tfn_o)[ir].p;
      pl = (*tfn_o)[il].p;
      const float a = lerp((*tfn_o)[il].a, (*tfn_o)[ir].a, pl, pr, p);
      alpha->v[i].x = p;
      alpha->v[i].y = a;
      // palette
      palette[i * 4 + 0] = static_cast<uint8_t>(r * 255.f);
      palette[i * 4 + 1] = static_cast<uint8_t>(g * 255.f);
      palette[i * 4 + 2] = static_cast<uint8_t>(b * 255.f);
      palette[i * 4 + 3] = 255;
    }
    tfn_changed = false;
    
    // Render palette again
    glBindTexture(GL_TEXTURE_2D, tfn_palette);
    if (paltte_size_changed) {
      // LOGICALLY we need to resize texture of texture is resized
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tfn_w, tfn_h,
		   0, GL_RGBA, GL_UNSIGNED_BYTE,
		   static_cast<const void*>(palette.data()));
    }
    else {
       glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tfn_w, tfn_h, GL_RGBA,
		       GL_UNSIGNED_BYTE, static_cast<const void*>(palette.data()));
    }
    // Restore previous binded texture
    if (prevBinding) { glBindTexture(GL_TEXTURE_2D, prevBinding); }

    // NOTE(jda) - HACK! colors array isn't updating, so we have to forcefully
    //             say "make sure you update yourself"...???
    tfn_sg->add(colors);
    tfn_sg->add(alpha);
    colors->markAsModified();
  }
}

void ospray::tfn_widget::TransferFunctionWidget::load(const ospcommon::FileName &fileName)
{
  tfn_reader::TransferFunction loaded(fileName);
  tfn_readers.emplace_back(fileName);
  const auto tfn_new = tfn_readers.back();
  const int c_size = tfn_new.rgbValues.size();
  const int o_size = tfn_new.opacityValues.size();  
  // load data
  //tfn_c_list.emplace_back(0);
  //tfn_o_list.emplace_back(o_size);
  tfn_c_list.emplace_back(c_size);
  tfn_o_list.emplace_back(o_size);
  tfn_editable.emplace_back(false); // TODO we dont want to edit loaded TFN
  SetTFNSelection(tfn_c_list.size()-1); // set the loaded function as current
  //----- color ---
  if (c_size < 2) {
    std::runtime_error("transfer function contains too few color points");
  }
  // (*tfn_c).emplace_back(0.f,
  // 			tfn_new.rgbValues[0].x,
  // 			tfn_new.rgbValues[0].y,
  // 			tfn_new.rgbValues[0].z);
  // (*tfn_c).emplace_back(1.f,
  // 			tfn_new.rgbValues[c_size-1].x,
  // 			tfn_new.rgbValues[c_size-1].y,
  // 			tfn_new.rgbValues[c_size-1].z);
  // float pp = 0.f;
  // float pr = (*tfn_c)[0].r, pkr = ((*tfn_c)[1].r - pr);
  // float pg = (*tfn_c)[0].g, pkg = ((*tfn_c)[1].g - pg);
  // float pb = (*tfn_c)[0].b, pkb = ((*tfn_c)[1].b - pb);
  // const float c_step = 1.f / (c_size-1);
  // for (int i = 1; i < (c_size-1); ++i) {
  //   const float p = static_cast<float>(i) * c_step;
  //   const float r = tfn_new.rgbValues[i].x;
  //   const float g = tfn_new.rgbValues[i].y;
  //   const float b = tfn_new.rgbValues[i].z;
  //   const float kr = (r - pr) / (p - pp);
  //   const float kg = (g - pg) / (p - pp);
  //   const float kb = (b - pb) / (p - pp);
  //   if (std::abs(kr-pkr) > 0.01f ||
  // 	std::abs(kg-pkg) > 0.01f ||
  // 	std::abs(kb-pkb) > 0.01f)
  //   {
  //     (*tfn_c).emplace(tfn_c->end()-1, p, r, g, b);
  //     pp = p;
  //     pr = r; pg = g; pb = b;
  //     pkr = kr; pkg = kg; pkb = kb;
  //   }    
  // }
  const float c_step = 1.f / (c_size-1);
  for (int i = 0; i < c_size; ++i) {
    const float p = static_cast<float>(i) * c_step;
    (*tfn_c)[i].p = p;
    (*tfn_c)[i].r = tfn_new.rgbValues[i].x;
    (*tfn_c)[i].g = tfn_new.rgbValues[i].y;
    (*tfn_c)[i].b = tfn_new.rgbValues[i].z;
  }  
  if (o_size < 2) {
    std::runtime_error("transfer function contains too few opacity points");
  }
  for (int i = 0; i < o_size; ++i) {
    (*tfn_o)[i].p = tfn_new.opacityValues[i].x;
    (*tfn_o)[i].a = tfn_new.opacityValues[i].y;
  }
  tfn_changed = true;
}

void ospray::tfn_widget::TransferFunctionWidget::save(const ospcommon::FileName &fileName)
  const
{
  PING;
  // // For opacity we can store the associated data value and only have 1 line,
  // // so just save it out directly
  // tfn_reader::TransferFunction output(transferFunctions[tfn_selection].name,
  // 				      std::vector<vec3f>(), rgbaLines[3].line, 0, 1, 1);
  
  // // Pull the RGB line values to compute the transfer function and save it out
  // // here we may need to do some interpolation, if the RGB lines have differing numbers
  // // of control points
  // // Find which x values we need to sample to get all the control points for the tfcn.
  // std::vector<float> controlPoints;
  // for (size_t i = 0; i < 3; ++i) {
  //   for (const auto &x : rgbaLines[i].line)
  //     controlPoints.push_back(x.x);
  // }

  // // Filter out same or within epsilon control points to get unique list
  // std::sort(controlPoints.begin(), controlPoints.end());
  // auto uniqueEnd = std::unique(controlPoints.begin(), controlPoints.end(),
  // 			       [](const float &a, const float &b) { return std::abs(a - b) < 0.0001; });
  // controlPoints.erase(uniqueEnd, controlPoints.end());

  // // Step along the lines and sample them
  // std::array<std::vector<vec2f>::const_iterator, 3> lit = {
  //   rgbaLines[0].line.begin(), rgbaLines[1].line.begin(),
  //   rgbaLines[2].line.begin()
  // };

  // for (const auto &x : controlPoints) {
  //   std::array<float, 3> sampleColor;
  //   for (size_t j = 0; j < 3; ++j) {
  //     if (x > (lit[j] + 1)->x)
  // 	++lit[j];

  //     assert(lit[j] != rgbaLines[j].line.end());
  //     const float t = (x - lit[j]->x) / ((lit[j] + 1)->x - lit[j]->x);
  //     // It's hard to click down at exactly 0, so offset a little bit
  //     sampleColor[j] = clamp(lerp(lit[j]->y - 0.001, (lit[j] + 1)->y - 0.001, t));
  //   }
  //   output.rgbValues.push_back(vec3f(sampleColor[0], sampleColor[1], sampleColor[2]));
  // }

  // output.save(fileName);
}

// void TransferFunctionWidget::setColorMap(const bool useOpacity)
// {
//   const auto &colors    = transferFunctions[tfcnSelection].rgbValues;
//   const auto &opacities = transferFunctions[tfcnSelection].opacityValues;

//   for (size_t i = 0; i < 3; ++i)
//     rgbaLines[i].line.clear();

//   const float nColors = static_cast<float>(colors.size()) - 1;
//   for (size_t i = 0; i < colors.size(); ++i) {
//     for (size_t j = 0; j < 3; ++j)
//       rgbaLines[j].line.push_back(vec2f(i / nColors, colors[i][j]));
//   }

//   if (useOpacity && !opacities.empty()) {
//     rgbaLines[3].line.clear();
//     const double valMin = transferFunctions[tfcnSelection].dataValueMin;
//     const double valMax = transferFunctions[tfcnSelection].dataValueMax;
//     // Setup opacity if the colormap has it
//     for (size_t i = 0; i < opacities.size(); ++i) {
//       const float x = (opacities[i].x - valMin) / (valMax - valMin);
//       rgbaLines[3].line.push_back(vec2f(x, opacities[i].y));
//     }
//   }
//   fcnChanged = true;
// }

// void TransferFunctionWidget::loadColorMapPresets()
// {
//   std::vector<vec3f> colors;
//   // The presets have no existing opacity value
//   const std::vector<vec2f> opacity;
//   // From the old volume viewer, these are based on ParaView
//   // Jet transfer function
//   colors.push_back(vec3f(0       , 0, 0.562493));
//   colors.push_back(vec3f(0       , 0, 1       ));
//   colors.push_back(vec3f(0       , 1, 1       ));
//   colors.push_back(vec3f(0.500008, 1, 0.500008));
//   colors.push_back(vec3f(1       , 1, 0       ));
//   colors.push_back(vec3f(1       , 0, 0       ));
//   colors.push_back(vec3f(0.500008, 0, 0       ));
//   transferFunctions.emplace_back("Jet", colors, opacity, 0, 1, 1);
//   colors.clear();

//   colors.push_back(vec3f(0        , 0          , 0          ));
//   colors.push_back(vec3f(0        , 0.120394   , 0.302678   ));
//   colors.push_back(vec3f(0        , 0.216587   , 0.524575   ));
//   colors.push_back(vec3f(0.0552529, 0.345022   , 0.659495   ));
//   colors.push_back(vec3f(0.128054 , 0.492592   , 0.720287   ));
//   colors.push_back(vec3f(0.188952 , 0.641306   , 0.792096   ));
//   colors.push_back(vec3f(0.327672 , 0.784939   , 0.873426   ));
//   colors.push_back(vec3f(0.60824  , 0.892164   , 0.935546   ));
//   colors.push_back(vec3f(0.881376 , 0.912184   , 0.818097   ));
//   colors.push_back(vec3f(0.9514   , 0.835615   , 0.449271   ));
//   colors.push_back(vec3f(0.904479 , 0.690486   , 0          ));
//   colors.push_back(vec3f(0.854063 , 0.510857   , 0          ));
//   colors.push_back(vec3f(0.777096 , 0.330175   , 0.000885023));
//   colors.push_back(vec3f(0.672862 , 0.139086   , 0.00270085 ));
//   colors.push_back(vec3f(0.508812 , 0          , 0          ));
//   colors.push_back(vec3f(0.299413 , 0.000366217, 0.000549325));
//   colors.push_back(vec3f(0.0157473, 0.00332647 , 0          ));
//   transferFunctions.emplace_back("Ice Fire", colors, opacity, 0, 1, 1);
//   colors.clear();

//   colors.push_back(vec3f(0.231373, 0.298039 , 0.752941));
//   colors.push_back(vec3f(0.865003, 0.865003 , 0.865003));
//   colors.push_back(vec3f(0.705882, 0.0156863, 0.14902));
//   transferFunctions.emplace_back("Cool Warm", colors, opacity, 0, 1, 1);
//   colors.clear();

//   colors.push_back(vec3f(0, 0, 1));
//   colors.push_back(vec3f(1, 0, 0));
//   transferFunctions.emplace_back("Blue Red", colors, opacity, 0, 1, 1);
//   colors.clear();

//   colors.push_back(vec3f(0));
//   colors.push_back(vec3f(1));
//   transferFunctions.emplace_back("Grayscale", colors, opacity, 0, 1, 1);
//   colors.clear();


// }// ::ospray

