#pragma once
#include "GraphicsContext.hpp"
#include "Pipeline.hpp"
#include "VulkanBuffer.hpp"

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

	struct MaterialData
	{
		alignas(16) glm::vec3 Ambient;
		alignas(16) glm::vec3 Diffuse;
		alignas(16) glm::vec3 Specular;
		float Shininess;
	};

	struct CameraData
	{
		glm::mat4 ViewProjectionMatrix;
		alignas(16) glm::vec3 CameraTranslation;
	};

	struct LightData
	{
		alignas(16) glm::vec3 Position;
		alignas(16) glm::vec3 Color;
	};

	struct SceneData
	{
		CameraData Camera;
		LightData Light;
	};

	class Renderer
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnResize();

	public:
		static void BeginScene(const SceneData& scene);
		static void EndScene();

		static void DrawCube(const Transform& transform, const MaterialData& material);

	public:
		static VkRenderPass GetMainRenderPass() { return sData->MainRenderPass; }

	private:
		static void CreateOrRecreateDepthTexture();
		static void CreateOrRecreateFramebuffers();

	private:
		struct PushConstants
		{
			glm::mat4 CubeTransform;
		};

		struct ObjectData
		{
			glm::mat4 Transform;
			MaterialData Material;
		};
		
		struct RendererData
		{
			VkRenderPass MainRenderPass;
			std::shared_ptr<Pipeline> CubePipeline;
			std::vector<VkFramebuffer> Framebuffers;
			std::unique_ptr<VulkanBuffer> VertexBuffer, IndexBuffer;

			VkImage DepthTexture;
			VkImageView DepthTextureView;
			VkDeviceMemory DepthTextureMemory;

			std::unique_ptr<VulkanBuffer> SceneDataUniformBuffer;
			VkDescriptorSetLayout SceneDataDescriptorSetLayout;
			VkDescriptorSet SceneDataDescriptorSet;

			static constexpr uint32_t MaxObjectCount = 10000;

			std::unique_ptr<VulkanBuffer> ObjectDataUniformBuffer;
			VkDescriptorSetLayout ObjectDataDescriptorSetLayout;
			VkDescriptorSet ObjectDataDescriptorSet;

			SceneData* MappedSceneData = nullptr;
			ObjectData* MappedObjectData = nullptr;

			std::array<ObjectData, MaxObjectCount> Objects;
			uint32_t ObjectIndex = 0;
		};

		inline static RendererData* sData = nullptr;
	};

}
