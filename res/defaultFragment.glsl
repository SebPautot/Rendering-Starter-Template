#version 410

out vec4 out_color;
uniform vec4 color;
in vec3 vertex_position;

void main()
{
    out_color = vec4(vertex_position, 1.);
}