// ============================================================================
// POST-PROCESSING SHADER - HD-2D STYLE
// ============================================================================
// Inspiré des shaders SFML officiels (blur.frag) et exemples GitHub
// Effets: Tilt-shift, Bloom, AO diffus, Saturation
// ============================================================================

#version 120

uniform sampler2D tex;
uniform vec2 texSize;
uniform vec2 resolution;

// Paramètres d'effets (compatibles CRTEffect.cpp)
uniform float vignetteStrength;    // AO diffus
uniform float glowIntensity;       // Bloom
uniform float saturation;          // Saturation
uniform float ambientOcclusion;    // Tilt-shift

// ============================================================================
// TILT-SHIFT BLUR
// Inspiré du blur.frag de SFML avec blur variable selon position
// ============================================================================
vec3 tiltShift(vec2 uv) {
    // Distance verticale du centre (0 = centre, 1 = bord)
    float dist = abs(uv.y - 0.5) * 2.0;

    // Calculer l'intensité du blur (0 au centre, max aux bords)
    float blurAmount = smoothstep(0.2, 0.8, dist) * ambientOcclusion;

    // Si pas de blur, retourner la texture directe
    if (blurAmount < 0.01) {
        return texture2D(tex, uv).rgb;
    }

    // Rayon de blur en UV (adapté à texSize comme SFML blur_radius)
    vec2 radius = vec2(blurAmount / texSize.x, blurAmount / texSize.y) * 3.0;

    // Offsets comme dans blur.frag de SFML
    vec2 offx = vec2(radius.x, 0.0);
    vec2 offy = vec2(0.0, radius.y);

    // Blur gaussien 3x3 (9 samples comme SFML blur.frag)
    vec4 pixel = texture2D(tex, uv) * 4.0 +
                 texture2D(tex, uv - offx) * 2.0 +
                 texture2D(tex, uv + offx) * 2.0 +
                 texture2D(tex, uv - offy) * 2.0 +
                 texture2D(tex, uv + offy) * 2.0 +
                 texture2D(tex, uv - offx - offy) * 1.0 +
                 texture2D(tex, uv - offx + offy) * 1.0 +
                 texture2D(tex, uv + offx - offy) * 1.0 +
                 texture2D(tex, uv + offx + offy) * 1.0;

    return (pixel / 16.0).rgb;
}

// ============================================================================
// BLOOM
// Inspiré des exemples GLSL bloom, simplifié pour SFML
// ============================================================================
vec3 bloom(vec2 uv, vec3 baseColor) {
    // Luminosité de la couleur de base
    float brightness = dot(baseColor, vec3(0.299, 0.587, 0.114));

    // Bloom seulement si assez lumineux
    if (brightness < 0.6 || glowIntensity <= 0.0) {
        return baseColor;
    }

    // Rayon de blur pour bloom
    vec2 radius = vec2(1.0 / texSize.x, 1.0 / texSize.y) * 2.0;
    vec2 offx = vec2(radius.x, 0.0);
    vec2 offy = vec2(0.0, radius.y);

    // Échantillonnage 3x3 pour le bloom
    vec3 bloomColor = texture2D(tex, uv).rgb * 4.0 +
                      texture2D(tex, uv - offx).rgb * 2.0 +
                      texture2D(tex, uv + offx).rgb * 2.0 +
                      texture2D(tex, uv - offy).rgb * 2.0 +
                      texture2D(tex, uv + offy).rgb * 2.0 +
                      texture2D(tex, uv - offx - offy).rgb +
                      texture2D(tex, uv - offx + offy).rgb +
                      texture2D(tex, uv + offx - offy).rgb +
                      texture2D(tex, uv + offx + offy).rgb;

    bloomColor /= 16.0;

    // Ajouter le bloom proportionnellement à la luminosité
    float bloomFactor = (brightness - 0.6) * glowIntensity;
    return baseColor + bloomColor * bloomFactor;
}

// ============================================================================
// AMBIENT OCCLUSION DIFFUS
// Assombrissement doux depuis les bords (version pré-Octopath)
// ============================================================================
float ambientOcclusionDiffuse(vec2 uv) {
    if (vignetteStrength <= 0.0) return 1.0;

    // Distance de chaque bord
    float distX = min(uv.x, 1.0 - uv.x);
    float distY = min(uv.y, 1.0 - uv.y);

    // Fade doux avec pow pour diffusion
    float fadeX = pow(distX * 2.0, 0.5);
    float fadeY = pow(distY * 2.0, 0.5);
    float edgeFade = fadeX * fadeY;

    // Assombrissement des coins
    vec2 centered = uv * 2.0 - 1.0;
    float cornerDist = length(centered);
    float cornerFade = 1.0 - smoothstep(0.5, 1.5, cornerDist) * 0.3;

    float ao = edgeFade * cornerFade;

    // Retourner le facteur de darkening
    return mix(1.0 - vignetteStrength * 0.6, 1.0, ao);
}

// ============================================================================
// SATURATION
// ============================================================================
vec3 adjustSaturation(vec3 color, float sat) {
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    return mix(vec3(gray), color, sat);
}

// ============================================================================
// MAIN
// ============================================================================
void main()
{
    // Normaliser UV de pixel coords à 0-1, et flip Y (SFML RenderTexture)
    vec2 uv = gl_TexCoord[0].xy / texSize;
    uv.y = 1.0 - uv.y;

    // 1. Tilt-shift blur (net au centre, flou aux bords)
    vec3 color = tiltShift(uv);

    // 2. Bloom (glow sur zones lumineuses)
    color = bloom(uv, color);

    // 3. Saturation
    color = adjustSaturation(color, saturation);

    // 4. Ambient occlusion diffus
    color *= ambientOcclusionDiffuse(uv);

    // Output final
    gl_FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
