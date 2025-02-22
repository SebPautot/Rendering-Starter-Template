#version 410

layout(location = 0) in vec2 in_position;
uniform float aspect_ratio;
uniform float time;
uniform mat4 view_projection_matrix;
out vec2 vertex_position;

void main()
{
    vec2 position = in_position;
    gl_Position = view_projection_matrix * vec4(position.x, position.y+sin(time*0.25), 0., 1.);
}