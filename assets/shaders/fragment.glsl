#version 130

in vec2 vertexPosition;
in vec4 color;
in vec2 fragmentUV;

out vec4 out_color;

uniform sampler2D sampler;

void main()
{
    vec4 textureColor = texture2D(sampler, fragmentUV);

    out_color = textureColor * color;
}