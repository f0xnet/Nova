// ============================================================================
// SHADER CRT COMPLET - FORMAT .FRAG
// ============================================================================
// Basé sur analyse approfondie de 2 images
// 15 effets distincts pour reproduction fidèle
// Compatible SFML
// ============================================================================

#version 120

uniform sampler2D texture;
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

const float PI = 3.14159265359;

// ============================================================================
// COURBURE CRT
// ============================================================================
vec2 curveCRT(vec2 uv) {
    if (curvature <= 0.0) return uv;

    uv = uv * 2.0 - 1.0;
    vec2 offset = abs(uv.yx) / vec2(5.0, 4.0);
    uv = uv + uv * offset * offset * curvature;

    return uv * 0.5 + 0.5;
}

// ============================================================================
// ABERRATION CHROMATIQUE
// ============================================================================
vec3 chromaticAberrationEffect(sampler2D tex, vec2 uv, float amount) {
    if (amount <= 0.0) return texture2D(tex, uv).rgb;

    vec2 direction = uv - 0.5;
    float dist = length(direction);
    direction = normalize(direction);

    float aberration = amount * dist * 1.5;

    float r = texture2D(tex, uv + direction * aberration).r;
    float g = texture2D(tex, uv).g;
    float b = texture2D(tex, uv - direction * aberration).b;

    return vec3(r, g, b);
}

// ============================================================================
// RGB SHIFT HORIZONTAL (VHS)
// ============================================================================
vec3 rgbShift(sampler2D tex, vec2 uv, float amount) {
    if (amount <= 0.0) return texture2D(tex, uv).rgb;

    float shift = sin(uv.y * 100.0 + time * 2.0) * amount;

    float r = texture2D(tex, uv + vec2(shift, 0.0)).r;
    float g = texture2D(tex, uv).g;
    float b = texture2D(tex, uv - vec2(shift, 0.0)).b;

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
// GLOW/BLOOM
// ============================================================================
vec3 applyGlow(sampler2D tex, vec2 uv, float intensity) {
    vec3 color = texture2D(tex, uv).rgb;
    float brightness = dot(color, vec3(0.299, 0.587, 0.114));

    if (brightness > 0.6) {
        vec2 pixelSize = 1.0 / resolution;
        vec3 glow = vec3(0.0);

        for (float x = -3.0; x <= 3.0; x += 1.0) {
            for (float y = -3.0; y <= 3.0; y += 1.0) {
                vec2 offset = vec2(x, y) * pixelSize * 1.5;
                glow += texture2D(tex, uv + offset).rgb;
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
vec3 applyGhosting(sampler2D tex, vec2 uv) {
    vec3 color = texture2D(tex, uv).rgb;

    vec2 offset = vec2(0.003, 0.0);
    vec3 ghost1 = texture2D(tex, uv + offset).rgb * 0.15;
    vec3 ghost2 = texture2D(tex, uv + offset * 2.0).rgb * 0.08;

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
// INTERFERENCE PATTERN
// ============================================================================
float interference(vec2 uv) {
    float pattern = sin(uv.y * 200.0 + time * 5.0) * 0.5 + 0.5;
    pattern = smoothstep(0.3, 0.7, pattern);

    return mix(1.0, pattern, 0.03);
}

// ============================================================================
// MAIN
// ============================================================================
void main()
{
    vec2 uv = gl_TexCoord[0].xy;

    // Courbure CRT
    vec2 curvedUV = curveCRT(uv);

    // Bords noirs
    if (curvedUV.x < 0.0 || curvedUV.x > 1.0 ||
        curvedUV.y < 0.0 || curvedUV.y > 1.0) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    // Aberration chromatique
    vec3 color = chromaticAberrationEffect(texture, curvedUV, chromaticAberration);

    // RGB Shift
    color = mix(color, rgbShift(texture, curvedUV, rgbShiftAmount), 0.6);

    // Ghosting
    color = mix(color, applyGhosting(texture, curvedUV), 0.3);

    // Glow
    color = applyGlow(texture, curvedUV, glowIntensity);

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

    // Color banding
    color = posterize(color, colorBanding);

    // Noise
    color = applyNoise(color, curvedUV, noiseIntensity);

    // Contraste
    color = ((color - 0.5) * 1.15) + 0.5;

    // Teinte chaleureuse
    vec3 warmTint = vec3(1.0, 0.98, 0.95);
    color *= mix(vec3(1.0), warmTint, 0.12);

    // Clamp
    color = clamp(color, 0.0, 1.0);

    gl_FragColor = vec4(color, 1.0);
}
