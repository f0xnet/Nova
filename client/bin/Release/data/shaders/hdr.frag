uniform sampler2D texture; // Texture d'entrée
uniform float exposure = 1.0; // Exposition
uniform float gamma = 2.2; // Correction gamma
uniform float saturationStrength = 0.1; // Force de saturation
uniform float auraStrength = 0.2; // Force de l'aura
uniform float auraBoost = 0.5; // Augmentation de l'aura pour les zones claires

void main() {
    vec3 hdrColor = texture2D(texture, gl_TexCoord[0].xy).rgb; // Couleur HDR de la texture

    // Tonemapping
    vec3 mappedColor = hdrColor / (hdrColor + vec3(1.0));
    mappedColor = vec3(1.0) - exp(-mappedColor * exposure);

    // Correction gamma
    mappedColor = pow(mappedColor, vec3(1.0 / gamma));

    // Saturation des couleurs
    vec3 intensity = vec3(dot(mappedColor, vec3(0.2125, 0.7154, 0.0721)));
    mappedColor = mix(intensity, mappedColor, saturationStrength);

    // Effet d'aura
    vec3 auraColor = vec3(1.0) - exp(-hdrColor * auraStrength);

    // Booster l'effet d'aura sur les zones claires
    float boostFactor = smoothstep(0.5, 0.8, intensity.x); // Adaptez ces valeurs en fonction de l'intensité du texte et du bouton
    auraColor += auraBoost * boostFactor;

    // Appliquer l'aura
    mappedColor += auraColor;

    gl_FragColor = vec4(mappedColor, 1.0);
}


/* Fragment shader pour HDR avec tonemapping

        shader.setUniform("texture", sf::Shader::CurrentTexture);
        shader.setUniform("exposure", 6.0f); // Modifier si nécessaire
        shader.setUniform("gamma", 0.30f); // Modifier si nécessaire
        shader.setUniform("saturationStrength", 1.4f); // Modifier si nécessaire
        shader.setUniform("auraStrength", 0.5f); // Modifier si nécessair
        shader.setUniform("auraBoost", 0.1f); // Modifier si nécessaire

*/