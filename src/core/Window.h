#pragma once

#include <string>

struct GLFWwindow;

namespace sandbox {

class Window
{
public:
    struct Settings
    {
        int width = 1280;
        int height = 720;
        std::string title = "Sandbox";
        bool vsync = true;
        int glMajor = 4;
        int glMinor = 6;
        int samples = 4;
    };

    explicit Window(const Settings& settings = {});
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&&) = delete;
    Window& operator=(Window&&) = delete;

    [[nodiscard]] bool shouldClose() const;
    void swapBuffers() const;
    void pollEvents() const;

    [[nodiscard]] int width() const noexcept { return m_width; }
    [[nodiscard]] int height() const noexcept { return m_height; }

    [[nodiscard]] GLFWwindow* handle() const noexcept { return m_handle; }

private:
    static void framebufferSizeCallback(GLFWwindow* handle, int width, int height);
    void onFramebufferResize(int width, int height);

    GLFWwindow* m_handle = nullptr;
    int m_width = 0;
    int m_height = 0;
};

}
