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

#include "../Game/script.hpp"
#include "../Utility/timer.h"
#include "../Utility/window.h"

#include "../Utility/componentIncludes.h"


namespace Principia {

	class Renderer : public RenderBase {
	public:
		Renderer();
		virtual ~Renderer();

		virtual void StartUp() = 0;
		virtual void Initialize() = 0;
		virtual void StartFrame(uint32_t& image_index) = 0;
		virtual void EndFrame(const uint32_t& image_index) = 0;

		virtual void CleanUp() { RenderBase::cleanup(); }
		virtual void CleanUpSwapChain() { RenderBase::cleanupSwapChain(); }
		virtual void RecreateSwapChain() { RenderBase::recreateSwapChain(); }


		const std::vector<VkFramebuffer> GetFrameBuffers() const { return swapChainFramebuffers; }
		const VkExtent2D GetSwapChainExtent() const { return swapChainExtent; }
		const VkDeviceInfo GetDeviceInfo() const { return dev_info_; }
		const VkSubmitInfo GetSubmitInfo() const { return submit_info_; }
		const float GetRenderTime() const { return render_time_.ms; };
		Camera& GetCamera() { return camera_; }


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
			scaled_swap_.width = swapChainExtent.width * f;
			scaled_swap_.height = swapChainExtent.height * f;
		}
		virtual void UpdateCamera(CameraComponent* c) = 0;

	protected:
		VkDeviceInfo dev_info_ = {};
		VkSubmitInfo submit_info_ = {};
		Timer render_time_ = Timer("Renderer");
		VkExtent2D scaled_swap_;
		Camera camera_;
		
	};
}