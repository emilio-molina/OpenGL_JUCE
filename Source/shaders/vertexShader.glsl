#version 330

in vec4 vertex;
in vec4 normal;
in vec2 textureCoord;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;

out vec2 fragTextureCoord;
out vec3 fragNormal;
out vec3 fragVertex;


// Cook Torrance requirements
out vec3 viewDir;
out vec3 lightDir;
uniform vec3 eyePosition;
uniform vec4 lightPosition;

void main()
{
    mat4 modelView = viewMatrix * modelMatrix;
    //mat4 normalMatrix = transpose(inverse(modelView));

    fragVertex = vec3(modelView * vertex);
    fragNormal = vec3(normalize(mat3(normalMatrix) * normalize(normal.xyz)));
    fragTextureCoord = textureCoord;

    viewDir = normalize(eyePosition - fragVertex.xyz);
    lightDir = normalize(lightPosition.xyz - fragVertex.xyz);

    gl_Position = projectionMatrix * modelView * vertex;
}
