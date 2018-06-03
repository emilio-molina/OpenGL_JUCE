/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

namespace BinaryData
{

//================== blurPassFrag.glsl ==================
static const unsigned char temp_binary_data_0[] =
"#version 330\r\n"
"\r\n"
"in vec2 fragTextureCoord;\r\n"
"in vec4 fragColor;\r\n"
"\r\n"
"uniform sampler2D texture0;\r\n"
"\r\n"
"//declare uniforms\r\n"
"uniform float resolution;\r\n"
"uniform float radius;\r\n"
"uniform vec3 dir;\r\n"
"\r\n"
"layout(location = 0) out vec4 colorOut;\r\n"
"\r\n"
"void main() {\r\n"
"\t//this will be our RGBA sum\r\n"
"\tvec4 sum = vec4(0.0);\r\n"
"\t\r\n"
"\t//our original texcoord for this fragment\r\n"
"\tvec2 tc = fragTextureCoord;\r\n"
"\t\r\n"
"\t//the amount to blur, i.e. how far off center to sample from \r\n"
"\t//1.0 -> blur by one pixel\r\n"
"\t//2.0 -> blur by two pixels, etc.\r\n"
"\tfloat blur = radius/resolution; \r\n"
"    \r\n"
"\t//the direction of our blur\r\n"
"\t//(1.0, 0.0) -> x-axis blur\r\n"
"\t//(0.0, 1.0) -> y-axis blur\r\n"
"\tfloat hstep = dir.x;\r\n"
"\tfloat vstep = dir.y;\r\n"
"    \r\n"
"\t//apply blurring, using a 9-tap filter with predefined gaussian weights\r\n"
"    \r\n"
"\tsum += texture(texture0, vec2(tc.x - 4.0*blur*hstep, tc.y - 4.0*blur*vstep)) * 0.0162162162;\r\n"
"\tsum += texture(texture0, vec2(tc.x - 3.0*blur*hstep, tc.y - 3.0*blur*vstep)) * 0.0540540541;\r\n"
"\tsum += texture(texture0, vec2(tc.x - 2.0*blur*hstep, tc.y - 2.0*blur*vstep)) * 0.1216216216;\r\n"
"\tsum += texture(texture0, vec2(tc.x - 1.0*blur*hstep, tc.y - 1.0*blur*vstep)) * 0.1945945946;\r\n"
"\t\r\n"
"\tsum += texture(texture0, vec2(tc.x, tc.y)) * 0.2270270270;\r\n"
"\t\r\n"
"\tsum += texture(texture0, vec2(tc.x + 1.0*blur*hstep, tc.y + 1.0*blur*vstep)) * 0.1945945946;\r\n"
"\tsum += texture(texture0, vec2(tc.x + 2.0*blur*hstep, tc.y + 2.0*blur*vstep)) * 0.1216216216;\r\n"
"\tsum += texture(texture0, vec2(tc.x + 3.0*blur*hstep, tc.y + 3.0*blur*vstep)) * 0.0540540541;\r\n"
"\tsum += texture(texture0, vec2(tc.x + 4.0*blur*hstep, tc.y + 4.0*blur*vstep)) * 0.0162162162;\r\n"
"\r\n"
"\t//discard alpha for our simple demo, multiply by vertex color and return\r\n"
"\tcolorOut = fragColor * vec4(sum.rgb, 1.0);\r\n"
"}\r\n";

const char* blurPassFrag_glsl = (const char*) temp_binary_data_0;

//================== brightPassFrag.glsl ==================
static const unsigned char temp_binary_data_1[] =
"#version 330\r\n"
"\r\n"
"\r\n"
"in vec2 fragTextureCoord;\r\n"
"in vec4 fragColor;\r\n"
"\r\n"
"uniform sampler2D texture0;\r\n"
"\r\n"
"float brightPassThreshold = 0.3f; // lower threshold\r\n"
"\r\n"
"layout(location = 0) out vec4 colorOut;\r\n"
"\r\n"
"void main()\r\n"
"{\r\n"
"\r\n"
"    vec3 luminanceVector = vec3(0.2125, 0.7154, 0.0721);\r\n"
"    vec4 c = texture(texture0, fragTextureCoord) * fragColor;\r\n"
"\r\n"
"    float luminance = dot(luminanceVector, c.xyz);\r\n"
"    luminance = max(0.0, luminance - brightPassThreshold);\r\n"
"    c.xyz *= sign(luminance);\r\n"
"    c.a = 1.0;\r\n"
"\r\n"
"    colorOut = c * 1.2f;\r\n"
"\r\n"
"    /*vec4 color = texture(texture,fragTextureCoord);\r\n"
"    float brightness = (color.r * 0.2126) + (color.g * 0.7152) + (color.b * 0.0722);\r\n"
"    //gl_FragColor = color * brightness;\r\n"
"    if ( brightness > 0.5 )\r\n"
"    {\r\n"
"        gl_FragColor = color;\r\n"
"    }\r\n"
"    else\r\n"
"    {\r\n"
"        gl_FragColor = vec4(0.0);\r\n"
"    }*/\r\n"
"}\r\n";

const char* brightPassFrag_glsl = (const char*) temp_binary_data_1;

//================== finalPassFrag.glsl ==================
static const unsigned char temp_binary_data_2[] =
"#version 330\r\n"
"\r\n"
"in vec2 fragTextureCoord;\r\n"
"in vec4 fragColor;\r\n"
"\r\n"
"uniform sampler2D texture0; // base output\r\n"
"uniform sampler2D texture1; // blur output\r\n"
"\r\n"
"layout(location = 0) out vec4 colorOut;\r\n"
"\r\n"
"void main() {\r\n"
"    vec4 c1 = texture(texture0, fragTextureCoord);\r\n"
"    vec4 c2 = texture(texture1, fragTextureCoord);\r\n"
"    colorOut = c1 + c2;\r\n"
"}\r\n";

const char* finalPassFrag_glsl = (const char*) temp_binary_data_2;

//================== fragmentShader.glsl ==================
static const unsigned char temp_binary_data_3[] =
"#version 330\r\n"
"\r\n"
"#define PI 3.14159265\r\n"
"\r\n"
"uniform mat4 projectionMatrix;\r\n"
"uniform mat4 viewMatrix;\r\n"
"uniform mat4 modelMatrix;\r\n"
"\r\n"
"uniform vec4 lightPosition;\r\n"
"uniform vec4 lightColor;\r\n"
"uniform vec4 color;\r\n"
"\r\n"
"in vec2 fragTextureCoord;\r\n"
"in vec3 fragNormal;\r\n"
"in vec3 fragVertex;\r\n"
"\r\n"
"\r\n"
"// Cook Torrance settings\r\n"
"uniform vec3 specularColor = vec3(0.8, 0.8, 0.8);\r\n"
"uniform float fresnel = 3.0f;\r\n"
"uniform float roughness = 0.20f;\r\n"
"\r\n"
"\r\n"
"in vec3 viewDir;\r\n"
"in vec3 lightDir;\r\n"
"uniform vec3 eyePosition;\r\n"
"\r\n"
"layout(location = 0) out vec4 colorOut;\r\n"
"\r\n"
"vec3 CookTorrance()\r\n"
"{\r\n"
"  float k = 0.2;\r\n"
"\tfloat NdotL = max(0, dot(fragNormal, lightDir));\r\n"
"\tfloat Rs = 0.0;\r\n"
"\tif (NdotL > 0) \r\n"
"\t{\r\n"
"\t\tvec3 H = normalize(lightDir + viewDir);\r\n"
"\t\tfloat NdotH = max(0, dot(fragNormal, H));\r\n"
"\t\tfloat NdotV = max(0, dot(fragNormal, viewDir));\r\n"
"\t\tfloat VdotH = max(0, dot(lightDir, H));\r\n"
"\r\n"
"\t\t// Fresnel reflectance\r\n"
"\t\tfloat F = pow(1.0 - VdotH, 5.0);\r\n"
"\t\tF *= (1.0 - fresnel);\r\n"
"\t\tF += fresnel;\r\n"
"\r\n"
"\t\t// Microfacet distribution by Beckmann\r\n"
"\t\tfloat m_squared = roughness * roughness;\r\n"
"\t\tfloat r1 = 1.0 / (4.0 * m_squared * pow(NdotH, 4.0));\r\n"
"\t\tfloat r2 = (NdotH * NdotH - 1.0) / (m_squared * NdotH * NdotH);\r\n"
"\t\tfloat D = r1 * exp(r2);\r\n"
"\r\n"
"\t\t// Geometric shadowing\r\n"
"\t\tfloat two_NdotH = 2.0 * NdotH;\r\n"
"\t\tfloat g1 = (two_NdotH * NdotV) / VdotH;\r\n"
"\t\tfloat g2 = (two_NdotH * NdotL) / VdotH;\r\n"
"\t\tfloat G = min(1.0, min(g1, g2));\r\n"
"\r\n"
"\t\tRs = (F * D * G) / (PI * NdotL * NdotV);\r\n"
"\t}\r\n"
"\r\n"
"\treturn color.rgb * lightColor.rgb * NdotL + lightColor.rgb * specularColor *\r\n"
"\t      NdotL * (k + Rs * (1.0 - k));\r\n"
"}\r\n"
"\r\n"
"vec3 Simple()\r\n"
"{\r\n"
"    vec3 normal = normalize(fragNormal);\r\n"
"\r\n"
"    vec3 direction = normalize(lightPosition.xyz - fragVertex);\r\n"
"    float intensity = max(0.0, dot(direction, fragNormal));\r\n"
"    vec4 curLight = vec4(intensity, intensity, intensity, 1) * lightColor * color;\r\n"
"    vec3 ambient = vec3(0.1);\r\n"
"    return curLight.rgb + ambient;\r\n"
"}\r\n"
"\r\n"
"#define FOG_DENSITY 0.02\r\n"
"\r\n"
"float fogFactorExp2(float density) {\r\n"
"  float dist = distance(eyePosition, fragVertex);\r\n"
"  const float LOG2 = -1.442695;\r\n"
"  float d = density * dist;\r\n"
"  return 1.0 - clamp(exp2(d * d * LOG2), 0.0, 1.0);\r\n"
"}\r\n"
"\r\n"
"vec4 withFogCookTorrance()\r\n"
"{\r\n"
"    vec4 fogColor = vec4(0.1,0.1,0.1,1.0); // white\r\n"
"    float fogAmount = fogFactorExp2(FOG_DENSITY);\r\n"
"\r\n"
"    return mix(vec4(CookTorrance(), color.a), fogColor, fogAmount);\r\n"
"}\r\n"
"\r\n"
"vec4 withoutFogCookTorrance()\r\n"
"{\r\n"
"    return vec4(CookTorrance(), color.a);\r\n"
"}\r\n"
"\r\n"
"vec4 withFogSimple()\r\n"
"{\r\n"
"    vec4 fogColor = vec4(0.1,0.1,0.1,1.0); // white\r\n"
"    float fogAmount = fogFactorExp2(FOG_DENSITY);\r\n"
"\r\n"
"    return mix(vec4(Simple(), color.a), fogColor, fogAmount);\r\n"
"}\r\n"
"\r\n"
"\r\n"
"vec4 withoutFogSimple()\r\n"
"{\r\n"
"    return vec4(Simple(), color.a);\r\n"
"}\r\n"
"\r\n"
"\r\n"
"vec4 normalMap(){\r\n"
"  return vec4(fragNormal.rgb, 1.0);\r\n"
"}\r\n"
"\r\n"
"\r\n"
"vec4 bright(){\r\n"
"  float intensity = max(0.0, dot(viewDir, fragNormal)) * 2.0;\r\n"
"\r\n"
"  return vec4(color.rgb * intensity, 1.0);\r\n"
"}\r\n"
"\r\n"
"\r\n"
"vec4 glow(){\r\n"
"    float u_glow = 0.3f;\r\n"
"\r\n"
"    float shininess = 20.0;\r\n"
"    vec3  normalV = normalize( fragNormal );\r\n"
"    vec3  eyeV    = normalize( -fragVertex );\r\n"
"    vec3  halfV   = normalize( eyeV + normalV );\r\n"
"    float NdotH   = max( 0.0, dot( normalV, halfV ) );\r\n"
"    float glowFac = ( shininess + 2.0 ) * pow( NdotH, shininess ) / ( 2.0 * 3.14159265 );\r\n"
"\r\n"
"    return u_glow * (color * glowFac);\r\n"
"}\r\n"
"\r\n"
"vec4 glowWithFogExperiment(){\r\n"
"    vec4 fogColor = vec4(0.1,0.1,0.1,0.0); // white\r\n"
"    float fogAmount = fogFactorExp2(FOG_DENSITY);\r\n"
"\r\n"
"    return mix(glow(), fogColor, fogAmount);\r\n"
"}\r\n"
"\r\n"
"void main()\r\n"
"{\r\n"
"  colorOut = glowWithFogExperiment();\r\n"
"}\r\n";

const char* fragmentShader_glsl = (const char*) temp_binary_data_3;

//================== screenFrag.glsl ==================
static const unsigned char temp_binary_data_4[] =
"#version 330\r\n"
"\r\n"
"\r\n"
"in vec2 fragTextureCoord;\r\n"
"in vec4 fragColor;\r\n"
"\r\n"
"uniform sampler2D texture0;\r\n"
"\r\n"
"layout(location = 0) out vec4 colorOut;\r\n"
"\r\n"
"void main()\r\n"
"{\r\n"
"    colorOut = fragColor * texture(texture0,fragTextureCoord);\r\n"
"    //gl_FragColor = vec4(1,1,1,1);\r\n"
"}\r\n";

const char* screenFrag_glsl = (const char*) temp_binary_data_4;

//================== screenVert.glsl ==================
static const unsigned char temp_binary_data_5[] =
"#version 330\r\n"
"\r\n"
"in vec4 vertex;\r\n"
"in vec4 color;\r\n"
"in vec2 textureCoord;\r\n"
"\r\n"
"uniform mat4 projectionMatrix;\r\n"
"uniform mat4 modelMatrix;\r\n"
"\r\n"
"out vec2 fragTextureCoord;\r\n"
"out vec4 fragColor;\r\n"
"\r\n"
"void main()\r\n"
"{\r\n"
"    fragColor = color;\r\n"
"    fragTextureCoord = textureCoord;\r\n"
"    gl_Position = projectionMatrix * modelMatrix * vertex;\r\n"
"}\r\n";

const char* screenVert_glsl = (const char*) temp_binary_data_5;

//================== selectFragment.glsl ==================
static const unsigned char temp_binary_data_6[] =
"#version 330\r\n"
"\r\n"
"in vec4 color_out;\r\n"
"\r\n"
"layout(location = 0) out vec4 colorOut;\r\n"
"\r\n"
"void main()\r\n"
"{\r\n"
"    colorOut =  color_out;\r\n"
"}\r\n";

const char* selectFragment_glsl = (const char*) temp_binary_data_6;

//================== selectVertex.glsl ==================
static const unsigned char temp_binary_data_7[] =
"#version 330\r\n"
"\r\n"
"in vec4 vertex;\r\n"
"\r\n"
"\r\n"
"uniform mat4 projectionMatrix;\r\n"
"uniform mat4 viewMatrix;\r\n"
"uniform mat4 modelMatrix;\r\n"
"uniform vec4 color;\r\n"
"\r\n"
"out vec4 color_out;\r\n"
"\r\n"
"void main()\r\n"
"{\r\n"
"    color_out = color;\r\n"
"    gl_Position =  projectionMatrix * viewMatrix * modelMatrix * vertex;\r\n"
"}\r\n";

const char* selectVertex_glsl = (const char*) temp_binary_data_7;

//================== vertexShader.glsl ==================
static const unsigned char temp_binary_data_8[] =
"#version 330\r\n"
"\r\n"
"in vec4 vertex;\r\n"
"in vec4 normal;\r\n"
"in vec2 textureCoord;\r\n"
"\r\n"
"uniform mat4 projectionMatrix;\r\n"
"uniform mat4 viewMatrix;\r\n"
"uniform mat4 modelMatrix;\r\n"
"uniform mat4 normalMatrix;\r\n"
"\r\n"
"out vec2 fragTextureCoord;\r\n"
"out vec3 fragNormal;\r\n"
"out vec3 fragVertex;\r\n"
"\r\n"
"\r\n"
"// Cook Torrance requirements\r\n"
"out vec3 viewDir;\r\n"
"out vec3 lightDir;\r\n"
"uniform vec3 eyePosition;\r\n"
"uniform vec4 lightPosition;\r\n"
"\r\n"
"void main()\r\n"
"{\r\n"
"    mat4 modelView = viewMatrix * modelMatrix;\r\n"
"    //mat4 normalMatrix = transpose(inverse(modelView));\r\n"
"\r\n"
"    fragVertex = vec3(modelView * vertex);\r\n"
"    fragNormal = vec3(normalize(mat3(normalMatrix) * normalize(normal.xyz)));\r\n"
"    fragTextureCoord = textureCoord;\r\n"
"\r\n"
"    viewDir = normalize(eyePosition - fragVertex.xyz);\r\n"
"    lightDir = normalize(lightPosition.xyz - fragVertex.xyz);\r\n"
"\r\n"
"    gl_Position = projectionMatrix * modelView * vertex;\r\n"
"}\r\n";

const char* vertexShader_glsl = (const char*) temp_binary_data_8;


const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)
{
    unsigned int hash = 0;
    if (resourceNameUTF8 != 0)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0x50feddf3:  numBytes = 1723; return blurPassFrag_glsl;
        case 0x9591e640:  numBytes = 848; return brightPassFrag_glsl;
        case 0x4a90bd44:  numBytes = 345; return finalPassFrag_glsl;
        case 0x114811a8:  numBytes = 3549; return fragmentShader_glsl;
        case 0x38f5191f:  numBytes = 261; return screenFrag_glsl;
        case 0xd045b140:  numBytes = 327; return screenVert_glsl;
        case 0x0c1032f1:  numBytes = 128; return selectFragment_glsl;
        case 0x286ca11d:  numBytes = 285; return selectVertex_glsl;
        case 0x2aad25d4:  numBytes = 863; return vertexShader_glsl;
        default: break;
    }

    numBytes = 0;
    return nullptr;
}

const char* namedResourceList[] =
{
    "blurPassFrag_glsl",
    "brightPassFrag_glsl",
    "finalPassFrag_glsl",
    "fragmentShader_glsl",
    "screenFrag_glsl",
    "screenVert_glsl",
    "selectFragment_glsl",
    "selectVertex_glsl",
    "vertexShader_glsl"
};

const char* originalFilenames[] =
{
    "blurPassFrag.glsl",
    "brightPassFrag.glsl",
    "finalPassFrag.glsl",
    "fragmentShader.glsl",
    "screenFrag.glsl",
    "screenVert.glsl",
    "selectFragment.glsl",
    "selectVertex.glsl",
    "vertexShader.glsl"
};

const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8)
{
    for (unsigned int i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)
    {
        if (namedResourceList[i] == resourceNameUTF8)
            return originalFilenames[i];
    }

    return nullptr;
}

}
