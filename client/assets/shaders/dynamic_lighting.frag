#version 120

#define MAX_LIGHTS 8

uniform sampler2D tex;
uniform vec2 texSize;
uniform int numLights;                      // Nombre de lumières actives
uniform vec2 lightPositions[MAX_LIGHTS];    // Positions normalisées (0-1)
uniform vec3 lightColors[MAX_LIGHTS];       // Couleurs RGB (0-1)
uniform float lightRadius[MAX_LIGHTS];      // Rayons normalisés (0-1)
uniform float lightIntensity[MAX_LIGHTS];   // Intensité de teinte (0-1)
uniform float ambientDarkness;              // Obscurité ambiante (0-1)

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

        vec2 lightPos = lightPositions[i];
        vec3 lightColor = lightColors[i];
        float radius = lightRadius[i];
        float tintIntensity = lightIntensity[i];

        // Distance normalisée entre le fragment et la lumière
        float dist = distance(uv, lightPos) / radius;

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
