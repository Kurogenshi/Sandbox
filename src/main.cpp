#include <core/Window.h>
#include <renderer/Shader.h>
#include <renderer/Texture.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <stb_image.h>

#include <cstdio>
#include <exception>
#include <format>
#include <stdexcept>
#include <iterator>
#include <filesystem>

struct StbDeleter
{
    void operator()(unsigned char* p) const noexcept { stbi_image_free(p); }
};

sandbox::Texture loadTexture(const std::filesystem::path& path, bool srgb = true, bool flip = false)
{
    stbi_set_flip_vertically_on_load(flip);

    int width = 0, height = 0, channels = 0;

    std::unique_ptr<unsigned char, StbDeleter> pixels(stbi_load(path.string().c_str(), &width, &height, &channels, 4));

    if (!pixels)
        throw std::runtime_error(std::format("Failed to load {}: {}", path.string(), stbi_failure_reason()));

    sandbox::Texture::Specification spec;
    spec.width = width;
    spec.height = height;
    spec.internalFormat = srgb ? sandbox::Texture::Format::SRGB8_ALPHA8 : sandbox::Texture::Format::RGBA8;
    spec.name = path.filename().string();

    return sandbox::Texture{ spec, pixels.get() };
}

int main()
{
    try
    {
        sandbox::Window::Settings settings;

        sandbox::Window window(settings);

        sandbox::Shader shader(std::filesystem::path(SANDBOX_ASSET_DIR) / "shaders/basic.vert", std::filesystem::path(SANDBOX_ASSET_DIR) / "shaders/basic.frag", "BasicShader");

        float vertices[] = {
            -0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   0.0f, 1.0f,

             0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,

            -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,

             0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 0.0f,   0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 0.0f,   1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 0.0f,   1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f,

            -0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 1.0f,   0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 1.0f,   1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 1.0f,   1.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 1.0f,   0.0f, 1.0f,

            -0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 1.0f,   0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,   1.0f, 0.0f, 1.0f,   1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 1.0f,   1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 1.0f,   0.0f, 1.0f,
        };

        unsigned int indices[] = {
             0,  1,  2,  2,  3,  0,
             4,  5,  6,  6,  7,  4,
             8,  9, 10, 10, 11,  8,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 21, 22, 22, 23, 20,
        };

        GLuint vbo = 0;
        glCreateBuffers(1, &vbo);
        glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);

        GLuint ebo = 0;
        glCreateBuffers(1, &ebo);
        glNamedBufferStorage(ebo, sizeof(indices), indices, 0);

        GLuint vao = 0;
        glCreateVertexArrays(1, &vao);

        glVertexArrayVertexBuffer(vao, 0, vbo, 0, 8 * sizeof(float));
        glVertexArrayElementBuffer(vao, ebo);

        glEnableVertexArrayAttrib(vao, 0);
        glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(vao, 0, 0);

        glEnableVertexArrayAttrib(vao, 1);
        glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
        glVertexArrayAttribBinding(vao, 1, 0);

        glEnableVertexArrayAttrib(vao, 2);
        glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float));
        glVertexArrayAttribBinding(vao, 2, 0);

        glObjectLabel(GL_VERTEX_ARRAY, vao, -1, "QuadVAO");
        glObjectLabel(GL_BUFFER, vbo, -1, "QuadVBO");
        glObjectLabel(GL_BUFFER, ebo, -1, "QuadEBO");

        sandbox::Texture texture1 = loadTexture(std::filesystem::path(SANDBOX_ASSET_DIR) / "textures/test.jpg", true);
        sandbox::Texture texture2 = loadTexture(std::filesystem::path(SANDBOX_ASSET_DIR) / "textures/test2.png", true, true);

        glm::mat4 proj = glm::mat4(1.0f);

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

        glm::vec3 cubePositions[] = {
            glm::vec3(0.0f,  0.0f,  0.0f),
            glm::vec3(2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3(2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3(1.3f, -2.0f, -2.5f),
            glm::vec3(1.5f,  2.0f, -2.5f),
            glm::vec3(1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
        };

        while (!window.shouldClose())
        {
            window.pollEvents();

            if (glfwGetKey(window.handle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window.handle(), GLFW_TRUE);
            }

            glClearColor(0.10f, 0.11f, 0.13f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            proj = glm::perspective(glm::radians(45.0f), static_cast<float>(window.width()) / static_cast<float>(window.height()), 0.1f, 100.0f);

            shader.bind();

            shader.set(1, view);
            shader.set(2, proj);

            texture1.bind(3);
            texture2.bind(4);

            glBindVertexArray(vao);
            for (unsigned int i = 0; i < 10; i++)
            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, cubePositions[i]);
                float angle = 20.0f * static_cast<float>(i);
                model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

                shader.set(0, model);

                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(std::size(indices)), GL_UNSIGNED_INT, nullptr);
            }
            glBindVertexArray(0);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            //ImGui::ShowDemoWindow();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            window.swapBuffers();
        }

        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
    }
    catch (const std::exception& e)
    {
        std::fprintf(stderr, "Fatal error : %s\n", e.what());
        return 1;
    }

    return 0;
}
