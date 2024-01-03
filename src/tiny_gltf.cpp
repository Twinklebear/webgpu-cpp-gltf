#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

// We're not going to ever write out gltf files, so just stub this out
namespace tinygltf {
bool WriteImageData(const std::string *,
                    const std::string *,
                    const Image *,
                    bool,
                    const URICallbacks *,
                    std::string *,
                    void *)
{
    return true;
}
}
