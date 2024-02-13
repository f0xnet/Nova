#version 120

uniform sampler2D texture;     // Texture de la scène
uniform float time;            // Temps actuel
uniform vec2 resolution;       // Résolution de l'écran ou de la texture

float noise(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

float layeredNoise(vec2 uv, float time) {
    float totalNoise = 0.0;
    float scale = 0.5;
    float weight = 1.0;
    float weightSum = 0.0;

    for (int i = 0; i < 3; i++) {
        totalNoise += weight * noise(uv * scale + time);
        weightSum += weight;
        scale *= 3.0;
        weight *= 0.5;
    }

    return totalNoise / weightSum;
}

void main() {
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    uv.y = 1.0 - uv.y; // Inverser l'axe Y

    float fogSpeed = 0.1;
    vec2 fogMovement = vec2(fogSpeed * time, fogSpeed * time / 2.0);

    float v = layeredNoise(uv + fogMovement, time);

    vec4 sceneColor = texture2D(texture, uv);

    // Gradation de l'intensité de la brume de bas en haut
    float fogHeight = 0.9; // Hauteur à partir de laquelle la brume commence à s'estomper
    float fogIntensity = smoothstep(0.0, fogHeight, uv.y) * 0.7;

    vec4 fogColor = vec4(v * fogIntensity, v * fogIntensity, v * fogIntensity, 0.6);
    vec4 finalColor = mix(sceneColor, fogColor, v * fogIntensity);

    gl_FragColor = finalColor;
}
