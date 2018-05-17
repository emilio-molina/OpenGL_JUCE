#version 330

in vec2 fragTextureCoord;
in vec4 fragColor;

uniform sampler2D texture0; // base output
uniform sampler2D texture1; // blur output


void main() {
    vec4 c1 = texture2D(texture0, fragTextureCoord);
    vec4 c2 = texture2D(texture1, fragTextureCoord);
    gl_FragColor = c1 + c2;
}
