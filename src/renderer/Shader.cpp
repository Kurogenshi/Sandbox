#include <renderer/Shader.h>

#include <core/ScopeExit.h>

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cstdio>
#include <format>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace sandbox {
    namespace {

        [[nodiscard]] GLenum toGlEnum(ShaderStage stage) noexcept
        {
            switch (stage)
            {
            case ShaderStage::Vertex:                 return GL_VERTEX_SHADER;
            case ShaderStage::TessellationControl:    return GL_TESS_CONTROL_SHADER;
            case ShaderStage::TessellationEvaluation: return GL_TESS_EVALUATION_SHADER;
            case ShaderStage::Geometry:               return GL_GEOMETRY_SHADER;
            case ShaderStage::Fragment:               return GL_FRAGMENT_SHADER;
            case ShaderStage::Compute:                return GL_COMPUTE_SHADER;
            }
            return GL_NONE;
        }

        [[nodiscard]] std::string readFile(const std::filesystem::path& path)
        {
            std::ifstream file(path, std::ios::in | std::ios::binary);
            if (!file)
                throw std::runtime_error(std::format("Cannot open shader file: {}", path.string()));

            std::ostringstream contents;
            contents << file.rdbuf();
            if (file.bad())
                throw std::runtime_error(std::format("Error while reading: {}", path.string()));

            return std::move(contents).str();
        }

        [[nodiscard]] std::string shaderLog(GLuint shader)
        {
            int length = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
            if (length <= 1)
                return "no log available";

            std::string log(static_cast<std::size_t>(length), '\0');
            glGetShaderInfoLog(shader, length, nullptr, log.data());
            log.resize(static_cast<std::size_t>(length) - 1);
            return log;
        }

        [[nodiscard]] std::string programLog(GLuint program)
        {
            int length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            if (length <= 1)
                return "no log available";

            std::string log(static_cast<std::size_t>(length), '\0');
            glGetProgramInfoLog(program, length, nullptr, log.data());
            log.resize(static_cast<std::size_t>(length) - 1);
            return log;
        }

        [[nodiscard]] GLuint compileStage(const Shader::Stage& stage)
        {
            const std::string source = readFile(stage.path);
            const char* const sourcePtr = source.c_str();
            const auto length = static_cast<int>(source.size());

            const GLuint shader = glCreateShader(toGlEnum(stage.stage));
            glShaderSource(shader, 1, &sourcePtr, &length);
            glCompileShader(shader);

            int compiled = GL_FALSE;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if (compiled != GL_TRUE)
            {
                const std::string log = shaderLog(shader);
                glDeleteShader(shader);
                throw std::runtime_error(std::format("Compilation failed: {}\n{}", stage.path.string(), log));
            }

            return shader;
        }

    }

    Shader::Shader(std::vector<Stage> stages, std::string debugName)
        : m_stages(std::move(stages)), m_debugName(std::move(debugName))
    {
        if (m_stages.empty())
            throw std::runtime_error("Shader created with no stages");

        build();
    }

    Shader::Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath, std::string debugName)
        : Shader(std::vector<Stage>{{ShaderStage::Vertex, vertexPath}, { ShaderStage::Fragment, fragmentPath }}, std::move(debugName))
    {}

    Shader::~Shader()
    {
        destroy();
    }

    Shader::Shader(Shader&& other) noexcept
        : m_id(std::exchange(other.m_id, 0)) , m_stages(std::move(other.m_stages)) , m_debugName(std::move(other.m_debugName))
    {}

    Shader& Shader::operator=(Shader&& other) noexcept
    {
        if (this != &other)
        {
            destroy();
            m_id = std::exchange(other.m_id, 0);
            m_stages = std::move(other.m_stages);
            m_debugName = std::move(other.m_debugName);
        }
        return *this;
    }

    void Shader::build()
    {
        std::vector<GLuint> compiled;
        compiled.reserve(m_stages.size());

        SANDBOX_SCOPE_EXIT
        {
            for (const GLuint shader : compiled)
                glDeleteShader(shader);
        };

        for (const Stage& stage : m_stages)
            compiled.push_back(compileStage(stage));

        const GLuint program = glCreateProgram();

        for (const GLuint shader : compiled)
            glAttachShader(program, shader);

        glLinkProgram(program);

        for (const GLuint shader : compiled)
            glDetachShader(program, shader);

        int linked = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linked);
        if (linked != GL_TRUE)
        {
            const std::string log = programLog(program);
            glDeleteProgram(program);
            throw std::runtime_error(std::format("Link failed: {}\n{}", m_debugName, log));
        }

        if (!m_debugName.empty())
            glObjectLabel(GL_PROGRAM, program, static_cast<GLsizei>(m_debugName.size()), m_debugName.c_str());

        destroy();
        m_id = program;
    }

    void Shader::destroy() noexcept
    {
        glDeleteProgram(m_id);
        m_id = 0;
    }

    bool Shader::reload()
    {
        try
        {
            build();
            return true;
        }
        catch (const std::exception& e)
        {
            std::fprintf(stderr, "Shader reload failed: %s\n", e.what());
            return false;
        }
    }

    void Shader::bind() const noexcept
    {
        glUseProgram(m_id);
    }

    void Shader::set(int location, bool value) const noexcept
    {
        glProgramUniform1i(m_id, location, static_cast<int>(value));
    }

    void Shader::set(int location, int value) const noexcept
    {
        glProgramUniform1i(m_id, location, value);
    }

    void Shader::set(int location, GLuint value) const noexcept
    {
        glProgramUniform1ui(m_id, location, value);
    }

    void Shader::set(int location, float value) const noexcept
    {
        glProgramUniform1f(m_id, location, value);
    }

    void Shader::set(int location, const glm::vec2& value) const noexcept
    {
        glProgramUniform2fv(m_id, location, 1, glm::value_ptr(value));
    }

    void Shader::set(int location, const glm::vec3& value) const noexcept
    {
        glProgramUniform3fv(m_id, location, 1, glm::value_ptr(value));
    }

    void Shader::set(int location, const glm::vec4& value) const noexcept
    {
        glProgramUniform4fv(m_id, location, 1, glm::value_ptr(value));
    }

    void Shader::set(int location, const glm::mat3& value) const noexcept
    {
        glProgramUniformMatrix3fv(m_id, location, 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::set(int location, const glm::mat4& value) const noexcept
    {
        glProgramUniformMatrix4fv(m_id, location, 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::setTextureUnit(int location, int unit) const noexcept
    {
        glProgramUniform1i(m_id, location, unit);
    }

}