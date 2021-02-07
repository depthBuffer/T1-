#include "MaterialLibrary.h"

#include "render_target.h"

#include "../shading/material.h"
#include "../resources/resources.h"

#include <t1_utility/logging/log.h>
#include <t1_utility/string_id.h>

namespace T1
{
    // --------------------------------------------------------------------------------------------
    MaterialLibrary::MaterialLibrary(RenderTarget* gBuffer)
    {
        generateDefaultMaterials();
        generateInternalMaterials(gBuffer);
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
        delete debugLightMaterial;

        delete defaultBlitMaterial;
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
        // default render material (deferred path)
        Shader* defaultShader = Resources::LoadShader("default", "shaders/deferred/g_buffer.vs", "shaders/deferred/g_buffer.fs");
        Material* defaultMat = new Material(defaultShader);
        defaultMat->Type = MATERIAL_DEFAULT;
        defaultMat->SetTexture("TexAlbedo", Resources::LoadTexture("default albedo", "textures/checkerboard.png", GL_TEXTURE_2D, GL_RGB), 3);
        defaultMat->SetTexture("TexNormal", Resources::LoadTexture("default normal", "textures/norm.png"), 4);
        defaultMat->SetTexture("TexMetallic", Resources::LoadTexture("default metallic", "textures/black.png"), 5);
        defaultMat->SetTexture("TexRoughness", Resources::LoadTexture("default roughness", "textures/checkerboard.png"), 6);
        m_DefaultMaterials[SID("default")] = defaultMat;
    }
    // --------------------------------------------------------------------------------------------
    void MaterialLibrary::generateInternalMaterials(RenderTarget *gBuffer)
    {
        // post-processing
        Shader* defaultBlitShader = T1::Resources::LoadShader("blit", "shaders/screen_quad.vs", "shaders/default_blit.fs");
        defaultBlitMaterial = new Material(defaultBlitShader);
        
        // deferred
        deferredAmbientShader     = T1::Resources::LoadShader("deferred ambient", "shaders/deferred/screen_ambient.vs", "shaders/deferred/ambient.fs");
        deferredDirectionalShader = T1::Resources::LoadShader("deferred directional", "shaders/deferred/screen_directional.vs", "shaders/deferred/directional.fs");
        deferredPointShader       = T1::Resources::LoadShader("deferred point", "shaders/deferred/point.vs", "shaders/deferred/point.fs");

        deferredAmbientShader->Use();
        deferredAmbientShader->SetInt("gPositionMetallic", 0);
        deferredAmbientShader->SetInt("gNormalRoughness", 1);
        deferredAmbientShader->SetInt("gAlbedoAO", 2);
        deferredDirectionalShader->Use();
        deferredDirectionalShader->SetInt("gPositionMetallic", 0);
        deferredDirectionalShader->SetInt("gNormalRoughness", 1);
        deferredDirectionalShader->SetInt("gAlbedoAO", 2);
        deferredPointShader->Use();
        deferredPointShader->SetInt("gPositionMetallic", 0);
        deferredPointShader->SetInt("gNormalRoughness", 1);
        deferredPointShader->SetInt("gAlbedoAO", 2);

        // debug
        Shader *debugLightShader = Resources::LoadShader("debug light", "shaders/light.vs", "shaders/light.fs");
        debugLightMaterial = new Material(debugLightShader);
    }
}