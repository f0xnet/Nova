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
uniform float timeOfDay;                    // Temps de la journée (0.0 = minuit, 0.5 = midi, 1.0 = minuit)

// Convertir position monde → position écran normalisée (0-1)
vec2 worldToScreen(vec2 worldPos) {
    vec2 screenPos = (worldPos - cameraPosition + viewportSize * 0.5) / viewportSize;
    // Flip Y pour correspondre au flip Y de uv
    screenPos.y = 1.0 - screenPos.y;
    return screenPos;
}

// Calculer la teinte ambiante selon l'heure de la journée
vec3 getTimeOfDayTint(float time) {
    // Couleurs pour différents moments de la journée
    vec3 nightColor = vec3(0.2, 0.2, 0.4);      // Bleu sombre (nuit)
    vec3 dawnColor = vec3(1.0, 0.6, 0.4);       // Orange (aube)
    vec3 dayColor = vec3(1.0, 1.0, 1.0);        // Blanc (jour)
    vec3 duskColor = vec3(1.0, 0.5, 0.3);       // Orange-rouge (crépuscule)

    // Transitions douces entre les périodes
    // 0.0 - 0.2 : Nuit
    // 0.2 - 0.3 : Aube
    // 0.3 - 0.7 : Jour
    // 0.7 - 0.8 : Crépuscule
    // 0.8 - 1.0 : Nuit

    vec3 tint = nightColor;

    // Aube (0.2 -> 0.3)
    if (time > 0.2 && time <= 0.3) {
        float t = (time - 0.2) / 0.1;
        tint = mix(nightColor, dawnColor, smoothstep(0.0, 1.0, t));
    }
    // Transition aube -> jour (0.3 -> 0.35)
    else if (time > 0.3 && time <= 0.35) {
        float t = (time - 0.3) / 0.05;
        tint = mix(dawnColor, dayColor, smoothstep(0.0, 1.0, t));
    }
    // Jour (0.35 -> 0.65)
    else if (time > 0.35 && time <= 0.65) {
        tint = dayColor;
    }
    // Transition jour -> crépuscule (0.65 -> 0.7)
    else if (time > 0.65 && time <= 0.7) {
        float t = (time - 0.65) / 0.05;
        tint = mix(dayColor, duskColor, smoothstep(0.0, 1.0, t));
    }
    // Crépuscule (0.7 -> 0.8)
    else if (time > 0.7 && time <= 0.8) {
        float t = (time - 0.7) / 0.1;
        tint = mix(duskColor, nightColor, smoothstep(0.0, 1.0, t));
    }
    // Nuit (0.8 -> 1.0 et 0.0 -> 0.2)
    else {
        tint = nightColor;
    }

    return tint;
}

// Calculer l'obscurité ambiante selon l'heure
float getAmbientBrightness(float time) {
    // Plus sombre la nuit, plus clair le jour
    float brightness = 0.3; // Nuit par défaut

    // Aube (0.2 -> 0.35)
    if (time > 0.2 && time <= 0.35) {
        float t = (time - 0.2) / 0.15;
        brightness = mix(0.3, 1.0, smoothstep(0.0, 1.0, t));
    }
    // Jour (0.35 -> 0.65)
    else if (time > 0.35 && time <= 0.65) {
        brightness = 1.0;
    }
    // Crépuscule (0.65 -> 0.8)
    else if (time > 0.65 && time <= 0.8) {
        float t = (time - 0.65) / 0.15;
        brightness = mix(1.0, 0.3, smoothstep(0.0, 1.0, t));
    }

    return brightness;
}

void main() {
    // UV normalisés (0-1) - diviser par texSize pour normaliser de pixels → 0-1
    vec2 uv = gl_TexCoord[0].xy / texSize;

    // Flip Y pour RenderTexture
    uv.y = 1.0 - uv.y;

    // Récupérer la couleur de la texture
    vec4 color = texture2D(tex, uv);

    // Calculer la teinte et la luminosité selon l'heure
    vec3 timeOfDayTint = getTimeOfDayTint(timeOfDay);
    float ambientBrightness = getAmbientBrightness(timeOfDay);

    // Appliquer la teinte et la luminosité ambiante
    vec3 finalColor = color.rgb * timeOfDayTint * ambientBrightness;

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
