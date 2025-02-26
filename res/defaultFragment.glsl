#version 410

out vec4 out_color;
uniform vec4 color;
uniform sampler2D texture_sample;
in vec3 vertex_position;

void main()
{
    vec4 tex_color = texture(texture_sample, vec2(vertex_position.x, vertex_position.y));
    out_color = vec4(color * tex_color);
}