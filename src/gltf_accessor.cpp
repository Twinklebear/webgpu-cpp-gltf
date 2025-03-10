#include "gltf_accessor.h"
#include "gltf_util.h"

GLTFAccessor::GLTFAccessor(const GLTFBufferView *view, const tinygltf::Accessor *accessor)
    : view(view), accessor(accessor)
{
}

size_t GLTFAccessor::offset() const
{
    return accessor ? accessor->byteOffset : 0;
}

size_t GLTFAccessor::size() const
{
    return accessor ? accessor->count : 0;
}

size_t GLTFAccessor::byte_length() const
{
    return stride() * size();
}

size_t GLTFAccessor::stride() const
{
    return accessor ? accessor->ByteStride(*view->view) : 0;
}

wgpu::VertexFormat GLTFAccessor::vertex_format() const
{
    return accessor ? gltf_wgpu_vertex_type(accessor->componentType, accessor->type)
                    : wgpu::VertexFormat::Undefined;
}

wgpu::IndexFormat GLTFAccessor::index_format() const
{
    if (!accessor || accessor->type != TINYGLTF_TYPE_SCALAR) {
        return wgpu::IndexFormat::Undefined;
    }
    switch (accessor->componentType) {
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        return wgpu::IndexFormat::Uint16;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        return wgpu::IndexFormat::Uint32;
    default:
        return wgpu::IndexFormat::Undefined;
    }
    return wgpu::IndexFormat::Undefined;
}
