#pragma once
/*  
	Renderer Copywright (C) by Mike Murrell 2022
	Base class for the renderer
	what you iz finna do is....
	make the render system HAVE
	a renderer, instead of BE 1
*/


#include "rendermanagers.h"
#include "renderbase.h"
#include "shaderStructures.hpp"

#include "../Utility/timer.hpp"
#include "../Utility/window.h"

#include "../Utility/componentIncludes.h"


namespace Principia {
	static const int MAX_TEXTURES = 5;
	static int NUM_BINDLESS_TEXTURES = 6;

	class Renderer : public RenderBase {
	public:
		Renderer() {};
		virtual ~Renderer() {};

		virtual void StartUp(artemis::World* world) = 0;
		virtual void Initialize(artemis::ComponentMapper<RenderComponent>* render_mapper) = 0;
		virtual void StartFrame(uint32_t& image_index) = 0;
		virtual void EndFrame(const uint32_t& image_index) = 0;

		virtual void Added(artemis::Entity& e) = 0;
		virtual void Removed(artemis::Entity& e) = 0;
		virtual void ProcessEntity(artemis::Entity& e) = 0;
		virtual void End() = 0;

		virtual void CleanUp() = 0;// { RenderBase::cleanup(); }
		virtual void CleanUpSwapChain() = 0;// { RenderBase::cleanupSwapChain(); }
		virtual void RecreateSwapChain() = 0;// { RenderBase::recreateSwapChain(); }

		virtual void AddMaterial(glm::vec3 diff, float rfl, float rough, float trans, float ri) = 0;
		virtual void AddNode(NodeComponent* node) = 0;
		virtual void UpdateDescriptors() = 0;
		virtual void UpdateMaterial(int id) = 0;


		const std::vector<VkFramebuffer> GetFrameBuffers() const { return swapChainFramebuffers; }
		const VkExtent2D GetSwapChainExtent() const { return swapChainExtent; }
		VkDeviceInfo& GetDeviceInfo() { return dev_info_; }
		const float GetRenderTime() const { return render_time_.get_msecs(); }
		VkSubmitInfo& GetSubmitInfo() { return submit_info_; }
		Camera& GetCamera() { return camera_; }

		enum RenderUpdate {
			kUpdateBox = 0x01,
			kUpdateSphere = 0x02,
			kUpdatePlane = 0x04,
			kUpdateCylinder = 0x08,
			kUpdateCone = 0x10,
			kUpdateMesh = 0x20,
			kUpdateMaterial = 0x40,
			kUpdateNone = 0x80,
			kUpdateObject = 0x100,
			kUpdateLight = 0x200,
			kUpdateGui = 0x400,
			kUpdateBvh = 0x800,
		};

		int32_t update_flags_ = kUpdateNone;
		void SetRenderUpdate(RenderUpdate ru) {
			update_flags_ |= ru;
			if (update_flags_ & kUpdateNone)
				update_flags_ &= ~kUpdateNone;
		}

		void UpdateDeviceInfo() {
			dev_info_.device = &vkDevice;
			dev_info_.copyQueue = graphicsQueue;
			dev_info_.framebuffers = swapChainFramebuffers;
			dev_info_.colorFormat = swapChainImageFormat;
			dev_info_.depthFormat = findDepthFormat();
			dev_info_.width = WINDOW.getWidth();
			dev_info_.height = WINDOW.getHeight();
		}
		void UpdateSwapScale(float f) {
			scaled_swap_.width = static_cast<uint32_t>((float)swapChainExtent.width * f);
			scaled_swap_.height = static_cast<uint32_t>((float)swapChainExtent.height * f);
		}
		virtual void UpdateCamera(CameraComponent* c) = 0;
		virtual void TogglePlayMode(bool b) = 0;

	protected:
		VkDeviceInfo dev_info_ = {};
		VkSubmitInfo submit_info_ = {};
		Timer render_time_ = Timer("Renderer");
		VkExtent2D scaled_swap_ = {};
		Camera camera_ = {};
		artemis::ComponentMapper<RenderComponent>* mapper_ = nullptr;
		artemis::World* world = nullptr;
	};
}