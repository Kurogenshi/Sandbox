#include <core/Window.h>
#include <renderer/Shader.h>
#include <renderer/Texture.h>
#include <io/Binary.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <stb_image.h>

#include <cstdio>
#include <exception>
#include <format>
#include <stdexcept>
#include <iterator>
#include <span>
#include <cstddef>
#include <iostream>
#include <vector>
#include <filesystem>

struct StbDeleter
{
    void operator()(unsigned char* p) const noexcept { stbi_image_free(p); }
};

std::vector<std::byte> loadData(const std::filesystem::path& path)
{
    sandbox::io::BinaryReader reader(path);
    return reader.readBytes(reader.remaining());
}

sandbox::Texture loadTexture(std::span<const std::byte> data, std::string_view name, bool srgb = true, bool flip = false)
{
    stbi_set_flip_vertically_on_load(flip);

    int width = 0, height = 0, channels = 0;

    std::unique_ptr<unsigned char, StbDeleter> pixels(stbi_load_from_memory(reinterpret_cast<const unsigned char*>(data.data()), static_cast<int>(data.size_bytes()), &width, &height, &channels, 4));

    if (!pixels)
        throw std::runtime_error(std::format("Failed to load {}: {}", name, stbi_failure_reason()));

    sandbox::Texture::TextureSpecification spec;
    spec.width = width;
    spec.height = height;
    spec.internalFormat = srgb ? sandbox::Texture::Format::SRGB8_ALPHA8 : sandbox::Texture::Format::RGBA8;
    spec.srgb = srgb;
    spec.flip = flip;

    return sandbox::Texture{ spec, pixels.get() };
}

sandbox::Texture loadTexture(std::span<const std::byte> data, const sandbox::Texture::TextureSpecification& spec) {
    stbi_set_flip_vertically_on_load(spec.flip);

    int width = 0, height = 0, channels = 0;

    std::unique_ptr<unsigned char, StbDeleter> pixels(stbi_load_from_memory(reinterpret_cast<const unsigned char*>(data.data()), static_cast<int>(data.size_bytes()), &width, &height, &channels, 4));

    if (!pixels)
        throw std::runtime_error(std::format("Failed to load: {}", stbi_failure_reason()));

    return sandbox::Texture{ spec, pixels.get() };
}

enum class ResourceType : uint8_t {
    Texture = 0,
    Mesh
};

struct ResourceHeader {
    ResourceType type;
    uint64_t dataSize;
};

struct MeshSpecification {
    uint64_t vertexSize;
    uint64_t vertexCount;
    uint64_t indexCount;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;
};

void exportResource(const std::filesystem::path& path, const sandbox::Texture::TextureSpecification& spec, std::span<const std::byte> data) {
    sandbox::io::BinaryWriter writer(path);

    ResourceHeader header{};
    header.type = ResourceType::Texture;
    header.dataSize = data.size_bytes();

    writer.write(header);
    writer.write(spec);
    writer.writeBytes(data);

    writer.finish();
}

void exportResource(const std::filesystem::path& path, const MeshSpecification& spec, std::span<Vertex> vertices, std::span<uint32_t> indices) {
    sandbox::io::BinaryWriter writer(path);

    ResourceHeader header{};
    header.type = ResourceType::Mesh;
    header.dataSize = (vertices.size_bytes() + indices.size_bytes());

    writer.write(header);
    writer.write(spec);
    writer.writeArray(vertices);
    writer.writeArray(indices);

    writer.finish();
}

void importResource(const std::filesystem::path& path, sandbox::Texture::TextureSpecification& spec, std::vector<std::byte>& data) {
    sandbox::io::BinaryReader reader(path);

    const auto header = reader.read<ResourceHeader>();
    if (header.type != ResourceType::Texture)
        throw std::runtime_error(std::format("{}: not a texture resource", path.string()));

    spec = reader.read<sandbox::Texture::TextureSpecification>();
    data = reader.readBytes(header.dataSize);
}

void importResource(const std::filesystem::path& path, MeshSpecification& spec, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    sandbox::io::BinaryReader reader(path);

    const auto header = reader.read<ResourceHeader>();
    if (header.type != ResourceType::Mesh)
        throw std::runtime_error(std::format("{}: not a mesh resource", path.string()));

    spec = reader.read<MeshSpecification>();

    if (spec.vertexSize != sizeof(Vertex))
        throw std::runtime_error(std::format("{}: vertex size mismatch, file has {}, code expects {}", path.string(), spec.vertexSize, sizeof(Vertex)));

    vertices = reader.readArray<Vertex>(spec.vertexCount);
    indices = reader.readArray<std::uint32_t>(spec.indexCount);
}

int main()
{
    try
    {
        sandbox::Window::Settings settings;

        sandbox::Window window(settings);

        sandbox::Shader shader(std::filesystem::path(SANDBOX_ASSET_DIR) / "shaders/basic.vert", std::filesystem::path(SANDBOX_ASSET_DIR) / "shaders/basic.frag", "BasicShader");

        std::vector<Vertex> vertices = {
            // Front
            {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

            // Back
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

            // Left
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

            // Right
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

            // Bottom
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

            // Top
            {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        };

        std::vector<uint32_t> indices = {
             0,  1,  2,  2,  3,  0,
             4,  5,  6,  6,  7,  4,
             8,  9, 10, 10, 11,  8,
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 21, 22, 22, 23, 20,
        };

        MeshSpecification meshSpec;
        meshSpec.vertexSize = sizeof(Vertex);
        meshSpec.vertexCount = static_cast<uint64_t>(vertices.size());
        meshSpec.indexCount = static_cast<uint64_t>(indices.size());

        GLuint vbo = 0;
        glCreateBuffers(1, &vbo);
        glNamedBufferStorage(vbo, vertices.size() * sizeof(Vertex), vertices.data(), 0);

        GLuint ebo = 0;
        glCreateBuffers(1, &ebo);
        glNamedBufferStorage(ebo, indices.size() * sizeof(uint32_t), indices.data(), 0);

        GLuint vao = 0;
        glCreateVertexArrays(1, &vao);

        glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(vao, ebo);

        glEnableVertexArrayAttrib(vao, 0);
        glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexArrayAttribBinding(vao, 0, 0);

        glEnableVertexArrayAttrib(vao, 1);
        glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, color));
        glVertexArrayAttribBinding(vao, 1, 0);

        glEnableVertexArrayAttrib(vao, 2);
        glVertexArrayAttribFormat(vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texCoord));
        glVertexArrayAttribBinding(vao, 2, 0);

        glObjectLabel(GL_VERTEX_ARRAY, vao, -1, "QuadVAO");
        glObjectLabel(GL_BUFFER, vbo, -1, "QuadVBO");
        glObjectLabel(GL_BUFFER, ebo, -1, "QuadEBO");

        std::filesystem::path texturePath1 = std::filesystem::path(SANDBOX_ASSET_DIR) / "textures/test.jpg";
        std::filesystem::path texturePath2 = std::filesystem::path(SANDBOX_ASSET_DIR) / "textures/test2.png";

        std::vector<std::byte> textureData1 = loadData(texturePath1);
        std::vector<std::byte> textureData2 = loadData(texturePath2);

        sandbox::Texture texture1 = loadTexture(textureData1, texturePath1.filename().string(), true);
        sandbox::Texture texture2 = loadTexture(textureData2, texturePath2.filename().string(), true, true);

        exportResource(std::filesystem::path(SANDBOX_ASSET_DIR) / "textures/test.texture", texture1.specification(), textureData1);
        exportResource(std::filesystem::path(SANDBOX_ASSET_DIR) / "textures/test2.texture", texture2.specification(), textureData2);

        exportResource(std::filesystem::path(SANDBOX_ASSET_DIR) / "textures/test.mesh", meshSpec, vertices, indices);

        sandbox::Texture::TextureSpecification importedTexture1Spec;
        std::vector<std::byte> importedTexture1Data;
        importResource(std::filesystem::path(SANDBOX_ASSET_DIR) / "textures/test.texture", importedTexture1Spec, importedTexture1Data);

        sandbox::Texture::TextureSpecification importedTexture2Spec;
        std::vector<std::byte> importedTexture2Data;
        importResource(std::filesystem::path(SANDBOX_ASSET_DIR) / "textures/test2.texture", importedTexture2Spec, importedTexture2Data);

        sandbox::Texture importedTexture1 = loadTexture(importedTexture1Data, importedTexture1Spec);
        sandbox::Texture importedTexture2 = loadTexture(importedTexture2Data, importedTexture2Spec);

        glm::mat4 proj = glm::mat4(1.0f);

        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

        glm::vec3 cubePositions[] = {
            glm::vec3(0.0f,  0.0f,  0.0f),
            glm::vec3(2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3(2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3(1.3f, -2.0f, -2.5f),
            glm::vec3(1.5f,  2.0f, -2.5f),
            glm::vec3(1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
        };

        while (!window.shouldClose())
        {
            window.pollEvents();

            if (glfwGetKey(window.handle(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(window.handle(), GLFW_TRUE);
            }

            glClearColor(0.10f, 0.11f, 0.13f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            proj = glm::perspective(glm::radians(45.0f), static_cast<float>(window.width()) / static_cast<float>(window.height()), 0.1f, 100.0f);

            shader.bind();

            shader.set(1, view);
            shader.set(2, proj);

            importedTexture1.bind(3);
            importedTexture2.bind(4);

            glBindVertexArray(vao);
            for (unsigned int i = 0; i < 10; i++)
            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, cubePositions[i]);
                float angle = 20.0f * static_cast<float>(i);
                model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

                shader.set(0, model);

                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(std::size(indices)), GL_UNSIGNED_INT, nullptr);
            }
            glBindVertexArray(0);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            //ImGui::ShowDemoWindow();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            window.swapBuffers();
        }

        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
    }
    catch (const std::exception& e)
    {
        std::fprintf(stderr, "Fatal error : %s\n", e.what());
        return 1;
    }

    return 0;
}
