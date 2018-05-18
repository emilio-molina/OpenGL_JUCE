#version 330

in vec4 color_out;

layout(location = 0) out vec4 colorOut;

void main()
{
    colorOut =  color_out;
}
