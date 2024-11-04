#version 410
vec3 kd;
vec3 ks;
float shininess;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

uniform sampler2D colorMap;
uniform bool hasTexCoords;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform bool useKd;
uniform bool useKs;

out vec4 fragColor;

void main() {
    // Normalize vectors
    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPosition);
    vec3 lightDir = normalize(lightPos - fragPosition);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    
    // Get base color
    vec3 baseColor;
    if (hasTexCoords) {
        baseColor = texture(colorMap, fragTexCoord).rgb;
    } else if (useKd) {
        baseColor = kd;
    } else {
        baseColor = vec3(0.0);
    }
    
    // Calculate lighting
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = lightColor * diff * baseColor;
    
    // Calculate specular
    float spec = 0.0;
    if (useKs) {
        float specPower = 32.0;
        spec = pow(max(dot(normal, halfwayDir), 0.0), specPower);
    }
    vec3 specular = lightColor * spec * (useKs ? ks : vec3(0.1));
    
    // Add ambient light
    vec3 ambient = 0.1 * baseColor;
    
    // Combine all lighting components
    fragColor = vec4(ambient + diffuse + specular, 1.0);
}
