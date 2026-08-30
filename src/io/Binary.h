#pragma once

#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace sandbox::io {

	class BinaryWriter {
	public:
		explicit BinaryWriter(const std::filesystem::path& path) :
			m_path(path),
			m_stream(path, std::ios::binary | std::ios::trunc),
			m_finished(false)
		{
			if (!m_stream)
				throw std::runtime_error(std::format("Cannot open for writing: {}", path.string()));
		}

		template <typename T>
		void write(const T& value)
		{
			static_assert(std::is_trivially_copyable_v<T>);
			static_assert(!std::is_pointer_v<T>);

			m_stream.write(reinterpret_cast<const char*>(&value), static_cast<std::streamsize>(sizeof(T)));
			check("write");
		}

		void writeBytes(std::span<const std::byte> data)
		{
			if (data.empty())
				return;

			m_stream.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size_bytes()));
			check("write");
		}

		template <typename Range>
		void writeArray(const Range& values)
		{
			using T = std::remove_cvref_t<decltype(*std::data(values))>;
			static_assert(std::is_trivially_copyable_v<T>);

			const auto count = std::size(values);
			if (count == 0)
				return;

			m_stream.write(reinterpret_cast<const char*>(std::data(values)), static_cast<std::streamsize>(count * sizeof(T)));
			check("write");
		}

		void finish()
		{
			m_stream.flush();
			check("flush");
			m_stream.close();
			check("close");
			m_finished = true;
		}

		~BinaryWriter()
		{
			if (!m_finished && m_stream.is_open())
			{
				m_stream.close();

				if (!m_stream)
					std::fprintf(stderr, "Warning: unchecked close on %s\n", m_path.string().c_str());
			}
		}

	private:
		void check(const char* what) const
		{
			if (!m_stream)
				throw std::runtime_error(std::format("Failed to {} to {}", what, m_path.string()));
		}

		std::filesystem::path m_path;
		std::ofstream m_stream;
		bool m_finished = false;
	};

	class BinaryReader
	{
	public:
		explicit BinaryReader(const std::filesystem::path& path) :
			m_path(path),
			m_stream(m_path, std::ios::binary),
			m_size(0)
		{
			if (!m_stream)
				throw std::runtime_error(std::format("Cannot open for reading: {}", path.string()));

			m_size = std::filesystem::file_size(path);
		}

		template <typename T>
		[[nodiscard]] T read()
		{
			static_assert(std::is_trivially_copyable_v<T>);

			T value{};
			m_stream.read(reinterpret_cast<char*>(&value), static_cast<std::streamsize>(sizeof(T)));

			checkCount(sizeof(T));
			return value;
		}

		[[nodiscard]] std::vector<std::byte> readBytes(std::uint64_t count)
		{
			if (count > remaining())
				throw std::runtime_error(std::format("{}: asked for {} bytes, only {} remain", m_path.string(), count, remaining()));

			std::vector<std::byte> buffer(static_cast<std::size_t>(count));
			if (count == 0)
				return buffer;

			m_stream.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(count));
			checkCount(count);
			return buffer;
		}

		template <typename T>
		[[nodiscard]] std::vector<T> readArray(std::uint64_t count)
		{
			static_assert(std::is_trivially_copyable_v<T>);

			const std::uint64_t bytes = count * sizeof(T);
			if (count != 0 && bytes / count != sizeof(T))
				throw std::runtime_error("Element count overflows");
			if (bytes > remaining())
				throw std::runtime_error(std::format("{}: asked for {} bytes, only {} remain", m_path.string(), bytes, remaining()));

			std::vector<T> values(static_cast<std::size_t>(count));
			if (count == 0)
				return values;

			m_stream.read(reinterpret_cast<char*>(values.data()), static_cast<std::streamsize>(bytes));
			checkCount(bytes);
			return values;
		}

		[[nodiscard]] std::uint64_t remaining()
		{
			const auto position = static_cast<std::uint64_t>(m_stream.tellg());
			return position >= m_size ? 0 : m_size - position;
		}

	private:
		void checkCount(std::uint64_t expected) const
		{
			const auto got = static_cast<std::uint64_t>(m_stream.gcount());
			if (got != expected)
				throw std::runtime_error(std::format("{}: truncated file, expected {} bytes, got {}", m_path.string(), expected, got));
		}

		std::filesystem::path m_path;
		std::ifstream m_stream;
		std::uint64_t m_size;
	};

}