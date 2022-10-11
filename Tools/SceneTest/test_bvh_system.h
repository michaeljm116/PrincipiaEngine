#pragma once

//#include <Artemis/EntityProcessingSystem.h>
//#include <Artemis/ComponentMapper.h>
//#include <Utility/transformComponent.hpp>
//#include <Rendering/Components/renderComponents.hpp>
//
//namespace test {
//	class Sys_BVH : public artemis::EntityProcessingSystem {
//	public:
//		Sys_BVH();
//		~Sys_BVH();
//
//		void initialize() override;
//		void build();
//		void processEntity(artemis::Entity& e) override;
//		void begin() override;
//		void end() override;
//	private:
//		artemis::ComponentMapper<Principia::TransformComponent> transMapper;
//		artemis::ComponentMapper<Principia::PrimitiveComponent> primMapper;
//
//		std::vector<artemis::Entity*> prims;
//		
//	};
//}