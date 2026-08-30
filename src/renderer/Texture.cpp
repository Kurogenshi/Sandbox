#include <renderer/Texture.h>

#include <glad/gl.h>

#include <algorithm>
#include <bit>
#include <utility>
#include <array>
#include <format>
#include <stdexcept>
#include <type_traits>
#include <cstdint>

namespace sandbox {
    namespace {

        template <typename Enum, std::size_t N>
        [[nodiscard]] GLenum lookup(const std::array<GLenum, N>& table, Enum value, const char* enumName)
        {
            static_assert(N == static_cast<std::size_t>(Enum::Count), "Conversion table size does not match the enum");

            const auto index = static_cast<std::size_t>(value);
            if (index >= N)
                throw std::invalid_argument(std::format("Invalid {} value: {}", enumName, index));

            return table[index];
        }

        [[nodiscard]] GLfloat maxAnisotropy() noexcept
        {
            static const GLfloat value = [] {
                GLfloat result = 1.0f;
                glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &result);
                return result;
                }();
            return value;
        }

        [[nodiscard]] GLsizei mipLevelCount(GLsizei width, GLsizei height) noexcept
        {
            const auto largest = static_cast<unsigned>(std::max(width, height));
            if (largest == 0)
                return 1;
            return static_cast<GLsizei>(std::bit_width(largest));
        }

        [[nodiscard]] GLenum toGl(Texture::Format format)
        {
            static constexpr std::array<GLenum, 20> table{
                GL_R8,
                GL_RG8,
                GL_RGB8,
                GL_RGBA8,
                GL_SRGB8,
                GL_SRGB8_ALPHA8,
                GL_R16F,
                GL_RG16F,
                GL_RGBA16F,
                GL_R32F,
                GL_RG32F,
                GL_RGBA32F,
                GL_R11F_G11F_B10F,
                GL_R32UI,
                GL_RG32UI,
                GL_RGBA32UI,
                GL_DEPTH_COMPONENT24,
                GL_DEPTH_COMPONENT32F,
                GL_DEPTH24_STENCIL8,
                GL_DEPTH32F_STENCIL8,
            };
            return lookup(table, format, "Texture::Format");
        }

        [[nodiscard]] GLenum toGl(Texture::PixelLayout layout)
        {
            static constexpr std::array<GLenum, 12> table{
                GL_RED,
                GL_RG,
                GL_RGB,
                GL_RGBA,
                GL_BGR,
                GL_BGRA,
                GL_RED_INTEGER,
                GL_RG_INTEGER,
                GL_RGB_INTEGER,
                GL_RGBA_INTEGER,
                GL_DEPTH_COMPONENT,
                GL_DEPTH_STENCIL,
            };
            return lookup(table, layout, "Texture::PixelLayout");
        }

        [[nodiscard]] GLenum toGl(Texture::DataType dataType)
        {
            static constexpr std::array<GLenum, 8> table{
                GL_BYTE,
                GL_UNSIGNED_BYTE,
                GL_SHORT,
                GL_UNSIGNED_SHORT,
                GL_INT,
                GL_UNSIGNED_INT,
                GL_FLOAT,
                GL_HALF_FLOAT,
            };
            return lookup(table, dataType, "Texture::DataType");
        }

        [[nodiscard]] GLenum toGl(Texture::Filter filter)
        {
            static constexpr std::array<GLenum, 6> table{
                GL_NEAREST,
                GL_LINEAR,
                GL_NEAREST_MIPMAP_NEAREST,
                GL_LINEAR_MIPMAP_NEAREST,
                GL_NEAREST_MIPMAP_LINEAR,
                GL_LINEAR_MIPMAP_LINEAR,
            };
            return lookup(table, filter, "Texture::Filter");
        }

        [[nodiscard]] GLenum toGl(Texture::Wrap wrap)
        {
            static constexpr std::array<GLenum, 5> table{
                GL_REPEAT,
                GL_MIRRORED_REPEAT,
                GL_CLAMP_TO_EDGE,
                GL_CLAMP_TO_BORDER,
                GL_MIRROR_CLAMP_TO_EDGE,
            };
            return lookup(table, wrap, "Texture::Wrap");
        }

    }

    Texture::Texture(const TextureSpecification& spec, const void* pixels) : m_width(spec.width), m_height(spec.height), m_specification(spec)
	{
        GLuint texture = 0;
        glCreateTextures(GL_TEXTURE_2D, 1, &texture);

        const GLsizei levels = spec.generateMipmaps ? mipLevelCount(spec.width, spec.height) : 1;

        glTextureStorage2D(texture, levels, toGl(spec.internalFormat), spec.width, spec.height);

        if (pixels != nullptr)
        {
            glTextureSubImage2D(texture, 0, 0, 0, spec.width, spec.height, toGl(spec.pixelLayout), toGl(spec.dataType), pixels);

            if (spec.generateMipmaps)
                glGenerateTextureMipmap(texture);
        }

        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(toGl(spec.minFilter)));
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(toGl(spec.magFilter)));
        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, static_cast<GLint>(toGl(spec.wrapS)));
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, static_cast<GLint>(toGl(spec.wrapT)));
        glTextureParameterf(texture, GL_TEXTURE_MAX_ANISOTROPY, maxAnisotropy());

        //if (!spec.name.empty())
            //glObjectLabel(GL_TEXTURE, texture, static_cast<GLsizei>(spec.name.size()), spec.name.c_str());

        m_id = texture;
	}

    Texture::~Texture()
    {
        destroy();
    }

    Texture::Texture(Texture&& other) noexcept
        : m_id(std::exchange(other.m_id, 0)), m_width(std::exchange(other.m_width, 0)), m_height(std::exchange(other.m_height, 0)), m_specification(std::exchange(other.m_specification, {}))
    {}

    Texture& Texture::operator=(Texture&& other) noexcept
    {
        if (this != &other)
        {
            destroy();
            m_id = std::exchange(other.m_id, 0);
            m_width = std::exchange(other.m_width, 0);
            m_height = std::exchange(other.m_height, 0);
            m_specification = std::exchange(other.m_specification, {});
        }
        return *this;
    }

    void Texture::destroy() noexcept
    {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }

    void Texture::bind(uint32_t unit) const noexcept
    {
        glBindTextureUnit(unit, m_id);
    }

}