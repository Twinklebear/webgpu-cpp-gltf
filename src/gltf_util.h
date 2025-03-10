#pragma once

#include <cstddef>
#include <stdexcept>
#include <string>
#include "tiny_gltf.h"
#include <webgpu/webgpu_cpp.h>

inline size_t align_to(const size_t x, const size_t align)
{
    return ((x + align - 1) / align) * align;
}

inline wgpu::VertexFormat gltf_wgpu_vertex_type(const int gltf_component_type,
                                                const int gltf_type)
{
    switch (gltf_component_type) {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
        switch (gltf_type) {
        case TINYGLTF_TYPE_VEC2:
            return wgpu::VertexFormat::Sint8x2;
        case TINYGLTF_TYPE_VEC4:
            return wgpu::VertexFormat::Sint8x2;
        default:
            break;
        }
        break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        switch (gltf_type) {
        case TINYGLTF_TYPE_VEC2:
            return wgpu::VertexFormat::Uint8x2;
        case TINYGLTF_TYPE_VEC4:
            return wgpu::VertexFormat::Uint8x2;
        default:
            break;
        }
        break;
    case TINYGLTF_COMPONENT_TYPE_SHORT:
        switch (gltf_type) {
        case TINYGLTF_TYPE_VEC2:
            return wgpu::VertexFormat::Sint16x2;
        case TINYGLTF_TYPE_VEC4:
            return wgpu::VertexFormat::Sint16x4;
        default:
            break;
        }
        break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        switch (gltf_type) {
        case TINYGLTF_TYPE_VEC2:
            return wgpu::VertexFormat::Uint16x2;
        case TINYGLTF_TYPE_VEC4:
            return wgpu::VertexFormat::Uint16x4;
        default:
            break;
        }
        break;
    case TINYGLTF_COMPONENT_TYPE_INT:
        switch (gltf_type) {
        case TINYGLTF_TYPE_SCALAR:
            return wgpu::VertexFormat::Sint32;
        case TINYGLTF_TYPE_VEC2:
            return wgpu::VertexFormat::Sint32x2;
        case TINYGLTF_TYPE_VEC3:
            return wgpu::VertexFormat::Sint32x3;
        case TINYGLTF_TYPE_VEC4:
            return wgpu::VertexFormat::Sint32x4;
        default:
            break;
        }
        break;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        switch (gltf_type) {
        case TINYGLTF_TYPE_SCALAR:
            return wgpu::VertexFormat::Uint32;
        case TINYGLTF_TYPE_VEC2:
            return wgpu::VertexFormat::Uint32x2;
        case TINYGLTF_TYPE_VEC3:
            return wgpu::VertexFormat::Uint32x3;
        case TINYGLTF_TYPE_VEC4:
            return wgpu::VertexFormat::Uint32x4;
        default:
            break;
        }
        break;
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
        switch (gltf_type) {
        case TINYGLTF_TYPE_SCALAR:
            return wgpu::VertexFormat::Float32;
        case TINYGLTF_TYPE_VEC2:
            return wgpu::VertexFormat::Float32x2;
        case TINYGLTF_TYPE_VEC3:
            return wgpu::VertexFormat::Float32x3;
        case TINYGLTF_TYPE_VEC4:
            return wgpu::VertexFormat::Float32x4;
        default:
            break;
        }
        break;
    default:
        return wgpu::VertexFormat::Undefined;
    }
    return wgpu::VertexFormat::Undefined;
}
