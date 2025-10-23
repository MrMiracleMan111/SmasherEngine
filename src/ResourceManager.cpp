#include "Smasher/Base.h"
#include "Smasher/ResourceManager.h"

namespace Smasher {
	std::shared_ptr<Smasher::ShaderResource> ResourceManager::LoadVertFragShaderResource(const std::string& vert, const std::string& frag)
    {
        uint64_t hash1 = hash_str(vert.c_str());
        uint64_t hash2 = hash_str(vert.c_str());
        uint64_t combinedHash = hash1 ^ hash2;

        ResourceID resourceID{ combinedHash };
        return std::make_shared<Smasher::ShaderResource>(resourceID, vert, frag);
    }
}
