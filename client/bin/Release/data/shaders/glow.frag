#version 330 core

uniform sampler2D scene; 
uniform vec2 imageSize; 
uniform vec2 imagePosition;

out vec4 FragColor;

// Fonction pour appliquer un flou gaussien plus intense
vec4 blur(sampler2D image, vec2 uv, vec2 onePixel) {
    vec4 color = vec4(0.0);
    float sigma = 0.8; // Augmenter sigma pour un flou plus fort
    float Z = 0.0;
    for (int x = -3; x <= 3; x++) {
        for (int y = -3; y <= 3; y++) {
            float weight = exp(-(x * x + y * y) / (2.0 * sigma * sigma));
            color += texture(image, uv + vec2(float(x), float(y)) * onePixel) * weight;
            Z += weight;
        }
    }
    return color / Z;
}

// Fonction pour calculer la luminosité d'un pixel
float luminance(vec4 color) {
    return 0.299 * color.r + 0.587 * color.g + 0.114 * color.b;
}

void main() {
    vec2 uv = (gl_FragCoord.xy - imagePosition) / imageSize;
    // Inversion de l'axe Y si nécessaire
    // uv.y = 1.0 - uv.y;

    vec4 original = texture(scene, uv);
    vec2 onePixel = vec2(1.0) / imageSize;

    // Appliquer le flou sur les pixels plus lumineux
    if (luminance(original) > 0.1) { // Baisser le seuil de luminosité pour inclure plus de pixels
        vec4 blurred = blur(scene, uv, onePixel);
        FragColor = mix(original, blurred, 1); // Augmenter le facteur de mélange pour un glow plus visible
    } else {
        FragColor = original;
    }
}
