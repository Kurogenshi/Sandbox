#pragma once

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

        [[nodiscard]] uint32_t id() const noexcept { return m_id; }

        void set(int location, bool value) const noexcept;
        void set(int location, int value) const noexcept;
        void set(int location, uint32_t value) const noexcept;
        void set(int location, float value) const noexcept;
        void set(int location, const glm::vec2& value) const noexcept;
        void set(int location, const glm::vec3& value) const noexcept;
        void set(int location, const glm::vec4& value) const noexcept;
        void set(int location, const glm::mat3& value) const noexcept;
        void set(int location, const glm::mat4& value) const noexcept;

        void setTextureUnit(int location, int unit) const noexcept;

        bool reload();

    private:
        void build();
        void destroy() noexcept;

        uint32_t m_id = 0;
        std::vector<Stage> m_stages;
        std::string m_debugName;
    };

} 