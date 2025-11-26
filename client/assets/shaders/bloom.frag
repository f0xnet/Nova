// ============================================================================
// BLOOM SHADER
// ============================================================================
// Effet de glow sur les zones lumineuses
// ============================================================================
#version 120

uniform sampler2D tex;
uniform vec2 texSize;
uniform vec2 resolution;
uniform float intensity;  // Intensité du bloom (0.0 - 1.0+)

// ============================================================================
// TEXTURE SAMPLING
// ============================================================================
vec3 sampleTex(vec2 uv) {
    return texture2D(tex, clamp(uv, 0.0, 1.0)).rgb;
}

// ============================================================================
// BLOOM
// ============================================================================
vec3 applyBloom(vec2 uv, vec3 baseColor) {
    if (intensity <= 0.0) return baseColor;

    float brightness = dot(baseColor, vec3(0.299, 0.587, 0.114));

    // Bloom seulement sur zones lumineuses
    if (brightness < 0.6) return baseColor;

    vec2 pixelSize = 1.0 / resolution;
    vec3 bloom = vec3(0.0);
    float totalWeight = 0.0;

    // Échantillonnage 5x5
    for (float x = -2.0; x <= 2.0; x += 1.0) {
        for (float y = -2.0; y <= 2.0; y += 1.0) {
            vec2 offset = vec2(x, y) * pixelSize * 2.0;
            float dist = length(vec2(x, y));
            float weight = exp(-dist * dist / 4.0);

            bloom += sampleTex(uv + offset) * weight;
            totalWeight += weight;
        }
    }

    bloom /= totalWeight;
    float bloomAmount = (brightness - 0.6) * intensity * 2.0;

    return baseColor + bloom * bloomAmount;
}

// ============================================================================
// MAIN
// ============================================================================
void main()
{
    // Normaliser UV de pixel coords à 0-1, et flip Y (SFML RenderTexture)
    vec2 uv = gl_TexCoord[0].xy / texSize;
    uv.y = 1.0 - uv.y;

    // Échantillonner et appliquer bloom
    vec3 color = sampleTex(uv);
    color = applyBloom(uv, color);

    gl_FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
