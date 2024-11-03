#version 410

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalModelMatrix;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;

void main() {
    fragPosition = vec3(modelMatrix * vec4(position, 1.0));
    fragNormal = normalModelMatrix * normal;
    fragTexCoord = texCoord;
    gl_Position = mvpMatrix * vec4(position, 1.0);
}
