#include <iostream>

#include <t1_math/t1_math.h>
#include <t1_utility/logging/log.h>
#include <t1_engine/t1_engine.h>

#include <t1_utility/timing/diagnostics.h>

#include <GLFW/glfw3.h>

void keyFunc(GLFWwindow *window, int key, int scancode, int action, int mods);

bool keysPressed[1024];
bool keysActive[1024];
bool wireframe = false;

/* NOTE:

  This is a demo program set up to demonstrate the graphic fidelity
  of T1 and demonstrates one way of configuring and using T1
  from a developer's perspective.

*/
int main(int argc, char *argv[])
{
    /* 

      Initialize the windowing library.

      T1 is a graphics engine independent of a windowing system. 
      This is to keep T1 cross-platform and independent of any
      windowing system developers prefer to use. T1 does expect
      an OpenGL context set up before initializing T1.

      Here we use GLFW(3) to initialize a window, initialize an
      OpenGL context and in this case also initialize all OpenGL
      function pointers. Note that loading function pointers is
      something we want T1 to manage in the future as well.

    */
    Log::Message("Initializing GLFW", LOG_INIT);
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    
        GLFWwindow* window = glfwCreateWindow(1920, 1080, "T1", nullptr, nullptr);           
        if (window == nullptr)
        {
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);

        // register callbacks
        glfwSetKeyCallback(window, keyFunc);

    Log::Message("GLFW initialized", LOG_INIT);

    Log::Message("Initializing render system", LOG_INIT);
        T1::Init((GLADloadproc)glfwGetProcAddress);
    Log::Message("Render system initialized", LOG_INIT);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwSwapBuffers(window);
    }

    // clean up T1
    T1::Clean();

    glfwTerminate();

    return 0;
}

void keyFunc(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keysPressed[key] = true;
        else if (action == GLFW_RELEASE)
        {
            keysPressed[key] = false;
            keysActive[key] = false;
        }
    }
}
