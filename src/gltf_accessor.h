#pragma once

#include "gltf_buffer_view.h"
#include "tiny_gltf.h"
#include <webgpu/webgpu_cpp.h>

struct GLTFAccessor {
    const GLTFBufferView *view = nullptr;
    const tinygltf::Accessor *accessor = nullptr;

    GLTFAccessor() = default;

    GLTFAccessor(const GLTFBufferView *view, const tinygltf::Accessor *accessor);

    size_t offset() const;

    size_t size() const;

    size_t byte_length() const;

    size_t stride() const;

    wgpu::VertexFormat vertex_format() const;

    wgpu::IndexFormat index_format() const;
};
