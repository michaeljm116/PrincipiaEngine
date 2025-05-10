#include "../pch.h"
#include "VulkanInitializers.hpp"
#include "compute-raytracer.h"
#include <set>
#include <unordered_map>
#include "../Utility/resourceManager.h"
#include "../Utility/Input.h"
#include <embree4/rtcore.h>
#include <embree4/rtcore_builder.h>
#include "../Utility/helpers.h"
namespace Principia {

	static int curr_id = 0;	// Id used to identify objects by the ray tracing shader
	static const int MAX_MATERIALS = 256;
	static const int MAX_MESHES = 2048;
	static const int MAX_VERTS = 32768;
	static const int MAX_INDS = 16384;
	static const int MAX_OBJS = 4096;
	static const int MAX_LIGHTS = 32;
	static const int MAX_GUIS = 96;
	static const int MAX_NODES = 2048;
	static const int MAX_BINDLESS_TEXTURES = 256;

	ComputeRaytracer::ComputeRaytracer()
	{
	}

	ComputeRaytracer::~ComputeRaytracer()
	{
	}

	void ComputeRaytracer::PrepareStorageBuffers()
	{	//objects.reserve(MAXOBJS);
	//verts.reserve(MAXVERTS);
	//indices.reserve(MAXINDS);
		materials_.reserve(MAX_MATERIALS);
		lights_.reserve(MAX_LIGHTS);


		//THESE SHOULD BE STAGED MEOW
		//compute_.storage_buffers.verts.InitStorageBufferWithStaging(vkDevice, verts, verts.size());
		//compute_.storage_buffers.indices.InitStorageBufferWithStaging(vkDevice, indices, indices.size());

		//compute_.storage_buffers.verts.InitStorageBufferCustomSize(vkDevice, verts, verts.size(), MAXVERTS);
		//compute_.storage_buffers.indices.InitStorageBufferCustomSize(vkDevice, indices, indices.size(), MAXINDS);

		//these are changable
		//std::vector<PrimitiveComponent> temp;
		//temp.push_back(PrimitiveComponent());
		compute_.storage_buffers.primitives.InitStorageBufferCustomSize(vkDevice, primitives_, primitives_.size(), MAX_OBJS);
		compute_.storage_buffers.materials.InitStorageBufferCustomSize(vkDevice, materials_, materials_.size(), MAX_MATERIALS);
		compute_.storage_buffers.lights.InitStorageBufferCustomSize(vkDevice, lights_, lights_.size(), MAX_LIGHTS);


		//create 1 gui main global kind of gui for like title/menu screen etc...
		GUIComponent* guiComp = (GUIComponent*)world->getSingleton()->getComponent<GUIComponent>();
		ssGUI gui = ssGUI(guiComp->min, guiComp->extents, guiComp->alignMin, guiComp->alignExt, guiComp->layer, guiComp->id);
		gui.alpha = guiComp->alpha;

		//Give the component a reference to it and initialize
		guiComp->ref = static_cast<int>(guis_.size());
		guis_.push_back(gui);
		compute_.storage_buffers.guis.InitStorageBufferCustomSize(vkDevice, guis_, guis_.size(), MAX_GUIS);

		//std::vector<ssBVHNode> tempbvh;
		//tempbvh.push_back(ssBVHNode());
		compute_.storage_buffers.bvh.InitStorageBufferCustomSize(vkDevice, bvh_, bvh_.size(), MAX_NODES);

	}

	void ComputeRaytracer::CreateUniformBuffers()
	{
		compute_.uniform_buffer.Initialize(vkDevice, 1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		compute_.uniform_buffer.ApplyChanges(vkDevice, compute_.ubo);
	}

	void ComputeRaytracer::PrepareTextureTarget(Texture* tex, uint32_t width, uint32_t height, VkFormat format)
	{
		// Get device properties for the requested texture format
		VkFormatProperties formatProperties;

		vkGetPhysicalDeviceFormatProperties(vkDevice.physicalDevice, format, &formatProperties);
		// Check if requested image format supports image storage operations
		assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);

		// Prepare blit target texture
		tex->width = width;
		tex->height = height;

		VkImageCreateInfo imageCreateInfo = vks::initializers::imageCreateInfo();
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.extent = { width, height, 1 };
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		// Image will be sampled in the fragment shader and used as storage target in the compute shader
		imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		imageCreateInfo.flags = 0;

		VkMemoryAllocateInfo memAllocInfo = vks::initializers::memoryAllocateInfo();
		VkMemoryRequirements memReqs;

		VK_CHECKRESULT(vkCreateImage(vkDevice.logicalDevice, &imageCreateInfo, nullptr, &tex->image), "CREATE IMAGE");
		vkGetImageMemoryRequirements(vkDevice.logicalDevice, tex->image, &memReqs);
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = vkDevice.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECKRESULT(vkAllocateMemory(vkDevice.logicalDevice, &memAllocInfo, nullptr, &tex->memory), "ALLOCATE TXTR MEMORY");
		VK_CHECKRESULT(vkBindImageMemory(vkDevice.logicalDevice, tex->image, tex->memory, 0), "BIND IMAGE MEMORY");

		VkCommandBuffer layoutCmd = vkDevice.beginSingleTimeCommands(); //VulkanExampleBase::createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		tex->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		vkDevice.setImageLayout(
			layoutCmd,
			tex->image,
			VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			tex->imageLayout);

		vkDevice.endSingleTimeCommands(layoutCmd); //VulkanExampleBase::flushCommandBuffer(layoutCmd, queue, true);

		// Create sampler
		VkSamplerCreateInfo sampler = vks::initializers::samplerCreateInfo();
		sampler.magFilter = VK_FILTER_LINEAR;
		sampler.minFilter = VK_FILTER_LINEAR;
		sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		sampler.addressModeV = sampler.addressModeU;
		sampler.addressModeW = sampler.addressModeU;
		sampler.mipLodBias = 0.0f;
		sampler.maxAnisotropy = 1.0f;
		sampler.compareOp = VK_COMPARE_OP_NEVER;
		sampler.minLod = 0.0f;
		sampler.maxLod = 0.0f;
		sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VK_CHECKRESULT(vkCreateSampler(vkDevice.logicalDevice, &sampler, nullptr, &tex->sampler), "CREATE SAMPLER");

		// Create image view
		VkImageViewCreateInfo view = vks::initializers::imageViewCreateInfo();
		view.viewType = VK_IMAGE_VIEW_TYPE_2D;
		view.format = format;
		view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		view.image = tex->image;
		VK_CHECKRESULT(vkCreateImageView(vkDevice.logicalDevice, &view, nullptr, &tex->view), "CREATE IMAGE");

		// Initialize a descriptor for later use
		tex->descriptor.imageLayout = tex->imageLayout;
		tex->descriptor.imageView = tex->view;
		tex->descriptor.sampler = tex->sampler;
		//tex->device = vulkanDevice;
	}

	void ComputeRaytracer::CreateComputeCommandBuffer()
	{
		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VK_CHECKRESULT(vkBeginCommandBuffer(compute_.command_buffer, &cmdBufInfo), "CREATE COMPUTE COMMAND BUFFER");

		vkCmdBindPipeline(compute_.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_.pipeline);
		vkCmdBindDescriptorSets(compute_.command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute_.pipeline_layout, 0, 1, &compute_.descriptor_set, 0, 0);

		vkCmdDispatch(compute_.command_buffer, compute_texture_.width / 16, compute_texture_.height / 16, 1);

		vkEndCommandBuffer(compute_.command_buffer);
	}

	void ComputeRaytracer::PrepareCompute()
	{
		// Create a compute capable device queue
		// The VulkanDevice::createLogicalDevice functions finds a compute capable queue and prefers queue families that only support compute
		// Depending on the implementation this may result in different queue family indices for graphics and computes,
		// requiring proper synchronization (see the memory barriers in buildComputeCommandBuffer)
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pNext = NULL;
		queueCreateInfo.queueFamilyIndex = vkDevice.qFams.computeFamily;
		queueCreateInfo.queueCount = 1;
		vkGetDeviceQueue(vkDevice.logicalDevice, vkDevice.qFams.computeFamily, 0, &compute_.queue);

		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			// Binding 0: Storage image (raytraced output)
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				VK_SHADER_STAGE_COMPUTE_BIT,
				0),
			// Binding 1: Uniform buffer block
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				VK_SHADER_STAGE_COMPUTE_BIT,
				1),
			// Binding 2: Shader storage buffer for the verts
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				VK_SHADER_STAGE_COMPUTE_BIT,
				2),
			// Binding 3: Shader storage buffer for the indices
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				VK_SHADER_STAGE_COMPUTE_BIT,
				3),
			// Binding 4: Shader storage buffer for the blas
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				VK_SHADER_STAGE_COMPUTE_BIT,
				4),
			// Binding 5: Shader storage buffer for the shapes
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				VK_SHADER_STAGE_COMPUTE_BIT,
				5),
			// Binding 6: Shader storage buffer for the objects
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				VK_SHADER_STAGE_COMPUTE_BIT,
				6),
			// Binding 7: Shader storage buffer for the materials
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				VK_SHADER_STAGE_COMPUTE_BIT,
				7),
			// Binding 8: Shader storage buffer for the lights
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				VK_SHADER_STAGE_COMPUTE_BIT,
				8),
			// binding 9: Shader storage buffer for the guis_
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				VK_SHADER_STAGE_COMPUTE_BIT,
				9),
			// Binding 10: Shader storage buffer for the bvh
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				VK_SHADER_STAGE_COMPUTE_BIT,
				10),
			// Binding 12: the textures
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_COMPUTE_BIT,
				11, MAX_TEXTURES),
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_COMPUTE_BIT,
				12, MAX_BINDLESS_TEXTURES)
		};		

		VkDescriptorSetLayoutCreateInfo descriptorLayout =
			vks::initializers::descriptorSetLayoutCreateInfo(
				setLayoutBindings.data(),
				static_cast<uint32_t>(setLayoutBindings.size()));
		descriptorLayout.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;
		
		VkDescriptorBindingFlags bindless_flags = 
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
			VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
		VkDescriptorBindingFlags binding_flags[13] = {};
		binding_flags[12] = bindless_flags;

		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_flags_info{
			VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
			nullptr
		};
		extended_flags_info.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		extended_flags_info.pBindingFlags = binding_flags;
		descriptorLayout.pNext = &extended_flags_info;


		VK_CHECKRESULT(vkCreateDescriptorSetLayout(vkDevice.logicalDevice, &descriptorLayout, nullptr, &compute_.descriptor_set_layout), "CREATE COMPUTE DSL");

		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
			vks::initializers::pipelineLayoutCreateInfo(
				&compute_.descriptor_set_layout,
				1);

		VK_CHECKRESULT(vkCreatePipelineLayout(vkDevice.logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &compute_.pipeline_layout), "CREATECOMPUTE PIEPLINEEEE");

		VkDescriptorSetAllocateInfo allocInfo =
			vks::initializers::descriptorSetAllocateInfo(
				descriptor_pool_,
				&compute_.descriptor_set_layout,
				1);

		VK_CHECKRESULT(vkAllocateDescriptorSets(vkDevice.logicalDevice, &allocInfo, &compute_.descriptor_set), "ALLOCATE DOMPUTE DSET");

		VkDescriptorImageInfo textureimageinfos[MAX_TEXTURES] = {
			gui_textures_[0].descriptor,
			gui_textures_[1].descriptor,
			gui_textures_[2].descriptor,
			gui_textures_[3].descriptor,
			gui_textures_[4].descriptor
		};
		std::vector<VkDescriptorImageInfo> bindless_image_infos;
		auto num_textures = bindless_textures.size();
		bindless_image_infos.reserve(num_textures);
		for (auto t : bindless_textures) {
			bindless_image_infos.push_back(t.descriptor);
		}

		compute_write_descriptor_sets_ =
		{
			// Binding 0: Output storage image
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
				0,
				&compute_texture_.descriptor),
			// Binding 1: Uniform buffer block
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				1,
				&compute_.uniform_buffer.bufferInfo),
			// Binding 2: Shader storage buffer for the verts
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				2,
				&compute_.storage_buffers.verts.bufferInfo),
			// Binding 3: Shader storage buffer for the indices
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				3,
				&compute_.storage_buffers.faces.bufferInfo),
			// Binding 4: for blas
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				4,
				&compute_.storage_buffers.blas.bufferInfo),
			//Binding 5: for shapes
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				5,
				&compute_.storage_buffers.shapes.bufferInfo),
			// Binding 6: for objectss
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				6,
				&compute_.storage_buffers.primitives.bufferInfo),
			//Binding 8 for materials
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				7,
				&compute_.storage_buffers.materials.bufferInfo),
			//Binding 9 for lights
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				8,
				&compute_.storage_buffers.lights.bufferInfo),
			//Binding 10 for guis_
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				9,
				&compute_.storage_buffers.guis.bufferInfo),
			//Binding 11 for bvhs
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				10,
				&compute_.storage_buffers.bvh.bufferInfo),
			//bINDING 12 FOR TEXTURES
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				11,
				textureimageinfos, MAX_TEXTURES),
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				12,
				bindless_image_infos.data(), static_cast<uint32_t>(num_textures))
		};

		vkUpdateDescriptorSets(vkDevice.logicalDevice, static_cast<uint32_t>(compute_write_descriptor_sets_.size()), compute_write_descriptor_sets_.data(), 0, NULL);

		// Create compute shader pipelines
		VkComputePipelineCreateInfo computePipelineCreateInfo =
			vks::initializers::computePipelineCreateInfo(
				compute_.pipeline_layout,
				0);

		computePipelineCreateInfo.stage = vkDevice.createShader("../Assets/Shaders/raytracing.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
		VK_CHECKRESULT(vkCreateComputePipelines(vkDevice.logicalDevice, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &compute_.pipeline), "CREATE COMPUTE PIPELINE");

		// Separate command pool as queue family for compute may be different than graphics
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = vkDevice.qFams.computeFamily;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		VK_CHECKRESULT(vkCreateCommandPool(vkDevice.logicalDevice, &cmdPoolInfo, nullptr, &compute_.command_pool), "CREATE COMMAND POOL");

		// Create a command buffer for compute operations
		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
			vks::initializers::commandBufferAllocateInfo(
				compute_.command_pool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1);

		VK_CHECKRESULT(vkAllocateCommandBuffers(vkDevice.logicalDevice, &cmdBufAllocateInfo, &compute_.command_buffer), "ALLOCATE COMMAND BUFFERS");

		// Fence for compute CB sync
		VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
		VK_CHECKRESULT(vkCreateFence(vkDevice.logicalDevice, &fenceCreateInfo, nullptr, &compute_.fence), "CREATE FENCE");

		// Build a single command buffer containing the compute dispatch commands
		CreateComputeCommandBuffer();
		vkDestroyShaderModule(vkDevice.logicalDevice, computePipelineCreateInfo.stage.module, nullptr);
	}

	void ComputeRaytracer::DestroyCompute()
	{
		compute_.uniform_buffer.Destroy(vkDevice);
		compute_.storage_buffers.verts.Destroy(vkDevice);
		compute_.storage_buffers.faces.Destroy(vkDevice);
		compute_.storage_buffers.blas.Destroy(vkDevice);
		compute_.storage_buffers.shapes.Destroy(vkDevice);
		compute_.storage_buffers.primitives.Destroy(vkDevice);
		compute_.storage_buffers.materials.Destroy(vkDevice);
		compute_.storage_buffers.lights.Destroy(vkDevice);
		compute_.storage_buffers.guis.Destroy(vkDevice);
		compute_.storage_buffers.bvh.Destroy(vkDevice);

		compute_texture_.destroy(vkDevice.logicalDevice);
		for (int i = 0; i < MAX_TEXTURES; ++i)
			gui_textures_[i].destroy(vkDevice.logicalDevice);
		for (auto& t : bindless_textures) {
			t.destroy(vkDevice.logicalDevice);
		}
		vkDestroyPipelineCache(vkDevice.logicalDevice, pipelineCache, nullptr);
		vkDestroyPipeline(vkDevice.logicalDevice, compute_.pipeline, nullptr);
		vkDestroyPipelineLayout(vkDevice.logicalDevice, compute_.pipeline_layout, nullptr);
		vkDestroyDescriptorSetLayout(vkDevice.logicalDevice, compute_.descriptor_set_layout, nullptr);
		vkDestroyFence(vkDevice.logicalDevice, compute_.fence, nullptr);
		vkDestroyCommandPool(vkDevice.logicalDevice, compute_.command_pool, nullptr);
	}

	VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptor_type, uint32_t descriptor_count, VkShaderStageFlags flags)
	{
		VkDescriptorSetLayoutBinding dslb;

		dslb.binding = binding;
		dslb.descriptorType = descriptor_type;
		dslb.descriptorCount = descriptor_count;
		dslb.stageFlags = flags;
		dslb.pImmutableSamplers = nullptr;

		return dslb;
	}

	void ComputeRaytracer::UpdateBuffers()
	{
		vkWaitForFences(vkDevice.logicalDevice, 1, &compute_.fence, VK_TRUE, UINT64_MAX);
		if (update_flags_ & kUpdateNone)
			return;
		if (update_flags_ & kUpdateObject) {
			//compute_.storage_buffers.primitives.UpdateBuffers(vkDevice, primitives);
			update_flags_ &= ~kUpdateObject;
		}
		if (update_flags_ & kUpdateMaterial) {
			update_flags_ &= ~kUpdateMaterial;
		}
		if (update_flags_ & kUpdateLight) {
			compute_.storage_buffers.lights.UpdateBuffers(vkDevice, lights_);
			update_flags_ &= ~kUpdateLight;
		}
		if (update_flags_ & kUpdateGui) {
			compute_.storage_buffers.guis.UpdateBuffers(vkDevice, guis_);
			update_flags_ &= ~kUpdateGui;
		}

		if (update_flags_ & kUpdateBvh) {
			compute_.storage_buffers.primitives.UpdateAndExpandBuffers(vkDevice, primitives_, primitives_.size());
			compute_.storage_buffers.bvh.UpdateAndExpandBuffers(vkDevice, bvh_, bvh_.size());
			update_flags_ &= ~kUpdateBvh;
		}

		update_flags_ |= kUpdateNone;
		//compute_.storage_buffers.objects.UpdateAndExpandBuffers(vkDevice, objects, objects.size());
		//compute_.storage_buffers.bvh.UpdateAndExpandBuffers(vkDevice, bvh, bvh.size());
		UpdateDescriptors();
	}

	void ComputeRaytracer::UpdateCamera(CameraComponent* c)
	{
		compute_.ubo.aspect_ratio = c->aspectRatio;
		compute_.ubo.fov = glm::tan(c->fov * 0.03490658503f);
		compute_.ubo.rotM = c->rotM;
		compute_.ubo.rand = static_cast<int>(random_int());
		compute_.uniform_buffer.ApplyChanges(vkDevice, compute_.ubo);
	}
	auto print_bvh_nodes = [](std::vector<ssBVHNode> bvh) {
		int i = 0;
		for (const auto &n : bvh) {

				std::cout << "(" << i++ << ") Lower X:" << n.lower.x << " Y:" << n.lower.y << " Z:" << n.lower.z << " Upper X:" << n.upper.x << " Y:" << n.upper.y << " Z:" << n.upper.z
				<< " Offset: " << n.offset << " Children: " << n.numChildren << std::endl;
			}
		};

	void ComputeRaytracer::UpdateBVH(const std::vector<RTCBuildPrimitive>& ordered_prims, const std::vector<artemis::Entity*>& prims, BvhNode* root, int num_nodes)
	{
		int num_prims = static_cast<int>(ordered_prims.size());
		if (num_prims == 0)return;
		primitives_.clear();
		primitives_.reserve(num_prims);

		//fill in the new objects array; 
		ordered_prims_map.clear();
		ordered_prims_map.resize(num_prims);
		for (int i = 0; i < num_prims; ++i) {
			ordered_prims_map[ordered_prims[i].primID] = i;
			auto* prim = prims[ordered_prims[i].primID];
			PrimitiveComponent* pc = (PrimitiveComponent*)prim->getComponent<PrimitiveComponent>();
			if (pc) {
				primitives_.emplace_back(ssPrimitive(pc));
			}
		}

		int offset = 0;
		bvh_.resize(num_nodes);
		auto* first_node = (InnerBvhNode*)root;
		auto root_box = first_node->merge(first_node->bounds[0], first_node->bounds[1]);
		FlattenBVH(root, root_box, &offset, bvh_);
		SetRenderUpdate(kUpdateBvh);
	}

	int ComputeRaytracer::FlattenBVH(BvhNode* node, const BvhBounds& bounds, int* offset, std::vector<ssBVHNode>& bvh)
	{
		ssBVHNode* bvh_node = &bvh[*offset];
		int myOffset = (*offset)++;

		if (node->isLeaf()) {
			auto* leaf = (LeafBvhNode*)node;	
			bvh_node->upper = leaf->bounds.upper;
			bvh_node->lower = leaf->bounds.lower;
			bvh_node->numChildren = 0;
			bvh_node->offset =  ordered_prims_map[leaf->id]; //TODO THIS IS UGH.... I FORGOT ABOUT THIS....FIRSTPRIMOFFSET
		}
		else 
		{
			auto* inner = (InnerBvhNode*)node;
			bvh_node->upper = bounds.upper;
			bvh_node->lower = bounds.lower;
			bvh_node->numChildren = 2;
			//bvh_node->offset = myOffset;

			FlattenBVH(inner->children[0], inner->bounds[0], offset, bvh);
			bvh_node->offset = FlattenBVH(inner->children[1], inner->bounds[1], offset, bvh);	
		}
		return myOffset;
	}

	void ComputeRaytracer::TogglePlayMode(bool play_mode)
	{
		if (play_mode) {
			WINDOW.resize();
			RenderBase::recreateSwapChain();
			CreateDescriptorSetLayout();
			CreateGraphicsPipeline();
			CreateCommandBuffers(1.f, 0, 0);
		}
		else {
			recreateSwapChain();
		}
	}

	void ComputeRaytracer::SetStuffUp()
	{
		//camera_.type = Camera::CameraType::lookat;
		//camera_.setPerspective(13.0f, 1280.f / 720.f, 0.1f, 1256.0f);
		//camera_.setRotation(glm::vec3(35.0f, 90.0f, 45.0f));
		//camera_.setTranslation(glm::vec3(0.0f, 0.0f, -4.0f));
		//camera_.rotationSpeed = 0.0f;
		//camera_.movementSpeed = 7.5f;

		compute_.ubo.aspect_ratio = 1280.f / 720.f;// camera_.aspect;
		//compute_.ubo.lookat = glm::vec3(1.f, 1.f, 1.f);// testScript.vData[6];// camera_.rotation;
		//compute_.ubo.pos = camera_.position * -1.0f;
		compute_.ubo.fov = glm::tan(13.f * 0.03490658503f); //0.03490658503 = pi / 180 / 2
		compute_.ubo.rotM = glm::mat4();
		compute_.ubo.rand = static_cast<int>(random_int());
	}

	void ComputeRaytracer::CreateGraphicsPipeline()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
			vks::initializers::pipelineInputAssemblyStateCreateInfo(
				VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
				0,
				VK_FALSE);

		VkPipelineRasterizationStateCreateInfo rasterizationState =
			vks::initializers::pipelineRasterizationStateCreateInfo(
				VK_POLYGON_MODE_FILL,
				VK_CULL_MODE_FRONT_BIT,
				VK_FRONT_FACE_COUNTER_CLOCKWISE,
				0);

		VkPipelineColorBlendAttachmentState blendAttachmentState =
			vks::initializers::pipelineColorBlendAttachmentState(
				0xf,
				VK_FALSE);

		VkPipelineColorBlendStateCreateInfo colorBlendState =
			vks::initializers::pipelineColorBlendStateCreateInfo(
				1,
				&blendAttachmentState);

		VkPipelineDepthStencilStateCreateInfo depthStencilState =
			vks::initializers::pipelineDepthStencilStateCreateInfo(
				VK_FALSE,
				VK_FALSE,
				VK_COMPARE_OP_LESS_OR_EQUAL);

		VkPipelineViewportStateCreateInfo viewportState =
			vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

		VkPipelineMultisampleStateCreateInfo multisampleState =
			vks::initializers::pipelineMultisampleStateCreateInfo(
				VK_SAMPLE_COUNT_1_BIT,
				0);

		std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};
		VkPipelineDynamicStateCreateInfo dynamicState =
			vks::initializers::pipelineDynamicStateCreateInfo(
				dynamicStateEnables.data(),
				static_cast<uint32_t>(dynamicStateEnables.size()),
				0);
		auto vertShaderCode = readFile("../Assets/Shaders/texture.vert.spv");
		auto fragShaderCode = readFile("../Assets/Shaders/texture.frag.spv");

		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		vertShaderModule = vkDevice.createShaderModule(vertShaderCode);
		fragShaderModule = vkDevice.createShaderModule(fragShaderCode);

		//Create the structure for the vertex shader
		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

		VkGraphicsPipelineCreateInfo pipelineCreateInfo =
			vks::initializers::pipelineCreateInfo(
				graphics_.pipeline_layout,
				renderPass,
				0);

		VkPipelineVertexInputStateCreateInfo emptyInputState{};
		emptyInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		emptyInputState.vertexAttributeDescriptionCount = 0;
		emptyInputState.pVertexAttributeDescriptions = nullptr;
		emptyInputState.vertexBindingDescriptionCount = 0;
		emptyInputState.pVertexBindingDescriptions = nullptr;
		pipelineCreateInfo.pVertexInputState = &emptyInputState;

		pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
		pipelineCreateInfo.pRasterizationState = &rasterizationState;
		pipelineCreateInfo.pColorBlendState = &colorBlendState;
		pipelineCreateInfo.pMultisampleState = &multisampleState;
		pipelineCreateInfo.pViewportState = &viewportState;
		pipelineCreateInfo.pDepthStencilState = &depthStencilState;
		pipelineCreateInfo.pDynamicState = &dynamicState;
		pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.renderPass = renderPass;

		VK_CHECKRESULT(vkCreateGraphicsPipelines(vkDevice.logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &graphics_.pipeline), "CREATE GRAPHICS PIPELINE");

		//must be destroyed at the end of the object
		vkDestroyShaderModule(vkDevice.logicalDevice, fragShaderModule, nullptr);
		vkDestroyShaderModule(vkDevice.logicalDevice, vertShaderModule, nullptr);
	}

	void ComputeRaytracer::CreateDescriptorPool()
	{
		std::vector<VkDescriptorPoolSize> poolSizes =
		{
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),			// Compute UBO
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3 + MAX_TEXTURES),	// Graphics image samplers || +4 FOR TEXTURE
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),				// Storage image for ray traced image output
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 9),			// Storage buffer for the scene primitives
			//vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_BINDLESS_TEXTURES)
		};

		VkDescriptorPoolCreateInfo descriptorPoolInfo =
			vks::initializers::descriptorPoolCreateInfo(
				static_cast<uint32_t>(poolSizes.size()),
				poolSizes.data(),
				3);
		descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
		VK_CHECKRESULT(vkCreateDescriptorPool(vkDevice.logicalDevice, &descriptorPoolInfo, nullptr, &descriptor_pool_), "CREATE DESCRIPTOR POOL");

	}

	void ComputeRaytracer::CreateDescriptorSets()
	{
		VkDescriptorSetAllocateInfo allocInfo =
			vks::initializers::descriptorSetAllocateInfo(
				descriptor_pool_,
				&graphics_.descriptor_set_layout,
				1);

		VK_CHECKRESULT(vkAllocateDescriptorSets(vkDevice.logicalDevice, &allocInfo, &graphics_.descriptor_set), "ALLOCATE DESCRIPTOR SET");

		std::vector<VkWriteDescriptorSet> writeDescriptorSets =
		{
			// Binding 0 : Fragment shader texture sampler
			vks::initializers::writeDescriptorSet(
				graphics_.descriptor_set,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				0,
				&compute_texture_.descriptor)
		};

		vkUpdateDescriptorSets(vkDevice.logicalDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);

	}

	void ComputeRaytracer::CreateDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
		{
			// Binding 0 : Fragment shader image sampler
			vks::initializers::descriptorSetLayoutBinding(
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				0)
		};

		VkDescriptorSetLayoutCreateInfo descriptorLayout =
			vks::initializers::descriptorSetLayoutCreateInfo(
				setLayoutBindings.data(),
				static_cast<uint32_t>(setLayoutBindings.size()));

		VK_CHECKRESULT(vkCreateDescriptorSetLayout(vkDevice.logicalDevice, &descriptorLayout, nullptr, &graphics_.descriptor_set_layout), "CREATE DESCRIPTOR SET LAYOUT");

		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
			vks::initializers::pipelineLayoutCreateInfo(
				&graphics_.descriptor_set_layout,
				1);

		VK_CHECKRESULT(vkCreatePipelineLayout(vkDevice.logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &graphics_.pipeline_layout), "CREATE PIPELINE LAYOUT");

	}

	void ComputeRaytracer::CreateCommandBuffers(float swap_ratio, int32_t offset_width, int32_t offset_height)
	{
		commandBuffers.resize(swapChainFramebuffers.size());
		UpdateSwapScale(swap_ratio);
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //specifies if its a primary or secondary buffer
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		//@COMPUTEHERE be sure to have compute-specific command buffers too
		if (vkAllocateCommandBuffers(vkDevice.logicalDevice, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}


		for (size_t i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; //The cmdbuf will be rerecorded right after executing it 1s
			beginInfo.pInheritanceInfo = nullptr; // Optional //only for secondary buffers

			vkBeginCommandBuffer(commandBuffers[i], &beginInfo);
			// Image memory barrier to make sure that compute shader writes are finished before sampling from the texture
			VkImageMemoryBarrier imageMemoryBarrier = {};
			imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			imageMemoryBarrier.image = compute_texture_.image;
			imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			vkCmdPipelineBarrier(
				commandBuffers[i],
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &imageMemoryBarrier);

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass;
			renderPassInfo.framebuffer = swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { offset_width, offset_height }; //size of render area, should match size of attachments
			renderPassInfo.renderArea.extent = scaled_swap_;// swapChainExtent; //scaledSwap;//

			std::array<VkClearValue, 2> clearValues = {};
			clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; //derp
			clearValues[1].depthStencil = { 1.0f, 0 }; //1.0 = farplane, 0.0 = nearplane HELP

			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size()); //cuz
			renderPassInfo.pClearValues = clearValues.data(); //duh

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport = vks::initializers::viewport(static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f);
			vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

			VkRect2D scissor = vks::initializers::rect2D(swapChainExtent.width, swapChainExtent.height, 0, 0);
			vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);

			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_.pipeline_layout, 0, 1, &graphics_.descriptor_set, 0, NULL);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_.pipeline);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);
			vkCmdEndRenderPass(commandBuffers[i]);

			VK_CHECKRESULT(vkEndCommandBuffer(commandBuffers[i]), "END COMMAND BUFFER");
		}
	}

	void ComputeRaytracer::UpdateDescriptors()
	{
		vkWaitForFences(vkDevice.logicalDevice, 1, &compute_.fence, VK_TRUE, UINT64_MAX);

		compute_write_descriptor_sets_ =
		{
			// Binding 5: for objects
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				6,
				&compute_.storage_buffers.primitives.bufferInfo),
			//Binding 6 for materials
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				7,
				&compute_.storage_buffers.materials.bufferInfo),
			//Binding 7 for lights
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				8,
				&compute_.storage_buffers.lights.bufferInfo),
			//Binding 8 for gui
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				9,
				&compute_.storage_buffers.guis.bufferInfo),
			//Binding 10 for bvhnodes
			vks::initializers::writeDescriptorSet(
				compute_.descriptor_set,
				VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				10,
				&compute_.storage_buffers.bvh.bufferInfo)
		};
		vkUpdateDescriptorSets(vkDevice.logicalDevice, static_cast<uint32_t>(compute_write_descriptor_sets_.size()), compute_write_descriptor_sets_.data(), 0, NULL);
		//vkUpdateDescriptorSets(vkDevice.logicalDevice, compute_write_descriptor_sets_.size(), compute_write_descriptor_sets_.data(), 0, NULL);
		CreateComputeCommandBuffer();
	}

	void ComputeRaytracer::StartUp(artemis::World* world)
	{
		this->world = world;
		initVulkan();
		SetStuffUp();
		std::vector<rMaterial> copy = RESOURCEMANAGER.getMaterials();
		for (std::vector<rMaterial>::iterator itr = copy.begin(); itr != copy.end(); ++itr) {
			materials_.push_back(ssMaterial(itr->diffuse, itr->reflective, itr->roughness, itr->transparency, itr->refractiveIndex, itr->textureID));
			itr->renderedMat = &materials_.back();// materials_.end();
		}
		LoadResources();
	}

	void ComputeRaytracer::Initialize(artemis::ComponentMapper<RenderComponent>* render_mapper)
	{
		mapper_ = render_mapper;
		//renderMapper.init(*world);
		PrepareStorageBuffers();
		CreateUniformBuffers();
		PrepareTextureTarget(&compute_texture_, 1280, 720, VK_FORMAT_R8G8B8A8_UNORM);
		CreateDescriptorSetLayout();
		CreateGraphicsPipeline();
		CreateDescriptorPool();
		CreateDescriptorSets();
		PrepareCompute();
#ifdef UIIZON
		CreateCommandBuffers(0.733333333333f, (int32_t)(WINDOW.getWidth() * 0.16666666666f), 36);
#else
		CreateCommandBuffers(1.f, 0, 0);
#endif // UIIZON

		UpdateDescriptors();

		//setupUI();
		prepared_ = true;
	}


	/*
	* 1. Aquires Next Image
	* 2. Sets the image as the framebuffer's color attachment
	* 3. Sets the ImageAvailable and RenderFinished semaphores
	*/
	void ComputeRaytracer::StartFrame(uint32_t& image_index)
	{
		render_time_.start();

		//startDrawImGui();
		//endDrawImGui(image_index);
		VkResult result = vkAcquireNextImageKHR(vkDevice.logicalDevice, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &image_index);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			RecreateSwapChain();
			return;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		
		submit_info_.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info_.commandBufferCount = 1;
		submit_info_.pCommandBuffers = &commandBuffers[image_index];
		
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submit_info_.waitSemaphoreCount = 1;
		submit_info_.pWaitSemaphores = &imageAvailableSemaphore;// waitSemaphores;
		submit_info_.pWaitDstStageMask = waitStages;

		submit_info_.signalSemaphoreCount = 1;
		submit_info_.pSignalSemaphores = &renderFinishedSemaphore;
		VK_CHECKRESULT(vkQueueSubmit(graphicsQueue, 1, &submit_info_, VK_NULL_HANDLE), "GRAPHICS QUEUE SUBMIT");
	}

	// 1. Waits for that image based off hte image index
	// 2. Presents the image
	// 3. CPU waites for a compute fence
	// 4. Cpu submits a compute fence
	void ComputeRaytracer::EndFrame(const uint32_t& image_index)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = submit_info_.signalSemaphoreCount;
		presentInfo.pWaitSemaphores = submit_info_.pSignalSemaphores;//ui->visible ? &uiSemaphore : &renderFinishedSemaphore;// signalSemaphores;
		presentInfo.pResults = nullptr; //optional

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &image_index;

		VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}
		vkQueueWaitIdle(presentQueue);// sync with gpu

		//Possible compute here? image is in swapchain so maybe you can use image for compute stuff...
		vkWaitForFences(vkDevice.logicalDevice, 1, &compute_.fence, VK_TRUE, UINT64_MAX);
		vkResetFences(vkDevice.logicalDevice, 1, &compute_.fence);

		VkSubmitInfo compute_submit_info = {};
		compute_submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		compute_submit_info.commandBufferCount = 1;
		compute_submit_info.pCommandBuffers = &compute_.command_buffer;// &computeCommandBuffer;

		if (vkQueueSubmit(computeQueue, 1, &compute_submit_info, compute_.fence) != VK_SUCCESS)
			throw std::runtime_error("failed to submit compute command buffer!");

		render_time_.end();
		pINPUT.renderTime = render_time_.get_msecs();
	}

	void ComputeRaytracer::Added(artemis::Entity& e)
	{
		RenderType t = ((RenderComponent*)e.getComponent<RenderComponent>())->type;// renderMapper.get(e)->type;

		if (t & RENDER_MATERIAL) {

		}
		if (t & RENDER_PRIMITIVE) {
			PrimitiveComponent* primComp = (PrimitiveComponent*)e.getComponent<PrimitiveComponent>();
			//AABBComponent* aabb = (AABBComponent*)e.getComponent<AABBComponent>();
			MaterialComponent* mat = (MaterialComponent*)e.getComponent<MaterialComponent>();
			TransformComponent* trans = (TransformComponent*)e.getComponent<TransformComponent>();

			primComp->matId = mat->matID;
			primComp->world = trans->world;
			primComp->extents = trans->local.scale;
			if (primComp->id > 0) {
				std::pair<int, int> temp = mesh_assigner_[primComp->id];
				primComp->startIndex = temp.first;
				primComp->endIndex = temp.second;
			}

			SetRenderUpdate(RenderUpdate::kUpdateObject);
		}
		if (t & RENDER_LIGHT) {

			LightComponent* lightComp = (LightComponent*)e.getComponent<LightComponent>();
			TransformComponent* transComp = (TransformComponent*)e.getComponent<TransformComponent>();
			ssLight light;
			light.pos = transComp->global.position;
			light.color = lightComp->color;
			light.intensity = lightComp->intensity;
			light.id = e.getUniqueId();// lightComp->id;
			lightComp->id = light.id;
			lights_.push_back(light);
			light_comps_.push_back(lightComp);

			//NodeComponent* node = (NodeComponent*)e.getComponent<NodeComponent>();
			//addNode(node);

			compute_.storage_buffers.lights.UpdateAndExpandBuffers(vkDevice, lights_, lights_.size());
			//UpdateDescriptors();
		}
		if (t & RENDER_GUI) {
			GUIComponent* gc = (GUIComponent*)e.getComponent<GUIComponent>();
			ssGUI gui = ssGUI(gc->min, gc->extents, gc->alignMin, gc->alignExt, gc->layer, gc->id);
			gc->ref = static_cast<int>(guis_.size());
			gui.alpha = gc->alpha;
			guis_.push_back(gui);
			SetRenderUpdate(kUpdateGui);
		}
		if (t & RENDER_GUINUM) {
			GUINumberComponent* gnc = (GUINumberComponent*)e.getComponent<GUINumberComponent>();
			std::vector<int> nums = intToArrayOfInts(gnc->number);
			int num_size = static_cast<int>(nums.size());
			for (int i = 0; i < num_size; ++i) {
				ssGUI gui = ssGUI(gnc->min, gnc->extents, glm::vec2(0.1f * nums[i], 0.f), glm::vec2(0.1f, 1.f), 0, 0);
				gnc->shaderReferences.push_back(static_cast<int>(guis_.size()));
				gui.alpha = gnc->alpha;
				guis_.push_back(gui);
			}
			gnc->ref = gnc->shaderReferences[0];
			SetRenderUpdate(kUpdateGui);
		}
		if (t & RENDER_CAMERA) {
			CameraComponent* cam = (CameraComponent*)e.getComponent<CameraComponent>();
			TransformComponent* transComp = (TransformComponent*)e.getComponent<TransformComponent>();
			compute_.ubo.aspect_ratio = cam->aspectRatio;
			compute_.ubo.rotM = transComp->world;
			compute_.ubo.fov = cam->fov;
		}
	}

	void ComputeRaytracer::Removed(artemis::Entity& e)
	{
		RenderType t = ((RenderComponent*)e.getComponent<RenderComponent>())->type;// renderMapper.get(e)->type;

		if (t & RENDER_LIGHT) {
			if (lights_.size() == 1) {
				assert(lights_.at(0).id == light_comps_.at(0)->id);
				lights_.clear();
				light_comps_.clear();
			}
			else {
				auto* lc = (LightComponent*)e.getComponent<LightComponent>();
				std::erase_if(lights_, [lc](ssLight l) {return lc->id == l.id;});
				std::erase_if(light_comps_, [lc](LightComponent* l) {return lc->id == l->id; });
			}
		}
		/*else if (t == RENDER_GUINUM) {
			auto* gnc = (GUINumberComponent*)e.getComponent<GUINumberComponent>();
			//gnc->

		}*/

	}

	void ComputeRaytracer::ProcessEntity(artemis::Entity& e)
	{
		RenderType type = ((RenderComponent*)e.getComponent<RenderComponent>())->type;// renderMapper.get(e)->type;
		if (type == RENDER_NONE) return;
		switch (type)
		{
		case RENDER_MATERIAL:
			SetRenderUpdate(RenderUpdate::kUpdateMaterial);
			break;
		case RENDER_PRIMITIVE:
			SetRenderUpdate(RenderUpdate::kUpdateObject);
			break;
		case RENDER_LIGHT:
			SetRenderUpdate(RenderUpdate::kUpdateLight);
			break;
		case RENDER_GUI: {
			GUIComponent* gui = (GUIComponent*)e.getComponent<GUIComponent>();
			UpdateGui(gui);
			break; }
		case RENDER_GUINUM: {
			GUINumberComponent* gnc = (GUINumberComponent*)e.getComponent<GUINumberComponent>();
			//if (gnc->number > 9) {
			//	/*auto* nodular = (NodeComponent*)e.getComponent<NodeComponent>();
			//	std::cout << nodular->name + ": " << gnc->number;*/
			//}
			if (gnc->update) {
				gnc->update = false;
				UpdateGuiNumber(gnc);
			}
			break; }
		default:
			break;
		}
		type = RENDER_NONE;
	}

	void ComputeRaytracer::End()
	{
		UpdateBuffers();
		UpdateDescriptors();
		if (glfwWindowShouldClose(WINDOW.getWindow())) {
			world->setShutdown();
			vkDeviceWaitIdle(vkDevice.logicalDevice); //so it can destroy properly
		}
	}

	void ComputeRaytracer::CleanUp()
	{
		vkDeviceWaitIdle(vkDevice.logicalDevice);
		CleanUpSwapChain();

		DestroyCompute();

		vkDestroyDescriptorPool(vkDevice.logicalDevice, descriptor_pool_, nullptr);
		vkDestroyDescriptorSetLayout(vkDevice.logicalDevice, graphics_.descriptor_set_layout, nullptr);

		vkDestroyCommandPool(vkDevice.logicalDevice, commandPool, nullptr);
		//vkDestroyCommandPool(vkDevice.logicalDevice, compute_.commandPool, nullptr);

		RenderBase::cleanup();
	}

	void ComputeRaytracer::CleanUpSwapChain()
	{
		vkDestroyPipeline(vkDevice.logicalDevice, graphics_.pipeline, nullptr);
		//vkDestroyPipeline(vkDevice.logicalDevice, graphics_.raster.pipeline, nullptr);
		vkDestroyPipelineLayout(vkDevice.logicalDevice, graphics_.pipeline_layout, nullptr);
		//vkDestroyPipelineLayout(vkDevice.logicalDevice, graphics_.raster.pipeline_layout, nullptr);

		RenderBase::cleanupSwapChain();
	}

	void ComputeRaytracer::RecreateSwapChain()
	{	//WINDOW.resize();
		RenderBase::recreateSwapChain();
		CreateDescriptorSetLayout();
		CreateGraphicsPipeline();
		//editor ?
		CreateCommandBuffers(0.7333333333f, (int32_t)(WINDOW.getWidth() * 0.16666666666f), 36);
		//	createCommandBuffers(0.6666666666666f, 0, 0);
		swapChainFramebuffers;
		//return swapChainFramebuffers;
		//ui->visible = false;
		//ui->resize(swapChainExtent.width, swapChainExtent.height, swapChainFramebuffers);
	}

	void ComputeRaytracer::UpdateUniformBuffer()
	{
		compute_.uniform_buffer.ApplyChanges(vkDevice, compute_.ubo);
	}

	void ComputeRaytracer::LoadResources()
	{
		//get all the models and load err thang
		std::vector<ssVert> verts;
		std::vector<ssIndex> faces;
		std::vector<ssShape> shapes;
		std::vector<ssBVHNode> blas;

		const std::vector<rModel>& models = RESOURCEMANAGER.getModels();
		for (const rModel& mod : models)
		{
			for (int i = 0; i < static_cast<int>(mod.meshes.size()); ++i) {
				//map that connects the model with its index;
				rMesh rmesh = mod.meshes[i];

				//toss in the vertice data
				int prevVertSize = static_cast<int>(verts.size());
				int prevIndSize = static_cast<int>(faces.size());
				int prevBlasSize = static_cast<int>(blas.size());

				//load up ze vertices;
				verts.reserve(prevVertSize + rmesh.verts.size());
				for (std::vector<rVertex>::const_iterator itr = rmesh.verts.begin(); itr != rmesh.verts.end(); ++itr) {
					verts.emplace_back(ssVert(itr->pos / rmesh.extents, itr->norm, itr->uv.x, itr->uv.y));
				}

				//Load up da indices
				faces.reserve(prevIndSize + rmesh.faces.size());
				for (std::vector<glm::ivec4>::const_iterator itr = rmesh.faces.begin(); itr != rmesh.faces.end(); ++itr) {
					faces.emplace_back(ssIndex(*itr + prevVertSize));// , ++currId));
				}

				//Load up da bottom level acceleration structure
				blas.reserve(prevBlasSize + rmesh.bvh.size());
				for (auto itr : rmesh.bvh) {
					itr.numChildren > 0 ? itr.offset += prevIndSize : itr.offset += prevBlasSize;
					blas.emplace_back(itr);
				}

				//finish mia
				//meshAssigner[mod.uniqueID + i ] = std::pair<int, int>(prevBlasSize, blas.size());
				mesh_assigner_[mod.uniqueID + i] = std::pair<int, int>(prevIndSize, faces.size());
			}

		}

		shapes.push_back(ssShape(glm::vec3(0.f), glm::vec3(1.f), 1));
		compute_.storage_buffers.verts.InitStorageBufferWithStaging(vkDevice, verts, verts.size());
		compute_.storage_buffers.faces.InitStorageBufferWithStaging(vkDevice, faces, faces.size());
		compute_.storage_buffers.blas.InitStorageBufferWithStaging(vkDevice, blas, blas.size());
		compute_.storage_buffers.shapes.InitStorageBufferWithStaging(vkDevice, shapes, shapes.size());

		//compute_.storage_buffers.verts.InitStorageBufferCustomSize(vkDevice, verts, verts.size(), MAXVERTS);
		//compute_.storage_buffers.indices.InitStorageBufferCustomSize(vkDevice, indices, indices.size(), MAXINDS);
		//compute_.storage_buffers.meshes.InitStorageBufferCustomSize(vkDevice, meshes, meshes.size(), MAXMESHES);

		gui_textures_[0].path = "../Assets/Levels/1_Jungle/Textures/numbers.png";
		gui_textures_[0].CreateTexture(vkDevice);
		gui_textures_[1].path = "../Assets/Levels/1_Jungle/Textures/pause.png";
		gui_textures_[1].CreateTexture(vkDevice);
		gui_textures_[2].path = "../Assets/Levels/1_Jungle/Textures/circuit.jpg";
		gui_textures_[2].CreateTexture(vkDevice);
		gui_textures_[3].path = "../Assets/Levels/1_Jungle/Textures/ARROW.png";
		gui_textures_[3].CreateTexture(vkDevice);
		gui_textures_[4].path = "../Assets/Levels/1_Jungle/Textures/debugr.png";
		gui_textures_[4].CreateTexture(vkDevice);

		bindless_textures.push_back(Texture("../Assets/Levels/1_Jungle/Textures/numbers.png", vkDevice));
		bindless_textures.push_back(Texture("../Assets/Levels/1_Jungle/Textures/title.png", vkDevice));
		bindless_textures.push_back(Texture("../Assets/Levels/1_Jungle/Textures/pause.png", vkDevice));
		bindless_textures.push_back(Texture("../Assets/Levels/1_Jungle/Textures/ARROW.png", vkDevice));
		bindless_textures.push_back(Texture("../Assets/Levels/1_Jungle/Textures/debugr.png", vkDevice));
		bindless_textures.push_back(Texture("../Assets/Levels/1_Jungle/Textures/circuit.jpg", vkDevice));
		NUM_BINDLESS_TEXTURES = static_cast<int>(bindless_textures.size());
	}

	void ComputeRaytracer::AddMaterial(glm::vec3 diff, float rfl, float rough, float trans, float ri)
	{
		ssMaterial mat = ssMaterial(diff, rfl, rough, trans, ri, 0);
		materials_.push_back(mat);
		compute_.storage_buffers.materials.UpdateAndExpandBuffers(vkDevice, materials_, materials_.size());
		UpdateDescriptors();
	}

	void ComputeRaytracer::AddNode(NodeComponent* node)
	{
		if (node->engineFlags & COMPONENT_MODEL) {
			return;
		}
		if (node->engineFlags & COMPONENT_LIGHT) {
			return;
			LightComponent* lightComp = (LightComponent*)node->data->getComponent<LightComponent>();
			TransformComponent* transComp = (TransformComponent*)node->data->getComponent<TransformComponent>();
			ssLight light;
			light.pos = transComp->global.position;
			light.color = lightComp->color;
			light.intensity = lightComp->intensity;
			light.id = lightComp->id;

			lights_.push_back(light);
			light_comps_.push_back(lightComp);

			compute_.storage_buffers.lights.UpdateAndExpandBuffers(vkDevice, lights_, lights_.size());
			UpdateDescriptors();
		}
		if (node->engineFlags & COMPONENT_CAMERA) {
			CameraComponent* cam = (CameraComponent*)node->data->getComponent<CameraComponent>();
			TransformComponent* transComp = (TransformComponent*)node->data->getComponent<TransformComponent>();
			compute_.ubo.aspect_ratio = cam->aspectRatio;
			compute_.ubo.rotM = transComp->world;
			compute_.ubo.fov = cam->fov;

			//if (rasterize) {
			//	graphics_.camera.view = transComp->world;
			//	camera_.fov = cam->fov;
			//	camera_.updateaspect_ratio(cam->aspect_ratio);
			//}
		}
	}

	void ComputeRaytracer::UpdateMaterial(int id)
	{
		rMaterial* m = &RESOURCEMANAGER.getMaterial(id);

		materials_[id].diffuse = m->diffuse;
		materials_[id].reflective = m->reflective;
		materials_[id].roughness = m->roughness;
		materials_[id].transparency = m->transparency;
		materials_[id].refractiveIndex = m->refractiveIndex;
		materials_[id].textureID = m->textureID;

		compute_.storage_buffers.materials.UpdateBuffers(vkDevice, materials_);
	}

	void ComputeRaytracer::UpdateGui(GUIComponent* gc)
	{
		ssGUI& g = guis_[gc->ref];
		g.min = gc->min;
		g.extents = gc->extents;
		g.alignMin = gc->alignMin;
		g.alignExt = gc->alignExt;
		g.layer = gc->layer;
		g.id = gc->id;
		g.alpha = gc->alpha;
		SetRenderUpdate(kUpdateGui);
	}

	void ComputeRaytracer::AddGuiNumber(GUINumberComponent* gnc)
	{
		std::vector<int> nums = intToArrayOfInts(gnc->number);
		for (int i = 0; i < nums.size(); ++i) {
			ssGUI gui = ssGUI(gnc->min, gnc->extents, glm::vec2(0.1f * nums[i], 0.f), glm::vec2(0.1f, 1.f), 0, 0);
			gnc->shaderReferences.push_back(static_cast<int>(guis_.size()));
			gui.alpha = gnc->alpha;
			guis_.push_back(gui);
		}
		gnc->ref = gnc->shaderReferences[0];
		SetRenderUpdate(kUpdateGui);
	}

	void ComputeRaytracer::UpdateGuiNumber(GUINumberComponent* gnc)
	{
		std::vector<int> nums = intToArrayOfInts(gnc->number);
		int num_size = static_cast<int>(nums.size());
		bool change_occured = num_size != gnc->highest_active_digit_index + 1;
		if (change_occured == false) {
			for (int i = 0; i < gnc->highest_active_digit_index + 1; ++i) {
				guis_[gnc->shaderReferences[i]].alignMin = glm::vec2(0.1f * nums[i], 0.f);
				guis_[gnc->shaderReferences[i]].alpha = gnc->alpha;
			}
		}
		else {
 			bool increased = num_size > gnc->highest_active_digit_index + 1;
			if (increased) {
				bool needs_shader_ref = num_size > gnc->shaderReferences.size();
				if (needs_shader_ref) {
					for (int i = num_size - static_cast<int>(gnc->shaderReferences.size()); i > 0; --i) {
						ssGUI gui = ssGUI(gnc->min, gnc->extents, glm::vec2(0.1f * nums[num_size - 1], 0.f), glm::vec2(0.1f, 1.f), 0, 0);
						gnc->shaderReferences.push_back(static_cast<int>(guis_.size()));
						guis_.push_back(gui);
					}
					compute_.storage_buffers.guis.UpdateAndExpandBuffers(vkDevice, guis_, static_cast<int>(guis_.size()));
				}
				for (int i = 0; i < num_size; ++i) {
					guis_[gnc->shaderReferences[i]].alignMin = glm::vec2(0.1f * nums[i], 0.f);
					guis_[gnc->shaderReferences[i]].alpha = gnc->alpha;
					guis_[gnc->shaderReferences[i]].min.x = gnc->min.x - ((num_size - 1 - i) * gnc->extents.x);
				}
				gnc->highest_active_digit_index = num_size - 1;
			}
			else { //decreased
				for (int i = 0; i < num_size; ++i) {
					guis_[gnc->shaderReferences[i]].alignMin = glm::vec2(0.1f * nums[i], 0.f);
					guis_[gnc->shaderReferences[i]].alpha = gnc->alpha;
					guis_[gnc->shaderReferences[i]].min.x = gnc->min.x - ((num_size - 1 -  i) * gnc->extents.x);
				}
				for (int i = gnc->highest_active_digit_index; i > num_size - 1; --i) {
					guis_[gnc->shaderReferences[i]].alpha = 0;
				}
				gnc->highest_active_digit_index = num_size - 1;
			}
		}

		//if (nums.size() < gnc->highest_active_digit_index + 1) {
		//	for (int i = gnc->highest_active_digit_index; i > nums.size() - 1; --i) {
		//		guis_[i].alpha = 0.f;
		//	}
		//	for (int i = 0; i < nums.size(); ++i) {
		//		guis_[gnc->shaderReferences[i]].alignMin = glm::vec2(0.1f * nums[i], 0.f);
		//		guis_[gnc->shaderReferences[i]].alpha = gnc->alpha;
		//	}
		//	gnc->highest_active_digit_index = nums.size() - 1;
		//}

		////if (nums.size() < gnc->shaderReferences.size()) { //aka it went from like... 10 to 9
		////	for (int i = 0; i < nums.size(); ++i) {
		////		guis_[gnc->shaderReferences[i]].alignMin = glm::vec2(0.1f * nums[i], 0.f);
		////		guis_[gnc->shaderReferences[i]].alpha = gnc->alpha;
		////	}

		////	//Shift everything to the left and when you get to the last one... delete the ref? this works ONLY for raytracy bird
		////	/*auto ref = gnc->shaderReferences[0];
		////	for (int i = gnc->shaderReferences.size(); i > 0; --i) {
		////		gnc->shaderReferences[i - 1] = gnc->shaderReferences[i];
		////	}
		////	guis_[ref].alpha = 0;
		////	gnc->shaderReferences.pop_back();*/
		////}
		//else {
		//	if (nums.size() > gnc->shaderReferences.size()) { //aka it went from like 9 to 10
		//  //First create the resource
		//		ssGUI gui = ssGUI(gnc->min + glm::vec2(gnc->extents.x, 0.f), gnc->extents, glm::vec2(0.1f * nums[nums.size() - 1], 0.f), glm::vec2(0.1f, 1.f), 0, 0);

		//		//This expands the array of references and shifts everything to the right
		//		gnc->shaderReferences.push_back(0);
		//		for (int i = 0; i < gnc->shaderReferences.size() - 1; ++i) {
		//			gnc->shaderReferences[i + 1] = gnc->shaderReferences[i];
		//		}
		//		gnc->shaderReferences[0] = guis_.size();
		//		gnc->highest_active_digit_index++;

		//		// Now place it in the shader structs and update
		//		guis_.push_back(gui);
		//		compute_.storage_buffers.guis.UpdateAndExpandBuffers(vkDevice, guis_, guis_.size());
		//	}
		//}


		SetRenderUpdate(kUpdateGui);
	}
}
