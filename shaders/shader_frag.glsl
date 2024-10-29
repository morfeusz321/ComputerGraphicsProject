#version 410

layout(std140) uniform Material {
    vec3 kd; // Diffuse reflectivity
    vec3 ks; // Specular reflectivity
    float shininess;
    float transparency;
};

uniform sampler2D colorMap;
uniform bool hasTexCoords;
uniform bool useMaterial;

uniform vec3 lightPos;       // Position of the light
uniform vec3 lightColor;     // Color of the light
uniform vec3 viewPos;        // Position of the camera/viewer

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

uniform bool userOverrideKd;
uniform bool userOverrideKs;

layout(location = 0) out vec4 fragColor;

void main()
{

    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPosition);    // Direction from fragment to light
    vec3 viewDir = normalize(viewPos - fragPosition);      // Direction from fragment to viewer

    // Ambient component
    vec3 ambient = 0.1 * kd * lightColor;

    // Diffuse component
    vec3 diffuseColor = kd;
    if (hasTexCoords) {
        diffuseColor *= texture(colorMap, fragTexCoord).rgb;
    }
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * diffuseColor * lightColor;

    // Specular component
    vec3 reflectDir = reflect(-lightDir, normal);
    float clampedShininess = max(shininess, 0.1); // Avoids issues at very low shininess
    vec3 specular = ks * pow(max(dot(reflect(-lightDir, normal), viewDir), 0.0), clampedShininess);

    vec3 finalColor = ambient + diffuse + specular;
    fragColor = vec4(finalColor, 1.0 - transparency);
}
