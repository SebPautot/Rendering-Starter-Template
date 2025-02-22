#version 410

out vec4 out_color;
uniform vec4 color;
uniform sampler2D texture_sample;
uniform float luminance = 0.f;
in vec2 vertex_position;

void main()
{
    vec4 tex_color = texture(texture_sample, /* why */ vec2((vertex_position.x/2)+.5, (vertex_position.y/2)+.5) /* (X/2)+1 works smh */);
    out_color = vec4(tex_color);
}