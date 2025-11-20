// ============================================================================
// OCTOPATH TRAVELER STYLE SHADER
// ============================================================================
// Effets HD-2D inspirés d'Octopath Traveler :
// - Tilt-shift blur (effet miniature/diorama)
// - Bloom/glow intense
// - Saturation et contraste élevés
// - Vignette douce
// - God rays (rayons de lumière volumétriques)
// ============================================================================

#version 120

uniform sampler2D tex;
uniform vec2 texSize;
uniform vec2 resolution;
uniform float time;

// Paramètres ajustables (gardés pour compatibilité avec CRTEffect.cpp)
uniform float scanlineIntensity;      // Non utilisé
uniform float pixelGridIntensity;     // Non utilisé
uniform float chromaticAberration;    // Non utilisé
uniform float rgbShiftAmount;         // Non utilisé
uniform float curvature;              // Non utilisé
uniform float vignetteStrength;       // Utilisé pour vignette
uniform float glowIntensity;          // Utilisé pour bloom
uniform float noiseIntensity;         // Non utilisé
uniform float colorBanding;           // Non utilisé
uniform float saturation;             // Utilisé pour saturation
uniform float ambientOcclusion;       // Utilisé pour intensité tilt-shift

const float PI = 3.14159265359;

// ============================================================================
// SAFE TEXTURE SAMPLING
// ============================================================================
vec4 sampleTexture(vec2 uv) {
    vec2 clampedUV = clamp(uv, 0.0, 1.0);
    return texture2D(tex, clampedUV);
}

// ============================================================================
// TILT-SHIFT BLUR (effet miniature/diorama)
// ============================================================================
vec3 applyTiltShift(vec2 uv, float intensity) {
    if (intensity <= 0.0) return sampleTexture(uv).rgb;

    vec2 pixelSize = 1.0 / resolution;

    // Distance from horizontal center (0 at center, 1 at top/bottom)
    float distFromCenter = abs(uv.y - 0.5) * 2.0;

    // Focus zone in the middle (0.3 = sharp zone size)
    float blurAmount = smoothstep(0.2, 0.8, distFromCenter) * intensity;

    vec3 color = vec3(0.0);
    float totalWeight = 0.0;

    // Gaussian-like blur with variable radius based on distance
    float radius = blurAmount * 8.0;
    int samples = int(radius) + 1;

    for (int x = -samples; x <= samples; x++) {
        for (int y = -samples; y <= samples; y++) {
            vec2 offset = vec2(float(x), float(y)) * pixelSize;
            float dist = length(offset / pixelSize);
            float weight = exp(-dist * dist / (radius * radius * 0.5));

            color += sampleTexture(uv + offset).rgb * weight;
            totalWeight += weight;
        }
    }

    return color / totalWeight;
}

// ============================================================================
// BLOOM/GLOW (effet de lumière intense)
// ============================================================================
vec3 applyBloom(vec2 uv, float intensity) {
    vec3 color = sampleTexture(uv).rgb;
    float brightness = dot(color, vec3(0.299, 0.587, 0.114));

    if (brightness > 0.5) {
        vec2 pixelSize = 1.0 / resolution;
        vec3 bloom = vec3(0.0);
        float totalWeight = 0.0;

        // Large radius bloom
        for (float x = -5.0; x <= 5.0; x += 1.0) {
            for (float y = -5.0; y <= 5.0; y += 1.0) {
                vec2 offset = vec2(x, y) * pixelSize * 2.0;
                float dist = length(vec2(x, y));
                float weight = exp(-dist * dist / 16.0);

                bloom += sampleTexture(uv + offset).rgb * weight;
                totalWeight += weight;
            }
        }

        bloom /= totalWeight;
        float bloomAmount = pow(brightness - 0.5, 1.5) * intensity * 3.0;
        color += bloom * bloomAmount;
    }

    return color;
}

// ============================================================================
// GOD RAYS (rayons de lumière volumétriques)
// ============================================================================
vec3 applyGodRays(vec2 uv, vec3 color) {
    // Light source position (top center, slightly animated)
    vec2 lightPos = vec2(0.5, 0.2 + sin(time * 0.3) * 0.05);

    vec2 dir = uv - lightPos;
    float dist = length(dir);

    // Sample along ray from pixel to light source
    vec3 rayColor = vec3(0.0);
    int samples = 8;
    float decay = 0.96;
    float weight = 1.0;

    for (int i = 0; i < samples; i++) {
        vec2 samplePos = uv - dir * (float(i) / float(samples));
        vec3 sampleCol = sampleTexture(samplePos).rgb;
        float brightness = dot(sampleCol, vec3(0.299, 0.587, 0.114));

        rayColor += sampleCol * weight * brightness;
        weight *= decay;
    }

    rayColor /= float(samples);

    // Fade rays based on distance from light
    float rayStrength = exp(-dist * dist * 2.0) * 0.3;

    return color + rayColor * rayStrength;
}

// ============================================================================
// VIGNETTE (assombrissement des bords)
// ============================================================================
float applyVignette(vec2 uv, float strength) {
    vec2 centered = uv * 2.0 - 1.0;
    float dist = length(centered);
    float vig = smoothstep(1.8, 0.5, dist);

    return mix(1.0, vig, strength);
}

// ============================================================================
// SATURATION
// ============================================================================
vec3 applySaturation(vec3 color, float sat) {
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    return mix(vec3(gray), color, sat);
}

// ============================================================================
// COLOR GRADING (ajustement des couleurs style Octopath)
// ============================================================================
vec3 applyColorGrading(vec3 color) {
    // Légère teinte chaude et magique
    vec3 warmTint = vec3(1.02, 0.99, 0.96);
    color *= warmTint;

    // Boost des midtones
    float luma = dot(color, vec3(0.299, 0.587, 0.114));
    float midtoneBoost = smoothstep(0.3, 0.7, luma) * 0.1;
    color += vec3(midtoneBoost * 0.8, midtoneBoost, midtoneBoost * 0.6);

    return color;
}

// ============================================================================
// MAIN
// ============================================================================
void main()
{
    // Normalize UV from pixel coords to 0-1 and flip Y
    vec2 uv = gl_TexCoord[0].xy / texSize;
    uv.y = 1.0 - uv.y;

    // Tilt-shift blur (effet diorama)
    vec3 color = applyTiltShift(uv, ambientOcclusion);

    // Bloom intense (caractéristique d'Octopath)
    color = applyBloom(uv, glowIntensity);

    // God rays (rayons volumétriques)
    color = applyGodRays(uv, color);

    // Saturation élevée (style HD-2D)
    color = applySaturation(color, saturation);

    // Color grading
    color = applyColorGrading(color);

    // Contraste fort
    color = ((color - 0.5) * 1.3) + 0.5;

    // Vignette douce
    color *= applyVignette(uv, vignetteStrength);

    // Clamp
    color = clamp(color, 0.0, 1.0);

    gl_FragColor = vec4(color, 1.0);
}
