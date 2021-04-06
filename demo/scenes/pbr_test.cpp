#include "pbr_test.h"

#include <t1_engine/shading/material.h>
#include <t1_engine/resources/resources.h>
#include <t1_engine/scene/scene.h>
#include <t1_engine/scene/scene_node.h>
#include <t1_engine/camera/camera.h>

ScenePbrTest::ScenePbrTest(T1::Renderer *renderer, T1::Camera *camera)
{
     m_Renderer = renderer;
     m_Camera = camera;

     m_PbrShader = T1::Resources::LoadShader("pbr", "shaders/pbr.vs", "shaders/pbr.frag");
     m_PBRMaterial.SetShader(m_PbrShader);
}

ScenePbrTest::~ScenePbrTest()
{

}

void ScenePbrTest::Init()
{
    // pre-compute irradiance/prefilter/integrate map  for testing
}

void ScenePbrTest::Update(float dt)
{
    m_Camera->Update(dt);
}

void ScenePbrTest::Render()
{

}