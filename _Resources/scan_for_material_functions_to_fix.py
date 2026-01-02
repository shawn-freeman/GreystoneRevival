import unreal

print("="*70)
print("Finding Material Functions that Reference VT Textures")
print("="*70)

asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

# Get all textures with VT enabled
filter_tex = unreal.ARFilter(class_names=["Texture2D"], recursive_paths=True)
texture_assets = asset_registry.get_assets(filter_tex)

vt_textures = []
for asset_data in texture_assets:
  try:
      tex = unreal.load_asset(asset_data.package_name)
      if tex and tex.get_editor_property("virtual_texture_streaming"):
          vt_textures.append((asset_data.package_name, str(asset_data.asset_name)))
  except:
      continue

print(f"Found {len(vt_textures)} textures with VT enabled")
print(f"Finding what references them...\n")

material_functions_with_vt = {}

# Check first 50 VT textures (to avoid taking forever)
for i, (tex_path, tex_name) in enumerate(vt_textures[:100]):
  if i % 10 == 0:
      print(f"Progress: {i}/{min(100, len(vt_textures))} textures checked...")

  try:
      # Get what references this texture
      referencers = unreal.EditorAssetLibrary.find_package_referencers_for_asset(tex_path)

      for ref_path in referencers:
          # Check if it's a Material Function
          if ref_path:
              # Load the asset to check its type
              try:
                  ref_asset_data = asset_registry.get_asset_by_object_path(ref_path)
                  if ref_asset_data:
                      class_name = str(ref_asset_data.asset_class_path).split('.')[-1].strip("'")

                      if "MaterialFunction" in class_name:
                          ref_name = str(ref_asset_data.asset_name)
                          if ref_name not in material_functions_with_vt:
                              material_functions_with_vt[ref_name] = []
                          material_functions_with_vt[ref_name].append(tex_name)
              except:
                  pass

  except Exception as e:
      continue

print(f"\n{'='*70}")
print(f"Material Functions using VT textures ({len(material_functions_with_vt)}):")
print(f"{'='*70}\n")

if material_functions_with_vt:
  print("MANUALLY FIX THESE Material Functions:\n")
  for func_name in sorted(material_functions_with_vt.keys()):
      textures = material_functions_with_vt[func_name]
      print(f"{func_name}:")
      for tex in set(textures):  # Remove duplicates
          print(f"  - {tex}")
      print()
else:
  print("No Material Functions found (checking only first 100 VT textures)")
  print("This might mean:")
  print("1. Material Functions use textures not in first 100 VT textures")
  print("2. The reference finding API isn't working")
  print("\nTry manually opening the Material Functions mentioned in errors:")
  print("  - ML_SoftMetal01")
  print("  - ML_Synthetic_Leather01")
  print("  - ML_Honeycomb")
  print("  - MF_OrionCharacterAO")
  print("  - ML_MetallicClearCoat02")

print(f"\n{'='*70}")
print("To fix Material Functions:")
print("1. Open in Content Browser")
print("2. Select TextureSample nodes")
print("3. Details → Sampler Type:")
print("   Normal → Virtual Normal")
print("   Color → Virtual Color")
print("   Masks → Virtual Masks")
print("4. Click Apply and Save")
print(f"{'='*70}")