#include "MaterialLibrary.h"


#include "../shading/material.h"
#include "../resources/resources.h"

#include <t1_utility/logging/log.h>
#include <t1_utility/string_id.h>

namespace T1
{
    // --------------------------------------------------------------------------------------------
    MaterialLibrary::MaterialLibrary()
    {
        generateDefaultMaterials();
        generateInternalMaterials();
    }
    // --------------------------------------------------------------------------------------------
    MaterialLibrary::~MaterialLibrary()
    {
        for (auto it = m_DefaultMaterials.begin(); it != m_DefaultMaterials.end(); ++it)
        {
            delete it->second;
        }
        for (unsigned int i = 0; i < m_Materials.size(); ++i)
        {
            delete m_Materials[i];
        }
    }
    // --------------------------------------------------------------------------------------------
    Material* MaterialLibrary::CreateMaterial(std::string base)
    {
        auto found = m_DefaultMaterials.find(SID(base));
        if (found != m_DefaultMaterials.end())
        {
            Material copy = found->second->Copy();
            Material* mat = new Material(copy);
            m_Materials.push_back(mat); // a bit ugly for now, come up with a bettermemory management scheme for materials
            return mat;
        }
        else
        {
            Log::Message("Material of template: " + base + " requested, but template did not exist.", LOG_ERROR);
            return nullptr;
        }
    }
    // --------------------------------------------------------------------------------------------
    Material* MaterialLibrary::CreateCustomMaterial(Shader* shader)
    {
        Material* mat = new Material(shader);
        mat->Type = MATERIAL_CUSTOM;
        m_Materials.push_back(mat);
        return mat;
    }
    // --------------------------------------------------------------------------------------------    
    void MaterialLibrary::generateDefaultMaterials()
    {
        // default render material (forward for now)
        Shader* defaultShader = Resources::LoadShader("default", "shaders/forward_render.vs", "shaders/forward_render.fs");
        Material* defaultMat = new Material(defaultShader);
        defaultMat->Type = MATERIAL_CUSTOM;
        defaultMat->SetTexture("TexAlbedo", Resources::LoadTexture("default albedo", "textures/checkerboard.png", GL_TEXTURE_2D, GL_RGB), 0);
        defaultMat->SetTexture("TexNormal", Resources::LoadTexture("default normal", "textures/norm.png"), 1);
        defaultMat->SetTexture("TexMetallic", Resources::LoadTexture("default metallic", "textures/black.png"), 2);
        defaultMat->SetTexture("TexRoughness", Resources::LoadTexture("default roughness", "textures/checkerboard.png"), 3);
        m_DefaultMaterials[SID("default")] = defaultMat;
    }
    // --------------------------------------------------------------------------------------------
    void MaterialLibrary::generateInternalMaterials()
    {
    }
}