///
/// \file ConstantBuffer.h
/// \brief Defines ConstantBuffer type.
///

#pragma once
/*Constant buffer Copyright (C) by Mike Murrell 
iirc this was basically done in frank luna's 
intro to dx11 book, but now its vulkanized
*/

#include "vulkanbase.h"

namespace Principia {

	template<typename T>
	class VBuffer
	{
	public:
		VBuffer() :
			mInitialized(false)
		{
		}

		~VBuffer()
		{
		}

		T Data;

		VkBuffer* Buffer()const
		{
			return buffer;
		}
		VkDeviceMemory Memory()const {
			return bufferMemory;
		}
		VkDescriptorBufferInfo Descriptor() const {
			return bufferInfo;
		}


		void Destroy(VulkanDevice& vkDevice)
		{
			vkDestroyBuffer(vkDevice.logicalDevice, buffer, nullptr);
			vkFreeMemory(vkDevice.logicalDevice, bufferMemory, nullptr);
		}

		void Initialize(VulkanDevice& vkDevice)
		{
			VkDeviceSize bufferSize = sizeof(Data);
			vkDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				buffer, bufferMemory);


			mInitialized = true;

			bufferInfo.buffer = buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(T);
		}

		void Initialize(VulkanDevice& vkDevice, size_t mul, VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties) {

			VkDeviceSize bufferSize = sizeof(Data) * mul;
			vkDevice.createBuffer(bufferSize, usage, properties, buffer, bufferMemory);
			mInitialized = true;

			bufferInfo.buffer = buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = bufferSize;// sizeof(T);
		}

		void InitStorageBuffer(VulkanDevice& vkDevice, std::vector<T> objects, size_t mul,
			VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {

			VkDeviceSize bufferSize = sizeof(Data) * mul;
			vkDevice.createBuffer(bufferSize, usage, properties, buffer, bufferMemory);

			void* data;
			vkMapMemory(vkDevice.logicalDevice, bufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, objects.data(), (size_t)bufferSize);
			vkUnmapMemory(vkDevice.logicalDevice, bufferMemory);

			mInitialized = true;

			bufferInfo.buffer = buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = bufferSize;// sizeof(T);
		}
		void InitStorageBufferCustomSize(VulkanDevice& vkDevice, std::vector<T> objects, size_t mul, size_t max,
			VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {

			VkDeviceSize bufferSize = sizeof(Data) * mul;
			VkDeviceSize maxBufferSize = sizeof(Data) * max;
			vkDevice.createBuffer(maxBufferSize, usage, properties, buffer, bufferMemory);

			void* data;
			vkMapMemory(vkDevice.logicalDevice, bufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, objects.data(), (size_t)bufferSize);
			vkUnmapMemory(vkDevice.logicalDevice, bufferMemory);

			mInitialized = true;

			bufferInfo.buffer = buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = bufferSize;// sizeof(T);
		}

		void InitStorageBufferWithStaging(VulkanDevice& vkDevice, std::vector<T> objects, size_t mul,
			VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT) {
			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;
			VkDeviceSize bufferSize = sizeof(Data) * mul;
			void* data;

			vkDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				stagingBuffer, stagingBufferMemory);

			vkMapMemory(vkDevice.logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
			memcpy(data, objects.data(), (size_t)bufferSize);// (size_t)planes.size() * sizeof(Plane));
			vkUnmapMemory(vkDevice.logicalDevice, stagingBufferMemory);

			vkDevice.createBuffer(bufferSize, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);
			vkDevice.copyBuffer(stagingBuffer, buffer, bufferSize);
			vkDestroyBuffer(vkDevice.logicalDevice, stagingBuffer, nullptr);
			vkFreeMemory(vkDevice.logicalDevice, stagingBufferMemory, nullptr);

			mInitialized = true;

			bufferInfo.buffer = buffer;
			bufferInfo.offset = 0;
			bufferInfo.range = bufferSize;// sizeof(T);
		}

		void UpdateBuffers(VulkanDevice& device, std::vector<T> object) {
			if (mInitialized) {
				void* data;
				vkMapMemory(device.logicalDevice, bufferMemory, 0, (size_t)bufferInfo.range, 0, &data);
				memcpy(data, object.data(), (size_t)bufferInfo.range);
				vkUnmapMemory(device.logicalDevice, bufferMemory);
			}
		}
		void UpdateAndExpandBuffers(VulkanDevice& device, std::vector<T> object, size_t size) {
			bufferInfo.range = sizeof(Data) * size;
			if (mInitialized) {
				void* data;
				vkMapMemory(device.logicalDevice, bufferMemory, 0, (size_t)bufferInfo.range, 0, &data);
				memcpy(data, object.data(), (size_t)bufferInfo.range);
				vkUnmapMemory(device.logicalDevice, bufferMemory);
			}
		}
		void ApplyChanges(VulkanDevice& vkDevice)
		{
			if (mInitialized) {
				void* data;
				vkMapMemory(vkDevice.logicalDevice, bufferMemory, 0, sizeof(Data), 0, &data);
				memcpy(data, &Data, sizeof(T));
				vkUnmapMemory(vkDevice.logicalDevice, bufferMemory);
			}
		}
		void ApplyChanges(VulkanDevice& vkDevice, T dat) {
			if (mInitialized) {
				void* data;
				vkMapMemory(vkDevice.logicalDevice, bufferMemory, 0, sizeof(dat), 0, &data);
				memcpy(data, &dat, sizeof(dat));
				vkUnmapMemory(vkDevice.logicalDevice, bufferMemory);
			}
		}

	private:
		VBuffer(const VBuffer<T>& rhs);
		VBuffer<T>& operator=(const VBuffer<T>& rhs) {};

	public:
		bool mInitialized = false;

		VkBuffer buffer = {};
		VkDeviceMemory bufferMemory = {};
		VkDescriptorBufferInfo bufferInfo = {};
	};

}