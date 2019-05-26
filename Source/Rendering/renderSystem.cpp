#include "../pch.h"
#include "renderSystem.h"
#include <set>
#include "../Utility/componentIncludes.h"
#include "../Utility/resourceManager.h"
#include "../Utility/Input.h"
//#include "../Utility/octree.hpp"

static int currId = 0;	// Id used to identify objects by the ray tracing shader
static const int MAXMATERIALS	= 256;
static const int MAXMESHES		= 256;
static const int MAXVERTS		= 256;
static const int MAXINDS		= 256;
static const int MAXOBJS		= 256;
static const int MAXLIGHTS		= 256;
static const int MAXGUIS		= 256;

// Setup and fill the compute shader storage buffers containing primitives for the raytraced scene
void RenderSystem::prepareStorageBuffers()
{
	objects.reserve(MAXOBJS);
	//verts.reserve(MAXVERTS);
	//indices.reserve(MAXINDS);
	//meshes.reserve(MAXMESHES);
	materials.reserve(MAXMATERIALS);
	lights.reserve(MAXLIGHTS);


	//THESE SHOULD BE STAGED MEOW
	//compute.storageBuffers.verts.InitStorageBufferWithStaging(vkDevice, verts, verts.size());
	//compute.storageBuffers.indices.InitStorageBufferWithStaging(vkDevice, indices, indices.size());
	//compute.storageBuffers.meshes.InitStorageBufferWithStaging(vkDevice, meshes, meshes.size());

	//compute.storageBuffers.verts.InitStorageBufferCustomSize(vkDevice, verts, verts.size(), MAXVERTS);
	//compute.storageBuffers.indices.InitStorageBufferCustomSize(vkDevice, indices, indices.size(), MAXINDS);
	//compute.storageBuffers.meshes.InitStorageBufferCustomSize(vkDevice, meshes, meshes.size(), MAXMESHES);

	//these are changable 
	compute.storageBuffers.objects.InitStorageBufferCustomSize(vkDevice, objects, objects.size(), MAXOBJS);
	compute.storageBuffers.materials.InitStorageBufferCustomSize(vkDevice, materials, materials.size(), MAXMATERIALS);
	compute.storageBuffers.lights.InitStorageBufferCustomSize(vkDevice, lights, lights.size(), MAXLIGHTS);


	//create 1 gui main global kind of gui for like title/menu screen etc...
	GUIComponent* guiComp = (GUIComponent*)world->getSingleton()->getComponent<GUIComponent>();
	ssGUI gui = ssGUI(guiComp->min, guiComp->extents, guiComp->alignMin, guiComp->alignExt, guiComp->layer, guiComp->id);
	gui.visible = guiComp->visible;

	//Give the component a reference to it and initialize
	guiComp->ref = guis.size();
	guis.push_back(gui);
	compute.storageBuffers.guis.InitStorageBufferCustomSize(vkDevice, guis, guis.size(), MAXGUIS);
}

// Prepare a texture target that is used to store compute shader calculations
void RenderSystem::prepareTextureTarget(Texture *tex, uint32_t width, uint32_t height, VkFormat format)
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


void RenderSystem::updateUniformBuffer() {
	compute.uniformBuffer.ApplyChanges(vkDevice, compute.ubo);
}

void RenderSystem::processEntity(artemis::Entity & e)
{
	RenderType type = renderMapper.get(e)->type;
	if (type == RENDER_NONE) return;
	switch (type)
	{
	case RENDER_MATERIAL:
		setRenderUpdate(RenderUpdate::UPDATE_MATERIAL);
		break;
	case RENDER_PRIMITIVE:
		setRenderUpdate(RenderUpdate::UPDATE_OBJECT);
		break;
	case RENDER_LIGHT:
		setRenderUpdate(RenderUpdate::UPDATE_LIGHT);
		break;
	case RENDER_GUI: {
		GUIComponent* gui = (GUIComponent*)e.getComponent<GUIComponent>();
		updateGui(gui);
		break; }
	case RENDER_GUINUM:{
		GUINumberComponent* gnc = (GUINumberComponent*)e.getComponent<GUINumberComponent>();
		updateGuiNumber(gnc);
		break;}
	default:
		break;
	}
	type = RENDER_NONE;
}

void RenderSystem::end()
{
	updateBuffers();
	mainLoop();
	//updateBuffers();
}

void RenderSystem::loadResources()
{
	//get all the models and load err thang
	std::vector<ssVert> verts;
	std::vector<ssIndex> faces;
	std::vector<ssMesh> meshes;

	std::vector<rModel>& models = RESOURCEMANAGER.getModels();
	for each (rModel mod in models)
	{
		for (size_t i = 0; i < mod.meshes.size(); ++i) {
			//map that connects the model with its index;
			rMesh rmesh = mod.meshes[i];
			MeshIdAssigner mia;
			mia.uniqueID = mod.uniqueID + i;

			//toss in the vertice data
			int prevVertSize = verts.size();
			int prevIndSize = faces.size();

			ssMesh mesh;
			mesh.startVert = prevVertSize;
			mesh.endVert = mesh.startVert + rmesh.verts.size() - 1;
			mesh.startIndex = prevIndSize;
			mesh.center = glm::vec4(rmesh.center, 1.f);
			mesh.extents = glm::vec4(rmesh.extents, 1.f);

			//////////////////////////////////////THISshould be reserve+emplacedbackinstead/////////////////////////////////////
			//load up ze vertices;
			for (std::vector<rVertex>::const_iterator itr = rmesh.verts.begin(); itr != rmesh.verts.end(); ++itr)
				verts.push_back(ssVert(itr->pos, itr->norm, itr->uv.x, itr->uv.y));
			//Load up da indices
			for (std::vector<glm::ivec4>::const_iterator itr = rmesh.faces.begin(); itr != rmesh.faces.end(); ++itr) {
				faces.push_back(ssIndex(*itr + prevVertSize));// , ++currId));
			}

			//finish loading the data
			mesh.endIndex = faces.size();
			meshes.push_back(mesh);

			//finish mia
			mia.index = meshes.size() - 1;
			mia.center = rmesh.center;
			mia.extents = rmesh.extents;
			miaList.push_back(mia);
		}
	}

	compute.storageBuffers.verts.InitStorageBufferWithStaging(vkDevice, verts, verts.size());
	compute.storageBuffers.faces.InitStorageBufferWithStaging(vkDevice, faces, faces.size());
	compute.storageBuffers.meshes.InitStorageBufferWithStaging(vkDevice, meshes, meshes.size());

	//compute.storageBuffers.verts.InitStorageBufferCustomSize(vkDevice, verts, verts.size(), MAXVERTS);
	//compute.storageBuffers.indices.InitStorageBufferCustomSize(vkDevice, indices, indices.size(), MAXINDS);
	//compute.storageBuffers.meshes.InitStorageBufferCustomSize(vkDevice, meshes, meshes.size(), MAXMESHES);

	guiTextures[0].path = "../Assets/Levels/Pong/Textures/numbers.png";
	guiTextures[0].CreateTexture(vkDevice);
	guiTextures[1].path = "../Assets/Levels/Pong/Textures/title.png";
	guiTextures[1].CreateTexture(vkDevice);
	guiTextures[2].path = "../Assets/Levels/Pong/Textures/menu.png";
	guiTextures[2].CreateTexture(vkDevice);
	guiTextures[3].path = "../Assets/Levels/Pong/Textures/pause.png";
	guiTextures[3].CreateTexture(vkDevice);

}

void RenderSystem::addLight(artemis::Entity & e)
{
	NodeComponent* node = (NodeComponent*)e.getComponent<NodeComponent>();
	addNode(node);

	compute.storageBuffers.lights.UpdateAndExpandBuffers(vkDevice, lights, lights.size());
	updateDescriptors();
}

void RenderSystem::addCamera(artemis::Entity & e)
{
	//so fo da cam cam wat u finta do is....
	CameraComponent* comp = (CameraComponent*)e.getComponent<CameraComponent>();
	comp->fov = compute.ubo.fov;
	comp->lookat = compute.ubo.lookat;
	//comp->pos = &compute.ubo.pos;
	comp->pos = m_Cam.position;
	
	//compute.ubo.pos = (TransformComponent*)e.getComponent<TransformComponent>()
}

void RenderSystem::addMaterial(glm::vec3 diff, float rfl, float rough, float trans, float ri)
{
	ssMaterial mat = ssMaterial(diff, rfl, rough, trans, ri );
	materials.push_back(mat);
	compute.storageBuffers.materials.UpdateAndExpandBuffers(vkDevice, materials, materials.size());
	updateDescriptors();
}

void RenderSystem::addNodes(std::vector<NodeComponent*> nodes) {
	for (int i = 0; i < nodes.size(); ++i) {
		addNode(nodes[i]);
		if (nodes[i]->children.size() > 0) {
			addNodes(nodes[i]->children);
		}
	}
}

void RenderSystem::addNode(NodeComponent* node) {
	if (node->flags & COMPONENT_MODEL) {
		ssPrimitive prim;
		PrimitiveComponent* primComp = (PrimitiveComponent*)node->data->getComponent<PrimitiveComponent>();
		TransformComponent* trans = (TransformComponent*)node->data->getComponent<TransformComponent>();
		prim.world = trans->world;
		prim.extents = trans->local.scale;
		rModel& mod = RESOURCEMANAGER.getModelU(primComp->uniqueID);
		prim.numChildren = mod.meshes.size();

		prim.id = 0;
		primComp->objIndex = objects.size(); 
		objects.push_back(prim);
		objectComps.push_back(primComp);

		return;
	}
	if (node->flags & COMPONENT_PRIMITIVE) {
		//start constructing the object;
		ssPrimitive object;
		PrimitiveComponent* objComp = (PrimitiveComponent*)node->data->getComponent<PrimitiveComponent>();
		//AABBComponent* aabb = (AABBComponent*)node->data->getComponent<AABBComponent>();
		MaterialComponent* mat = (MaterialComponent*)node->data->getComponent<MaterialComponent>();
		TransformComponent* trans = (TransformComponent*)node->data->getComponent<TransformComponent>();

		object.world = trans->world;
		//object.center = trans->world[3];// aabb->center;
		object.extents = trans->local.scale;// aabb->extents;
		object.matId = mat->matID;

		//set up the ids
		//this is O(n) SL0W y not hash?
		if (objComp->uniqueID > 0) {
			for (size_t i = 0; i < miaList.size(); ++i) {
				if (objComp->uniqueID == miaList[i].uniqueID) {
					object.id = miaList[i].index;
					objComp->gpuIndex = object.id;
					objComp->center = miaList[i].center;
					objComp->extents = miaList[i].extents;
				}
			}
		}
		else
			object.id = objComp->uniqueID;

		//put into list
		objComp->objIndex = objects.size();
		objects.push_back(object);
		objectComps.push_back(objComp);

		
		//updateObjectMemory();
		//setRenderUpdate(RenderUpdate::UPDATE_OBJECT);
	}

	if (node->flags & COMPONENT_LIGHT) {
		LightComponent* lightComp = (LightComponent*)node->data->getComponent<LightComponent>();
		TransformComponent* transComp = (TransformComponent*)node->data->getComponent<TransformComponent>();
		ssLight light;
		light.pos = transComp->global.position;
		light.color = lightComp->color;
		light.intensity = lightComp->intensity;
		light.id = lightComp->id;

		lights.push_back(light);
		lightComps.push_back(lightComp);
	}
	if (node->flags & COMPONENT_CAMERA) {
		CameraComponent* cam = (CameraComponent*)node->data->getComponent<CameraComponent>();
		TransformComponent* transComp = (TransformComponent*)node->data->getComponent<TransformComponent>();
		compute.ubo.lookat = cam->lookat;
		compute.ubo.pos = transComp->global.position;
		compute.ubo.fov = cam->fov;
	}
}

void RenderSystem::updateGui(GUIComponent * gc)
{
	ssGUI& g = guis[gc->ref];
	g.min = gc->min;
	g.extents = gc->extents;
	g.alignMin = gc->alignMin;
	g.alignExt = gc->alignExt;
	g.layer = gc->layer;
	g.id = gc->id;
	g.visible = gc->visible;
	setRenderUpdate(UPDATE_GUI);
}

void RenderSystem::addGuiNumber(GUINumberComponent * gnc)
{
	std::vector<int> nums = intToArrayOfInts(gnc->number);
	for (int i = 0; i < nums.size(); ++i) {
		ssGUI gui = ssGUI(gnc->min, gnc->extents, glm::vec2(0.1f * nums[i], 0.f), glm::vec2(0.1f, 1.f), 0, 0);
		gnc->shaderReferences.push_back(guis.size());
		gui.visible = gnc->visible;
		guis.push_back(gui);
	}
	gnc->ref = gnc->shaderReferences[0];
	setRenderUpdate(UPDATE_GUI);
}

void RenderSystem::updateGuiNumber(GUINumberComponent * gnc)
{
	std::vector<int> nums = intToArrayOfInts(gnc->number);
	for (int i = 0; i < gnc->shaderReferences.size(); ++i) {
		guis[gnc->shaderReferences[i]].alignMin = glm::vec2(0.1f * nums[i], 0.f);
		guis[gnc->shaderReferences[i]].visible = gnc->visible;
	}
	if (nums.size() > gnc->shaderReferences.size()) { //aka it went from like 9 to 10
		ssGUI gui = ssGUI(gnc->min + glm::vec2(gnc->extents.x, 0.f), gnc->extents, glm::vec2(0.1f * nums[nums.size() - 1], 0.f), glm::vec2(0.1f, 1.f), 0, 0);
		gnc->shaderReferences.push_back(guis.size());
		guis.push_back(gui);
		compute.storageBuffers.guis.UpdateAndExpandBuffers(vkDevice, guis, guis.size());
	}
	setRenderUpdate(UPDATE_GUI);
}

void RenderSystem::deleteMesh(NodeComponent * node)
{
//	//Find the lowest and highest verts/inds/subs in the mesh
//	int startVert = INT32_MAX, startIndex = INT32_MAX, startMesh = INT32_MAX;
//	int endVert = INT32_MIN, endIndex = INT32_MIN, endMesh = INT32_MIN;
//	for each (NodeComponent* child in node->children)
//	{
//		MeshComponent* meshComp = (MeshComponent*)child->data->getComponent<MeshComponent>();
//		ssMesh& mesh = meshes[meshComp->meshIndex];
//
//		startVert = minVal(startVert, mesh.startVert);
//		startIndex = minVal(startIndex, mesh.startIndex);
//		startMesh = minVal(startMesh, meshComp->meshIndex);
//
//		endVert = maxVal(endVert, mesh.endVert);
//		endIndex = maxVal(endIndex, mesh.endIndex);
//		endMesh = maxVal(endMesh, meshComp->meshIndex);
//	}
//
//	//Get the differences between them
//	int diffVert = endVert - startVert + 1;
//	int diffIndex = endIndex - startIndex;
//	int diffMesh = endMesh - startMesh + 1;
//
//	//delete their info
//	verts.erase(verts.begin() + startVert, verts.begin() + endVert + 1);
//	indices.erase(indices.begin() + startIndex, indices.begin() + endIndex);
//	meshes.erase(meshes.begin() + startMesh, meshes.begin() + endMesh + 1);
//	meshComps.erase(meshComps.begin() + startMesh, meshComps.begin() + endMesh + 1);
//
//	//Update everything to the left of them in memory;
//	for (int i = startMesh; i < meshes.size(); ++i) {
//		meshes[i].startVert -= diffVert;// -1;
//		meshes[i].endVert -= diffVert;// -1;
//		meshes[i].startIndex -= diffIndex;// -1;
//		meshes[i].endIndex -= diffIndex;// -1;
//
//		meshComps[i]->meshIndex -= diffMesh;// +1;
//	}
//	glm::ivec3 diffVerts = glm::ivec3(diffVert);
//#pragma omp parallel for
//	for(int i = startIndex; i < indices.size(); ++i)
//		indices[i].v -= diffVerts;
//
//	//tell the gpu what u dun did y0
//	updateMeshMemory();
}
void RenderSystem::deleteNode(NodeComponent * node)
{
	/*
	if (node->flags & COMPONENT_MESH) {
		MeshComponent* meshComp = (MeshComponent*)node->data->getComponent<MeshComponent>();
		ssMesh& mesh = meshes[meshComp->meshIndex];

		//get the differences between them
		int diffVert = mesh.endVert - mesh.startVert + 1;
		int diffIndex = mesh.endIndex - mesh.startIndex;

		//delete their info
		verts.erase(verts.begin() + mesh.startVert, verts.begin() + mesh.endVert + 1);
		indices.erase(indices.begin() + mesh.startIndex, indices.begin() + mesh.endIndex);
		meshes.erase(meshes.begin() + meshComp->meshIndex);
		meshComps.erase(meshComps.begin() + meshComp->meshIndex);

		//Update everything to the left of them in memory;
		for (int i = meshComp->meshIndex; i < meshes.size(); ++i) {
			meshes[i].startVert -= diffVert;// -1;
			meshes[i].endVert -= diffVert;// -1;
			meshes[i].startIndex -= diffIndex;// -1;
			meshes[i].endIndex -= diffIndex;// -1;

			meshComps[i]->meshIndex--;// -= diffmesh;// +1;
		}
		glm::ivec3 diffVerts = glm::ivec3(diffVert);
#pragma omp parallel for
		for (int i = mesh.startIndex; i < indices.size(); ++i)
			indices[i].v -= diffVerts;

		//tell the gpu what u dun did y0
		updateMeshMemory();
	}
	if (node->flags & COMPONENT_SPHERE) {
		SphereComponent* s = (SphereComponent*)node->data->getComponent<SphereComponent>();
		spheres.erase(spheres.begin() + s->sphereIndex);
		sphereComps.erase(sphereComps.begin() + s->sphereIndex);
		for (int i = s->sphereIndex; i < spheres.size(); ++i)
			sphereComps[i]->sphereIndex--;
		compute.storageBuffers.spheres.UpdateAndExpandBuffers(vkDevice, spheres, spheres.size());
		updateDescriptors();
	}
	if (node->flags & COMPONENT_BOX) {
		BoxComponent* b = (BoxComponent*)node->data->getComponent<BoxComponent>();
		boxes.erase(boxes.begin() + b->boxIndex);
		BoxComps.erase(BoxComps.begin() + b->boxIndex);
		for (int i = b->boxIndex; i < boxes.size(); ++i)
			BoxComps[i]->boxIndex--;
		compute.storageBuffers.boxes.UpdateAndExpandBuffers(vkDevice, boxes, boxes.size());
		updateDescriptors();
	}
	if (node->flags & COMPONENT_CYLINDER) {
		CylinderComponent* c = (CylinderComponent*)node->data->getComponent<CylinderComponent>();
		cylinders.erase(cylinders.begin() + c->cylinderIndex);
		cylinderComps.erase(cylinderComps.begin() + c->cylinderIndex);
		for (int i = c->cylinderIndex; i < cylinders.size(); ++i)
			cylinderComps[i]->cylinderIndex--;
		compute.storageBuffers.cylinders.UpdateAndExpandBuffers(vkDevice, cylinders, cylinders.size());
		updateDescriptors();
	}
	if (node->flags & COMPONENT_PLANE) {
		PlaneComponent* p = (PlaneComponent*)node->data->getComponent <PlaneComponent>();
		planes.erase(planes.begin() + p->planeIndex);
		PlaneComps.erase(PlaneComps.begin() + p->planeIndex);
		for (int i = p->planeIndex; i < planes.size(); ++i)
			PlaneComps[i]->planeIndex--;
		compute.storageBuffers.planes.UpdateAndExpandBuffers(vkDevice, planes, planes.size());
		updateDescriptors();
	}*/

	if (node->flags & COMPONENT_PRIMITIVE) {
		PrimitiveComponent* o = (PrimitiveComponent*)node->data->getComponent<PrimitiveComponent>();
		objects.erase(objects.begin() + o->objIndex);
		objectComps.erase(objectComps.begin() + o->objIndex);
		for (int i = o->objIndex; i < objects.size(); ++i)
			objectComps[i]->objIndex--;
		compute.storageBuffers.objects.UpdateAndExpandBuffers(vkDevice, objects, objects.size());
		updateDescriptors();
	}
	
}


void RenderSystem::updateObjectMemory()
{
	compute.storageBuffers.objects.UpdateAndExpandBuffers(vkDevice, objects, objects.size());
	updateDescriptors();
}

void RenderSystem::updateMeshMemory() {
	//compute.storageBuffers.meshes.UpdateAndExpandBuffers(vkDevice, meshes, meshes.size());
	//compute.storageBuffers.verts.UpdateAndExpandBuffers(vkDevice, verts, verts.size());
	//compute.storageBuffers.indices.UpdateAndExpandBuffers(vkDevice, indices, indices.size());

	//updateDescriptors();
}

void RenderSystem::updateGeometryMemory(ObjectType type)
{
	//switch (type)
	//{
	//case ObjectType::Sphere:{
	//	compute.storageBuffers.spheres.UpdateAndExpandBuffers(vkDevice, spheres, spheres.size());
	//	break;}
	//case ObjectType::Box:{
	//	compute.storageBuffers.boxes.UpdateAndExpandBuffers(vkDevice, boxes, boxes.size());
	//	break;}
	//case ObjectType::Plane: {
	//	compute.storageBuffers.planes.UpdateAndExpandBuffers(vkDevice, planes, planes.size());
	//	break;}
	//case ObjectType::Cylinder: {
	//	compute.storageBuffers.cylinders.UpdateAndExpandBuffers(vkDevice, cylinders, cylinders.size());
	//	break;}
	//default: {
	//	break; }
	//}
	//updateDescriptors();
}


void RenderSystem::updateBuffers()
{
	if (updateflags & UPDATE_NONE)
		return;
	if (updateflags & UPDATE_OBJECT) {
		compute.storageBuffers.objects.UpdateBuffers(vkDevice, objects);
		updateflags &= ~UPDATE_OBJECT;
	}
	if (updateflags & UPDATE_MATERIAL) {
		updateflags &= ~UPDATE_MATERIAL;
	}
	if (updateflags & UPDATE_LIGHT) {
		compute.storageBuffers.lights.UpdateBuffers(vkDevice, lights);
		updateflags &= ~UPDATE_LIGHT;
	}
	if (updateflags & UPDATE_GUI) {
		compute.storageBuffers.guis.UpdateBuffers(vkDevice, guis);
		updateflags &= ~UPDATE_GUI;
	}

	updateflags |= UPDATE_NONE;
	updateDescriptors();
}

void RenderSystem::updateCamera(CameraComponent* c) {
	compute.ubo.lookat = c->lookat;
	//compute.ubo.fov = c->fov;
	compute.ubo.pos = c->pos;
	compute.uniformBuffer.ApplyChanges(vkDevice, compute.ubo);

	//updateUniformBuffer();
}
//void RenderSystem::updateLight(LightComponent* l) {
//	compute.ubo.lightPos = l->pos;
//	compute.uniformBuffer.ApplyChanges(vkDevice, compute.ubo);
//
//	//updateUniformBuffer();
//}

void RenderSystem::SetStuffUp()
{
	m_Cam.type = Camera::CameraType::lookat;
	m_Cam.setPerspective(60.0f, 1280.f / 720.f, 0.1f, 1256.0f);
	m_Cam.setRotation(testScript.vData[6]);// glm::vec3(5.0f, 90.0f, 0.0f));
	m_Cam.setTranslation(glm::vec3(0.0f, 0.0f, -4.0f));
	m_Cam.rotationSpeed = 0.0f;
	m_Cam.movementSpeed = 7.5f;

	compute.ubo.aspectRatio = m_Cam.aspect;
	compute.ubo.lookat = glm::vec3(0.f, 90.f, 0.f);// testScript.vData[6];// m_Cam.rotation;
	//compute.ubo.pos = m_Cam.position * -1.0f;
	compute.ubo.fov = glm::tan(m_Cam.fov * 0.03490658503); //0.03490658503 = pi / 180 / 2
}


#pragma region Startup
RenderSystem::RenderSystem()
{
	addComponentType<RenderComponent>();
	updateflags = UPDATE_NONE;
	//m_RenderTime = LocalTimer("Render Time: ");
}
RenderSystem::~RenderSystem()
{
}


void RenderSystem::preInit()
{
	testScript.loadScript("Game/testscript.xml");
	std::vector<rMaterial> copy = RESOURCEMANAGER.getMaterials();
	for (std::vector<rMaterial>::iterator itr = copy.begin(); itr != copy.end(); ++itr) {
		materials.push_back(ssMaterial(itr->diffuse, itr->reflective, itr->roughness, itr->transparency, itr->refractiveIndex));
		itr->renderedMat = &materials.back();// materials.end();
	}

	initVulkan();
	SetStuffUp();

	loadResources();
}

void RenderSystem::initialize() {

	renderMapper.init(*world);
	prepareStorageBuffers();
	createUniformBuffers();
	prepareTextureTarget(&computeTexture, 1920, 1080, VK_FORMAT_R8G8B8A8_UNORM);
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createDescriptorPool();
	createDescriptorSets();
	prepareCompute();
	createCommandBuffers(0.6666666666666f, (int32_t)(WINDOW.getWidth() * 0.16666666666f), 36);
	setupUI();
	prepared = true;
	
	//glfwMaximizeWindow(WINDOW.getWindow());
	updateDescriptors();
}
void RenderSystem::mainLoop() {
	//Keeps the app running until an error occurs
	//Or the window is closed
	//while (!glfwWindowShouldClose(WINDOW.getWindow())) {
	//	glfwPollEvents();
		updateBuffers();
		//INPUT.update();
		if (!prepared)
			return;
		drawFrame();
		//updateUniformBuffer();
		//if (INPUT.playToggled)
		//	togglePlayMode();
		if(ui->visible)
			ui->updateOverlay();
	//}
	if(glfwWindowShouldClose(WINDOW.getWindow()))
		vkDeviceWaitIdle(vkDevice.logicalDevice); //so it can destroy properly
}
void RenderSystem::drawFrame() {//1.get img frm swapc 2.do da cmdbuf wit da image 3.put it back in da swapc
	//Timer timer("Rendering: ");
	//Timer timer("Render time: ");

	m_RenderTime.Start();
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(vkDevice.logicalDevice, swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acquire swap chain image!");
	}
	
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
	//VK_CHECKRESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE), "GRAPHICS QUEUE SUBMIT");
	
	//VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &imageAvailableSemaphore;// waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderFinishedSemaphore;

	VK_CHECKRESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE), "GRAPHICS QUEUE SUBMIT");

	if (ui->visible) {
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &renderFinishedSemaphore;
		// Signal ready with UI overlay complete semaphpre
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &uiSemaphore;

		// Submit current UI overlay command buffer
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &ui->cmdBuffers[imageIndex];
		VK_CHECKRESULT(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE), "SUBMIT UI STUFF");
	}

	//submitUI(submitInfo, imageIndex);
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = ui->visible ? &uiSemaphore : &renderFinishedSemaphore;// signalSemaphores;
	presentInfo.pResults = nullptr; //optional

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}
	vkQueueWaitIdle(presentQueue);// sync with gpu

	//Possible compute here? image is in swapchain so maybe you can use image for compute stuff...
	vkWaitForFences(vkDevice.logicalDevice, 1, &compute.fence, VK_TRUE, UINT64_MAX);
	vkResetFences(vkDevice.logicalDevice, 1, &compute.fence);

	VkSubmitInfo computeSubmitInfo = {};
	computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	computeSubmitInfo.commandBufferCount = 1;
	computeSubmitInfo.pCommandBuffers = &compute.commandBuffer;// &computeCommandBuffer;

	if (vkQueueSubmit(computeQueue, 1, &computeSubmitInfo, compute.fence) != VK_SUCCESS)
		throw std::runtime_error("failed to submit compute commadn buffer!");
	m_RenderTime.End();
	INPUT.renderTime = m_RenderTime.ms;
}
void RenderSystem::cleanup() {
	vkDeviceWaitIdle(vkDevice.logicalDevice);
	cleanupSwapChain();

	ui->CleanUp();
	destroyCompute();

	vkDestroyDescriptorPool(vkDevice.logicalDevice, descriptorPool, nullptr);
	vkDestroyDescriptorSetLayout(vkDevice.logicalDevice, graphics.descriptorSetLayout, nullptr);
	
	vkDestroyCommandPool(vkDevice.logicalDevice, commandPool, nullptr);
	//vkDestroyCommandPool(vkDevice.logicalDevice, compute.commandPool, nullptr);
	

	RenderBase::cleanup();
}
void RenderSystem::cleanupSwapChain() {
	vkDestroyPipeline(vkDevice.logicalDevice, graphics.pipeline, nullptr);
	vkDestroyPipelineLayout(vkDevice.logicalDevice, graphics.pipelineLayout, nullptr);

	RenderBase::cleanupSwapChain();
}
void RenderSystem::recreateSwapChain()
{
	WINDOW.resize();
	RenderBase::recreateSwapChain();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createCommandBuffers(0.6666666666666f, (int32_t)(WINDOW.getWidth() * 0.16666666666f), 36);
	ui->visible = false;
	ui->resize(swapChainExtent.width, swapChainExtent.height, swapChainFramebuffers);
}

#pragma endregion

#pragma region CREATION FUNCTIONS

void RenderSystem::buildBVH()
{
	topLevelBVH.build(SplitMethod::Middle, TreeType::Recursive, objectComps, objects);
	int a = 4;
}

void RenderSystem::createGraphicsPipeline() {
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
			dynamicStateEnables.size(),
			0);

	auto vertShaderCode = readFile("Rendering/Shaders/texture.vert.spv");
	auto fragShaderCode = readFile("Rendering/Shaders/texture.frag.spv");

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

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
			graphics.pipelineLayout,
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
	pipelineCreateInfo.stageCount = shaderStages.size();
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.renderPass = renderPass;

	VK_CHECKRESULT(vkCreateGraphicsPipelines(vkDevice.logicalDevice, pipelineCache, 1, &pipelineCreateInfo, nullptr, &graphics.pipeline), "CREATE GRAPHICS PIPELINE");

	//must be destroyed at the end of the object
	vkDestroyShaderModule(vkDevice.logicalDevice, fragShaderModule, nullptr);
	vkDestroyShaderModule(vkDevice.logicalDevice, vertShaderModule, nullptr);
}

void RenderSystem::createCommandBuffers(float swapratio, int32_t offsetWidth, int32_t offsetHeight) {
	commandBuffers.resize(swapChainFramebuffers.size());
	swapRatio(swapratio);
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
		imageMemoryBarrier.image = computeTexture.image;
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
		renderPassInfo.renderArea.offset = { offsetWidth, offsetHeight }; //size of render area, should match size of attachments
		renderPassInfo.renderArea.extent = scaledSwap;// swapChainExtent; //scaledSwap;//

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f }; //derp
		clearValues[1].depthStencil = { 1.0f, 0 }; //1.0 = farplane, 0.0 = nearplane HELP

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size()); //cuz
		renderPassInfo.pClearValues = clearValues.data(); //duh

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = vks::initializers::viewport(swapChainExtent.width, swapChainExtent.height, 0.0f, 1.0f);
		vkCmdSetViewport(commandBuffers[i], 0, 1, &viewport);

		VkRect2D scissor = vks::initializers::rect2D(swapChainExtent.width, swapChainExtent.height, 0, 0);
		vkCmdSetScissor(commandBuffers[i], 0, 1, &scissor);
		
		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipelineLayout, 0, 1, &graphics.descriptorSet, 0, NULL);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics.pipeline);
		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]);

		VK_CHECKRESULT(vkEndCommandBuffer(commandBuffers[i]), "END COMMAND BUFFER");
	}
}
void RenderSystem::updateDescriptors()
{
	vkWaitForFences(vkDevice.logicalDevice, 1, &compute.fence, VK_TRUE, UINT64_MAX);

	computeWriteDescriptorSets =
	{
		//// Binding 0: Output storage image
		//vks::initializers::writeDescriptorSet(
		//	compute.descriptorSet,
		//	VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		//	0,
		//	&computeTexture.descriptor),
		//// Binding 1: Uniform buffer block
		//vks::initializers::writeDescriptorSet(
		//	compute.descriptorSet,
		//	VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		//	1,
		//	&compute.uniformBuffer.Descriptor()),
		//// Binding 2: Shader storage buffer for the verts
		//vks::initializers::writeDescriptorSet(
		//	compute.descriptorSet,
		//	VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		//	2,
		//	&compute.storageBuffers.verts.Descriptor()),
		//// Binding 3: Shader storage buffer for the indices
		//vks::initializers::writeDescriptorSet(
		//	compute.descriptorSet,
		//	VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		//	3,
		//	&compute.storageBuffers.indices.Descriptor()),
		//// Binding 4: Shader storage buffer for the meshes
		//vks::initializers::writeDescriptorSet(
		//	compute.descriptorSet,
		//	VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		//	4,
		//	&compute.storageBuffers.meshes.Descriptor()),
		// Binding 5: for objects
		vks::initializers::writeDescriptorSet(
			compute.descriptorSet,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			5,
			&compute.storageBuffers.objects.Descriptor()),

		//Binding 6 for materials
		vks::initializers::writeDescriptorSet(
			compute.descriptorSet,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			6,
			&compute.storageBuffers.materials.Descriptor()),
		//Binding 7 for lights
		vks::initializers::writeDescriptorSet(
			compute.descriptorSet,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			7,
			&compute.storageBuffers.lights.Descriptor()),
		//Binding 8 for gui
		vks::initializers::writeDescriptorSet(
			compute.descriptorSet,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			8,
			&compute.storageBuffers.guis.Descriptor())
	};
	vkUpdateDescriptorSets(vkDevice.logicalDevice, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);
	//vkUpdateDescriptorSets(vkDevice.logicalDevice, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);
	createComputeCommandBuffer();
}
void RenderSystem::createDescriptorPool() {
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),			// Compute UBO
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 7),	// Graphics image samplers || +4 FOR TEXTURE
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1),				// Storage image for ray traced image output
		vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 7),			// Storage buffer for the scene primitives
		//vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo =
		vks::initializers::descriptorPoolCreateInfo(
			poolSizes.size(),
			poolSizes.data(),
			3);

	VK_CHECKRESULT(vkCreateDescriptorPool(vkDevice.logicalDevice, &descriptorPoolInfo, nullptr, &descriptorPool), "CREATE DESCRIPTOR POOL");

}
void RenderSystem::createDescriptorSets() {
	VkDescriptorSetAllocateInfo allocInfo =
		vks::initializers::descriptorSetAllocateInfo(
			descriptorPool,
			&graphics.descriptorSetLayout,
			1);

	VK_CHECKRESULT(vkAllocateDescriptorSets(vkDevice.logicalDevice, &allocInfo, &graphics.descriptorSet), "ALLOCATE DESCRIPTOR SET");

	std::vector<VkWriteDescriptorSet> writeDescriptorSets =
	{
		// Binding 0 : Fragment shader texture sampler
		vks::initializers::writeDescriptorSet(
			graphics.descriptorSet,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			0,
			&computeTexture.descriptor)
	};

	vkUpdateDescriptorSets(vkDevice.logicalDevice, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);

}
void RenderSystem::createDescriptorSetLayout() {

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
			setLayoutBindings.size());

	VK_CHECKRESULT(vkCreateDescriptorSetLayout(vkDevice.logicalDevice, &descriptorLayout, nullptr, &graphics.descriptorSetLayout), "CREATE DESCRIPTOR SET LAYOUT");

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
		vks::initializers::pipelineLayoutCreateInfo(
			&graphics.descriptorSetLayout,
			1);

	VK_CHECKRESULT(vkCreatePipelineLayout(vkDevice.logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &graphics.pipelineLayout), "CREATE PIPELINE LAYOUT");
}

void RenderSystem::createUniformBuffers()
{
	// Compute shader parameter uniform buffer block
	compute.uniformBuffer.Initialize(vkDevice, 1,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	compute.uniformBuffer.ApplyChanges(vkDevice, compute.ubo);
	//updateUniformBuffer();
}
#pragma endregion

#pragma region COMPUTE YO
void RenderSystem::createComputeCommandBuffer()
{
	VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

	VK_CHECKRESULT(vkBeginCommandBuffer(compute.commandBuffer, &cmdBufInfo), "CREATE COMPUTE COMMAND BUFFER");

	vkCmdBindPipeline(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipeline);
	vkCmdBindDescriptorSets(compute.commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, compute.pipelineLayout, 0, 1, &compute.descriptorSet, 0, 0);

	vkCmdDispatch(compute.commandBuffer, computeTexture.width / 16, computeTexture.height / 16, 1);

	vkEndCommandBuffer(compute.commandBuffer);
}

void RenderSystem::prepareCompute()
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
	vkGetDeviceQueue(vkDevice.logicalDevice, vkDevice.qFams.computeFamily, 0, &compute.queue);

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
		// Binding 4: Shader storage buffer for the meshes
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			VK_SHADER_STAGE_COMPUTE_BIT,
			4),
		// Binding 5: Shader storage buffer for the materials
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			VK_SHADER_STAGE_COMPUTE_BIT,
			5),
		// Binding 6: Shader storage buffer for the objects
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			VK_SHADER_STAGE_COMPUTE_BIT,
			6),
		// Binding 6: Shader storage buffer for the lights
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			VK_SHADER_STAGE_COMPUTE_BIT,
			7),
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			VK_SHADER_STAGE_COMPUTE_BIT,
			8),
		// Binding 8: the textures
		vks::initializers::descriptorSetLayoutBinding(
			VK_DESCRIPTOR_TYPE_SAMPLER,
			VK_SHADER_STAGE_COMPUTE_BIT,
			9, 4)
	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout =
		vks::initializers::descriptorSetLayoutCreateInfo(
			setLayoutBindings.data(),
			setLayoutBindings.size());

	VK_CHECKRESULT(vkCreateDescriptorSetLayout(vkDevice.logicalDevice, &descriptorLayout, nullptr, &compute.descriptorSetLayout), "CREATE COMPUTE DSL");

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
		vks::initializers::pipelineLayoutCreateInfo(
			&compute.descriptorSetLayout,
			1);

	VK_CHECKRESULT(vkCreatePipelineLayout(vkDevice.logicalDevice, &pPipelineLayoutCreateInfo, nullptr, &compute.pipelineLayout), "CREATECOMPUTE PIEPLINEEEE");

	VkDescriptorSetAllocateInfo allocInfo =
		vks::initializers::descriptorSetAllocateInfo(
			descriptorPool,
			&compute.descriptorSetLayout,
			1);

	VK_CHECKRESULT(vkAllocateDescriptorSets(vkDevice.logicalDevice, &allocInfo, &compute.descriptorSet), "ALLOCATE DOMPUTE DSET");

	VkDescriptorImageInfo textureimageinfos[MAXTEXTURES] = { guiTextures[0].descriptor, guiTextures[1].descriptor, guiTextures[2].descriptor, guiTextures[3].descriptor };
	computeWriteDescriptorSets =
	{
		// Binding 0: Output storage image
		vks::initializers::writeDescriptorSet(
			compute.descriptorSet,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			0,
			&computeTexture.descriptor),
		// Binding 1: Uniform buffer block
		vks::initializers::writeDescriptorSet(
			compute.descriptorSet,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			&compute.uniformBuffer.Descriptor()),
		// Binding 2: Shader storage buffer for the verts
		vks::initializers::writeDescriptorSet(
			compute.descriptorSet,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			2,
			&compute.storageBuffers.verts.Descriptor()),
		// Binding 3: Shader storage buffer for the indices
		vks::initializers::writeDescriptorSet(
			compute.descriptorSet,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			3,
			&compute.storageBuffers.faces.Descriptor()),
		// Binding 4: Shader storage buffer for the meshes
		vks::initializers::writeDescriptorSet(
			compute.descriptorSet,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			4,
			&compute.storageBuffers.meshes.Descriptor()),
		// Binding 5: for objectss
		vks::initializers::writeDescriptorSet(
			compute.descriptorSet,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			5,
			&compute.storageBuffers.objects.Descriptor()),

		//Binding 6 for materials
		vks::initializers::writeDescriptorSet(
			compute.descriptorSet,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			6,
			&compute.storageBuffers.materials.Descriptor()),
		//Binding 7 for lights
		vks::initializers::writeDescriptorSet(
			compute.descriptorSet,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			7,
			&compute.storageBuffers.lights.Descriptor()),
		//Binding 8 for guis
		vks::initializers::writeDescriptorSet(
			compute.descriptorSet,
			VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			8,
			&compute.storageBuffers.lights.Descriptor()),
		//bINDING 8 FOR TEXTURES
		vks::initializers::writeDescriptorSet(
			compute.descriptorSet, 
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			9, 
			textureimageinfos, MAXTEXTURES)
	};

	vkUpdateDescriptorSets(vkDevice.logicalDevice, computeWriteDescriptorSets.size(), computeWriteDescriptorSets.data(), 0, NULL);

	// Create compute shader pipelines
	VkComputePipelineCreateInfo computePipelineCreateInfo =
		vks::initializers::computePipelineCreateInfo(
			compute.pipelineLayout,
			0);

	computePipelineCreateInfo.stage = vkDevice.createShader("Rendering/Shaders/raytracing.comp.spv", VK_SHADER_STAGE_COMPUTE_BIT);
	VK_CHECKRESULT(vkCreateComputePipelines(vkDevice.logicalDevice, pipelineCache, 1, &computePipelineCreateInfo, nullptr, &compute.pipeline), "CREATE COMPUTE PIPELINE");

	// Separate command pool as queue family for compute may be different than graphics
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = vkDevice.qFams.computeFamily;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECKRESULT(vkCreateCommandPool(vkDevice.logicalDevice, &cmdPoolInfo, nullptr, &compute.commandPool), "CREATE COMMAND POOL");

	// Create a command buffer for compute operations
	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		vks::initializers::commandBufferAllocateInfo(
			compute.commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1);

	VK_CHECKRESULT(vkAllocateCommandBuffers(vkDevice.logicalDevice, &cmdBufAllocateInfo, &compute.commandBuffer), "ALLOCATE COMMAND BUFFERS");

	// Fence for compute CB sync
	VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	VK_CHECKRESULT(vkCreateFence(vkDevice.logicalDevice, &fenceCreateInfo, nullptr, &compute.fence), "CREATE FENCE");

	// Build a single command buffer containing the compute dispatch commands
	createComputeCommandBuffer();
	vkDestroyShaderModule(vkDevice.logicalDevice, computePipelineCreateInfo.stage.module, nullptr);
}

void RenderSystem::destroyCompute()
{
	compute.uniformBuffer.Destroy(vkDevice);
	compute.storageBuffers.verts.Destroy(vkDevice);
	compute.storageBuffers.faces.Destroy(vkDevice);
	compute.storageBuffers.meshes.Destroy(vkDevice);
	compute.storageBuffers.objects.Destroy(vkDevice);
	compute.storageBuffers.materials.Destroy(vkDevice);
	compute.storageBuffers.lights.Destroy(vkDevice);
	compute.storageBuffers.guis.Destroy(vkDevice);

	computeTexture.destroy(vkDevice.logicalDevice); 
	for(int i = 0; i < MAXTEXTURES; ++i)
		guiTextures[i].destroy(vkDevice.logicalDevice);

	vkDestroyPipelineCache(vkDevice.logicalDevice, pipelineCache, nullptr);
	vkDestroyPipeline(vkDevice.logicalDevice, compute.pipeline, nullptr);
	vkDestroyPipelineLayout(vkDevice.logicalDevice, compute.pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(vkDevice.logicalDevice, compute.descriptorSetLayout, nullptr);
	vkDestroyFence(vkDevice.logicalDevice, compute.fence, nullptr);
	vkDestroyCommandPool(vkDevice.logicalDevice, compute.commandPool, nullptr); 
}

VkDescriptorSetLayoutBinding RenderSystem::descriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType, uint32_t descriptorCount, VkShaderStageFlags flags)
{
	VkDescriptorSetLayoutBinding bob;

	bob.binding = binding;
	bob.descriptorType = descriptorType;
	bob.descriptorCount = descriptorCount;
	bob.stageFlags = flags;
	bob.pImmutableSamplers = nullptr;

	return bob;
}

void RenderSystem::setupUI()
{
	ui = (EngineUISystem*)world->getSystemManager()->getSystem<EngineUISystem>();

	UIOverlayCreateInfo createInfo = {};
	createInfo.device = &vkDevice;
	createInfo.copyQueue = graphicsQueue;
	createInfo.framebuffers = swapChainFramebuffers;
	createInfo.colorformat = swapChainImageFormat;
	createInfo.depthformat = findDepthFormat();
	createInfo.width = WINDOW.getWidth();
	createInfo.height = WINDOW.getHeight();
	createInfo.shaders = { 
		vkDevice.createShader("Rendering/Shaders/uioverlay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
		vkDevice.createShader("Rendering/Shaders/uioverlay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT) 
	};
	//ui = new EngineUI(createInfo);
	ui->init(createInfo);
	//updateOverlay();

	vkDestroyShaderModule(vkDevice.logicalDevice, createInfo.shaders[0].module, nullptr);
	vkDestroyShaderModule(vkDevice.logicalDevice, createInfo.shaders[1].module, nullptr);

}

void RenderSystem::swapRatio(float f)
{
	scaledSwap.width = swapChainExtent.width * f;
	scaledSwap.height = swapChainExtent.height * f;
}

void RenderSystem::updateMaterials()
{
	//compute.storageBuffers.materials.InitStorageBuffer(vkDevice, materials, materials.size());
	compute.storageBuffers.materials.UpdateBuffers(vkDevice, materials);
}

void RenderSystem::updateMaterial(int id)
{
	rMaterial* m = &RESOURCEMANAGER.getMaterial(id);

	materials[id].diffuse = m->diffuse;
	materials[id].reflective = m->reflective;
	materials[id].roughness = m->roughness;
	materials[id].transparency = m->transparency;
	materials[id].refractiveIndex = m->refractiveIndex;

	updateMaterials();
}

void RenderSystem::togglePlayMode(bool playMode)
{
	//INPUT.playToggled = !INPUT.playToggled;
	//playMode = !playMode;
	if (playMode) {
		WINDOW.resize();
		RenderBase::recreateSwapChain();
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createCommandBuffers(1.f, 0, 0);
		ui->visible = false;
		//ui->resize(swapChainExtent.width, swapChainExtent.height, swapChainFramebuffers);
	}
	else {
		recreateSwapChain();
	}
}

#pragma endregion