#include "renderer.h"

#include "render_target.h"
#include "MaterialLibrary.h"

#include "../mesh/mesh.h"
#include "../mesh/cube.h"
#include "../shading/material.h"
#include "../scene/scene.h"
#include "../scene/scene_node.h"
#include "../camera/camera.h"
#include "../camera/fly_camera.h"
#include "../resources/resources.h"

#include <t1_math/linear_algebra/vector.h>
#include <t1_math/linear_algebra/matrix.h>

#include <t1_utility/logging/log.h>
#include <t1_utility/string_id.h>

#include <stack>

namespace T1
{
    // ------------------------------------------------------------------------
    Renderer::Renderer()
    {

    }
    // ------------------------------------------------------------------------
    Renderer::~Renderer()
    {       
        delete m_CommandBuffer;

        delete m_NDCPlane;

        delete m_MaterialLibrary;

        delete m_CustomTarget;
    }
    // ------------------------------------------------------------------------
    void Renderer::Init(GLADloadproc loadProcFunc)
    {
        // initialize render items
        m_CommandBuffer = new CommandBuffer(this);

        // configure default OpenGL state
        m_GLCache.SetDepthTest(true);
        m_GLCache.SetCull(true);

        glViewport(0.0f, 0.0f, m_RenderSize.x, m_RenderSize.y);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0f);

        m_NDCPlane = new Quad;

        m_CustomTarget       = new RenderTarget(1, 1, GL_HALF_FLOAT, 1, true);

        // materials
        m_MaterialLibrary = new MaterialLibrary();

        // ubo
        glGenBuffers(1, &m_GlobalUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, m_GlobalUBO);
        glBufferData(GL_UNIFORM_BUFFER, 720, nullptr, GL_STREAM_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_GlobalUBO);
    }
    // ------------------------------------------------------------------------
    void Renderer::SetRenderSize(unsigned int width, unsigned int height)
    {
        m_RenderSize.x = width;
        m_RenderSize.y = height;

        m_CustomTarget->Resize(width, height);
    }
    // ------------------------------------------------------------------------
    math::vec2 Renderer::GetRenderSize()
    {
        return m_RenderSize;
    }
    // ------------------------------------------------------------------------
    void Renderer::SetTarget(RenderTarget* renderTarget, GLenum target)
    {
        m_CurrentRenderTargetCustom = renderTarget;
        if (renderTarget != nullptr) 
        {
            if (std::find(m_RenderTargetsCustom.begin(), m_RenderTargetsCustom.end(), renderTarget)
                == m_RenderTargetsCustom.end())
            {
                m_RenderTargetsCustom.push_back(renderTarget);
            }
        }
    }
    // ------------------------------------------------------------------------
    Camera* Renderer::GetCamera()
    {
        return m_Camera;
    }
    // ------------------------------------------------------------------------
    void Renderer::SetCamera(Camera* camera)
    {
        m_Camera = camera;
    }
    // ------------------------------------------------------------------------
    Material* Renderer::CreateMaterial(std::string base)
    {
        return m_MaterialLibrary->CreateMaterial(base);      
    }
    // ------------------------------------------------------------------------
    Material* Renderer::CreateCustomMaterial(Shader *shader)
    {
        return m_MaterialLibrary->CreateCustomMaterial(shader);
    }
    // ------------------------------------------------------------------------
    void Renderer::PushRender(Mesh* mesh, Material* material, math::mat4 transform, math::mat4 prevFrameTransform)
    {
        // get current render target
        RenderTarget* target = getCurrentRenderTarget();
        // don't render right away but push to the command buffer for later rendering.
        m_CommandBuffer->Push(mesh, material, transform, prevFrameTransform, target);
    }
    // ------------------------------------------------------------------------
    void Renderer::PushRender(SceneNode* node)
    {
        // update transform(s) before pushing node to render command buffer
        node->UpdateTransform(true);

        // get current render target
        RenderTarget* target = getCurrentRenderTarget();
        // traverse through all the scene nodes and for each node: push its render state to the 
        // command buffer together with a calculated transform matrix.
        std::stack<SceneNode*> nodeStack;
        nodeStack.push(node);
        for (unsigned int i = 0; i < node->GetChildCount(); ++i)
            nodeStack.push(node->GetChildByIndex(i));
        while (!nodeStack.empty())
        {
            SceneNode* node = nodeStack.top();
            nodeStack.pop();
            // only push render command if the child isn't a container node.
            if (node->Mesh)
            {
                 m_CommandBuffer->Push(node->Mesh, node->Material, node->GetTransform(), node->GetPrevTransform(), target);
            }
            for(unsigned int i = 0; i < node->GetChildCount(); ++i)
                nodeStack.push(node->GetChildByIndex(i));
        }
    }
    // ------------------------------------------------------------------------
    void Renderer::AddLight(DirectionalLight* light)
    {
        m_DirectionalLights.push_back(light);
    }
    // ------------------------------------------------------------------------
    void Renderer::AddLight(PointLight* light)
    {
        m_PointLights.push_back(light);
    }
    // ------------------------------------------------------------------------
    void Renderer::RenderPushedCommands()
    {      
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // sort all pushed render commands by heavy state-switches e.g. shader switches.
        m_CommandBuffer->Sort();

        // update (global) uniform buffers
        updateGlobalUBOs();

        // set default GL state
        m_GLCache.SetBlend(false);
        m_GLCache.SetCull(true);
        m_GLCache.SetCullFace(GL_BACK);
        m_GLCache.SetDepthTest(true);
        m_GLCache.SetDepthFunc(GL_LESS);

        glBindFramebuffer(GL_FRAMEBUFFER, m_CustomTarget->ID);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 6. custom forward render pass 
        // push default render target to the end of the render target buffer s.t. we always render 
        // the default buffer last.
        m_RenderTargetsCustom.push_back(nullptr);        
        for (unsigned int targetIndex = 0; targetIndex < m_RenderTargetsCustom.size(); ++targetIndex)
        {
            RenderTarget *renderTarget = m_RenderTargetsCustom[targetIndex];
            if (renderTarget)
            {
                glViewport(0, 0, renderTarget->Width, renderTarget->Height);
                glBindFramebuffer(GL_FRAMEBUFFER, renderTarget->ID);
                if (renderTarget->HasDepthAndStencil)
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                else
                    glClear(GL_COLOR_BUFFER_BIT);
                m_Camera->SetPerspective(m_Camera->FOV, 
                                         (float)renderTarget->Width / (float)renderTarget->Height, 
                                         0.1, 100.0f); 
            }
            else
            {
                // don't render to default framebuffer, but to custom target framebuffer which 
                // we'll use for post-processing.
                glViewport(0, 0, m_RenderSize.x, m_RenderSize.y);
                glBindFramebuffer(GL_FRAMEBUFFER, m_CustomTarget->ID);
                m_Camera->SetPerspective(m_Camera->FOV, m_RenderSize.x / m_RenderSize.y, 0.1, 
                                         100.0f);
            }

            // sort all render commands and retrieve the sorted array
            std::vector<RenderCommand> renderCommands = m_CommandBuffer->GetCustomRenderCommands(renderTarget);

            // terate over all the render commands and execute
            for (unsigned int i = 0; i < renderCommands.size(); ++i)
            {
                renderCustomCommand(&renderCommands[i], nullptr);
            }
        }

        // blit to default framebuffer
        Blit(m_CustomTarget->GetColorTexture(0));

        // clear the command buffer s.t. the next frame/call can start from an empty slate again.
        m_CommandBuffer->Clear();

        // clear render state
        m_RenderTargetsCustom.clear();
        m_CurrentRenderTargetCustom = nullptr;
    }
    // ------------------------------------------------------------------------
    void Renderer::Blit(Texture*      src,
                        RenderTarget* dst, 
                        Material*     material, 
                        std::string   textureUniformName)
    {
        // if a destination target is given, bind to its framebuffer
        if (dst)
        {
            glViewport(0, 0, dst->Width, dst->Height);
            glBindFramebuffer(GL_FRAMEBUFFER, dst->ID);
            if (dst->HasDepthAndStencil)
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            else
                glClear(GL_COLOR_BUFFER_BIT);
        }
        // else we bind to the default framebuffer
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, m_RenderSize.x, m_RenderSize.y);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }
        // if no material is given, use default blit material
        if (!material)
        {
            material = m_MaterialLibrary->defaultBlitMaterial;
        }
        // if a source render target is given, use its color buffer as input to material shader.
        if (src)
        {
            material->SetTexture(textureUniformName, src, 0);
        }
        // render screen-space material to quad which will be displayed in dst's buffers.
        RenderCommand command;
        command.Material = material;
        command.Mesh = m_NDCPlane;
        renderCustomCommand(&command, nullptr);
    }   
    // ------------------------------------------------------------------------
    void Renderer::renderCustomCommand(RenderCommand* command, Camera* customCamera, bool updateGLSettings)
    {
        Material *material = command->Material;
        Mesh     *mesh     = command->Mesh;

        // update global GL blend state based on material
        if (updateGLSettings)
        {
            m_GLCache.SetBlend(material->Blend);
            if(material->Blend)
            {
                m_GLCache.SetBlendFunc(material->BlendSrc, material->BlendDst);
            }
            m_GLCache.SetDepthFunc(material->DepthCompare);
            m_GLCache.SetDepthTest(material->DepthTest);
            m_GLCache.SetCull(material->Cull);
            m_GLCache.SetCullFace(material->CullFace);
        }

        // default uniforms that are always configured regardless of shader configuration (see them 
        // as a default set of shader uniform variables always there); with UBO
        material->GetShader()->Use();
        material->GetShader()->SetMatrix("model", command->Transform);

        // bind/active uniform sampler/texture objects
        auto* samplers = material->GetSamplerUniforms();
        for (auto it = samplers->begin(); it != samplers->end(); ++it)
        {
            if (it->second.Type == SHADER_TYPE_SAMPLERCUBE)
                it->second.TextureCube->Bind(it->second.Unit);
            else
                it->second.Texture->Bind(it->second.Unit);
        }

        // set uniform state of material
        auto* uniforms = material->GetUniforms();
        for (auto it = uniforms->begin(); it != uniforms->end(); ++it)
        {
            switch (it->second.Type)
            {
            case SHADER_TYPE_BOOL:
                material->GetShader()->SetBool(it->first, it->second.Bool);
                break;
            case SHADER_TYPE_INT:
                material->GetShader()->SetInt(it->first, it->second.Int);
                break;
            case SHADER_TYPE_FLOAT:
                material->GetShader()->SetFloat(it->first, it->second.Float);
                break;
            case SHADER_TYPE_VEC2:
                material->GetShader()->SetVector(it->first, it->second.Vec2);
                break;
            case SHADER_TYPE_VEC3:
                material->GetShader()->SetVector(it->first, it->second.Vec3);
                break;
            case SHADER_TYPE_VEC4:
                material->GetShader()->SetVector(it->first, it->second.Vec4);
                break;
            case SHADER_TYPE_MAT2:
                material->GetShader()->SetMatrix(it->first, it->second.Mat2);
                break;
            case SHADER_TYPE_MAT3:
                material->GetShader()->SetMatrix(it->first, it->second.Mat3);
                break;
            case SHADER_TYPE_MAT4:
                material->GetShader()->SetMatrix(it->first, it->second.Mat4);
                break;
            default:
                Log::Message("Unrecognized Uniform type set.", LOG_ERROR);
                break;
            }
        }

        renderMesh(mesh, material->GetShader());
    }
    // --------------------------------------------------------------------------------------------
    void Renderer::renderMesh(Mesh* mesh, Shader* shader)
    {
        glBindVertexArray(mesh->m_VAO);
        if (mesh->Indices.size() > 0)
        {
            glDrawElements(mesh->Topology == TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, mesh->Indices.size(), GL_UNSIGNED_INT, 0);
        }
        else
        {
            glDrawArrays(mesh->Topology == TRIANGLE_STRIP ? GL_TRIANGLE_STRIP : GL_TRIANGLES, 0, mesh->Positions.size());
        }
    }
    // --------------------------------------------------------------------------------------------
    void Renderer::updateGlobalUBOs()
    {
        glBindBuffer(GL_UNIFORM_BUFFER, m_GlobalUBO);
        // transformation matrices
        glBufferSubData(GL_UNIFORM_BUFFER,   0, sizeof(math::mat4), &(m_Camera->Projection * m_Camera->View)[0][0]); // sizeof(math::mat4) = 64 bytes
        glBufferSubData(GL_UNIFORM_BUFFER, 128, sizeof(math::mat4), &m_Camera->Projection[0][0]);
        glBufferSubData(GL_UNIFORM_BUFFER, 192, sizeof(math::mat4), &m_Camera->View[0][0]);
        glBufferSubData(GL_UNIFORM_BUFFER, 256, sizeof(math::mat4), &m_Camera->View[0][0]); // TODO: make inv function in math library
        // scene data
        glBufferSubData(GL_UNIFORM_BUFFER, 320, sizeof(math::vec4), &m_Camera->Position[0]);
        // lighting
        unsigned int stride = 2 * sizeof(math::vec4);
        for (unsigned int i = 0; i < m_DirectionalLights.size() && i < 4; ++i) // no more than 4 directional lights
        {
            glBufferSubData(GL_UNIFORM_BUFFER, 336 + i * stride,                      sizeof(math::vec4), &m_DirectionalLights[i]->Direction[0]);
            glBufferSubData(GL_UNIFORM_BUFFER, 336 + i * stride + sizeof(math::vec4), sizeof(math::vec4), &m_DirectionalLights[i]->Color[0]);
        }
        for (unsigned int i = 0; i < m_PointLights.size() && i < 8; ++i) //  constrained to max 8 point lights in forward context
        {
            glBufferSubData(GL_UNIFORM_BUFFER, 464 + i * stride,                      sizeof(math::vec4), &m_PointLights[i]->Position[0]);
            glBufferSubData(GL_UNIFORM_BUFFER, 464 + i * stride + sizeof(math::vec4), sizeof(math::vec4), &m_PointLights[i]->Color[0]);
        }
    }
    // --------------------------------------------------------------------------------------------
    RenderTarget* Renderer::getCurrentRenderTarget()
    {
        return m_CurrentRenderTargetCustom;
    }
}