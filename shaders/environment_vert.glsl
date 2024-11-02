// Vertex Shader - Version with correction
#version 410
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalModelMatrix;
uniform vec3 viewPos;  // Add this uniform

out vec3 fragPosition;
out vec3 fragNormal;
out vec3 reflectDir;
out vec2 fragTexCoord;

void main() {
    // Transform vertex position to world space
    fragPosition = vec3(modelMatrix * vec4(position, 1.0));
    
    // Transform normal to world space
    fragNormal = normalize(normalModelMatrix * normal);
    
    // Calculate reflection vector for environment mapping
    vec3 viewDir = normalize(fragPosition - viewPos);  // Use uniform viewPos
    reflectDir = reflect(viewDir, fragNormal);
    
    fragTexCoord = texCoord;
    gl_Position = mvpMatrix * vec4(position, 1.0);
}