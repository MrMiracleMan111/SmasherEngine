namespace Smasher {
	namespace TextSystem {
		template <class ManifestData>
		ErrorCode SetFontAsset(Component& component, entt::registry& registry) {
			ResourceManifestInfo info = ResourceManager::template GetManifestInfo<ManifestData>().Get();
			return SetFontAsset(component, registry, info.resourceId, info.paths, info.path_count);
		}
	}
}