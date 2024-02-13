#version 120

uniform sampler2D texture;     // Texture de la scène
uniform float time;            // Temps actuel
uniform vec2 resolution;       // Résolution de l'écran ou de la texture

// Fonction pour générer du bruit simple
float noise(vec2 p) {
    return fract(sin(dot(p ,vec2(12.9898,78.233))) * 43758.5453);
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    uv.y = 1.0 - uv.y; // Inverser l'axe Y

    vec4 sceneColor = texture2D(texture, uv);

    // Générer un bruit simple
    float v = noise(uv * time);

    // Utiliser le bruit pour moduler la couleur de la brume
    vec4 fogColor = vec4(v, v, v, 1.0); 
    vec4 finalColor = mix(sceneColor, fogColor, 0.5); // Mélange à 50%

    gl_FragColor = finalColor;
}
