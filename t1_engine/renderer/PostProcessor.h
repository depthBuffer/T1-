#ifndef T1_POST_PROCESSOR_H
#define T1_POST_PROCESSOR_H

#include <vector>

#include <t1_math/linear_algebra/vector.h>
#include <t1_math/linear_algebra/matrix.h>

namespace T1
{
    class Texture;
    class RenderTarget;
    class Shader;
    class Renderer;
    class Camera;

    /*

      Manages and maintains all data and functionality related to end-of-frame post-processing. 
      This doesn't only include post-processing effects like SSAO, Bloom, Vignette, but also 
      includes general functionality like blitting, blurring, HDR and gammac-orrection. 
      The post-processor's multi-pass effects support multiple resolutions and adjusts accordingly 
      when a resolution change occurs.

    */
    class PostProcessor
    {
    public:
        // resulting post-processor intermediate outputs
        Texture* SSAOOutput;

        // toggles
        bool SSAO       = true;

        // ssao
        int SSAOKernelSize = 64;      
    private:
        // global post-process state
        Shader*       m_PostProcessShader;
        RenderTarget* m_RTOutput;

        // ssao
        RenderTarget* m_SSAORenderTarget;
        Shader*       m_SSAOShader;
        Shader*       m_SSAOBlurShader;
        Texture*      m_SSAONoise;
    public:
        PostProcessor(Renderer* renderer);
        ~PostProcessor();

        // updates all render targets to match new render size
        void UpdateRenderSize(unsigned int width, unsigned int height);

        // process stages
        void ProcessPreLighting(Renderer* renderer, RenderTarget* gBuffer, Camera *camera);

        // blit all combined post-processing steps to default framebuffer
        void Blit(Renderer* renderer, Texture* soruce);

    private:
    };
}
#endif