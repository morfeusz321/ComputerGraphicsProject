#version 410

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform sampler2D colorMap;
uniform samplerCube cubemap;

// PBR material parameters
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

uniform bool useTexture;
uniform bool useEnvironmentMapping;
uniform float reflectivity;

layout(location = 0) out vec4 fragColor;
const float PI = 3.14159265359;

// PBR functions
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(viewPos - fragPosition);
    vec3 baseColor = useTexture ? texture(colorMap, fragTexCoord).rgb : albedo;
    
    // Calculate reflections if environment mapping is enabled
    vec3 envColor = vec3(0.0);
    if (useEnvironmentMapping) {
        vec3 R = reflect(-V, N);
        envColor = texture(cubemap, R).rgb;
    }

    // Calculate metallic F0 (surface reflection at zero incidence)
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, baseColor, metallic);

    // Lighting calculation
    vec3 Lo = vec3(0.0);
    
    // Calculate per-light radiance
    vec3 L = normalize(lightPos - fragPosition);
    vec3 H = normalize(V + L);
    float distance = length(lightPos - fragPosition);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lightColor * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * baseColor / PI + specular) * radiance * NdotL;

    // Ambient lighting
    vec3 ambient = vec3(0.03) * baseColor * ao;
    
    // Final color
    vec3 color = ambient + Lo;
    
    // Mix with environment reflection if enabled
    if (useEnvironmentMapping) {
        color = mix(color, envColor, reflectivity);
    }

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));

    fragColor = vec4(color, 1.0);
}