#pragma once

#include <webgpu/webgpu_cpp.h>
#include "tiny_gltf.h"

struct GLTFBufferView {
    const tinygltf::BufferView *view = nullptr;
    const uint8_t *buf = nullptr;

    bool needs_upload = false;
    wgpu::Buffer gpu_buffer;
    uint32_t usage_flags = 0;

    GLTFBufferView() = default;

    GLTFBufferView(const tinygltf::BufferView *view, const tinygltf::Buffer &buffer);

    size_t byte_length() const;

    size_t stride() const;

    void add_usage(const wgpu::BufferUsage usage);

    void upload(wgpu::Device &device);
};
