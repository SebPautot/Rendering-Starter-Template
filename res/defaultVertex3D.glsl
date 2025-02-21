#version 410

layout(location = 0) in vec3 in_position;
uniform float aspect_ratio;
uniform float time;
uniform mat4 view_projection_matrix;
out vec3 vertex_position;

void main()
{
    vec3 position = in_position;
    gl_Position = view_projection_matrix * vec4(position.x, position.y, position.z, 1.);
    vertex_position = position;
}