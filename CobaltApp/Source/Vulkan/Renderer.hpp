#pragma once
#include "GraphicsContext.hpp"
#include "ShaderStructs.hpp"
#include "Pipeline.hpp"
#include "VulkanBuffer.hpp"
#include "Texture.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <array>

namespace Cobalt
{

	struct Transform
	{
		glm::vec3 Translation;
		glm::vec3 Rotation;
		glm::vec3 Scale = glm::vec3(1.0f);

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation) * rotation * glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct DrawCall
	{
		VulkanBuffer* IndexBuffer;
		uint32_t IndexCount;
		Material* Material;
	};

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnResize();

	public:
		static TextureHandle CreateTexture(const TextureInfo& textureInfo);
		static std::unique_ptr<Material> CreateMaterial(const MaterialData& materialData);

		static Texture& GetTexture(TextureHandle textureHandle);

		static void BeginScene(const SceneData& scene);
		static void EndScene();

		static void DrawMesh(const Transform& transform, Mesh* mesh);

	public:
		static VkRenderPass GetMainRenderPass() { return sData->MainRenderPass; }

	private:
		static void CreateOrRecreateDepthTexture();
		static void CreateOrRecreateFramebuffers();

	private:
		struct PushConstants
		{
			//glm::mat4 CubeTransform;
		};

		struct RendererData
		{
			VkRenderPass MainRenderPass;
			std::vector<VkFramebuffer> Framebuffers;

			std::unique_ptr<Texture> DepthTexture;

			static constexpr uint32_t sGlobalDescriptorSetIndex   = 0;
			static constexpr uint32_t sMaterialDescriptorSetIndex = 1;
			static constexpr uint32_t sObjectDescriptorSetIndex   = 2;

			SceneData ActiveScene;

			// per frame-in-flight
			std::vector<std::unique_ptr<VulkanBuffer>> SceneDataUniformBuffers;
			std::vector<std::unique_ptr<VulkanBuffer>> ObjectStorageBuffers;
			std::vector<std::unique_ptr<VulkanBuffer>> MaterialDataStorageBuffers;

			std::vector<std::unique_ptr<Texture>> Textures;

			static constexpr uint32_t MaxMaterialCount = 100;
			static constexpr uint32_t MaxObjectCount = 10000;

			std::vector<MaterialData> Materials;
			std::vector<ObjectData> Objects;

			std::vector<DrawCall> DrawCalls;

			static constexpr const char* sDefaultShaderFilePath = "CobaltApp/Assets/Shaders/DefaultShader.glsl";
		};

		inline static RendererData* sData = nullptr;
	};

}
