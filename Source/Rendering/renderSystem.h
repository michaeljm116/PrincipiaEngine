#pragma once
/* Render System Copyright (C) by Mike Murrell 2017
everything here is so convoluted and rushed it needs
much rework but brotha aint got time fo dat
*/


#include "compute-raytracer.h"

#include "shaderStructures.hpp"
#include "../Utility/window.h"
#include "../Utility/bvhComponent.hpp"
#include "../Utility/timer.hpp"

#include "../Utility/componentIncludes.h"
#include <unordered_map>

namespace Principia {

	class RenderSystem : public artemis::EntityProcessingSystem
	{
	public:
		RenderSystem();
		~RenderSystem();
		void preInit();
		void initialize();
		void startFrame(uint32_t& imageIndex);
		void endFrame(const uint32_t& imageIndex);
		void processEntity(artemis::Entity &e);

		void added(artemis::Entity &e) override;
		void removed(artemis::Entity &e) override;
		void end() override;

		virtual void TogglePlayMode(bool play) { ((ComputeRaytracer*)renderer_)->TogglePlayMode(play); }

		//Other common functions
		//This could probably all be inlined, although private renderer hmm
		//maybe change them all to getrenderer()->dothething();
		Renderer* getRenderer() { return renderer_; }
		ComputeRaytracer* getComputeRaytracer() { return (ComputeRaytracer*)renderer_; }
		void setRenderUpdate(Renderer::RenderUpdate flags) {renderer_->SetRenderUpdate(flags);}
		void updateObjectMemory() {renderer_->UpdateDescriptors();}
		void addNode(NodeComponent* node) {renderer_->AddNode(node);}
		void addMaterial(glm::vec3 diff, float rfl, float rough, float trans, float ri) {renderer_->AddMaterial(diff, rfl, rough, trans, ri);}
		void updateMaterial(int id) { renderer_->UpdateMaterial(id); }
		void updateCamera(CameraComponent* cc) { renderer_->UpdateCamera(cc); }

		//Compute Raytracer functions
		void updateGui(GUIComponent* gui) {((ComputeRaytracer*)renderer_)->UpdateGui(gui);}		
		void updateGuiNumber(GUINumberComponent* gnc) { ((ComputeRaytracer*)renderer_)->UpdateGuiNumber(gnc); }
		
		ssLight& getLight(int id) { return ((ComputeRaytracer*)renderer_)->getLight(id); };


	private:

		artemis::ComponentMapper<RenderComponent> render_mapper_;
		Renderer* renderer_;

	};

}