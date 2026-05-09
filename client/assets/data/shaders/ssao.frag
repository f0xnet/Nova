// ============================================================================
// SSAO SHADER (Screen-Space Ambient Occlusion)
// ============================================================================
// Détecte les bords des objets et applique des ombres de contact.
// Kernel fixe 8 directions — coût constant, pas de boucle conditionnelle.
// ============================================================================
#version 120

uniform sampler2D tex;
uniform vec2 texSize;
uniform vec2 resolution;
uniform float aoStrength;  // Force de l'AO (0.0 - 1.0+)
uniform float aoRadius;    // Rayon de recherche en pixels

vec3 sampleTex(vec2 uv) {
    return texture2D(tex, clamp(uv, 0.0, 1.0)).rgb;
}

float computeAO(vec2 uv) {
    if (aoStrength <= 0.0) return 1.0;

    vec2 pixelSize = 1.0 / resolution;
    float centerLum = dot(sampleTex(uv), vec3(0.299, 0.587, 0.114));

    // 8 directions à rayon fixe — coût constant, aucune divergence de warp
    vec2 offsets[8];
    offsets[0] = vec2( 1.0,    0.0   );
    offsets[1] = vec2(-1.0,    0.0   );
    offsets[2] = vec2( 0.0,    1.0   );
    offsets[3] = vec2( 0.0,   -1.0   );
    offsets[4] = vec2( 0.707,  0.707 );
    offsets[5] = vec2(-0.707,  0.707 );
    offsets[6] = vec2( 0.707, -0.707 );
    offsets[7] = vec2(-0.707, -0.707 );

    float occlusion = 0.0;
    for (int i = 0; i < 8; i++) {
        vec2 sampleUV  = uv + offsets[i] * aoRadius * pixelSize;
        float sampleLum = dot(sampleTex(sampleUV), vec3(0.299, 0.587, 0.114));
        float diff = centerLum - sampleLum;
        if (diff > 0.15) occlusion += diff;
    }

    return 1.0 - clamp(occlusion * aoStrength, 0.0, 0.9);
}

void main()
{
    vec2 uv = gl_TexCoord[0].xy / texSize;
    uv.y = 1.0 - uv.y;

    vec3 color = sampleTex(uv);
    color *= computeAO(uv);

    gl_FragColor = vec4(clamp(color, 0.0, 1.0), 1.0);
}
