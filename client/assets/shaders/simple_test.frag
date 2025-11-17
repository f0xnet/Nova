#version 120

uniform sampler2D texture;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 color = texture2D(texture, uv);

    // Simple passthrough - devrait afficher l'image normalement
    gl_FragColor = color;
}
