#version 330

#define PI 3.14159265

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

uniform vec4 lightPosition;
uniform vec4 lightColor;
uniform vec4 color;

in vec2 fragTextureCoord;
in vec3 fragNormal;
in vec3 fragVertex;


// Cook Torrance settings
uniform vec3 specularColor = vec3(0.8, 0.8, 0.8);
uniform float fresnel = 3.0f;
uniform float roughness = 0.20f;


in vec3 viewDir;
in vec3 lightDir;
uniform vec3 eyePosition;

layout(location = 0) out vec4 colorOut;

vec3 CookTorrance()
{
  float k = 0.2;
	float NdotL = max(0, dot(fragNormal, lightDir));
	float Rs = 0.0;
	if (NdotL > 0) 
	{
		vec3 H = normalize(lightDir + viewDir);
		float NdotH = max(0, dot(fragNormal, H));
		float NdotV = max(0, dot(fragNormal, viewDir));
		float VdotH = max(0, dot(lightDir, H));

		// Fresnel reflectance
		float F = pow(1.0 - VdotH, 5.0);
		F *= (1.0 - fresnel);
		F += fresnel;

		// Microfacet distribution by Beckmann
		float m_squared = roughness * roughness;
		float r1 = 1.0 / (4.0 * m_squared * pow(NdotH, 4.0));
		float r2 = (NdotH * NdotH - 1.0) / (m_squared * NdotH * NdotH);
		float D = r1 * exp(r2);

		// Geometric shadowing
		float two_NdotH = 2.0 * NdotH;
		float g1 = (two_NdotH * NdotV) / VdotH;
		float g2 = (two_NdotH * NdotL) / VdotH;
		float G = min(1.0, min(g1, g2));

		Rs = (F * D * G) / (PI * NdotL * NdotV);
	}

	return color.rgb * lightColor.rgb * NdotL + lightColor.rgb * specularColor *
	      NdotL * (k + Rs * (1.0 - k));
}

vec3 Simple()
{
    vec3 normal = normalize(fragNormal);

    vec3 direction = normalize(lightPosition.xyz - fragVertex);
    float intensity = max(0.0, dot(direction, fragNormal));
    vec4 curLight = vec4(intensity, intensity, intensity, 1) * lightColor * color;
    vec3 ambient = vec3(0.1);
    return curLight.rgb + ambient;
}

#define FOG_DENSITY 0.02

float fogFactorExp2(float density) {
  float dist = distance(eyePosition, fragVertex);
  const float LOG2 = -1.442695;
  float d = density * dist;
  return 1.0 - clamp(exp2(d * d * LOG2), 0.0, 1.0);
}

vec4 withFogCookTorrance()
{
    vec4 fogColor = vec4(0.1,0.1,0.1,1.0); // white
    float fogAmount = fogFactorExp2(FOG_DENSITY);

    return mix(vec4(CookTorrance(), color.a), fogColor, fogAmount);
}

vec4 withoutFogCookTorrance()
{
    return vec4(CookTorrance(), color.a);
}

vec4 withFogSimple()
{
    vec4 fogColor = vec4(0.1,0.1,0.1,1.0); // white
    float fogAmount = fogFactorExp2(FOG_DENSITY);

    return mix(vec4(Simple(), color.a), fogColor, fogAmount);
}


vec4 withoutFogSimple()
{
    return vec4(Simple(), color.a);
}


vec4 normalMap(){
  return vec4(fragNormal.rgb, 1.0);
}


vec4 bright(){
  float intensity = max(0.0, dot(viewDir, fragNormal));

  return vec4(color.rgb * intensity * 1.2, 1.0);
}


vec4 glow(){
    float u_glow = 0.3f;

    float shininess = 20.0;
    vec3  normalV = normalize( fragNormal );
    vec3  eyeV    = normalize( -fragVertex );
    vec3  halfV   = normalize( eyeV + normalV );
    float NdotH   = max( 0.0, dot( normalV, halfV ) );
    float glowFac = ( shininess + 2.0 ) * pow( NdotH, shininess ) / ( 2.0 * 3.14159265 );

    return u_glow * (color * glowFac);
}

vec4 glowWithFogExperiment(){
    vec4 fogColor = vec4(0.1,0.1,0.1,0.0); // white
    float fogAmount = fogFactorExp2(FOG_DENSITY);

    return mix(glow(), fogColor, fogAmount);
}

void main()
{
  colorOut = vec4(mix(color.rgb, bright().rgb, 0.5) , 1.0);
}
