#version 420 core
out vec4 FragColor;

in vec2 TexCoords;

#include ../common/uniforms.glsl

uniform sampler2D gPositionMetallic;
uniform sampler2D gNormalRoughness;
uniform sampler2D gAlbedoAO;

uniform vec3 lightDir;
uniform vec3 lightColor;

void main()
{
    vec4 albedoAO = texture(gAlbedoAO, TexCoords);
    vec4 normalRoughness = texture(gNormalRoughness, TexCoords);
    vec4 positionMetallic = texture(gPositionMetallic, TexCoords);
    
    vec3 worldPos   = positionMetallic.xyz;
    vec3 albedo     = albedoAO.rgb;
    vec3 normal     = normalRoughness.rgb;
    float roughness = normalRoughness.a;
    float metallic  = positionMetallic.a;
       
    // lighting input
    vec3 N = normalize(normal);
    vec3 V = normalize(camPos.xyz - worldPos); // view-space camera is (0, 0, 0): (0, 0, 0) - viewPos = -viewPos
    vec3 L = normalize(-lightDir);
    vec3 H = normalize(V + L);     
	              
          
    // calculate light radiance    
    vec3 radiance = lightColor;        
    
    // blinn-phong for now, pbr later
    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 64.0) * (1.0 - roughness);
    vec3 Lo = (albedo * diff + vec3(spec)) * radiance;
        
    FragColor.rgb = Lo;
    FragColor.a = 1.0;
}