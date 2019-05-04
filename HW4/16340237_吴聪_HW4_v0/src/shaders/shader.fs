#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D ourTexture;
uniform vec3 myColor;

void main()
{
    FragColor = texture(ourTexture, TexCoord) * vec4(myColor, 1.0);
} 