// ============================================================================
// GAUSSIAN BLUR SHADER
// ============================================================================
// Simple blur gaussien 5x5 pour adoucir l'AO
// ============================================================================
#version 120

uniform sampler2D tex;
uniform vec2 texSize;
uniform vec2 resolution;
uniform float blurRadius;  // Rayon du blur en pixels

// ============================================================================
// MAIN
// ============================================================================
void main()
{
    // Normaliser UV de pixel coords à 0-1, et flip Y (SFML RenderTexture)
    vec2 uv = gl_TexCoord[0].xy / texSize;
    uv.y = 1.0 - uv.y;

    vec2 pixelSize = 1.0 / resolution;

    // Blur gaussien 5x5 avec poids
    float result = 0.0;
    float totalWeight = 0.0;

    for (float x = -2.0; x <= 2.0; x += 1.0) {
        for (float y = -2.0; y <= 2.0; y += 1.0) {
            vec2 offset = vec2(x, y) * pixelSize * blurRadius;
            float dist = length(vec2(x, y));
            float weight = exp(-dist * dist / 4.0);

            result += texture2D(tex, clamp(uv + offset, 0.0, 1.0)).r * weight;
            totalWeight += weight;
        }
    }

    result /= totalWeight;

    // Output grayscale
    gl_FragColor = vec4(result, result, result, 1.0);
}
