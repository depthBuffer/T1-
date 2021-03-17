#ifndef T1_H
#define T1_H

#include "renderer/renderer.h"
#include "renderer/render_target.h"
#include "renderer/pbr_capture.h"
#include "camera/fly_camera.h"
#include "resources/resources.h"
#include "mesh/mesh.h"
#include "mesh/quad.h"
#include "mesh/circle.h"
#include "mesh/line_strip.h"
#include "mesh/cube.h"
#include "mesh/plane.h"
#include "mesh/sphere.h"
#include "mesh/torus.h"
#include "shading/shader.h"
#include "shading/material.h"
#include "shading/texture.h"
#include "shading/texture_cube.h"
#include "scene/scene.h"
#include "scene/scene_node.h"
#include "scene/background.h"

#include "glad/glad.h"

class GLFWwindow;

namespace T1
{
    /* 

      Global entry-point for T1's initialization. Starts all T1's relevant initialization 
      calls, and makes sure T1 is ready to operate after calling its initialization function. 
      Note that all GL function pointers are initialized here as well (including any extensions) 
      w/ GLAD.

    */
    Renderer* Init(GLFWwindow* window, GLADloadproc loadProcFunc);


    // cleans up T1 of all resources that are no longer relevant.
    void Clean();

    /* 

      Initializes all render data required for processing a new frame. This is mostly relevant
      for rendering GUI items.

    */
    void NewFrame();

    /* 

      Renders (debugging) GUI (IMGUI) on top of T1's main renderer, by default the renderer's 
      configuration UI is rendered as well, which can be enabled/disabled on demand.

    */
    void RenderGUI();

    // pass mouse input to T1
    void InputMouse(int button, int action);

    // pass keyboard input to T1
    void InputKey(int key, int action);

    // pass scroll input to T1
    void InputScroll(float scrollOffset);
}

#endif