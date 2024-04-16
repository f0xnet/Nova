#define NUM_LIGHTS 3 // Définir le nombre de lumières

uniform sampler2D texture;
uniform float screenWidth;
uniform float screenHeight;
uniform vec2 lightPositions[NUM_LIGHTS]; // Déclare les positions des lumières
uniform vec3 lightColors[NUM_LIGHTS];    // Déclare les couleurs des lumières
uniform float lightRadius[NUM_LIGHTS];   // Déclare les rayons des lumières
uniform float lightTintIntensity[NUM_LIGHTS]; 

void main() {
    vec2 uv = gl_FragCoord.xy / vec2(screenWidth, screenHeight);
    vec4 color = texture2D(texture, uv);

    // Darken the texture
    vec3 finalColor = color.rgb * 0.01; // Assombris l'image

    // Apply lighting for each light
    for(int i = 0; i < NUM_LIGHTS; ++i) {
        vec2 lightPos = lightPositions[i];
        vec3 lightColor = lightColors[i];
        float radius = lightRadius[i]; // Récupère le rayon de la lumière

        // Calcule la distance normalisée entre le fragment et la lumière
        float distance = distance(lightPos, gl_FragCoord.xy) / radius;

        // Si le fragment est à l'intérieur du rayon de la lumière, applique l'illumination
        if (distance < 1.0) {
            // Utilise une plage plus large pour une transition plus douce de l'assombrissement
            float attenuation = smoothstep(0.0, 0.8, 1.0 - distance);

            // Applique la couleur de base mélangée avec l'atténuation
            finalColor = mix(finalColor, color.rgb, attenuation);

            // Applique la teinte de couleur de manière à ce qu'elle soit plus forte au centre et s'atténue vers les bords
            finalColor += lightColor * lightTintIntensity[i] * attenuation;
        }
    }

    // Output final color
    gl_FragColor = vec4(finalColor, color.a);
}

/* How to use : 

const int NUM_LIGHTS = 3;

main();

// Définir les positions, les couleurs et les rayons des lumières
    sf::Vector2f lightPositions[NUM_LIGHTS] = {
        sf::Vector2f(800.0f, 800.0f),   // Position de la première lumière (rouge)
        sf::Vector2f(140.0f, 400.0f),   // Position de la deuxième lumière (blanche)
        sf::Vector2f(000.0f, 000.0f)    // Position de la troisième lumière (verte)
    };
    sf::Vector3f lightColors[NUM_LIGHTS] = {
        sf::Vector3f(1.0f, 0.0f, 0.0f), // Couleur de la première lumière (rouge)
        sf::Vector3f(1.0f, 0.0f, 0.0f), // Couleur de la deuxième lumière (blanche)
        sf::Vector3f(0.0f, 1.0f, 0.0f)  // Couleur de la troisième lumière (verte)
    };
    float lightRadius[NUM_LIGHTS] = {
        600.0f,  // Rayon de la première lumière (rouge)
        600.0f,  // Rayon de la deuxième lumière (blanche)
        100.0f   // Rayon de la troisième lumière (verte)
    };
    float lightTintIntensityValue[NUM_LIGHTS] = {
        0.09,  // Intensité de la première lumière (rouge)
        0.9,  // Intensité de la deuxième lumière (blanche)
        0.9   // Intensité de la troisième lumière (verte)
    };

    while (renderWindow->isOpen()) {
        sf::Event event;
        while (renderWindow->pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                renderWindow->close();
        }
        renderWindow->clear(sf::Color::Transparent);

        this->uimanager->show(event);

        texture.update(*renderWindow);
        sprite.setTexture(texture);

        // Mettre à jour les uniformes du shader avec les données actuelles
        shader.setUniform("screenWidth", static_cast<float>(renderWindow->getSize().x));
        shader.setUniform("screenHeight", static_cast<float>(renderWindow->getSize().y));
        shader.setUniformArray("lightPositions", &lightPositions[0], NUM_LIGHTS);
        shader.setUniformArray("lightColors", &lightColors[0], NUM_LIGHTS);
        shader.setUniformArray("lightRadius", &lightRadius[0], NUM_LIGHTS); // Ajoute les rayons des lumières

// Avant de dessiner avec le shader
shader.setUniformArray("lightTintIntensity", &lightTintIntensityValue[0], NUM_LIGHTS); 
*/