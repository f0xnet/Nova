#version 120

uniform sampler2D texture;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    // SFML RenderTextures are vertically inverted
    uv.y = 1.0 - uv.y;
    gl_FragColor = texture2D(texture, uv);
}
