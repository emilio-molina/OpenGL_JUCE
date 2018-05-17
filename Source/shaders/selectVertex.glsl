attribute vec4 vertex;


uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform vec4 color;

varying vec4 color_out;

void main()
{
    color_out = color;
    gl_Position =  projectionMatrix * viewMatrix * modelMatrix * vertex;
}
