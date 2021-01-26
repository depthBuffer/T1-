#ifndef T1_RENDERER_COMMAND_BUFFER_H
#define T1_RENDERER_COMMAND_BUFFER_H

#include "render_command.h"

#include <t1_math/linear_algebra/matrix.h>

#include <map>
#include <vector>

namespace T1
{
    class Renderer;
    class Mesh;
    class Material;

    /* 

      Render command buffer, managing all per-frame render/draw calls and converting them to a 
      (more efficient) render-friendly format for the renderer to execute.

    */
    class CommandBuffer
    {
    public:

    private:
        Renderer* m_Renderer;

        std::vector<RenderCommand> m_CustomRenderCommands;


    public:
        CommandBuffer(Renderer* renderer);
        ~CommandBuffer(); 
            
        // pushes render state relevant to a single render call to the command buffer.
        void Push(Mesh* mesh, Material* material, math::mat4 transform = math::mat4(), math::mat4 prevTransform = math::mat4());

        // clears the command buffer; usually done after issuing all the stored render commands.
        void Clear();
        // sorts the command buffer; first by shader, then by texture bind.
        void Sort();

        // returns the list of custom render commands per render target.
        std::vector<RenderCommand> GetCustomRenderCommands();
    };
}

#endif