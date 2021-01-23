#include "command_buffer.h"

#include "renderer.h"
#include "../shading/material.h"
#include "../mesh/mesh.h"

#include <algorithm>
#include <tuple>

namespace T1
{
    // --------------------------------------------------------------------------------------------
    CommandBuffer::CommandBuffer(Renderer* renderer)
    {
        m_Renderer = renderer;
    }
    // --------------------------------------------------------------------------------------------
    CommandBuffer::~CommandBuffer()
    {
        Clear();
    }
    // --------------------------------------------------------------------------------------------
    void CommandBuffer::Push(Mesh* mesh, Material* material, math::mat4 transform, math::mat4 prevTransform)
    {
        RenderCommand command = {};
        command.Mesh          = mesh;
        command.Material      = material;
        command.Transform     = transform;
        command.PrevTransform = prevTransform;

        m_CustomRenderCommands.push_back(command);
    }
    // --------------------------------------------------------------------------------------------
    void CommandBuffer::Clear()
    {
        m_CustomRenderCommands.clear();
    }
    // --------------------------------------------------------------------------------------------
    std::vector<RenderCommand> CommandBuffer::GetCustomRenderCommands()
    {
        return m_CustomRenderCommands;
    }
};