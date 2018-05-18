#version 330


in vec2 fragTextureCoord;
in vec4 fragColor;

uniform sampler2D texture0;

layout(location = 0) out vec4 colorOut;

void main()
{
    colorOut = fragColor * texture(texture0,fragTextureCoord);
    //gl_FragColor = vec4(1,1,1,1);
}
