#version 330


in vec2 fragTextureCoord;
in vec4 fragColor;

uniform sampler2D texture0;

void main()
{
    gl_FragColor = fragColor * texture2D(texture0,fragTextureCoord);
    //gl_FragColor = vec4(1,1,1,1);
}
