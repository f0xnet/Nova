// ============================================================================
// POST-PROCESSING SHADER
// ============================================================================
// Effets de post-processing HD-2D style:
// - Tilt-shift blur (zone nette autour joueur, flou progressif)
// - Ambient occlusion diffus (assombrissement doux des bords)
// - Bloom (glow sur zones lumineuses)
// - Saturation (vibrance des couleurs)
// ============================================================================

#version 120

uniform sampler2D tex;
uniform vec2 texSize;
uniform vec2 resolution;

// Paramètres actifs
uniform float vignetteStrength;    // Force de l'ambient occlusion
uniform float glowIntensity;       // Intensité du bloom
uniform float saturation;          // Saturation des couleurs
uniform float ambientOcclusion;    // Intensité du tilt-shift blur

// ============================================================================
// TEXTURE SAMPLING
// ============================================================================
vec3 sampleTex(vec2 uv) {
    return texture2D(tex, clamp(uv, 0.0, 1.0)).rgb;
}

// ============================================================================
// TILT-SHIFT BLUR
// ============================================================================
vec3 applyTiltShift(vec2 uv, float intensity) {
    if (intensity <= 0.0) return sampleTex(uv);

    vec2 pixelSize = 1.0 / resolution;
    float distFromCenter = abs(uv.y - 0.5) * 2.0;

    // Zone nette au centre (autour du joueur)
    float blurAmount = smoothstep(0.15, 0.7, distFromCenter) * intensity;

    if (blurAmount < 0.01) return sampleTex(uv);

    vec3 color = vec3(0.0);
    float totalWeight = 0.0;

    // Blur gaussien avec rayon variable
    float radius = blurAmount * 8.0;
    int samples = int(radius) + 1;
    samples = min(samples, 10); // Limite pour performance

    for (int x = -samples; x <= samples; x++) {
        for (int y = -samples; y <= samples; y++) {
            vec2 offset = vec2(float(x), float(y)) * pixelSize;
            float dist = length(vec2(x, y));
            float weight = exp(-dist * dist / (2.0 * radius));

            color += sampleTex(uv + offset) * weight;
            totalWeight += weight;
        }
    }

    return color / totalWeight;
}

// ============================================================================
// BLOOM
// ============================================================================
vec3 applyBloom(vec2 uv, vec3 baseColor, float intensity) {
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
// AMBIENT OCCLUSION (assombrissement diffus depuis les bords)
// ============================================================================
float applyAO(vec2 uv, float strength) {
    if (strength <= 0.0) return 1.0;

    // Distance depuis chaque bord
    float distLeft = uv.x;
    float distRight = 1.0 - uv.x;
    float distTop = uv.y;
    float distBottom = 1.0 - uv.y;

    // Fade doux depuis les bords
    float fadeLeft = pow(distLeft, 0.5);
    float fadeRight = pow(distRight, 0.5);
    float fadeTop = pow(distTop, 0.5);
    float fadeBottom = pow(distBottom, 0.5);

    float edgeFade = fadeLeft * fadeRight * fadeTop * fadeBottom;

    // Assombrissement des coins
    vec2 centered = uv * 2.0 - 1.0;
    float cornerDist = length(centered);
    float cornerFade = 1.0 - smoothstep(0.5, 1.5, cornerDist) * 0.3;

    float ao = edgeFade * cornerFade;

    return mix(1.0 - strength * 0.6, 1.0, ao);
}

// ============================================================================
// SATURATION
// ============================================================================
vec3 applySaturation(vec3 color, float sat) {
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    return mix(vec3(gray), color, sat);
}

// ============================================================================
// MAIN
// ============================================================================
void main()
{
    // Normaliser les UV et inverser Y
    vec2 uv = gl_TexCoord[0].xy / texSize;
    uv.y = 1.0 - uv.y;

    // Tilt-shift blur
    vec3 color = applyTiltShift(uv, ambientOcclusion);

    // Bloom
    color = applyBloom(uv, color, glowIntensity);

    // Saturation
    color = applySaturation(color, saturation);

    // Ambient occlusion
    color *= applyAO(uv, vignetteStrength);

    // Clamp final
    color = clamp(color, 0.0, 1.0);

    gl_FragColor = vec4(color, 1.0);
}
