#pragma once

#include "renderer/GlHandle.h"

#include <glm/fwd.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace sandbox {

    enum class ShaderStage : std::uint8_t
    {
        Vertex,
        TessellationControl,
        TessellationEvaluation,
        Geometry,
        Fragment,
        Compute
    };

    class Shader
    {
    public:
        struct Stage
        {
            ShaderStage stage;
            std::filesystem::path path;
        };

        Shader() noexcept = default;

        explicit Shader(std::vector<Stage> stages, std::string debugName = {});
        Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath, std::string debugName = {});

        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&& other) noexcept;
        Shader& operator=(Shader&& other) noexcept;

        void bind() const noexcept;

        [[nodiscard]] ProgramId id() const noexcept { return m_id; }
        [[nodiscard]] bool valid() const noexcept { return m_id.valid(); }

        void set(GLint location, bool value) const noexcept;
        void set(GLint location, GLint value) const noexcept;
        void set(GLint location, GLuint value) const noexcept;
        void set(GLint location, float value) const noexcept;
        void set(GLint location, const glm::vec2& value) const noexcept;
        void set(GLint location, const glm::vec3& value) const noexcept;
        void set(GLint location, const glm::vec4& value) const noexcept;
        void set(GLint location, const glm::mat3& value) const noexcept;
        void set(GLint location, const glm::mat4& value) const noexcept;

        void setTextureUnit(GLint location, GLint unit) const noexcept;

        void set(GLint location, TextureId) const = delete;

        bool reload();

    private:
        void build();
        void destroy() noexcept;

        ProgramId m_id{};
        std::vector<Stage> m_stages;
        std::string m_debugName;
    };

} 