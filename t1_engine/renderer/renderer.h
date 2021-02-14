#ifndef T1_RENDERER_H
#define T1_RENDERER_H

#include <t1_math/linear_algebra/matrix.h>

#include "../lighting/point_light.h"
#include "../lighting/directional_light.h"
#include "../mesh/quad.h"
#include "command_buffer.h"
#include "gl_cache.h"

#include "../glad/glad.h"

namespace T1
{
    /* 

      Forward declare the types, s.t. we don't need to include the required header files here. 
      Seeing as several objects throughout T1 will link to renderer.h we want to reduce as much 
      unnecesary additional header code as much as possible (saves in compilation times). 

    */
    class Mesh;
    class Material;
    class Scene;
    class SceneNode;
    class Camera;
    class RenderTarget;
    class MaterialLibrary;

    /*

      The main renderer; responsible for maintaining a render buffer queue, providing the front 
      push commands for filling the buffer, sorting the buffer, manage multiple render passes and 
      render the buffer(s) accordingly.

    */
    class Renderer
    {
    public:
        // configuration
        bool Shadows      = true;
        bool Lights       = true;
        bool RenderLights = true;
        bool LightVolumes = false;
        bool Wireframe    = false;
    private:       
        // render state
        CommandBuffer* m_CommandBuffer {nullptr};
        GLCache        m_GLCache;
        math::vec2     m_RenderSize;

        // lighting
        std::vector<DirectionalLight*> m_DirectionalLights;
        std::vector<PointLight*>       m_PointLights;
        RenderTarget* m_GBuffer = nullptr;
        Mesh*         m_DeferredPointMesh;

        // materials
        MaterialLibrary* m_MaterialLibrary{nullptr};

        // camera
        Camera*    m_Camera{nullptr};

        // render-targets/post
        std::vector<RenderTarget*> m_RenderTargetsCustom;
        RenderTarget*              m_CurrentRenderTargetCustom = nullptr;
        RenderTarget*              m_CustomTarget {nullptr};
        Quad*                      m_NDCPlane {nullptr};

        // shadow buffers
        std::vector<RenderTarget*> m_ShadowRenderTargets;
        std::vector<math::mat4>    m_ShadowViewProjections;

        // ubo
        unsigned int m_GlobalUBO;

        // debug
        Mesh* m_DebugLightMesh {nullptr};

    public:
        Renderer();
        ~Renderer();

        void Init(GLADloadproc loadProcFunc);

        void SetRenderSize(unsigned int width, unsigned int height);
        math::vec2 GetRenderSize();

        void SetTarget(RenderTarget* renderTarget, GLenum target = GL_TEXTURE_2D);

        Camera* GetCamera();
        void    SetCamera(Camera* camera);

        // create either a deferred default material (based on default set of materials available (like glass)), or a custom material (with custom you have to supply your own shader)
        Material* CreateMaterial(std::string base = "default"); // these don't have the custom flag set (default material has default state and uses checkerboard texture as albedo (and black metallic, half roughness, purple normal, white ao)
        Material* CreateCustomMaterial(Shader* shader);         // these have the custom flag set (will be rendered in forward pass)

        void PushRender(Mesh* mesh, Material* material, math::mat4 transform = math::mat4(), math::mat4 prevFrameTransform = math::mat4());
        void PushRender(SceneNode* node);

        void AddLight(DirectionalLight *light);
        void AddLight(PointLight       *light);        

        void RenderPushedCommands();

        void Blit(Texture* src, RenderTarget* dst = nullptr, Material* material = nullptr, std::string textureUniformName = "TexSrc");
    private:
        // renderer-specific logic for rendering a custom (forward-pass) command
        void renderCustomCommand(RenderCommand* command, Camera* customCamera, bool updateGLSettings = true);
        // minimal render logic to render a mesh 
        void renderMesh(Mesh* mesh, Shader* shader);
        // updates the global uniform buffer objects
        void updateGlobalUBOs();
        // returns the currently active render target
        RenderTarget* getCurrentRenderTarget();

        // deferred logic:
        // renders all ambient lighting (including indirect IBL)
        void renderDeferredAmbient();
        // render directional light
        void renderDeferredDirLight(DirectionalLight* light);
        // render point light
        void renderDeferredPointLight(PointLight* light);

        // render mesh for shadow buffer generation
        void renderShadowCastCommand(RenderCommand* command, const math::mat4& projection, const math::mat4& view);
    };
}

#endif