uniform sampler2D texture;
uniform float screenWidth;
uniform float screenHeight;

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(screenWidth, screenHeight);
    vec4 color = texture2D(texture, uv);

    // Test: Renvoyer directement la couleur pour le débogage
    //gl_FragColor = color;

    // Une fois confirmé que cela fonctionne, réintroduire l'occlusion :
    float radius = 0.005; // Réduire le rayon
    float intensity = 0.00; // Diminuer l'intensité
    float threshold = 0.0; // Abaisser le seuil
    float total = 0.0;
    float occlusion = 0.0;

    for (float x = -radius; x <= radius; x += radius / 4.0) {
        for (float y = -radius; y <= radius; y += radius / 4.0) {
            vec2 sampleUv = uv + vec2(x, y);
            vec4 sampleColor = texture2D(texture, sampleUv);
            float diff = (color.r + color.g + color.b) - (sampleColor.r + sampleColor.g + sampleColor.b);
            occlusion += step(threshold, diff);
            total += 1.0;
        }
    }

    occlusion = 1.0 - (occlusion / total) * intensity;
    gl_FragColor = vec4(color.rgb * occlusion, color.a);
}
