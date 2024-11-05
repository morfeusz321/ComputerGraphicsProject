#version 410 core

in vec3 FragPosTS;
in vec3 ViewPosTS;
in vec3 LightPosTS;
in vec2 TexCoords;

uniform sampler2D colorMapCube;
uniform sampler2D normalMap;

// Material properties
uniform vec3 kd;        // Diffuse color coefficient
uniform vec3 ks;        // Specular color coefficient
uniform float shininess; // Shininess for specular highlight

// Lighting properties
uniform vec3 lightPos;
uniform vec3 viewPos;

layout(location = 0) out vec4 fragColor;

void main()
{
    // Sample color from the color map
    vec3 color = texture(colorMapCube, TexCoords).rgb;

    // Sample and normalize normal from the normal map, transforming it to [-1, 1] range
    vec3 normal = texture(normalMap, TexCoords).rgb;
    normal = normalize(normal * 2.0 - 1.0);

    // Transform the normal back to world space if needed for consistent lighting
    vec3 lightDirection = normalize(LightPosTS - FragPosTS);
    vec3 viewDir = normalize(ViewPosTS - FragPosTS);

    // Ambient component
    vec3 ambient = 0.1 * kd * color;

    // Diffuse component
    float diff = max(dot(lightDirection, normal), 0.0);
    vec3 diffuse = diff * kd * color;

    // Specular component
    vec3 halfwayDir = normalize(lightDirection + viewDir);
    float specAngle = max(dot(normal, halfwayDir), 0.0);
    float spec = pow(specAngle, shininess);
    vec3 specular = ks * spec;

    // Combine all lighting components
    vec3 result = ambient + diffuse + specular;
    fragColor = vec4(result, 1.0);
}
