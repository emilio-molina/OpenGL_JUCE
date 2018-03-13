varying vec4 destinationColour;
varying float lightIntensity;
varying vec2 textureCoordOut;
varying vec4 color;
varying float fogAmount;

uniform sampler2D demoTexture;
void main()
{
    float l = max (0.2, lightIntensity * 0.04);
    vec4 c = destinationColour;
    vec4 colour = vec4 (l * c[0], l * c[1], l * c[2], c[3]);
    float depth = gl_FragCoord.z;
    gl_FragColor =  colour;
}
