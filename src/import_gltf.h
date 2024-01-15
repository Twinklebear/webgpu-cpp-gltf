#pragma once

#include "flatten_gltf.h"
#include "gltf_buffer_view.h"
#include "gltf_mesh.h"
#include "gltf_node.h"
#include "tiny_gltf.h"

struct GLTFRenderData {
    tinygltf::Model model;
    std::vector<GLTFBufferView> buffers;
    std::vector<GLTFMesh> meshes;
    std::vector<GLTFNode> nodes;
};

// Import a GLTF binary file
std::unique_ptr<GLTFRenderData> import_gltf(const uint8_t *glb, const size_t glb_size);
