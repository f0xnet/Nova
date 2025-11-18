#version 120

uniform sampler2D texture;

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    gl_FragColor = texture2D(texture, uv);
}
