import unreal

print("="*70)
print("Scanning ALL Materials for Sampler Mismatches")
print("="*70)

asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
filter_materials = unreal.ARFilter(class_names=["Material"], recursive_paths=True)
material_assets = asset_registry.get_assets(filter_materials)

material_properties = [
  unreal.MaterialProperty.MP_BASE_COLOR,
  unreal.MaterialProperty.MP_METALLIC,
  unreal.MaterialProperty.MP_SPECULAR,
  unreal.MaterialProperty.MP_ROUGHNESS,
  unreal.MaterialProperty.MP_EMISSIVE_COLOR,
  unreal.MaterialProperty.MP_OPACITY,
  unreal.MaterialProperty.MP_NORMAL,
  unreal.MaterialProperty.MP_AMBIENT_OCCLUSION,
]

materials_with_issues = []

print(f"Checking {len(material_assets)} materials...\n")

for asset_data in material_assets:
  material = unreal.load_asset(asset_data.package_name)
  if not material:
	  continue

  material_name = str(asset_data.asset_name)
  has_mismatch = False
  mismatches = []

  for prop in material_properties:
	  try:
		  input_node = unreal.MaterialEditingLibrary.get_material_property_input_node(material, prop)

		  if input_node and "TextureSample" in input_node.get_class().get_name():
			  texture = input_node.get_editor_property("Texture")
			  sampler = input_node.get_editor_property("SamplerType")

			  if texture:
				  vt_enabled = texture.get_editor_property("virtual_texture_streaming")

				  # Check for mismatch
				  if vt_enabled:
					  if sampler in [unreal.MaterialSamplerType.SAMPLERTYPE_COLOR,
									unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL,
									unreal.MaterialSamplerType.SAMPLERTYPE_GRAYSCALE]:
						  has_mismatch = True
						  mismatches.append(f"{texture.get_name()}: {sampler} (needs Virtual)")
				  else:
					  if sampler in [unreal.MaterialSamplerType.SAMPLERTYPE_VIRTUAL_COLOR,
									unreal.MaterialSamplerType.SAMPLERTYPE_VIRTUAL_NORMAL,
									unreal.MaterialSamplerType.SAMPLERTYPE_VIRTUAL_GRAYSCALE]:
						  has_mismatch = True
						  mismatches.append(f"{texture.get_name()}: {sampler} (needs Regular)")
	  except:
		  continue

  if has_mismatch:
	  materials_with_issues.append((material_name, mismatches))

print("="*70)
print(f"Results: Found {len(materials_with_issues)} materials with issues\n")

if materials_with_issues:
  for mat_name, issues in materials_with_issues:
	  print(f"{mat_name}:")
	  for issue in issues:
		  print(f"  - {issue}")
	  print()
else:
  print("✓ No materials with sampler mismatches found!")
  print("\nAll materials are correctly configured!")

print("="*70)