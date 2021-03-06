#version 420 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D TexSrc;

void main()
{
    vec3 color = texture(TexSrc, TexCoords).rgb;
    vec2 texelSize = 1.0 / textureSize(TexSrc, 0).xy;    
    
    // HDR tonemapping
    const float exposure = 1.5;
    color *= exposure;
    color = color / (color + vec3(1.0));
	// gamma correct
	color = pow(color, vec3(1.0/2.2));     
    

    FragColor = vec4(color, 1.0);
}