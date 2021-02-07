#version 420 core
out vec4 FragColor;

in vec3 FragPos;
in vec4 ScreenPos;

#include ../common/uniforms.glsl

uniform sampler2D gPositionMetallic;
uniform sampler2D gNormalRoughness;
uniform sampler2D gAlbedoAO;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightRadius;

void main()
{
    vec2 uv = (ScreenPos.xy / ScreenPos.w) * 0.5 + 0.5;
    
    vec4 albedoAO = texture(gAlbedoAO, uv);
    vec4 normalRoughness = texture(gNormalRoughness, uv);
    vec4 positionMetallic = texture(gPositionMetallic, uv);
    
    vec3 worldPos   = positionMetallic.xyz;
    vec3 albedo     = albedoAO.rgb;
    vec3 normal     = normalRoughness.rgb;
    float roughness = normalRoughness.a;
    float metallic  = positionMetallic.a;
       
    // lighting input
    vec3 N = normalize(normal);
    vec3 V = normalize(camPos.xyz - worldPos); 
    vec3 L = normalize(lightPos - worldPos);
    vec3 H = normalize(V + L);     
	              
          
    // calculate light radiance
    float attenuation = max(0.95 - length(worldPos - lightPos) / lightRadius, 0.0);
    vec3 radiance = lightColor * attenuation;        
        
    // blinn-phong for now, pbr later
    float diff = max(dot(N, L), 0.0);
    float spec = pow(max(dot(N, H), 0.0), 64.0) * (1.0 - roughness);
    vec3 Lo = (albedo * diff + vec3(spec)) * radiance;
    
    FragColor.rgb = Lo;
    FragColor.a = 1.0;
}