#include "../pch.h"
#include "renderSystem.h"
#include <set>
#include "../Utility/resourceManager.h"
#include "../Utility/Input.h"
//#include "../Utility/octree.hpp"
namespace Principia {



	RenderSystem::RenderSystem()
	{
		addComponentType<RenderComponent>();
	}

	RenderSystem::~RenderSystem()
	{
	}

	void RenderSystem::preInit()
	{
		renderer_ = new ComputeRaytracer();
		renderer_->StartUp(world);
	}

	void RenderSystem::initialize()
	{
		render_mapper_.init(*world);
		renderer_->Initialize(&render_mapper_);
	}

	void RenderSystem::startFrame(uint32_t& imageIndex)
	{
		renderer_->StartFrame(imageIndex);
	}

	void RenderSystem::endFrame(const uint32_t& imageIndex)
	{
		renderer_->EndFrame(imageIndex);
	}

	void RenderSystem::processEntity(artemis::Entity& e)
	{
		renderer_->ProcessEntity(e);
	}

	void RenderSystem::added(artemis::Entity& e)
	{
		renderer_->Added(e);
	}

	void RenderSystem::removed(artemis::Entity& e)
	{
		renderer_->Removed(e);
	}

	void RenderSystem::end()
	{
		renderer_->End();
	}
}