#include "PostProcessor.h"

#include "render_target.h"
#include "renderer.h"

#include "../resources/resources.h"
#include "../shading/texture.h"
#include "../shading/shader.h"
#include "../camera/camera.h"

#include <t1_utility/logging/log.h>

namespace T1
{
    // --------------------------------------------------------------------------------------------
    PostProcessor::PostProcessor(Renderer* renderer)
    {
        // global post-processing shader
        {
            m_PostProcessShader = T1::Resources::LoadShader("post process", "shaders/screen_quad.vs", "shaders/post_processing.fs");
            m_PostProcessShader->Use();
            m_PostProcessShader->SetInt("TexSrc", 0);
        }
    }
    // --------------------------------------------------------------------------------------------
    PostProcessor::~PostProcessor()
    {
    }
    // --------------------------------------------------------------------------------------------
    void PostProcessor::UpdateRenderSize(unsigned int width, unsigned int height)
    {
        // resize all buffers
    }
    // --------------------------------------------------------------------------------------------
    void PostProcessor::Blit(Renderer* renderer, Texture* source)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, renderer->GetRenderSize().x, renderer->GetRenderSize().y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // bind input texture data
        source->Bind(0);

        // set settings 
        m_PostProcessShader->Use();

        renderer->renderMesh(renderer->m_NDCPlane, m_PostProcessShader);               
    }
}