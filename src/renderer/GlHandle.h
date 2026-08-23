#pragma once

#include <glad/gl.h>

#include <cstddef>
#include <functional>
#include <type_traits>

namespace sandbox {

template <typename Tag>
class GlHandle
{
public:
    using Underlying = GLuint;

    constexpr GlHandle() noexcept = default;
    constexpr explicit GlHandle(GLuint id) noexcept : m_id(id) {}

    [[nodiscard]] constexpr GLuint get() const noexcept { return m_id; }
    [[nodiscard]] constexpr bool valid() const noexcept { return m_id != 0; }

    constexpr explicit operator bool() const noexcept { return valid(); }

    [[nodiscard]] friend constexpr bool operator==(GlHandle, GlHandle) noexcept = default;

private:
    GLuint m_id = 0;
};

struct BufferTag;
struct FramebufferTag;
struct ProgramTag;
struct QueryTag;
struct RenderbufferTag;
struct SamplerTag;
struct ShaderTag;
struct TextureTag;
struct VertexArrayTag;

using BufferId       = GlHandle<BufferTag>;
using FramebufferId  = GlHandle<FramebufferTag>;
using ProgramId      = GlHandle<ProgramTag>;
using QueryId        = GlHandle<QueryTag>;
using RenderbufferId = GlHandle<RenderbufferTag>;
using SamplerId      = GlHandle<SamplerTag>;
using ShaderId       = GlHandle<ShaderTag>;
using TextureId      = GlHandle<TextureTag>;
using VertexArrayId  = GlHandle<VertexArrayTag>;

static_assert(sizeof(TextureId) == sizeof(GLuint));
static_assert(std::is_trivially_copyable_v<TextureId>);
static_assert(std::is_standard_layout_v<TextureId>);

}

template <typename Tag>
struct std::hash<sandbox::GlHandle<Tag>>
{
    [[nodiscard]] std::size_t operator()(sandbox::GlHandle<Tag> handle) const noexcept
    {
        return std::hash<GLuint>{}(handle.get());
    }
};