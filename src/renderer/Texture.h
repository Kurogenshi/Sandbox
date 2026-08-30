#pragma once

#include <cstdint>
#include <string>

namespace sandbox {

class Texture
{
public:
	enum class Format : std::uint8_t
	{
		R8 = 0, RG8, RGB8, RGBA8,
		SRGB8, SRGB8_ALPHA8,
		R16F, RG16F, RGBA16F,
		R32F, RG32F, RGBA32F,
		R11F_G11F_B10F,
		R32UI, RG32UI, RGBA32UI,
		Depth24, Depth32F, Depth24Stencil8, Depth32FStencil8,

		Count
	};

	enum class PixelLayout : std::uint8_t
	{
		R = 0, RG, RGB, RGBA, BGR, BGRA,
		RInteger, RGInteger, RGBInteger, RGBAInteger,
		DepthComponent, DepthStencil,

		Count
	};

	enum class DataType : std::uint8_t
	{
		BYTE = 0, UNSIGNED_BYTE,
		SHORT, UNSIGNED_SHORT,
		INT, UNSIGNED_INT,
		FLOAT, HALF_FLOAT,

		Count
	};

	enum class Filter : std::uint8_t
	{
		NEAREST = 0, LINEAR,
		NEAREST_MIPMAP_NEAREST,
		LINEAR_MIPMAP_NEAREST,
		NEAREST_MIPMAP_LINEAR,
		LINEAR_MIPMAP_LINEAR,

		Count
	};

	enum class Wrap : std::uint8_t
	{
		REPEAT = 0,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER,
		MIRROR_CLAMP_TO_EDGE,

		Count
	};

	struct TextureSpecification
	{
		int width = 0;
		int height = 0;
		Format internalFormat = Format::SRGB8_ALPHA8;
		PixelLayout pixelLayout = PixelLayout::RGBA;
		DataType dataType = DataType::UNSIGNED_BYTE;
		bool generateMipmaps = true;
		Filter minFilter = Filter::LINEAR_MIPMAP_LINEAR;
		Filter magFilter = Filter::LINEAR;
		Wrap wrapS = Wrap::REPEAT;
		Wrap wrapT = Wrap::REPEAT;
		bool srgb = false;
		bool flip = false;
	};

	Texture() noexcept = default;
	Texture(const TextureSpecification& spec, const void* pixels);
	~Texture();

	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	Texture(Texture&& other) noexcept;
	Texture& operator=(Texture&& other) noexcept;

	void bind(uint32_t unit = 0) const noexcept;

	[[nodiscard]] uint32_t id() const noexcept { return m_id; }
	[[nodiscard]] int width() const noexcept { return m_width; }
	[[nodiscard]] int height() const noexcept { return m_height; }

	[[nodiscard]] const TextureSpecification& specification() const noexcept { return m_Specification; }

private:
	void destroy() noexcept;

	uint32_t m_id = 0;
	int m_width = 0;
	int m_height = 0;

	TextureSpecification m_Specification;
};

}