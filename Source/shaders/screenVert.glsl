#version 330

in vec4 vertex;
in vec4 color;
in vec2 textureCoord;

uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

out vec2 fragTextureCoord;
out vec4 fragColor;

void main()
{
    fragColor = color;
    fragTextureCoord = textureCoord;
    gl_Position = projectionMatrix * modelMatrix * vertex;
}
