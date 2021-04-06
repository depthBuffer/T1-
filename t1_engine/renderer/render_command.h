#ifndef CELL_RENDERER_RENDER_COMMAND_H
#define CELL_RENDERER_RENDER_COMMAND_H

#include <t1_math/linear_algebra/matrix.h>

namespace T1
{
    class Mesh;
    class Material;

    /* 

      All render state required for submitting a render command.

    */
    struct RenderCommand
    {
        math::mat4 Transform;
        math::mat4 PrevTransform;
        Mesh*      Mesh;
        Material*  Material;
        math::vec3 BoxMin;
        math::vec3 BoxMax;
    };
}

#endif