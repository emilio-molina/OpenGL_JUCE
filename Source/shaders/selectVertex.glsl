#version 330

in vec4 vertex;


uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform vec4 color;

out vec4 color_out;

void main()
{
    color_out = color;
    gl_Position =  projectionMatrix * viewMatrix * modelMatrix * vertex;
}
