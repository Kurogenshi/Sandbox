#pragma once

#include <cstdint>

namespace sandbox {

	enum class AssetType : std::uint8_t
	{
		Texture = 0,
		Mesh
	};

	class Asset
	{
	public:
		explicit Asset(std::uint64_t id) noexcept : m_id(id) {}
		virtual ~Asset() = default;

		Asset(const Asset&) = delete;
		Asset& operator=(const Asset&) = delete;
		Asset(Asset&&) = default;
		Asset& operator=(Asset&&) = default;

		[[nodiscard]] std::uint64_t getId() const noexcept { return m_id; }
		[[nodiscard]] virtual AssetType getType() const noexcept = 0;

	protected:
		std::uint64_t m_id;
	};
}