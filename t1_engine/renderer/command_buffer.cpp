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
    // sort render state
    bool renderSortCustom(const RenderCommand &a, const RenderCommand &b)
    {       
        /* 

          We want strict weak ordering, which states that if two objects x and y are equivalent
          then both f(x,y) and f(y,x) should be false. Thus when comparing the object to itself
          the comparison should always equal false.

          We also want to do multiple sort comparisons in a single pass, so we encapsulate all 
          relevant properties inside an n-tuple with n being equal to the number of sort queries
          we want to do. The tuple < comparison operator initially compares its left-most element
          and then works along the next elements of the tuple until an outcome is clear.

          Another (non C++11) alternative is to write out both the < and > case with the == case
          defaulting to false as in:

          if(a1 < b1)
            return true;
          if(b1 > a1)
            return false;
          
          if(a2 < b2)
            return true;
          if(b2 > a2)
            return false;
          [...] and so on for each query you want to perform
          return false;

        */
        return std::make_tuple(a.Material->Blend, a.Material->GetShader()->ID) < 
               std::make_tuple(b.Material->Blend, b.Material->GetShader()->ID);
    }
    bool renderSortShader(const RenderCommand &a, const RenderCommand &b)
    {
        return a.Material->GetShader()->ID < b.Material->GetShader()->ID;
    }
    // --------------------------------------------------------------------------------------------
    void CommandBuffer::Sort()
    {
        std::sort(m_CustomRenderCommands.begin(), m_CustomRenderCommands.end(), renderSortCustom);
    }
    // --------------------------------------------------------------------------------------------
    std::vector<RenderCommand> CommandBuffer::GetCustomRenderCommands()
    {
        return m_CustomRenderCommands;
    }
};