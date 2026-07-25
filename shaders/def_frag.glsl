#version 430 core

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D image;
uniform vec4 mainColor = vec4(1.0);
uniform bool useTexture = false;

void main()
{
    vec4 texColor = vec4(1.0);

    if (useTexture)
        texColor = texture(image, TexCoords) * mainColor;
    else 
        texColor = mainColor;

    FragColor = texColor;
}