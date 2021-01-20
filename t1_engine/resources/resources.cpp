#include "resources.h"

#include "shader_loader.h"
#include "texture_loader.h"

#include <t1_utility/string_id.h>
#include <t1_utility/logging/log.h>

#include <stack>
#include <vector>

namespace T1
{
    std::map<unsigned int, Shader>      Resources::m_Shaders      = std::map<unsigned int, Shader>();
    std::map<unsigned int, Texture>     Resources::m_Textures     = std::map<unsigned int, Texture>();
    std::map<unsigned int, TextureCube> Resources::m_TexturesCube = std::map<unsigned int, TextureCube>();
    // --------------------------------------------------------------------------------------------
    void Resources::Init()
    {
        // initialize default assets/resources that should  always be available, regardless of 
        // configuration.        
        Texture placeholderTexture;
    }
    void Resources::Clean()
    {
    }

    // --------------------------------------------------------------------------------------------
    Shader* Resources::LoadShader(std::string name, std::string vsPath, std::string fsPath, std::vector<std::string> defines)
    {
        unsigned int id = SID(name);

        // if shader already exists, return that handle
        if(Resources::m_Shaders.find(id) != Resources::m_Shaders.end())
            return &Resources::m_Shaders[id];

        Shader shader = ShaderLoader::Load(name, vsPath, fsPath, defines);
        Resources::m_Shaders[id] = shader;
        return &Resources::m_Shaders[id];
    }
    // --------------------------------------------------------------------------------------------
    Shader* Resources::GetShader(std::string name)
    {
        unsigned int id = SID(name);

        // if shader exists, return that handle
        if (Resources::m_Shaders.find(id) != Resources::m_Shaders.end())
        {
            return &Resources::m_Shaders[id];
        }
        else
        {
            Log::Message("Requested shader: " + name + " not found!", LOG_WARNING);
            return nullptr;
        }
    }
    // --------------------------------------------------------------------------------------------
    Texture* Resources::LoadTexture(std::string name, std::string path, GLenum target, GLenum format, bool srgb)
    {
        unsigned int id = SID(name);

        // if texture already exists, return that handle
        if (Resources::m_Textures.find(id) != Resources::m_Textures.end())
            return &Resources::m_Textures[id];

        Log::Message("Loading texture file at: " + path + ".", LOG_INIT);

        Texture texture = TextureLoader::LoadTexture(path, target, format, srgb);

        Log::Message("Succesfully loaded: " + path + ".", LOG_INIT);

        // make sure texture got properly loaded
        if (texture.Width > 0)
        {
            Resources::m_Textures[id] = texture;
            return &Resources::m_Textures[id];
        }
        else
        {
            return nullptr;
        }
    }
    // --------------------------------------------------------------------------------------------
    Texture* Resources::GetTexture(std::string name)
    {
        unsigned int id = SID(name);

        // if shader exists, return that handle
        if (Resources::m_Textures.find(id) != Resources::m_Textures.end())
        {
            return &Resources::m_Textures[id];
        }
        else
        {
            Log::Message("Requested texture: " + name + " not found!", LOG_WARNING);
            return nullptr;
        }
    }
    // --------------------------------------------------------------------------------------------
    TextureCube* Resources::LoadTextureCube(std::string name, std::string folder)
    {
        unsigned int id = SID(name);

        // if texture already exists, return that handle
        if (Resources::m_TexturesCube.find(id) != Resources::m_TexturesCube.end())
            return &Resources::m_TexturesCube[id];

        TextureCube texture = TextureLoader::LoadTextureCube(folder);
        Resources::m_TexturesCube[id] = texture;
        return &Resources::m_TexturesCube[id];
    }
    // --------------------------------------------------------------------------------------------
    TextureCube* Resources::GetTextureCube(std::string name)
    {
        unsigned int id = SID(name);

        // if shader exists, return that handle
        if (Resources::m_TexturesCube.find(id) != Resources::m_TexturesCube.end())
        {
            return &Resources::m_TexturesCube[id];
        }
        else
        {
            Log::Message("Requested texture cube: " + name + " not found!", LOG_WARNING);
            return nullptr;
        }
    }
}