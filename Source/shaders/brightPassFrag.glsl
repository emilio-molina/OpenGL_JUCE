#version 330


in vec2 fragTextureCoord;
in vec4 fragColor;

uniform sampler2D texture0;

float brightPassThreshold = 0.2f; // lower threshold

layout(location = 0) out vec4 colorOut;

void main()
{

    vec3 luminanceVector = vec3(0.2125, 0.7154, 0.0721);
    vec4 c = texture(texture0, fragTextureCoord) * fragColor * 1.4;

    float luminance = dot(luminanceVector, c.xyz);
    luminance = max(0.0, luminance - brightPassThreshold);
    c.xyz *= sign(luminance);
    c.a = 1.0;

    colorOut = c * 1.4f;

    /*vec4 color = texture(texture,fragTextureCoord);
    float brightness = (color.r * 0.2126) + (color.g * 0.7152) + (color.b * 0.0722);
    //gl_FragColor = color * brightness;
    if ( brightness > 0.5 )
    {
        gl_FragColor = color;
    }
    else
    {
        gl_FragColor = vec4(0.0);
    }*/
}
