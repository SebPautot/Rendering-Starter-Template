#version 410

layout(location = 0) in vec2 in_position;
out vec2 vertex_position;

void main()
{
    vec2 position = in_position;
    gl_Position = vec4(position.x, position.y, 0., 1.);
    vertex_position = position;
}