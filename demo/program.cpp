#include <iostream>

#include <t1_math/t1_math.h>
#include <t1_utility/logging/log.h>
#include <t1_engine/t1_engine.h>
#include <t1_engine/imgui/imgui.h>

#include <t1_utility/timing/diagnostics.h>

#include <GLFW/glfw3.h>

void framebufferSizeFunc(GLFWwindow *window, int width, int height);
void keyFunc(GLFWwindow *window, int key, int scancode, int action, int mods);
void mousePosFunc(GLFWwindow *window, double xpos, double ypos);
void mouseButtonFunc(GLFWwindow*, int button, int action, int mods);
void mouseScrollFunc(GLFWwindow*, double xoffset, double yoffset);

T1::Renderer* renderer;
T1::FlyCamera camera(math::vec3(0.0f, 1.0f, 5.0f), math::vec3(0.0f, 0.0f, -1.0f));
float deltaTime     = 0.0f;
float lastFrameTime = 0.0f;
bool keysPressed[1024];
bool keysActive[1024];
bool wireframe = false;
bool renderGUI = false;

/* NOTE:

  This is a demo program set up to demonstrate the graphic fidelity
  of T1 and demonstrates one way of configuring and using T1
  from a developer's perspective.

  The demo program also uses Dear IMGUI to add a basic layer of 
  user input interaction.

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
        glfwWindowHint(GLFW_RESIZABLE, true);

    
        GLFWwindow* window = glfwCreateWindow(1920, 1080, "T1", nullptr, nullptr);           
        if (window == nullptr)
        {
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

        // register callbacks
        glfwSetFramebufferSizeCallback(window, framebufferSizeFunc);
        glfwSetKeyCallback(window, keyFunc);
        glfwSetCursorPosCallback(window, mousePosFunc);
        glfwSetMouseButtonCallback(window, mouseButtonFunc);
        glfwSetScrollCallback(window, mouseScrollFunc);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
    Log::Message("GLFW initialized", LOG_INIT);

    Log::Message("Initializing render system", LOG_INIT);
        renderer = T1::Init(window, (GLADloadproc)glfwGetProcAddress);
        renderer->SetRenderSize(width, height);
        renderer->SetCamera(&camera);
    Log::Message("Render system initialized", LOG_INIT);

    // basic shapes
    T1::Plane plane(16, 16);
    T1::Sphere sphere(64, 64);
    T1::Torus torus(2.0f, 0.4f, 32, 32);
    T1::Cube cube;

    // material setup
    T1::Material* matPbr = renderer->CreateMaterial();
    // configure camera
    camera.SetPerspective(math::Deg2Rad(60.0f), renderer->GetRenderSize().x / renderer->GetRenderSize().y ,0.1f, 100.0f);

    // scene setup
    T1::SceneNode* mainTorus   = T1::Scene::MakeSceneNode(&torus, matPbr);
    T1::SceneNode* secondTorus = T1::Scene::MakeSceneNode(&torus, matPbr);
    T1::SceneNode* thirdTorus  = T1::Scene::MakeSceneNode(&torus, matPbr);

    mainTorus->AddChild(secondTorus);
    secondTorus->AddChild(thirdTorus);

    mainTorus->SetScale(1.0f);
    mainTorus->SetPosition(math::vec3(0.0f, 0.0f, -3.0));
    secondTorus->SetScale(0.65f);
    secondTorus->SetRotation(math::vec4(0.0, 1.0, 0.0, math::Deg2Rad(90.0)));
    thirdTorus->SetScale(0.65f);

    // lighting
    T1::DirectionalLight dirLight;
    dirLight.Direction = math::vec3(-0.5f, -1.0f, -0.3f);
    dirLight.Color = math::vec3(1.0f, 0.89f, 0.7f);
    dirLight.Intensity = 5.0f;
    renderer->AddLight(&dirLight);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        T1::NewFrame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrameTime = glfwGetTime();
        deltaTime     = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        {
            //CLOCK(UPDATE);
            if (keysPressed[GLFW_KEY_W] || keysPressed[GLFW_KEY_UP])
                camera.InputKey(deltaTime, T1::CAMERA_FORWARD);
            if (keysPressed[GLFW_KEY_S] || keysPressed[GLFW_KEY_DOWN])
                camera.InputKey(deltaTime, T1::CAMERA_BACK);
            if (keysPressed[GLFW_KEY_A] || keysPressed[GLFW_KEY_LEFT])
                camera.InputKey(deltaTime, T1::CAMERA_LEFT);
            if (keysPressed[GLFW_KEY_D] || keysPressed[GLFW_KEY_RIGHT])
                camera.InputKey(deltaTime, T1::CAMERA_RIGHT);
            if (keysPressed[GLFW_KEY_E])
                camera.InputKey(deltaTime, T1::CAMERA_UP);
            if (keysPressed[GLFW_KEY_Q])
                camera.InputKey(deltaTime, T1::CAMERA_DOWN);

            // update render logic
            camera.Update(deltaTime);

            mainTorus->SetRotation(math::vec4(math::vec3(1.0f, 0.0f, 0.0f), glfwGetTime() * 2.0));
            secondTorus->SetRotation(math::vec4(math::vec3(0.0f, 1.0f, 0.0f), glfwGetTime() * 3.0));
            thirdTorus->SetRotation(math::vec4(math::vec3(0.0f, 1.0f, 0.0f), glfwGetTime() * 4.0));
        }

        {
            //CLOCK(PUSH);
            renderer->PushRender(mainTorus);
        }

        {
            //CLOCK(RENDER);
            // request T1 to render all currently pushed commands
            renderer->RenderPushedCommands();
        }

        // GUI
        if (renderGUI)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            T1::RenderGUI();
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        glfwSwapBuffers(window);
    }

    // clean up T1
    T1::Clean();

    glfwTerminate();

    return 0;
}

void framebufferSizeFunc(GLFWwindow *window, int width, int height)
{
    renderer->SetRenderSize(width, height);
    camera.SetPerspective(math::Deg2Rad(60.0f), (float)width / (float)height, 0.1f, 100.0f);
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
    T1::InputKey(key, action);
}

bool firstMouse = true;
float lastX = 640.0f;
float lastY = 360.0f;
void mousePosFunc(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos;  // reversed since y-coordinates go from bottom to left

        lastX = xpos;
        lastY = ypos;
        camera.InputMouse(xoffset, yoffset);
}

void mouseButtonFunc(GLFWwindow*, int button, int action, int mods)
{
    T1::InputMouse(button, action);
}

void mouseScrollFunc(GLFWwindow*, double xoffset, double yoffset)
{
    camera.InputScroll(xoffset, yoffset);
    T1::InputScroll(yoffset);
}