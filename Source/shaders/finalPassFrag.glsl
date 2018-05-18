#version 330

in vec2 fragTextureCoord;
in vec4 fragColor;

uniform sampler2D texture0; // base output
uniform sampler2D texture1; // blur output

layout(location = 0) out vec4 colorOut;

void main() {
    vec4 c1 = texture(texture0, fragTextureCoord);
    vec4 c2 = texture(texture1, fragTextureCoord);
    colorOut = c1 + c2;
}
