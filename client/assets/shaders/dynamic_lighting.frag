#version 120

#define MAX_LIGHTS 8

uniform sampler2D tex;
uniform vec2 texSize;
uniform int numLights;                      // Nombre de lumières actives
uniform vec2 lightPositions[MAX_LIGHTS];    // Positions monde (pixels)
uniform vec3 lightColors[MAX_LIGHTS];       // Couleurs RGB (0-1)
uniform float lightRadius[MAX_LIGHTS];      // Rayons monde (pixels)
uniform float lightIntensity[MAX_LIGHTS];   // Intensité de teinte (0-1)
uniform float ambientDarkness;              // Obscurité ambiante (0-1)
uniform vec2 cameraPosition;                // Position caméra monde (pixels)
uniform vec2 viewportSize;                  // Taille viewport (pixels)

// Convertir position monde → position écran normalisée (0-1)
vec2 worldToScreen(vec2 worldPos) {
    vec2 screenPos = (worldPos - cameraPosition + viewportSize * 0.5) / viewportSize;
    // Flip Y pour correspondre au flip Y de uv
    screenPos.y = 1.0 - screenPos.y;
    return screenPos;
}

void main() {
    // UV normalisés (0-1) - diviser par texSize pour normaliser de pixels → 0-1
    vec2 uv = gl_TexCoord[0].xy / texSize;

    // Flip Y pour RenderTexture
    uv.y = 1.0 - uv.y;

    // Récupérer la couleur de la texture
    vec4 color = texture2D(tex, uv);

    // Assombrir la scène entière
    vec3 finalColor = color.rgb * ambientDarkness;

    // Appliquer l'éclairage pour chaque lumière
    for(int i = 0; i < MAX_LIGHTS; ++i) {
        if(i >= numLights) break;

        // Convertir position monde → écran
        vec2 lightScreenPos = worldToScreen(lightPositions[i]);
        vec3 lightColor = lightColors[i];

        // Convertir rayon monde → rayon normalisé
        float normalizedRadius = lightRadius[i] / viewportSize.x;
        float tintIntensity = lightIntensity[i];

        // Distance normalisée entre le fragment et la lumière
        float dist = distance(uv, lightScreenPos) / normalizedRadius;

        // Si le fragment est dans le rayon de la lumière
        if(dist < 1.0) {
            // Atténuation douce (smoothstep pour transition progressive)
            float attenuation = smoothstep(0.0, 0.8, 1.0 - dist);

            // Mélanger la couleur d'origine avec la couleur assombrie
            finalColor = mix(finalColor, color.rgb, attenuation);

            // Appliquer la teinte de couleur (plus forte au centre)
            finalColor += lightColor * tintIntensity * attenuation;
        }
    }

    gl_FragColor = vec4(finalColor, color.a);
}
