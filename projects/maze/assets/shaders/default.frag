#version 410 core
in vec3 vFragPos;
in vec3 vNormal;
in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D uTexture;
uniform vec4 uColor;
uniform bool uUseTexture;
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbientColor;

out vec4 FragColor;

void main() {
    vec3 ambient = uAmbientColor * 0.3;
    vec3 norm = normalize(vNormal);
    vec3 lightDir = normalize(uLightDir);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = uLightColor * diff * 0.7;

    vec4 texColor = uUseTexture ? texture(uTexture, vTexCoord) : vec4(1.0);
    vec3 base = vColor.rgb * uColor.rgb;
    vec3 blended = mix(base, texColor.rgb, texColor.a);
    vec3 result = (ambient + diffuse) * blended;
    FragColor = vec4(result, 1.0);
}
