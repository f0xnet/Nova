// ============================================================================
// SSAO SHADER — ombre de contact entre zones de couleurs différentes
// ============================================================================
// 3 anneaux concentriques × 8 directions = 24 samples (vs 576 original).
// Les poids décroissants (1.0 / 0.45 / 0.15) reconstituent le gradient :
//   - pixel collé à un bord  → contraste détecté aux 3 anneaux → fort AO
//   - pixel à mi-distance    → contraste sur anneau 2 et 3 → AO moyen
//   - pixel loin de tout bord → aucun contraste → aucun AO
// ============================================================================
#version 120

uniform sampler2D tex;
uniform vec2 texSize;
uniform vec2 resolution;
uniform float aoStrength;
uniform float aoRadius;

vec3 sampleTex(vec2 uv) {
    return texture2D(tex, clamp(uv, 0.0, 1.0)).rgb;
}

float computeAO(vec2 uv) {
    if (aoStrength <= 0.0) return 1.0;

    vec2 pixelSize  = 1.0 / resolution;
    float centerLum = dot(sampleTex(uv), vec3(0.299, 0.587, 0.114));

    vec2 dirs[8];
    dirs[0] = vec2( 1.0,    0.0   );
    dirs[1] = vec2(-1.0,    0.0   );
    dirs[2] = vec2( 0.0,    1.0   );
    dirs[3] = vec2( 0.0,   -1.0   );
    dirs[4] = vec2( 0.707,  0.707 );
    dirs[5] = vec2(-0.707,  0.707 );
    dirs[6] = vec2( 0.707, -0.707 );
    dirs[7] = vec2(-0.707, -0.707 );

    float occlusion = 0.0;

    for (int i = 0; i < 8; i++) {
        vec2 dir = dirs[i] * pixelSize;

        float lum1 = dot(sampleTex(uv + dir * aoRadius * 0.30), vec3(0.299, 0.587, 0.114));
        float lum2 = dot(sampleTex(uv + dir * aoRadius * 0.60), vec3(0.299, 0.587, 0.114));
        float lum3 = dot(sampleTex(uv + dir * aoRadius       ), vec3(0.299, 0.587, 0.114));

        float diff1 = abs(centerLum - lum1);
        float diff2 = abs(centerLum - lum2);
        float diff3 = abs(centerLum - lum3);

        if (diff1 > 0.12) occlusion += diff1 * 1.00;
        if (diff2 > 0.12) occlusion += diff2 * 0.45;
        if (diff3 > 0.12) occlusion += diff3 * 0.15;
    }

    return 1.0 - clamp(occlusion * aoStrength * 0.25, 0.0, 0.85);
}

void main()
{
    vec2 uv = gl_TexCoord[0].xy / texSize;
    uv.y = 1.0 - uv.y;

    vec3 color = sampleTex(uv);
    color *= computeAO(uv);

    gl_FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
