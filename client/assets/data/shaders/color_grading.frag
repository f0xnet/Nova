// ============================================================================
// COLOR GRADING SHADER
// ============================================================================
// Ajustements de couleur: saturation, contraste, luminosité, etc.
// ============================================================================
#version 120

uniform sampler2D tex;
uniform vec2 texSize;
uniform float saturation;   // 0.0 = grayscale, 1.0 = normal, >1.0 = saturated
uniform float contrast;     // 0.5 = low, 1.0 = normal, 1.5 = high
uniform float brightness;   // -1.0 to 1.0

// ============================================================================
// MAIN
// ============================================================================
void main()
{
    // Normaliser UV de pixel coords à 0-1, et flip Y (SFML RenderTexture)
    vec2 uv = gl_TexCoord[0].xy / texSize;
    uv.y = 1.0 - uv.y;

    // Sample texture
    vec3 color = texture2D(tex, uv).rgb;

    // Apply brightness
    color += brightness;

    // Apply contrast
    color = ((color - 0.5) * contrast) + 0.5;

    // Apply saturation
    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    color = mix(vec3(gray), color, saturation);

    // Clamp to valid range
    color = clamp(color, 0.0, 1.0);

    gl_FragColor = vec4(color, 1.0);
}
