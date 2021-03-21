#ifndef SCENE_PBR_TEST_H
#define SCENE_PBR_TEST_H

#include <t1_engine/renderer/renderer.h>
#include <t1_engine/shading/material.h>
#include <t1_engine/shading/shader.h>

class ScenePbrTest 
{
private:
    T1::Renderer *m_Renderer;
    T1::Camera   *m_Camera;

    T1::Material m_PBRMaterial;
    T1::Material m_PBRIrradianceMaterial;
    T1::Material m_PBRPrefilterMaterial;
    T1::Material m_PBRBRDFIntegrateMaterial;
    T1::Shader   *m_PbrShader;
    T1::Shader   *m_PbrIrradianceShader;
    T1::Shader   *m_PbrPrefilterShader;
    T1::Shader   *m_PbrBRDFintegrateShader;

public:
    // pass over relevant data per scene: dependancy injection
    ScenePbrTest(T1::Renderer *renderer, T1::Camera *camera);
    ~ScenePbrTest();

    void Init();

    void Update(float dt);
    void Render();
};

#endif