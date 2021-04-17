#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#include <t1_math/t1_math.h>
#include <t1_utility/logging/log.h>
#include <t1_engine/t1_engine.h>

#include <t1_utility/random/random.h>

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

// showcase state (driven by hotkeys and the "Foundry Yard" GUI window)
bool  lightShow      = false;
int   timeOfDay      = 1;     // 0 = day, 1 = dusk, 2 = night

struct SunPreset
{
    math::vec3 Direction;
    math::vec3 Color;
    float      Intensity;
};
const SunPreset sunPresets[3] = {
    { math::vec3( 0.30f, -1.00f,  0.20f), math::vec3(1.00f, 0.95f, 0.85f), 40.0f }, // day
    { math::vec3( 0.75f, -0.40f, -0.35f), math::vec3(1.00f, 0.55f, 0.25f), 30.0f }, // dusk: low sun from the west
    { math::vec3( 0.40f, -0.80f, -0.30f), math::vec3(0.35f, 0.45f, 1.00f),  2.0f }, // night
};
const char* timeOfDayNames[3] = { "Day", "Dusk", "Night" };

struct OrbitLight
{
    T1::PointLight Light;
    float Radius;
    float Height;
    float Phase;
    float Speed;
    float Bob;
};

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

/* NOTE:

  The engine's procedural meshes (plane, cube, sphere, torus) don't generate tangent space
  vectors (Mesh::calculateTangents() is an unfinished stub), and the deferred G-buffer shader needs
  them for normal mapping; without them those meshes shade black. This helper derives per-vertex
  tangents/bitangents from each triangle's positions and UVs and re-uploads the mesh.

*/
void GenerateTangents(T1::Mesh& mesh)
{
    const size_t vertexCount = mesh.Positions.size();
    if (vertexCount == 0 || mesh.UV.size() != vertexCount || mesh.Normals.size() != vertexCount)
        return;

    std::vector<math::vec3> tangents(vertexCount, math::vec3(0.0f));
    std::vector<math::vec3> bitangents(vertexCount, math::vec3(0.0f));

    auto accumulate = [&](unsigned int i0, unsigned int i1, unsigned int i2) {
        if (i0 == i1 || i1 == i2 || i0 == i2) // degenerate (e.g. triangle strip restarts)
            return;
        math::vec3 e1 = mesh.Positions[i1] - mesh.Positions[i0];
        math::vec3 e2 = mesh.Positions[i2] - mesh.Positions[i0];
        math::vec2 d1 = mesh.UV[i1] - mesh.UV[i0];
        math::vec2 d2 = mesh.UV[i2] - mesh.UV[i0];
        float det = d1.x * d2.y - d2.x * d1.y;
        if (std::fabs(det) < 1e-8f)
            return;
        float r = 1.0f / det;
        math::vec3 t = (e1 * d2.y - e2 * d1.y) * r;
        math::vec3 b = (e2 * d1.x - e1 * d2.x) * r;
        tangents[i0]   = tangents[i0] + t;   tangents[i1]   = tangents[i1] + t;   tangents[i2]   = tangents[i2] + t;
        bitangents[i0] = bitangents[i0] + b; bitangents[i1] = bitangents[i1] + b; bitangents[i2] = bitangents[i2] + b;
    };

    const std::vector<unsigned int>& idx = mesh.Indices;
    if (mesh.Topology == T1::TRIANGLE_STRIP)
    {
        for (size_t i = 0; i + 2 < idx.size(); ++i)
            accumulate(idx[i], idx[i + 1], idx[i + 2]);
    }
    else
    {
        for (size_t i = 0; i + 2 < idx.size(); i += 3)
            accumulate(idx[i], idx[i + 1], idx[i + 2]);
    }

    // orthonormalize against the normal (Gram-Schmidt) and keep the bitangent's handedness
    for (size_t i = 0; i < vertexCount; ++i)
    {
        math::vec3 n = math::normalize(mesh.Normals[i]);
        math::vec3 t = tangents[i] - n * math::dot(n, tangents[i]);
        if (math::length(t) < 1e-6f) // no usable UV gradient (e.g. sphere poles): pick any perpendicular
            t = math::cross(n, std::fabs(n.y) < 0.99f ? math::vec3(0.0f, 1.0f, 0.0f) : math::vec3(1.0f, 0.0f, 0.0f));
        t = math::normalize(t);
        math::vec3 b = math::cross(n, t);
        if (math::dot(b, bitangents[i]) < 0.0f)
            b = b * -1.0f;
        tangents[i]   = t;
        bitangents[i] = b;
    }

    mesh.SetTangents(tangents, bitangents);
    mesh.Finalize();
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

    // cameras: the fly camera starts at the open west side of the yard, looking at the gyroscope
    float aspect = renderer->GetRenderSize().x / renderer->GetRenderSize().y;
    camera.SetPerspective(math::Deg2Rad(60.0f), aspect, 0.1f, 100.0f);
    camera.Yaw   =   0.0f;
    camera.Pitch = -10.0f;
    camera.InputMouse(0.0f, -100.0f); // target pitch of -10 degrees (mouse delta * sensitivity)

    // meshes
    T1::Plane  floorTile(16, 16);
    T1::Cube   cube;
    T1::Sphere sphere(64, 64);
    T1::Sphere orbSphere(128, 128);
    T1::Torus  ringOuter(2.20f, 0.15f, 64, 16);
    T1::Torus  ringMiddle(1.75f, 0.13f, 64, 16);
    T1::Torus  ringInner(1.30f, 0.11f, 64, 16);

    // The engine's plane mesh has its normals facing +Z while its triangle winding (front face)
    // faces -Z. The deferred pass always culls back faces (material cull settings only apply to
    // forward-rendered materials), so flip the floor's normals to match its front face; the
    // tiles are then rotated to face up (see the yard setup below).
    for (math::vec3& normal : floorTile.Normals)
        normal = normal * -1.0f;

    T1::Mesh* normalMapped[] = { &floorTile, &cube, &sphere, &ringOuter, &ringMiddle, &ringInner };
    for (T1::Mesh* mesh : normalMapped)
        GenerateTangents(*mesh);

    // materials
    T1::Material* matRust    = CreatePBRMaterial("rusted metal", true);
    T1::Material* matGold    = CreatePBRMaterial("gold", true);
    T1::Material* matCopper  = CreatePBRMaterial("copper", false);
    T1::Material* matPlastic = CreatePBRMaterial("plastic", true);

    T1::Material* matCrate = renderer->CreateMaterial();
    matCrate->SetTexture("TexAlbedo",    T1::Resources::LoadTexture("crate albedo", "textures/crate.png", GL_TEXTURE_2D, GL_RGB, true), 3);
    matCrate->SetTexture("TexNormal",    T1::Resources::LoadTexture("flat normal", "textures/norm.png"), 4);
    matCrate->SetTexture("TexMetallic",  T1::Resources::LoadTexture("black", "textures/black.png"), 5);
    matCrate->SetTexture("TexRoughness", T1::Resources::LoadTexture("white", "textures/white.png"), 6);
    matCrate->SetTexture("TexAO",        T1::Resources::LoadTexture("white", "textures/white.png"), 7);

    T1::Material* matChecker = renderer->CreateMaterial(); // engine default: checkerboard albedo/roughness
    matChecker->SetTexture("TexAO", T1::Resources::LoadTexture("white", "textures/white.png"), 7);

    T1::Material* matGlass = renderer->CreateMaterial("glass");

    T1::Shader*   plasmaOrbShader = T1::Resources::LoadShader("plasma orb", "shaders/custom/plasma_orb.vs", "shaders/custom/plasma_orb.fs");
    T1::Material* matPlasmaOrb    = renderer->CreateCustomMaterial(plasmaOrbShader);
    matPlasmaOrb->Cull     = false;
    matPlasmaOrb->Blend    = true;
    matPlasmaOrb->BlendSrc = GL_ONE;
    matPlasmaOrb->BlendDst = GL_ONE;
    matPlasmaOrb->SetTexture("TexPerllin", T1::Resources::LoadTexture("perlin noise", "textures/perlin.png"), 0);
    matPlasmaOrb->SetFloat("Strength", 1.2f);
    matPlasmaOrb->SetFloat("Speed", 0.1f);

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
    // a loose stack of crates near the entrance, with a glass cube resting on top
    addNode(&cube, matCrate, math::vec3(-8.5f, 0.75f, 6.5f), math::vec3(1.5f), math::vec4(math::vec3(0.0f, 1.0f, 0.0f), 0.30f));
    addNode(&cube, matCrate, math::vec3(-8.3f, 0.75f, 8.2f), math::vec3(1.5f), math::vec4(math::vec3(0.0f, 1.0f, 0.0f), -0.15f));
    addNode(&cube, matCrate, math::vec3(-8.4f, 2.25f, 7.3f), math::vec3(1.5f), math::vec4(math::vec3(0.0f, 1.0f, 0.0f), 0.10f));
    addNode(&cube, matGlass, math::vec3(-8.4f, 3.50f, 7.3f), math::vec3(1.0f), math::vec4(math::vec3(0.0f, 1.0f, 0.0f), 0.60f));
    addNode(&cube, matCrate, math::vec3(-8.0f, 0.75f, -7.0f), math::vec3(1.5f), math::vec4(math::vec3(0.0f, 1.0f, 0.0f), -0.40f));

    /*

      Centre piece: a raised platform with a gyroscope of three nested rings (gold, copper and
      plastic) spinning around a plasma orb. The rings form a small scene-graph hierarchy so each
      ring's rotation compounds on top of its parent's.

    */
    addNode(&cube, matRust, math::vec3(0.0f, 0.2f, 0.0f), math::vec3(5.0f, 0.4f, 5.0f));
    const float corner = 1.9f;
    addNode(&cube, matGlass, math::vec3( corner, 0.8f,  corner), math::vec3(0.8f));
    addNode(&cube, matGlass, math::vec3(-corner, 0.8f,  corner), math::vec3(0.8f));
    addNode(&cube, matGlass, math::vec3( corner, 0.8f, -corner), math::vec3(0.8f));
    addNode(&cube, matGlass, math::vec3(-corner, 0.8f, -corner), math::vec3(0.8f));

    const math::vec3 gyroCenter(0.0f, 3.2f, 0.0f);
    T1::SceneNode* gyroOuter  = T1::Scene::MakeSceneNode(&ringOuter,  matGold);
    T1::SceneNode* gyroMiddle = T1::Scene::MakeSceneNode(&ringMiddle, matCopper);
    T1::SceneNode* gyroInner  = T1::Scene::MakeSceneNode(&ringInner,  matPlastic);
    gyroOuter->AddChild(gyroMiddle);
    gyroMiddle->AddChild(gyroInner);
    gyroOuter->SetPosition(gyroCenter);

    addNode(&orbSphere, matPlasmaOrb, gyroCenter, math::vec3(0.55f));

    /*

      Material gallery: six pedestals in a ring around the platform, each showing one material on
      a sphere and lit by its own flickering lantern.

    */
    T1::Material* galleryMaterials[6] = { matGold, matCopper, matPlastic, matRust, matGlass, matChecker };
    std::vector<T1::PointLight> lanterns(6);
    std::vector<float>          lanternBaseIntensity(6);
    for (int i = 0; i < 6; ++i)
    {
        float angle = math::Deg2Rad(30.0f + 60.0f * i);
        math::vec3 dir(std::cos(angle), 0.0f, std::sin(angle));
        math::vec3 base = dir * 7.0f;

        addNode(&cube,   matRust,             base + math::vec3(0.0f, 0.7f, 0.0f),  math::vec3(1.0f, 1.4f, 1.0f));
        addNode(&sphere, galleryMaterials[i], base + math::vec3(0.0f, 2.05f, 0.0f), math::vec3(0.6f));

        T1::PointLight& lantern = lanterns[i];
        lantern.Position   = base - dir * 1.4f + math::vec3(0.0f, 2.8f, 0.0f);
        lantern.Color      = math::vec3(1.0f, 0.62f, 0.28f);
        lantern.Radius     = 3.5f;
        lantern.Intensity  = 30.0f;
        lantern.RenderMesh = true;
        lanternBaseIntensity[i] = lantern.Intensity;
    }

    // background: the engine's pre-filtered HDR sky capture
    T1::Background* background = new T1::Background;
    background->SetCubemap(renderer->GetSkypCature()->Prefiltered);
    background->Material->SetFloat("lodLevel", 1.5f);

    /*

      Lighting. Light objects are referenced by pointer from the renderer, so all containers are
      fully populated before any light is registered and never resized afterwards.

    */
    T1::DirectionalLight sun;
    sun.Direction = math::normalize(sunPresets[timeOfDay].Direction);
    sun.Color     = sunPresets[timeOfDay].Color;
    sun.Intensity = sunPresets[timeOfDay].Intensity;
    renderer->AddLight(&sun);

    for (int i = 0; i < (int)lanterns.size(); ++i)
        renderer->AddLight(&lanterns[i]);

    // fireflies: a few dozen small coloured lights orbiting the gyroscope
    std::vector<OrbitLight> fireflies(24);
    for (int i = 0; i < (int)fireflies.size(); ++i)
    {
        OrbitLight& f = fireflies[i];
        f.Radius = 3.2f + Random::Uniliteral() * 2.5f;
        f.Height = 0.8f + Random::Uniliteral() * 4.0f;
        f.Phase  = Random::Uniliteral() * 2.0f * PI;
        f.Speed  = (0.25f + Random::Uniliteral() * 0.5f) * (i % 2 == 0 ? 1.0f : -1.0f);
        f.Bob    = 0.3f + Random::Uniliteral() * 0.6f;
        f.Light.Color      = math::vec3(0.3f + Random::Uniliteral() * 0.7f, 0.3f + Random::Uniliteral() * 0.7f, 0.3f + Random::Uniliteral() * 0.7f);
        f.Light.Radius     = 1.5f + Random::Uniliteral() * 1.0f;
        f.Light.Intensity  = 15.0f + Random::Uniliteral() * 25.0f;
        f.Light.RenderMesh = true;
    }
    for (int i = 0; i < (int)fireflies.size(); ++i)
        renderer->AddLight(&fireflies[i].Light);

    // light show: 128 extra lights sweeping over the whole yard (toggle with L)
    std::vector<OrbitLight> showLights(128);
    for (int i = 0; i < (int)showLights.size(); ++i)
    {
        OrbitLight& s = showLights[i];
        s.Radius = 2.0f + Random::Uniliteral() * 8.5f;
        s.Height = 0.4f + Random::Uniliteral() * 3.0f;
        s.Phase  = Random::Uniliteral() * 2.0f * PI;
        s.Speed  = (0.1f + Random::Uniliteral() * 0.4f) * (i % 3 == 0 ? -1.0f : 1.0f);
        s.Bob    = 0.2f + Random::Uniliteral() * 0.8f;
        s.Light.Color  = math::vec3(Random::Uniliteral(), Random::Uniliteral(), Random::Uniliteral());
        s.Light.Radius = 1.0f + Random::Uniliteral() * 2.0f;
    }
    for (int i = 0; i < (int)showLights.size(); ++i)
        renderer->AddLight(&showLights[i].Light);

    /*

      Bake irradiance probes for diffuse global illumination: an inner ring around the platform,
      an outer ring between the gallery and the crate walls, and one above the gyroscope.

    */
    {
        for (int i = 0; i < 8; ++i)
        {
            float angle = 2.0f * PI * i / 8.0f;
            renderer->AddIrradianceProbe(math::vec3(std::cos(angle) * 4.5f, 1.5f, std::sin(angle) * 4.5f), 4.5f);
            renderer->AddIrradianceProbe(math::vec3(std::cos(angle) * 9.5f, 1.8f, std::sin(angle) * 9.5f), 5.0f);
        }
        renderer->AddIrradianceProbe(math::vec3(0.0f, 5.5f, 0.0f), 6.0f);

        renderer->BakeProbes();
    }

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
        if (pressedOnce(GLFW_KEY_L))   lightShow    = !lightShow;
        if (pressedOnce(GLFW_KEY_N))   timeOfDay    = (timeOfDay + 1) % 3;

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

        // sun: ease towards the selected time-of-day preset
        {
            const SunPreset& preset = sunPresets[timeOfDay];
            float blend = math::clamp01(deltaTime * 1.5f);
            math::vec3 targetDir = math::normalize(preset.Direction);
            math::vec3 targetCol = preset.Color;
            sun.Direction = math::normalize(math::lerp(sun.Direction, targetDir, blend));
            sun.Color     = math::lerp(sun.Color, targetCol, blend);
            sun.Intensity = math::lerp(sun.Intensity, preset.Intensity, blend);
        }

        // lanterns flicker
        for (int i = 0; i < (int)lanterns.size(); ++i)
        {
            float flicker = std::sin(t * 7.3f + i * 1.7f) * 0.5f + std::sin(t * 13.1f + i * 4.1f) * 0.3f;
            lanterns[i].Intensity = lanternBaseIntensity[i] * (1.0f + 0.25f * flicker);
        }

        // fireflies orbit and bob around the gyroscope
        for (int i = 0; i < (int)fireflies.size(); ++i)
        {
            OrbitLight& f = fireflies[i];
            float a = f.Phase + t * f.Speed;
            f.Light.Position = math::vec3(std::cos(a) * f.Radius, f.Height + std::sin(t * 1.3f + f.Phase) * f.Bob, std::sin(a) * f.Radius);
        }

        // light show lights; when disabled they are parked far below the yard so they get culled
        for (int i = 0; i < (int)showLights.size(); ++i)
        {
            OrbitLight& s = showLights[i];
            if (lightShow)
            {
                float a = s.Phase + t * s.Speed;
                s.Light.Position   = math::vec3(std::cos(a) * s.Radius, s.Height + std::sin(t * 0.9f + s.Phase) * s.Bob, std::sin(a) * s.Radius);
                s.Light.Intensity  = 20.0f + 15.0f * std::sin(t * 2.0f + s.Phase);
                s.Light.RenderMesh = true;
            }
            else
            {
                s.Light.Position   = math::vec3(0.0f, -500.0f, 0.0f);
                s.Light.Intensity  = 0.0f;
                s.Light.RenderMesh = false;
            }
        }

        // gyroscope rings and plasma orb
        gyroOuter->SetRotation(math::vec4(math::vec3(1.0f, 0.0f, 0.0f), t * 0.7f));
        gyroMiddle->SetRotation(math::vec4(math::vec3(0.0f, 1.0f, 0.0f), t * 1.1f));
        gyroInner->SetRotation(math::vec4(math::vec3(1.0f, 0.0f, 0.0f), t * 1.6f));
        matPlasmaOrb->SetFloat("Time", t);

        // push render commands
        for (int i = 0; i < (int)renderables.size(); ++i)
            renderer->PushRender(renderables[i]);

        // the gyroscope rings are pushed individually from their (hierarchical) world transforms
        gyroOuter->UpdateTransform(true);
        renderer->PushRender(gyroOuter->Mesh,  gyroOuter->Material,  gyroOuter->GetTransform(),  gyroOuter->GetPrevTransform());
        renderer->PushRender(gyroMiddle->Mesh, gyroMiddle->Material, gyroMiddle->GetTransform(), gyroMiddle->GetPrevTransform());
        renderer->PushRender(gyroInner->Mesh,  gyroInner->Material,  gyroInner->GetTransform(),  gyroInner->GetPrevTransform());

        renderer->PushRender(background);

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
