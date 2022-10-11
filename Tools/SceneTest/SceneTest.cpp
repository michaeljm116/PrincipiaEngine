#include <vector>
#include <iostream>

#include <bvh/bvh.hpp>
#include <bvh/vector.hpp>
#include <bvh/triangle.hpp>
#include <bvh/ray.hpp>
#include <bvh/sweep_sah_builder.hpp>
#include <bvh/single_ray_traverser.hpp>
#include <bvh/intersectors.hpp>
#include <bvh/linear_bvh_builder.hpp>

#include <glm/glm.hpp>

#include "test_scene.h"
#include "Utility/serialize-node.h"
#include "Rendering/shaderStructures.hpp"


using Scalar = glm::float32_t;
using Vector3 = bvh::Vector3<Scalar>;
using Triangle = bvh::Triangle<Scalar>;
using Ray = bvh::Ray<Scalar>;
using Bvh = bvh::Bvh<Scalar>;
using Morton = uint32_t;

bvh::Vector3<Scalar> glm_to_bvh_Vec3(glm::vec3 v) {
    return bvh::Vector3<Scalar>(v.x, v.y, v.z);
}
bvh::Vector3<Scalar> glm_to_bvh_Vec3(glm::vec4 v) {
    return bvh::Vector3<Scalar>(v.x, v.y, v.z);
}

struct CustomPrimitive  {
    CustomPrimitive(Principia::PrimitiveComponent* pc) {
        c = glm_to_bvh_Vec3(pc->center());
        e = glm_to_bvh_Vec3(pc->world[3]);
    }
    struct Intersection {
        Scalar t;

        // Required member: returns the distance along the ray
        Scalar distance() const { return t; }
    };

    // Required type: the floating-point type used
    using ScalarType = Scalar;
    // Required type: the intersection data type returned by the intersect() method
    using IntersectionType = Intersection;

    CustomPrimitive() = default;

    // Required member: returns the center of the primitive
    Vector3 center() const {
        return c;// Vector3(0, 0, 0);
    }

    // Required member: returns a bounding box for the primitive (tighter is better)
    bvh::BoundingBox<Scalar> bounding_box() const {
        return e;// bvh::BoundingBox(Vector3(-1, -1, -1), Vector3(1, 1, 1));
    }

    // Required member: computes the intersection between a ray and the primitive
    std::optional<Intersection> intersect(const Ray& ray) const {
        return std::make_optional<Intersection>(Intersection { (ray.tmin + ray.tmax) * Scalar(0.5) });
    }
    Vector3 c;
    bvh::BoundingBox<Scalar> e;
};



int main() {
    //Set up artemis
    artemis::World world;
    artemis::SystemManager* sm = world.getSystemManager();
    artemis::TagManager* tm = world.getTagManager();
    Principia::SERIALIZENODE.SetEntityManager(world.getEntityManager());


    //Load teh scene and prims
    test::TestScene test_scene("..\\..\\..\\ShinyAfroMan\\Assets\\Levels\\BlacMan\\Scenes\\testlvl5.xml");
    auto parents = test_scene.GetParents();
    std::vector<CustomPrimitive> custom_prims;
    for (auto p : parents) {
        auto* prim = (Principia::PrimitiveComponent*)p->data->getComponent<Principia::PrimitiveComponent>();
        auto* tran = (Principia::TransformComponent*)p->data->getComponent<Principia::TransformComponent>();
        if (prim) {
            prim->extents = tran->local.scale;
            prim->world = tran->world;
            custom_prims.push_back(prim);
        }
    }

    //Convert the prims into something the bvh can use
    Bvh bvh;
    auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(custom_prims.data(), custom_prims.size());
    auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), custom_prims.size());
    bvh::LinearBvhBuilder<Bvh, Morton> builder(bvh);
    builder.build(global_bbox, bboxes.get(), centers.get(), custom_prims.size());

    //Convert the bvh back into something the engine can use
    
    std::vector<Principia::ssBVHNode> bvh_nodes(bvh.node_count);
    for (int i = 0; i < bvh.node_count; ++i) {
        auto thing = bvh.nodes[i].bounding_box_proxy();
        auto min = thing.to_bounding_box().min;
        auto max = thing.to_bounding_box().max;
        bvh_nodes[i].lower = glm::vec3(min[0], min[1], min[2]);
        bvh_nodes[i].upper = glm::vec3(max[0], max[1], max[2]);
        bvh_nodes[i].numChildren = bvh.nodes[i].primitive_count;
        bvh_nodes[i].offset = bvh.nodes[i].first_child_or_primitive;
    }

    std::vector<bvh::BoundingBox<Scalar>> bbs(custom_prims.size());
    std::vector<Vector3> cs(custom_prims.size());
    for (int i = 0; i < custom_prims.size(); ++i) {
        bbs[i] = bboxes[i];
        cs[i] = centers[i];
    }
    return 0;
}



int main2() {
    // Create an array of triangles
    std::vector<Triangle> triangles;
    triangles.emplace_back(
        Vector3(1.0, -1.0, 1.0),
        Vector3(1.0, 1.0, 1.0),
        Vector3(-1.0, 1.0, 1.0)
    );
    triangles.emplace_back(
        Vector3(1.0, -1.0, 1.0),
        Vector3(-1.0, -1.0, 1.0),
        Vector3(-1.0, 1.0, 1.0)
    );

    Bvh bvh;

    // Create an acceleration data structure on those triangles
    bvh::SweepSahBuilder<Bvh> builder(bvh);
    auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(triangles.data(), triangles.size());
    auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), triangles.size());
    builder.build(global_bbox, bboxes.get(), centers.get(), triangles.size());

    // Intersect a ray with the data structure
    Ray ray(
        Vector3(0.0, 0.0, 0.0), // origin
        Vector3(0.0, 0.0, 1.0), // direction
        0.0,                    // minimum distance
        100.0                   // maximum distance
    );
    bvh::ClosestIntersector<false, Bvh, Triangle> intersector(bvh, triangles.data());
    bvh::SingleRayTraverser<Bvh> traverser(bvh);

    auto hit = traverser.traverse(ray, intersector);
    if (hit) {
        auto triangle_index = hit->primitive_index;
        auto intersection = hit->intersection;
        std::cout << "Hit triangle " << triangle_index << "\n"
            << "distance: " << intersection.t << "\n"
            << "u: " << intersection.u << "\n"
            << "v: " << intersection.v << std::endl;
        return 0;
    }
    return 1;
}
