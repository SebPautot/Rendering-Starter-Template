#version 410

out vec4 out_color;
uniform vec4 color;
uniform sampler2D texture_sample;
uniform float luminance = 0.f;
in vec2 vertex_position;

void main()
{
    vec4 tex_color = texture(texture_sample, vec2(vertex_position.x, vertex_position.y));
    out_color = vec4(color * tex_color);

    // float lum = (0.299*out_color.x + 0.587*out_color.y + 0.114*out_color.z);

    // out_color = vec4(mix(out_color.rgb, vec3(lum), luminance), out_color.a);
}