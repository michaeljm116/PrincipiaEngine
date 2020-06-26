#pragma once

//#define GLM_FORCE_SSE2 // or GLM_FORCE_SSE42 if your processor supports it
//#define GLM_FORCE_ALIGNED
//
//
//#include <vector>
//#include <glm/glm.hpp>
//#include <iostream>
//#include <glm/simd/common.h>
//
//#include "bvh/bvh.hpp"
//#include "bvh/ray.hpp"
//#include "bvh/vector.hpp"
//#include "bvh/intersectors.hpp"
//
//using Scalar        = float;
//using Vector3       = glm::tvec3<Scalar, glm::precision::aligned_highp>;
////using Vector3       = glm::vec3; //bvh::Vector3<Scalar>;// glm::vec3;
//using BoundingBox   = bvh::BoundingBox<Scalar>;
//using Ray           = bvh::Ray<Scalar>;
//using Bvh           = bvh::Bvh<Scalar>;
//
//
//struct CustomPrimitive {
//    struct Intersection {
//        Scalar t;
//
//        // Required member: returns the distance along the ray
//        Scalar distance() const { return t; }
//        Vector3<Scalar> f = glm::vec3(0.f);
//        
//    };
//
//    // Required type: the floating-point type used
//    using ScalarType = Scalar;
//    // Required type: the intersection data type returned by the intersect() method
//    using IntersectionType = Intersection;
//
//    CustomPrimitive() = default;
//
//    // Required member: returns the center of the primitive
//    Vector3 center() const {
//        return Vector3(0, 0, 0);
//    }
//
//    // Required member: returns a bounding box for the primitive (tighter is better)
//    BoundingBox bounding_box() const {
//        return BoundingBox(Vector3(-1.f, -1.f, -1.f), Vector3(1.f, 1.f, 1.f));
//    }
//
//    // Required member: computes the intersection between a ray and the primitive
//    std::optional<Intersection> intersect(const Ray& ray) const {
//        return std::make_optional<Intersection>(Intersection{ (ray.tmin + ray.tmax) * Scalar(0.5) });
//    }
//};
