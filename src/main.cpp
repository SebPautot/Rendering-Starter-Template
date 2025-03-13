#include "opengl-framework/opengl-framework.hpp" // Inclue la librairie qui va nous servir à faire du rendu
#include "iostream"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <cmath>

auto load_mesh(std::filesystem::path const &path) -> gl::Mesh
{
    // On lit le fichier avec tinyobj
    auto reader = tinyobj::ObjReader{};
    reader.ParseFromFile(gl::make_absolute_path(path).string(), {});

    if (!reader.Error().empty())
        throw std::runtime_error("Failed to read 3D model:\n" + reader.Error());
    if (!reader.Warning().empty())
        std::cout << "Warning while reading 3D model:\n" + reader.Warning();

    // On met tous les attributs dans un tableau
    auto vertices = std::vector<float>{};
    for (auto const &shape : reader.GetShapes())
    {
        for (auto const &idx : shape.mesh.indices)
        {
            // Position
            vertices.push_back(reader.GetAttrib().vertices[3 * idx.vertex_index + 0]);
            vertices.push_back(reader.GetAttrib().vertices[3 * idx.vertex_index + 2]);
            vertices.push_back(reader.GetAttrib().vertices[3 * idx.vertex_index + 1]);

            // UV
            vertices.push_back(reader.GetAttrib().texcoords[2 * idx.texcoord_index + 0]);
            vertices.push_back(reader.GetAttrib().texcoords[2 * idx.texcoord_index + 1]);

            // Normale
            vertices.push_back(reader.GetAttrib().normals[3 * idx.normal_index + 0]);
            vertices.push_back(reader.GetAttrib().normals[3 * idx.normal_index + 2]);
            vertices.push_back(reader.GetAttrib().normals[3 * idx.normal_index + 1]);
        };
    }

    auto mesh = gl::Mesh{
        {.vertex_buffers = {{
             .layout = {gl::VertexAttribute::Position3D{0}, gl::VertexAttribute::UV{1}, gl::VertexAttribute::Normal3D{2}},
             .data = vertices,
         }}}};

    return mesh;
}

int main()
{
    // Initialisation
    gl::init("TPs de Rendering"); // On crée une fenêtre et on choisit son nom
    gl::maximize_window();        // On peut la maximiser si on veut

    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_ONE);
    glEnable(GL_DEPTH_TEST);

    // glClearColor(0.f, 1.f, 1.f, .5f);
    // glClear(GL_COLOR_BUFFER_BIT);

    auto const shader = gl::Shader{{
        .vertex = gl::ShaderSource::File{"res/defaultVertex3D.glsl"},
        .fragment = gl::ShaderSource::File{"res/defaultFragment.glsl"},
    }};

    auto const post_processing = gl::Shader{{
        .vertex = gl::ShaderSource::File{"res/defaultVertex.glsl"},
        .fragment = gl::ShaderSource::File{"res/post_processing.glsl"},
    }};

    float previousTime = 0.f;
    float deltaTime = 0.f;
    float currentTime = 0.f;

    gl::Mesh boat_mesh = load_mesh("./res/boat/fourareen.obj");

    // Texture

    auto const texture = gl::Texture{
        gl::TextureSource::File{
            // Peut être un fichier, ou directement un tableau de pixels
            .path = "./res/boat/fourareen2K_albedo.jpg",
            .flip_y = true,                              // Il n'y a pas de convention universelle sur la direction de l'axe Y. Les fichiers (.png, .jpeg) utilisent souvent une direction différente de celle attendue par OpenGL. Ce booléen flip_y est là pour inverser la texture si jamais elle n'apparaît pas dans le bon sens.
            .texture_format = gl::InternalFormat::RGBA8, // Format dans lequel la texture sera stockée. On pourrait par exemple utiliser RGBA16 si on voulait 16 bits par canal de couleur au lieu de 8. (Mais ça ne sert à rien dans notre cas car notre fichier ne contient que 8 bits par canal, donc on ne gagnerait pas de précision). On pourrait aussi stocker en RGB8 si on ne voulait pas de canal alpha. On utilise aussi parfois des textures avec un seul canal (R8) pour des usages spécifiques.
        },
        gl::TextureOptions{
            .minification_filter = gl::Filter::Linear,  // Comment on va moyenner les pixels quand on voit l'image de loin ?
            .magnification_filter = gl::Filter::Linear, // Comment on va interpoler entre les pixels quand on zoom dans l'image ?
            .wrap_x = gl::Wrap::Repeat,                 // Quelle couleur va-t-on lire si jamais on essaye de lire en dehors de la texture ?
            .wrap_y = gl::Wrap::Repeat,                 // Idem, mais sur l'axe Y. En général on met le même wrap mode sur les deux axes.
        }};

    // Setup camera
    auto camera = gl::Camera{};

    // Render target

    auto render_target = gl::RenderTarget{gl::RenderTarget_Descriptor{
        .width = gl::framebuffer_width_in_pixels(),
        .height = gl::framebuffer_height_in_pixels(),
        .color_textures = {
            gl::ColorAttachment_Descriptor{
                .format = gl::InternalFormat_Color::RGBA8,
                .options = {
                    .minification_filter = gl::Filter::NearestNeighbour,  // On va toujours afficher la texture à la taille de l'écran,
                    .magnification_filter = gl::Filter::NearestNeighbour, // donc les filtres n'auront pas d'effet. Tant qu'à faire on choisit le moins coûteux.
                    .wrap_x = gl::Wrap::ClampToEdge,
                    .wrap_y = gl::Wrap::ClampToEdge,
                },
            },
        },
        .depth_stencil_texture = gl::DepthStencilAttachment_Descriptor{
            .format = gl::InternalFormat_DepthStencil::Depth32F,
            .options = {
                .minification_filter = gl::Filter::NearestNeighbour,
                .magnification_filter = gl::Filter::NearestNeighbour,
                .wrap_x = gl::Wrap::ClampToEdge,
                .wrap_y = gl::Wrap::ClampToEdge,
            },
        },
    }};

    gl::set_events_callbacks(
        {camera.events_callbacks(),
         {
             .on_mouse_pressed = [&](gl::MousePressedEvent const &e)
             {
                 std::cout << "Mouse pressed at " << e.position.x << " " << e.position.y << '\n';
             },
         },
         {.on_framebuffer_resized = [&](gl::FramebufferResizedEvent const &e)
          {
              if (e.width_in_pixels != 0 && e.height_in_pixels != 0) // OpenGL crash si on tente de faire une render target avec une taille de 0
                  render_target.resize(e.width_in_pixels, e.height_in_pixels);
          }}});
          

    while (gl::window_is_open())
    {

        currentTime = gl::time_in_seconds();
        deltaTime = currentTime - previousTime;

        glClearColor(1.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_target.render(
            [&]()
            {
                glm::mat4 const view_matrix = camera.view_matrix();

                glm::mat4 const projection_matrix = glm::infinitePerspective(glm::radians(45.f) /*field of view in radians*/, gl::framebuffer_aspect_ratio() /*aspect ratio*/, 0.001f /*near plane*/);
                const auto view_projection_matrix = projection_matrix * view_matrix;
                glm::mat4 const rotation = glm::rotate(glm::mat4{1.f}, gl::time_in_seconds() /*angle de la rotation*/, glm::vec3{0.f, 0.f, 1.f} /* axe autour duquel on tourne */);
                glm::mat4 const translation = glm::translate(glm::mat4{1.f}, glm::vec3{0.f, 1.f, 0.f} /* déplacement */);
                const auto model_matrix = translation * rotation;
                const auto normal_matrix = glm::inverse(glm::transpose(model_matrix));

                glClearColor(0.f, 1.f, 1.f, 1.f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Rendu à chaque frame

                /* Weird ass cube moving on 2D plane start */ // https: // youtu.be/GBXSUTgJRD4

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

                shader.bind();

                // auto triangle_mesh = gl::Mesh{
                //     {.vertex_buffers = {{
                //          .layout = {gl::VertexAttribute::Position3D{0}},
                //          .data = {
                //              -0.5f, -0.5f, 0.5f,
                //              +0.5f, -0.5f, 0.5f,
                //              +0.5f, +0.5f, 0.5f,
                //              -0.5f, +0.5f, 0.5f,
                //              -0.5f, -0.5f, -0.5f,
                //              +0.5f, -0.5f, -0.5f,
                //              +0.5f, +0.5f, -0.5f,
                //              -0.5f, +0.5f, -0.5f},
                //      }},
                //      .index_buffer = {0, 1, 2, 0, 2, 3,

                //                       4, 5, 6, 4, 6, 7,

                //                       1, 5, 6, 1, 6, 2,

                //                       0, 4, 7, 0, 7, 3,

                //                       3, 2, 6, 3, 7, 6,

                //                       0, 1, 5, 0, 5, 4}}};


                shader.set_uniform("aspect_ratio", gl::framebuffer_aspect_ratio());
                shader.set_uniform("time", currentTime);
                shader.set_uniform("view_projection_matrix", view_projection_matrix);
                shader.set_uniform("texture_sample", texture);
                shader.set_uniform("point_lights[0]", glm::vec4(0, 0, 10, 200));
                shader.set_uniform("point_lights[1]", glm::vec4(0, 0, 0, 1));
                shader.set_uniform("light_count", 2);
                shader.set_uniform("light_direction", glm::vec3(0, 0, 1));
                shader.set_uniform("light_intensity", 0.f);
                shader.set_uniform("model_matrix",model_matrix);
                shader.set_uniform("normal_matrix", normal_matrix);
                boat_mesh.draw();

                // /*glm::mat4 const rotation = glm::rotate(glm::mat4{1.f}, gl::time_in_seconds() /*angle de la rotation*/, glm::vec3{0.f, 0.f, 1.f} /* axe autour duquel on tourne */);*/
            });

        post_processing.bind();

        auto screen_mesh = gl::Mesh{
            {.vertex_buffers =
                 {{
                     .layout = {gl::VertexAttribute::Position2D{0}},
                     .data = {
                         -1.f, -1.f,
                         +1.f, -1.f,
                         +1.f, +1.f,
                         -1.f, +1.f},
                 }},
             .index_buffer = {0, 1, 2, 0, 2, 3}}};

        post_processing.set_uniform("color", glm::vec4{1, 1, 1, 1});
        post_processing.set_uniform("texture_sample", render_target.color_texture(0));
        post_processing.set_uniform("aspect_ratio", gl::framebuffer_aspect_ratio());
        screen_mesh.draw();



        previousTime = gl::time_in_seconds();
    }
}
