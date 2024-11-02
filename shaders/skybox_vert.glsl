#version 410 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos * 100.0, 1.0); // Scale the skybox
    gl_Position = pos.xyww; // This ensures the skybox is always rendered at maximum depth
}
