#version 410 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 biTangent;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 FragPosTS;
out vec3 LightPosTS;
out vec3 ViewPosTS;  
out vec2 TexCoords;

uniform vec3 lightPos;
uniform vec3 viewPos;


void main()
{
    
    // Transform position to world space
    vec3 FragPos = vec3(modelMatrix * vec4(position, 1.0));
    TexCoords = texCoords;

    mat3 modelV = transpose(inverse(mat3(modelMatrix)));

    // Calculate TBN matrix
    vec3 T = normalize(mat3(modelV) * tangent);
    vec3 B = normalize(mat3(modelV) * biTangent);
    vec3 N = normalize(mat3(modelV) * normal);

    mat3 TBN = transpose(mat3(T, B, N));

    FragPosTS = TBN * FragPos;
    LightPosTS = TBN * lightPos;
    ViewPosTS = TBN * viewPos;  
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}
