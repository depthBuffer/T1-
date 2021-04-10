#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#include <t1_math/t1_math.h>
#include <t1_utility/logging/log.h>
#include <t1_engine/t1_engine.h>

#include <GLFW/glfw3.h>

void framebufferSizeFunc(GLFWwindow *window, int width, int height);
void keyFunc(GLFWwindow *window, int key, int scancode, int action, int mods);
void mousePosFunc(GLFWwindow *window, double xpos, double ypos);
void mouseButtonFunc(GLFWwindow*, int button, int action, int mods);
void mouseScrollFunc(GLFWwindow*, double xoffset, double yoffset);

const float PI = 3.14159265f;

T1::Renderer* renderer;
T1::FlyCamera camera(math::vec3(-15.0f, 3.5f, 0.0f), math::vec3(1.0f, 0.0f, 0.0f));
float deltaTime     = 0.0f;
float lastFrameTime = 0.0f;
bool keysPressed[1024];
bool keysActive[1024];
bool renderGUI     = false;
bool cursorVisible = true;  // forces the cursor to be captured on the first frame
bool firstMouse    = true;

// creates a deferred PBR material from one of the texture sets in textures/pbr/<folder>/
T1::Material* CreatePBRMaterial(const std::string& folder, bool hasAO)
{
    std::string path = "textures/pbr/" + folder + "/";
    T1::Material* mat = renderer->CreateMaterial();
    mat->SetTexture("TexAlbedo",    T1::Resources::LoadTexture(folder + " albedo",    path + "albedo.png", GL_TEXTURE_2D, GL_RGB, true), 3);
    mat->SetTexture("TexNormal",    T1::Resources::LoadTexture(folder + " normal",    path + "normal.png"), 4);
    mat->SetTexture("TexMetallic",  T1::Resources::LoadTexture(folder + " metallic",  path + "metallic.png"), 5);
    mat->SetTexture("TexRoughness", T1::Resources::LoadTexture(folder + " roughness", path + "roughness.png"), 6);
    if (hasAO)
        mat->SetTexture("TexAO", T1::Resources::LoadTexture(folder + " ao", path + "ao.png"), 7);
    else
        mat->SetTexture("TexAO", T1::Resources::LoadTexture("white", "textures/white.png"), 7);
    return mat;
}

int main(int argc, char *argv[])
{
    /*

      Window and context setup; identical to the main demo. T1 expects an OpenGL 4.3 core
      context to be current before it is initialized.

    */
    Log::Message("Initializing GLFW", LOG_INIT);
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
        glfwWindowHint(GLFW_RESIZABLE, true);

        GLFWwindow* window = glfwCreateWindow(1920, 1080, "T1 - Foundry Yard", nullptr, nullptr);
        if (window == nullptr)
        {
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);

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

    // camera starts at the open west side of the yard
    float aspect = renderer->GetRenderSize().x / renderer->GetRenderSize().y;
    camera.SetPerspective(math::Deg2Rad(60.0f), aspect, 0.1f, 100.0f);
    camera.Yaw   =   0.0f;
    camera.Pitch = -10.0f;
    camera.InputMouse(0.0f, -100.0f); // target pitch of -10 degrees (mouse delta * sensitivity)

    // meshes
    T1::Plane  floorTile(16, 16);
    T1::Cube   cube;

    // materials
    T1::Material* matRust    = CreatePBRMaterial("rusted metal", true);

    T1::Material* matCrate = renderer->CreateMaterial();
    matCrate->SetTexture("TexAlbedo",    T1::Resources::LoadTexture("crate albedo", "textures/crate.png", GL_TEXTURE_2D, GL_RGB, true), 3);
    matCrate->SetTexture("TexNormal",    T1::Resources::LoadTexture("flat normal", "textures/norm.png"), 4);
    matCrate->SetTexture("TexMetallic",  T1::Resources::LoadTexture("black", "textures/black.png"), 5);
    matCrate->SetTexture("TexRoughness", T1::Resources::LoadTexture("white", "textures/white.png"), 6);
    matCrate->SetTexture("TexAO",        T1::Resources::LoadTexture("white", "textures/white.png"), 7);

    // all top-level nodes we push to the renderer every frame
    std::vector<T1::SceneNode*> renderables;
    auto addNode = [&renderables](T1::Mesh* mesh, T1::Material* material, math::vec3 position, math::vec3 scale, math::vec4 rotation = math::vec4(0.0f, 1.0f, 0.0f, 0.0f)) {
        T1::SceneNode* node = T1::Scene::MakeSceneNode(mesh, material);
        node->SetPosition(position);
        node->SetScale(scale);
        node->SetRotation(rotation);
        renderables.push_back(node);
        return node;
    };

    /*

      The yard: a 24x24 floor of rusted metal tiles (the plane mesh lies in XY, so each tile is
      rotated onto the XZ plane), enclosed on three sides by walls of stacked crates. The west
      side is left open as the entrance.

      A +90 degree rotation about X maps the tile's front face (local -Z, which now also holds
      its flipped normals) onto +Y, so the floor is front-facing and lit from above.

    */
    for (int x = 0; x < 6; ++x)
    {
        for (int z = 0; z < 6; ++z)
        {
            addNode(&floorTile, matRust, math::vec3(-10.0f + 4.0f * x, 0.0f, -10.0f + 4.0f * z), math::vec3(2.0f),
                    math::vec4(math::vec3(1.0f, 0.0f, 0.0f), PI * 0.5f));
        }
    }

    const float crateSize = 1.5f;
    for (int side = 0; side < 3; ++side)
    {
        for (int i = 0; i < 14; ++i)
        {
            float along  = -9.75f + crateSize * i;
            int   stack  = 1 + (i * 7 + side * 5) % 3;
            for (int level = 0; level < stack; ++level)
            {
                float yaw = math::Deg2Rad((float)((i * 37 + level * 17 + side * 11) % 13 - 6));
                math::vec3 pos;
                if (side == 0)      pos = math::vec3(along, 0.0f, -11.25f); // north wall
                else if (side == 1) pos = math::vec3(along, 0.0f,  11.25f); // south wall
                else                pos = math::vec3(11.25f, 0.0f, along);  // east wall
                pos.y = crateSize * 0.5f + crateSize * level;
                addNode(&cube, matCrate, pos, math::vec3(crateSize), math::vec4(math::vec3(0.0f, 1.0f, 0.0f), yaw));
            }
        }
    }

    /*

      Lighting. Light objects are referenced by pointer from the renderer, so all containers are
      fully populated before any light is registered and never resized afterwards.

    */
    T1::DirectionalLight sun;
    sun.Direction = math::normalize(math::vec3(0.75f, -0.40f, -0.35f));
    sun.Color     = math::vec3(1.0f, 0.55f, 0.25f);
    sun.Intensity = 30.0f;
    renderer->AddLight(&sun);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        T1::NewFrame();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrameTime = (float)glfwGetTime();
        deltaTime     = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        const float t = currentFrameTime;

        // hotkeys (edge triggered)
        auto pressedOnce = [](int key) {
            if (keysPressed[key] && !keysActive[key])
            {
                keysActive[key] = true;
                return true;
            }
            return false;
        };
        if (pressedOnce(GLFW_KEY_TAB)) renderGUI    = !renderGUI;

        // camera
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
        camera.Update(deltaTime);

        // push render commands
        for (int i = 0; i < (int)renderables.size(); ++i)
            renderer->PushRender(renderables[i]);

        renderer->RenderPushedCommands();

        // capture the mouse for camera control only while the GUI is hidden; when the cursor gets
        // (re)captured GLFW reports a jump in position, so restart the mouse delta tracking
        if (renderGUI != cursorVisible)
        {
            cursorVisible = renderGUI;
            glfwSetInputMode(window, GLFW_CURSOR, cursorVisible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }

        // GUI
        if (renderGUI)
        {
            T1::RenderGUI();
        }

        glfwSwapBuffers(window);
    }

    T1::Clean();
    glfwTerminate();

    return 0;
}

void framebufferSizeFunc(GLFWwindow *window, int width, int height)
{
    if (width == 0 || height == 0)
        return;
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

float lastX = 960.0f;
float lastY = 540.0f;
void mousePosFunc(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;  // reversed since y-coordinates go from bottom to top

    lastX = (float)xpos;
    lastY = (float)ypos;
    if (!renderGUI)
    {
        camera.InputMouse(xoffset, yoffset);
    }
}

void mouseButtonFunc(GLFWwindow*, int button, int action, int mods)
{
    T1::InputMouse(button, action);
}

void mouseScrollFunc(GLFWwindow*, double xoffset, double yoffset)
{
    camera.InputScroll((float)xoffset, (float)yoffset);
    T1::InputScroll((float)yoffset);
}
