#version 420 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPositionMetallic;
uniform sampler2D gNormalRoughness;
uniform sampler2D gAlbedoAO;

void main()
{
    // extract data from GBuffer
    vec4 albedoAO         = texture(gAlbedoAO, TexCoords);
    vec4 normalRoughness  = texture(gNormalRoughness, TexCoords);
    vec4 positionMetallic = texture(gPositionMetallic, TexCoords);
    float ao = 1.0;
    
    vec3 albedo = albedoAO.rgb;

    // constant ambient term until there's proper IBL
    FragColor.rgb = albedo * 0.03 * ao;
    FragColor.a = 1.0;
}
