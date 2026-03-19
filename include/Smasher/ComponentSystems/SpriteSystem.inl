namespace Smasher {
	namespace SpriteSystem {
		template <class ManifestData>
		ErrorCode SetTexture(Component &component, entt::registry &registry, const TextureOptions& opts) {
			ResourceManifestInfo info = ResourceManager::template GetManifestInfo<ManifestData>().Get();
			return SetTexture(component, registry, info.resourceId, info.paths, info.path_count, opts);
		}
	}
}