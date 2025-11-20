// ============================================================================
// SSAO SHADER (Screen-Space Ambient Occlusion)
// ============================================================================
// Détecte les bords des objets et calcule l'occlusion
// Output: Grayscale AO map (1.0 = pas d'ombre, <1.0 = ombre)
// ============================================================================
#version 120

uniform sampler2D tex;
uniform vec2 texSize;
uniform vec2 resolution;
uniform float aoStrength;  // Force de l'AO (0.0 - 1.0+)
uniform float aoRadius;    // Rayon de recherche en pixels

// ============================================================================
// TEXTURE SAMPLING
// ============================================================================
vec3 sampleTex(vec2 uv) {
    return texture2D(tex, clamp(uv, 0.0, 1.0)).rgb;
}

// ============================================================================
// AMBIENT OCCLUSION (ombres s'estompant depuis les bords des objets)
// ============================================================================
float computeAO(vec2 uv) {
    if (aoStrength <= 0.0) return 1.0;

    vec2 pixelSize = 1.0 / resolution;

    // Chercher la distance au bord le plus proche
    float minDistToEdge = 999.0;

    // Échantillonner en cercle pour trouver les bords
    for (float angle = 0.0; angle < 6.28318; angle += 0.523599) { // 12 directions
        vec2 dir = vec2(cos(angle), sin(angle));

        for (float dist = 1.0; dist <= aoRadius; dist += 1.0) {
            vec2 sampleUV = uv + dir * dist * pixelSize;

            // Détecter si ce point est un bord
            vec3 sampleColor = sampleTex(sampleUV);
            float sampleLum = dot(sampleColor, vec3(0.299, 0.587, 0.114));

            // Vérifier le contraste avec les voisins pour détecter un bord
            float edgeStrength = 0.0;
            for (float checkAngle = 0.0; checkAngle < 6.28318; checkAngle += 2.094395) { // 3 checks
                vec2 checkDir = vec2(cos(checkAngle), sin(checkAngle));
                vec2 checkUV = sampleUV + checkDir * pixelSize;
                float checkLum = dot(sampleTex(checkUV), vec3(0.299, 0.587, 0.114));
                edgeStrength += abs(sampleLum - checkLum);
            }

            // Si c'est un bord (fort contraste)
            if (edgeStrength > 0.2) {
                if (dist < minDistToEdge) minDistToEdge = dist;
                break; // Trouvé un bord dans cette direction
            }
        }
    }

    // Calculer l'occlusion basée sur la distance au bord
    float occlusion = 0.0;
    if (minDistToEdge < aoRadius) {
        // Plus on est proche d'un bord, plus l'ombre est forte
        float distFactor = minDistToEdge / aoRadius;
        occlusion = (1.0 - distFactor) * (1.0 - distFactor) * (1.0 - distFactor);
        occlusion *= aoStrength;
    }

    // Retourner le facteur d'assombrissement (1.0 = pas d'ombre, <1.0 = ombre)
    return 1.0 - clamp(occlusion * 0.9, 0.0, 0.9);
}

// ============================================================================
// MAIN
// ============================================================================
void main()
{
    // Normaliser UV de pixel coords à 0-1, et flip Y (SFML RenderTexture)
    vec2 uv = gl_TexCoord[0].xy / texSize;
    uv.y = 1.0 - uv.y;

    // Calculer l'AO
    float ao = computeAO(uv);

    // Output grayscale
    gl_FragColor = vec4(ao, ao, ao, 1.0);
}
