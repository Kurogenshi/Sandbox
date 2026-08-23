#include "core/Window.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <stdexcept>

namespace sandbox {
namespace {

void glfwErrorCallback(int code, const char* description)
{
    std::fprintf(stderr, "[GLFW] error %d : %s\n", code, description);
}

void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar* message, const void* /*userParam*/)
{
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    const char* sourceStr = [source] {
        switch (source) {
        case GL_DEBUG_SOURCE_API:             return "API";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "Window System";
        case GL_DEBUG_SOURCE_SHADER_COMPILER: return "Shader Compiler";
        case GL_DEBUG_SOURCE_THIRD_PARTY:     return "Third Party";
        case GL_DEBUG_SOURCE_APPLICATION:     return "Application";
        default:                              return "Other";
        }
        }();

    const char* typeStr = [type] {
        switch (type) {
        case GL_DEBUG_TYPE_ERROR:               return "Error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Deprecated";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "Undefined Behavior";
        case GL_DEBUG_TYPE_PORTABILITY:         return "Portability";
        case GL_DEBUG_TYPE_PERFORMANCE:         return "Performance";
        default:                                return "Other";
        }
        }();

    const char* severityStr = [severity] {
        switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:   return "HIGH";
        case GL_DEBUG_SEVERITY_MEDIUM: return "medium";
        case GL_DEBUG_SEVERITY_LOW:    return "low";
        default:                       return "note";
        }
        }();

    std::fprintf(stderr, "[GL %s] %s / %s (id %u)\n     %s\n", severityStr, sourceStr, typeStr, id, message);
}

}

Window::Window(const Settings& settings)
{
    glfwSetErrorCallback(glfwErrorCallback);

    if (glfwInit() != GLFW_TRUE)
        throw std::runtime_error("Failed to initialize GLFW");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, settings.glMajor);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, settings.glMinor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, settings.samples);

#ifdef SANDBOX_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

    m_handle = glfwCreateWindow(settings.width, settings.height, settings.title.c_str(), nullptr, nullptr);
    if (m_handle == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create window (requested OpenGL context unavailable?)");
    }

    m_width = settings.width;
    m_height = settings.height;

    glfwMakeContextCurrent(m_handle);
    glfwSwapInterval(settings.vsync ? 1 : 0);
    glfwSetFramebufferSizeCallback(m_handle, framebufferSizeCallback);

    const int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
    {
        glfwDestroyWindow(m_handle);
        glfwTerminate();
        throw std::runtime_error("Failed to load OpenGL function pointers");
    }

    std::printf("OpenGL %d.%d -- %s\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version), reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

#ifdef SANDBOX_DEBUG
    GLint flags = 0;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debugCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

    glViewport(0, 0, m_width, m_height);
}

Window::~Window()
{
    if (m_handle != nullptr)
        glfwDestroyWindow(m_handle);
    glfwTerminate();
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(m_handle) == GLFW_TRUE;
}

void Window::swapBuffers() const
{
    glfwSwapBuffers(m_handle);
}

void Window::pollEvents() const
{
    glfwPollEvents();
}

}
