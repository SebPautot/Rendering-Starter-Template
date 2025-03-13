#version 410

out vec4 out_color;
uniform vec3 light_direction = vec3(1, 1, 1);
uniform float light_intensity = 1.;
uniform vec4 color = vec4(1, 1, 1, 1);
uniform vec4[6] point_lights;
uniform int light_count = 0;
uniform sampler2D texture_sample;
in vec3 vertex_position;
in vec2 uv;
in vec3 normal;
in vec3 position_ws;
in vec3 normal_ws;



void main() {
    float dot_light = dot(normal_ws, normalize(light_direction)) * light_intensity;
    if(dot_light < 0)
        dot_light = 0;
    // dot_light += 0.3;

    for(int i; i < light_count; i++) {
        if(point_lights[i].w > 0) {
            vec3 point_light_direction = vec3(point_lights[i].x, point_lights[i].y, point_lights[i].z) - position_ws;
            float dot_point_light = dot(normalize(point_light_direction), normal_ws);
            float distance = (point_lights[i].w - length(point_light_direction));
            if(distance > 0) {
                dot_point_light *= distance;

                if(point_lights[i].w > 1)
                    dot_point_light /= point_lights[i].w;

                if(dot_point_light > dot_light)
                    dot_light = dot_point_light;

            }
        }
    }

    vec4 tex_color = texture(texture_sample, vec2(uv.x, uv.y));
    tex_color.x *= dot_light;
    tex_color.y *= dot_light;
    tex_color.z *= dot_light;

    out_color = vec4(color * tex_color);
}