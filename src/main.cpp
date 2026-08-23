#include <core/Window.h>
#include <renderer/GlHandle.h>
#include <renderer/Shader.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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

sandbox::TextureId loadTexture(const std::filesystem::path& path, bool flip = false)
{
    stbi_set_flip_vertically_on_load(flip);

    int width = 0, height = 0, channels = 0;

    std::unique_ptr<unsigned char, StbDeleter> pixels(stbi_load(path.string().c_str(), &width, &height, &channels, 4));

    if (!pixels)
        throw std::runtime_error(std::format("Failed to load {}: {}", path.string(), stbi_failure_reason()));

    GLuint texture = 0;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    const GLsizei levels = 1 + static_cast<GLsizei>(std::floor(std::log2(std::max(width, height))));

    glTextureStorage2D(texture, levels, GL_SRGB8_ALPHA8, width, height);
    glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());
    glGenerateTextureMipmap(texture);

    glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLfloat maxAnisotropy = 1.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
    glTextureParameterf(texture, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy);

    glObjectLabel(GL_TEXTURE, texture, -1, path.filename().string().c_str());

    return sandbox::TextureId{ texture };
}

int main()
{
    try
    {
        sandbox::Window::Settings settings;

        sandbox::Window window(settings);

        sandbox::Shader shader(std::filesystem::path(SANDBOX_ASSET_DIR) / "shaders/basic.vert", std::filesystem::path(SANDBOX_ASSET_DIR) / "shaders/basic.frag", "BasicShader");

        float vertices[] = {
             0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
             0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
            -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
            -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f
        };
        unsigned int indices[] = {
            0, 1, 3,
            1, 2, 3
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

        sandbox::TextureId texture1 = loadTexture(std::filesystem::path(SANDBOX_ASSET_DIR) / "textures/test.jpg");
        sandbox::TextureId texture2 = loadTexture(std::filesystem::path(SANDBOX_ASSET_DIR) / "textures/test2.png", true);

        while (!window.shouldClose())
        {
            window.pollEvents();

            if (glfwGetKey(window.handle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window.handle(), GLFW_TRUE);
            }

            glClearColor(0.10f, 0.11f, 0.13f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            shader.bind();

            glBindTextureUnit(0, texture1.get());
            glBindTextureUnit(1, texture2.get());

            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(std::size(indices)), GL_UNSIGNED_INT, nullptr);
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
