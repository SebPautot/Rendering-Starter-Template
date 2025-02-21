#include "opengl-framework/opengl-framework.hpp" // Inclue la librairie qui va nous servir à faire du rendu
#include "iostream"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
int main()
{
    // Initialisation
    gl::init("TPs de Rendering"); // On crée une fenêtre et on choisit son nom
    gl::maximize_window();        // On peut la maximiser si on veut

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);

    // glClearColor(0.f, 1.f, 1.f, .5f);
    // glClear(GL_COLOR_BUFFER_BIT);

    auto const shader = gl::Shader{{
        .vertex = gl::ShaderSource::File{"res/vertex.glsl"},
        .fragment = gl::ShaderSource::File{"res/fragment.glsl"},
    }};

    shader.bind();

    float previousTime = 0.f;
    float deltaTime = 0.f;
    float currentTime = 0.f;

    // Setup camera

    auto camera = gl::Camera{};

    gl::set_events_callbacks({
        camera.events_callbacks(),
        {
            .on_mouse_pressed = [&](gl::MousePressedEvent const &e)
            {
                std::cout << "Mouse pressed at " << e.position.x << " " << e.position.y << '\n';
            },
        },
    });

    while (gl::window_is_open())
    {
        glEnable(GL_BLEND);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);

        currentTime = gl::time_in_seconds();
        deltaTime = currentTime - previousTime;
        glm::mat4 const view_matrix = camera.view_matrix();

        glClearColor(0.f, 1.f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Rendu à chaque frame

        /* Weird ass cube moving on 2D plane start */

        // for (float i = 0; i < 30; i++)
        // {
        //     auto triangle_mesh = gl::Mesh{{.vertex_buffers = {{
        //                                        .layout = {gl::VertexAttribute::Position2D{0}},
        //                                        .data = {
        //                                            -0.5f, -0.5f,
        //                                            +0.5f, -0.5f,
        //                                            +0.5f, +0.5f,
        //                                            -0.5f, +0.5f},
        //                                    }},
        //                                    .index_buffer = {0, 1, 2, 0, 2, 3}}};

        //     shader.set_uniform("color", glm::vec4{1, 1, 1, 1-(i/30)});
        //     shader.set_uniform("aspect_ratio", gl::framebuffer_aspect_ratio());
        //     shader.set_uniform("time", currentTime - i*0.05f);
        //     triangle_mesh.draw();
        // }

        /* Weird ass cube moving on 2D plane end */

        /* Camera & matrices start */

        glm::mat4 const projection_matrix = glm::infinitePerspective(glm::radians(45.f) /*field of view in radians*/, gl::framebuffer_aspect_ratio() /*aspect ratio*/, 0.001f /*near plane*/);

        auto triangle_mesh = gl::Mesh{
            {.vertex_buffers = {{
                 .layout = {gl::VertexAttribute::Position2D{0}},
                 .data = {
                     -0.5f, -0.5f,
                     +0.5f, -0.5f,
                     +0.5f, +0.5f,
                     -0.5f, +0.5f},
             }},
             .index_buffer = {0, 1, 2, 0, 2, 3}}};

        shader.set_uniform("color", glm::vec4{1, 1, 1, 1});
        shader.set_uniform("aspect_ratio", gl::framebuffer_aspect_ratio());
        shader.set_uniform("time", currentTime);
        shader.set_uniform("view_projection_matrix", view_matrix * projection_matrix);
        triangle_mesh.draw();

        //TODO
        // /*glm::mat4 const rotation = glm::rotate(glm::mat4{1.f}, gl::time_in_seconds() /*angle de la rotation*/, glm::vec3{0.f, 0.f, 1.f} /* axe autour duquel on tourne */);*/

            /* Camera & matrices end */
            // printf("%f \n", 60 / deltaTime);

        previousTime = gl::time_in_seconds();
    }
}