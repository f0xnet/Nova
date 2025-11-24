#version 120

#define MAX_LIGHTS 8
#define LIGHT_TYPE_POINT 0
#define LIGHT_TYPE_DIRECTIONAL 1
#define LIGHT_TYPE_SPOT 2

uniform sampler2D tex;
uniform vec2 texSize;
uniform int numLights;                      // Nombre de lumières actives
uniform vec2 lightPositions[MAX_LIGHTS];    // Positions monde (pixels)
uniform vec3 lightColors[MAX_LIGHTS];       // Couleurs RGB (0-1)
uniform float lightRadius[MAX_LIGHTS];      // Rayons monde (pixels)
uniform float lightIntensity[MAX_LIGHTS];   // Intensité de teinte (0-1)
uniform float lightTypes[MAX_LIGHTS];       // Type de lumière (0=Point, 1=Directional, 2=Spot) - float for compatibility
uniform vec2 lightDirections[MAX_LIGHTS];   // Direction pour Directional/Spot
uniform float lightAngles[MAX_LIGHTS];      // Angle du cône pour Spot (degrés)
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

// Convertir direction monde → direction écran (flip Y pour correspondre au système écran)
vec2 worldDirectionToScreen(vec2 worldDir) {
    // Normaliser la direction et flipper Y
    float len = length(worldDir);
    if (len > 0.0) {
        vec2 normalized = worldDir / len;
        normalized.y = -normalized.y; // Flip Y pour correspondre au système écran
        return normalized;
    }
    return vec2(0.0, -1.0); // Direction par défaut vers le bas (après flip)
}

// Calculer la teinte ambiante selon l'heure de la journée
vec3 getTimeOfDayTint(float time) {
    // Couleurs pour différents moments de la journée
    vec3 nightColor = vec3(0.2, 0.2, 0.4);      // Bleu sombre (nuit)
    vec3 dawnColor = vec3(1.0, 0.6, 0.4);       // Orange (aube)
    vec3 dayColor = vec3(1.0, 1.0, 1.0);        // Blanc (jour)
    vec3 duskColor = vec3(1.0, 0.5, 0.3);       // Orange-rouge (crépuscule)

    // Transitions douces entre les périodes
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

    return tint;
}

// Calculer l'obscurité ambiante selon l'heure
float getAmbientBrightness(float time) {
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

// Calculer l'atténuation pour une lumière Point
float calculatePointLight(vec2 uv, vec2 lightScreenPos, float normalizedRadius) {
    float dist = distance(uv, lightScreenPos) / normalizedRadius;
    if (dist < 1.0) {
        return smoothstep(0.0, 0.8, 1.0 - dist);
    }
    return 0.0;
}

// Calculer l'atténuation pour une lumière Directional
float calculateDirectionalLight(vec2 uv, vec2 lightDirection, float normalizedRadius) {
    // Lumière directionnelle : même intensité partout dans le rayon
    // On peut utiliser le rayon pour contrôler l'intensité globale
    // Pour une vraie directionnelle (soleil), on voudrait radius = infini
    // Ici on simule avec un fade doux sur toute la scène
    return 1.0;
}

// Calculer l'atténuation pour une lumière Spot
float calculateSpotLight(vec2 uv, vec2 lightScreenPos, vec2 lightDirection, float angle, float normalizedRadius) {
    // Vecteur du centre de la lumière vers le fragment
    vec2 toFragment = uv - lightScreenPos;
    float dist = length(toFragment);

    // Si trop loin, pas d'éclairage
    float normalizedDist = dist / normalizedRadius;
    if (normalizedDist >= 1.0) {
        return 0.0;
    }

    // Normaliser la direction vers le fragment
    vec2 fragDir = toFragment / dist;

    // Calculer l'angle entre la direction de la lumière et la direction vers le fragment
    // dot product donne le cosinus de l'angle
    float cosAngle = dot(fragDir, lightDirection);
    float halfAngleRad = radians(angle * 0.5);
    float cosHalfAngle = cos(halfAngleRad);

    // Si hors du cône, pas d'éclairage
    if (cosAngle < cosHalfAngle) {
        return 0.0;
    }

    // Atténuation basée sur la distance
    float distAttenuation = smoothstep(0.0, 0.8, 1.0 - normalizedDist);

    // Atténuation basée sur l'angle (plus doux au bord du cône)
    float angleAttenuation = smoothstep(cosHalfAngle, 1.0, cosAngle);

    return distAttenuation * angleAttenuation;
}

void main() {
    // UV normalisés (0-1)
    vec2 uv = gl_TexCoord[0].xy / texSize;
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

        vec3 lightColor = lightColors[i];
        float tintIntensity = lightIntensity[i];
        float normalizedRadius = lightRadius[i] / viewportSize.x;

        float attenuation = 0.0;

        // Calculer l'atténuation selon le type de lumière
        if (lightTypes[i] == LIGHT_TYPE_POINT) {
            vec2 lightScreenPos = worldToScreen(lightPositions[i]);
            attenuation = calculatePointLight(uv, lightScreenPos, normalizedRadius);
        }
        else if (lightTypes[i] == LIGHT_TYPE_DIRECTIONAL) {
            vec2 lightDir = worldDirectionToScreen(lightDirections[i]);
            attenuation = calculateDirectionalLight(uv, lightDir, normalizedRadius);
            attenuation *= tintIntensity; // Pour directionnelle, utiliser intensity comme facteur global
        }
        else if (lightTypes[i] == LIGHT_TYPE_SPOT) {
            vec2 lightScreenPos = worldToScreen(lightPositions[i]);
            vec2 lightDir = worldDirectionToScreen(lightDirections[i]);
            attenuation = calculateSpotLight(uv, lightScreenPos, lightDir, lightAngles[i], normalizedRadius);
        }

        if (attenuation > 0.0) {
            // Mélanger la couleur d'origine avec la couleur assombrie
            finalColor = mix(finalColor, color.rgb, attenuation);

            // Appliquer la teinte de couleur (plus forte au centre)
            finalColor += lightColor * tintIntensity * attenuation;
        }
    }

    gl_FragColor = vec4(finalColor, color.a);
}
