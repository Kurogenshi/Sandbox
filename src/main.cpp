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

#include <cstdio>
#include <exception>
#include <format>
#include <stdexcept>
#include <iterator>
#include <filesystem>

int main()
{
    try
    {
        sandbox::Window::Settings settings;

        sandbox::Window window(settings);

        sandbox::Shader shader(std::filesystem::path(SANDBOX_ASSET_DIR) / "shaders/basic.vert", std::filesystem::path(SANDBOX_ASSET_DIR) / "shaders/basic.frag", "BasicShader");

        float vertices[] = {
            0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
            0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f
        };

        GLuint indices[] = {
            0, 1, 2
        };

        GLuint vbo = 0;
        glCreateBuffers(1, &vbo);
        glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);

        GLuint ebo = 0;
        glCreateBuffers(1, &ebo);
        glNamedBufferStorage(ebo, sizeof(indices), indices, 0);

        GLuint vao = 0;
        glCreateVertexArrays(1, &vao);

        glVertexArrayVertexBuffer(vao, 0, vbo, 0, 6 * sizeof(float));
        glVertexArrayElementBuffer(vao, ebo);

        glEnableVertexArrayAttrib(vao, 0);
        glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(vao, 0, 0);

        glEnableVertexArrayAttrib(vao, 1);
        glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float));
        glVertexArrayAttribBinding(vao, 1, 0);

        glObjectLabel(GL_VERTEX_ARRAY, vao, -1, "QuadVAO");
        glObjectLabel(GL_BUFFER, vbo, -1, "QuadVBO");
        glObjectLabel(GL_BUFFER, ebo, -1, "QuadEBO");

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
