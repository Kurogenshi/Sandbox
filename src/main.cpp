#include "core/Window.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <cstdio>
#include <exception>

int main()
{
    try
    {
        sandbox::Window::Settings settings;

        sandbox::Window window(settings);

        while (!window.shouldClose())
        {
            window.pollEvents();

            if (glfwGetKey(window.handle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window.handle(), GLFW_TRUE);
            }

            glClearColor(0.10f, 0.11f, 0.13f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::ShowDemoWindow();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            window.swapBuffers();
        }
    }
    catch (const std::exception& e)
    {
        std::fprintf(stderr, "Fatal error : %s\n", e.what());
        return 1;
    }

    return 0;
}
