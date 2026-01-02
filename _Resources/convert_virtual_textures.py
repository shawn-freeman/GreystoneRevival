import unreal

# Get asset registry
asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

# Find all textures in project
filter = unreal.ARFilter(
  class_names=["Texture2D"],
  recursive_paths=True
)

texture_assets = asset_registry.get_assets(filter)

textures_converted = 0
textures_skipped = 0
minimum_size = 2048  # 2K resolution

print(f"Found {len(texture_assets)} textures to check...")

for asset_data in texture_assets:
  try:
      # Get package name
      package_name = asset_data.package_name

      # Load the texture
      texture = unreal.load_asset(package_name)

      if not texture:
          print(f"Could not load: {package_name}")
          textures_skipped += 1
          continue

      # Get texture dimensions
      try:
          width = texture.get_editor_property("source_size_x")
          height = texture.get_editor_property("source_size_y")
      except:
          # Fallback method
          width = texture.blueprint_get_size_x()
          height = texture.blueprint_get_size_y()

      # Check if texture is 2K or larger
      if width >= minimum_size or height >= minimum_size:
          # Check if VT is already enabled
          vt_enabled = texture.get_editor_property("virtual_texture_streaming")

          if not vt_enabled:
              # Enable Virtual Texture Streaming
              texture.set_editor_property("virtual_texture_streaming", True)

              # Mark package dirty and save
              unreal.EditorAssetLibrary.save_loaded_asset(texture)

              textures_converted += 1
              print(f"✓ Converted: {asset_data.asset_name} ({width}x{height})")
          else:
              print(f"⊙ Already VT: {asset_data.asset_name} ({width}x{height})")
      else:
          print(f"- Skipped (too small): {asset_data.asset_name} ({width}x{height})")
          textures_skipped += 1

  except Exception as e:
      print(f"✗ Error processing {asset_data.asset_name}: {str(e)}")
      textures_skipped += 1
      continue

print(f"\n{'='*60}")
print(f"Summary:")
print(f"  Textures converted to Virtual Texture: {textures_converted}")
print(f"  Textures skipped: {textures_skipped}")
print(f"{'='*60}")