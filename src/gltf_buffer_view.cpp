#include "gltf_buffer_view.h"
#include <cstddef>
#include <cstring>
#include "gltf_util.h"
#include "webgpu_cpp.h"

GLTFBufferView::GLTFBufferView(const tinygltf::BufferView *view,
                               const tinygltf::Buffer &buffer)
    : view(view), buf(buffer.data.data() + view->byteOffset)
{
}

size_t GLTFBufferView::byte_length() const
{
    return view->byteLength;
}

size_t GLTFBufferView::stride() const
{
    return view->byteStride;
}

void GLTFBufferView::add_usage(const wgpu::BufferUsage usage)
{
    usage_flags |= static_cast<uint32_t>(usage);
}

void GLTFBufferView::upload(wgpu::Device &device)
{
    needs_upload = false;

    wgpu::BufferDescriptor desc;
    desc.size = align_to(byte_length(), 4);
    desc.usage = static_cast<wgpu::BufferUsage>(usage_flags);
    desc.mappedAtCreation = true;

    gpu_buffer = device.CreateBuffer(&desc);

    std::memcpy(gpu_buffer.GetMappedRange(), buf, byte_length());

    gpu_buffer.Unmap();
}
