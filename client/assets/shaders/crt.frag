// ============================================================================
// SHADER CRT COMPLET - FORMAT .FRAG
// ============================================================================
// 15 effets distincts pour reproduction fidèle
// Compatible SFML avec texSize pour normalisation UV
// ============================================================================

#version 120

uniform sampler2D tex;
uniform vec2 texSize;
uniform vec2 resolution;
uniform float time;

// Paramètres ajustables
uniform float scanlineIntensity;
uniform float pixelGridIntensity;
uniform float chromaticAberration;
uniform float rgbShiftAmount;
uniform float curvature;
uniform float vignetteStrength;
uniform float glowIntensity;
uniform float noiseIntensity;
uniform float colorBanding;
uniform float saturation;
uniform float ambientOcclusion;

const float PI = 3.14159265359;

// ============================================================================
// SAFE TEXTURE SAMPLING (prevents edge artifacts)
// ============================================================================
vec4 sampleTexture(vec2 uv) {
    // Clamp UVs with larger margin to prevent edge artifacts
    vec2 clampedUV = clamp(uv, 0.01, 0.99);
    return texture2D(tex, clampedUV);
}

// ============================================================================
// EDGE FADE (smooth transition at borders after curvature)
// ============================================================================
float edgeFade(vec2 uv) {
    // Clamp UV first to handle values outside 0-1
    vec2 safeUV = clamp(uv, 0.0, 1.0);
    float margin = 0.03;
    float fadeX = smoothstep(0.0, margin, safeUV.x) * smoothstep(1.0, 1.0 - margin, safeUV.x);
    float fadeY = smoothstep(0.0, margin, safeUV.y) * smoothstep(1.0, 1.0 - margin, safeUV.y);
    return fadeX * fadeY;
}

// ============================================================================
// COURBURE CRT
// ============================================================================
vec2 curveCRT(vec2 uv) {
    if (curvature <= 0.0) return uv;

    uv = uv * 2.0 - 1.0;
    // Less aggressive curvature
    vec2 offset = abs(uv.yx) / vec2(10.0, 8.0);
    uv = uv + uv * offset * offset * curvature;

    return uv * 0.5 + 0.5;
}

// ============================================================================
// ABERRATION CHROMATIQUE
// ============================================================================
vec3 chromaticAberrationEffect(vec2 uv, float amount) {
    if (amount <= 0.0) return sampleTexture(uv).rgb;

    vec2 direction = uv - 0.5;
    float dist = length(direction);
    direction = normalize(direction);

    float aberration = amount * dist * 1.5;

    float r = sampleTexture(uv + direction * aberration).r;
    float g = sampleTexture(uv).g;
    float b = sampleTexture(uv - direction * aberration).b;

    return vec3(r, g, b);
}

// ============================================================================
// RGB SHIFT HORIZONTAL (VHS)
// ============================================================================
vec3 rgbShift(vec2 uv, float amount) {
    if (amount <= 0.0) return sampleTexture(uv).rgb;

    float shift = sin(uv.y * 100.0 + time * 2.0) * amount;

    float r = sampleTexture(uv + vec2(shift, 0.0)).r;
    float g = sampleTexture(uv).g;
    float b = sampleTexture(uv - vec2(shift, 0.0)).b;

    return vec3(r, g, b);
}

// ============================================================================
// SCANLINES
// ============================================================================
float scanlines(vec2 coord) {
    float scanline = sin(coord.y * resolution.y * PI * 2.0);
    scanline = scanline * 0.5 + 0.5;
    scanline = pow(scanline, 0.85);

    float flicker = sin(time * 20.0 + coord.y * 50.0) * 0.01;
    scanline += flicker;

    return 1.0 - (scanline * scanlineIntensity);
}

// ============================================================================
// GRILLE DE PIXELS
// ============================================================================
float pixelGrid(vec2 coord) {
    vec2 pixel = coord * resolution;
    vec2 grid = fract(pixel);

    float gridX = smoothstep(0.0, 0.15, grid.x) * smoothstep(1.0, 0.85, grid.x);
    float gridY = smoothstep(0.0, 0.15, grid.y) * smoothstep(1.0, 0.85, grid.y);

    float gridEffect = gridX * gridY;

    return mix(0.80, 1.0, gridEffect * (1.0 - pixelGridIntensity) + pixelGridIntensity);
}

// ============================================================================
// VIGNETTE
// ============================================================================
float vignette(vec2 uv) {
    vec2 position = uv * 2.0 - 1.0;
    float dist = length(position);
    float vig = smoothstep(1.5, 0.4, dist);

    return mix(1.0, vig, vignetteStrength);
}

// ============================================================================
// DIFFUSE AO (Soft edge darkening spreading inward)
// ============================================================================
float applyDiffuseAO(vec2 uv, float intensity) {
    if (intensity <= 0.0) return 1.0;

    // Center the UV coordinates
    vec2 centered = uv * 2.0 - 1.0;

    // Distance from each edge (0 at edge, 1 at center)
    float distLeft = uv.x;
    float distRight = 1.0 - uv.x;
    float distTop = uv.y;
    float distBottom = 1.0 - uv.y;

    // Soft fade from edges - use pow for diffuse spread
    float fadeLeft = pow(distLeft, 0.5);
    float fadeRight = pow(distRight, 0.5);
    float fadeTop = pow(distTop, 0.5);
    float fadeBottom = pow(distBottom, 0.5);

    // Combine all edges with smooth multiplication
    float edgeFade = fadeLeft * fadeRight * fadeTop * fadeBottom;

    // Also add subtle corner darkening
    float cornerDist = length(centered);
    float cornerFade = 1.0 - smoothstep(0.5, 1.5, cornerDist) * 0.3;

    // Combine edge and corner effects
    float ao = edgeFade * cornerFade;

    // Apply intensity - more intensity = more darkening
    return mix(1.0 - intensity * 0.6, 1.0, ao);
}

// ============================================================================
// SATURATION
// ============================================================================
vec3 applySaturation(vec3 color, float sat) {
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    return mix(vec3(gray), color, sat);
}

// ============================================================================
// GLOW/BLOOM
// ============================================================================
vec3 applyGlow(vec2 uv, float intensity) {
    vec3 color = sampleTexture(uv).rgb;
    float brightness = dot(color, vec3(0.299, 0.587, 0.114));

    if (brightness > 0.6) {
        vec2 pixelSize = 1.0 / resolution;
        vec3 glow = vec3(0.0);

        for (float x = -3.0; x <= 3.0; x += 1.0) {
            for (float y = -3.0; y <= 3.0; y += 1.0) {
                vec2 offset = vec2(x, y) * pixelSize * 1.5;
                glow += sampleTexture(uv + offset).rgb;
            }
        }

        glow /= 49.0;
        float glowAmount = (brightness - 0.6) * intensity * 2.5;
        color += glow * glowAmount;
    }

    return color;
}

// ============================================================================
// NOISE/GRAIN
// ============================================================================
float noise(vec2 uv) {
    return fract(sin(dot(uv, vec2(12.9898, 78.233)) + time * 10.0) * 43758.5453);
}

vec3 applyNoise(vec3 color, vec2 uv, float intensity) {
    float n = (noise(uv * 1000.0) * 2.0 - 1.0) * intensity;
    return color + vec3(n);
}

// ============================================================================
// COLOR BANDING
// ============================================================================
vec3 posterize(vec3 color, float levels) {
    if (levels <= 0.0) return color;

    float numLevels = 256.0 - (levels * 128.0);
    return floor(color * numLevels + 0.5) / numLevels;
}

// ============================================================================
// GHOSTING
// ============================================================================
vec3 applyGhosting(vec2 uv) {
    vec3 color = sampleTexture(uv).rgb;

    vec2 offset = vec2(0.003, 0.0);
    vec3 ghost1 = sampleTexture(uv + offset).rgb * 0.15;
    vec3 ghost2 = sampleTexture(uv + offset * 2.0).rgb * 0.08;

    return color + ghost1 + ghost2;
}

// ============================================================================
// PHOSPHOR DECAY
// ============================================================================
vec3 phosphorDecay(vec3 color) {
    float brightness = dot(color, vec3(0.299, 0.587, 0.114));

    if (brightness > 0.7) {
        color += vec3(0.02, 0.03, 0.01) * (brightness - 0.7);
    }

    return color;
}

// ============================================================================
// INTERFERENCE PATTERN (disabled - was causing unwanted scanlines)
// ============================================================================
float interference(vec2 uv) {
    // Disabled - return 1.0 (no effect)
    return 1.0;
}

// ============================================================================
// MAIN
// ============================================================================
void main()
{
    // Normalize UV from pixel coords to 0-1 and flip Y
    vec2 uv = gl_TexCoord[0].xy / texSize;
    uv.y = 1.0 - uv.y;

    // Courbure CRT
    vec2 curvedUV = curveCRT(uv);

    // Bords noirs (outside curved area)
    if (curvedUV.x < -0.01 || curvedUV.x > 1.01 ||
        curvedUV.y < -0.01 || curvedUV.y > 1.01) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // Edge fade factor for smooth border transition
    float fade = edgeFade(curvedUV);

    // Aberration chromatique
    vec3 color = chromaticAberrationEffect(curvedUV, chromaticAberration);

    // RGB Shift (increased mix)
    color = mix(color, rgbShift(curvedUV, rgbShiftAmount), 0.5);

    // Ghosting (increased mix)
    color = mix(color, applyGhosting(curvedUV), 0.4);

    // Glow
    color = applyGlow(curvedUV, glowIntensity);

    // Phosphor decay
    color = phosphorDecay(color);

    // Grille de pixels
    color *= pixelGrid(curvedUV);

    // Scanlines
    color *= scanlines(curvedUV);

    // Interference
    color *= interference(curvedUV);

    // Vignette
    color *= vignette(curvedUV);

    // Diffuse AO (soft darkening from edges)
    color *= applyDiffuseAO(curvedUV, ambientOcclusion);

    // Color banding
    color = posterize(color, colorBanding);

    // Noise
    color = applyNoise(color, curvedUV, noiseIntensity);

    // Saturation
    color = applySaturation(color, saturation);

    // Contraste (increased)
    color = ((color - 0.5) * 1.2) + 0.5;

    // Teinte chaleureuse
    vec3 warmTint = vec3(1.0, 0.98, 0.95);
    color *= mix(vec3(1.0), warmTint, 0.12);

    // Apply edge fade to eliminate border artifacts
    color *= fade;

    // Clamp
    color = clamp(color, 0.0, 1.0);

    gl_FragColor = vec4(color, 1.0);
}
