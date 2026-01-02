import unreal

def is_virtual_texture(texture):
    """Check if a texture is a Virtual Texture"""
    if not texture:
        return False
    return texture.get_editor_property("VirtualTextureStreaming")

def get_expected_sampler_type(texture):
    """Determine the expected sampler type based on texture settings"""
    if not texture:
        return None

    is_vt = is_virtual_texture(texture)
    compression = texture.get_editor_property("CompressionSettings")
    srgb = texture.get_editor_property("SRGB")

    # Map compression settings to sampler types
    if compression == unreal.TextureCompressionSettings.TC_NORMALMAP:
        return unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if not is_vt else unreal.MaterialSamplerType.SAMPLERTYPE_VIRTUAL_NORMAL
    elif compression in [unreal.TextureCompressionSettings.TC_MASKS,
                         unreal.TextureCompressionSettings.TC_GRAYSCALE]:
        return unreal.MaterialSamplerType.SAMPLERTYPE_MASKS if not is_vt else unreal.MaterialSamplerType.SAMPLERTYPE_VIRTUAL_MASKS
    else:
        return unreal.MaterialSamplerType.SAMPLERTYPE_COLOR if not is_vt else unreal.MaterialSamplerType.SAMPLERTYPE_VIRTUAL_COLOR

def get_sampler_type_name(sampler_type):
    """Get readable name for sampler type"""
    sampler_names = {
        unreal.MaterialSamplerType.SAMPLERTYPE_COLOR: "Color",
        unreal.MaterialSamplerType.SAMPLERTYPE_GRAYSCALE: "Grayscale",
        unreal.MaterialSamplerType.SAMPLERTYPE_ALPHA: "Alpha",
        unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL: "Normal",
        unreal.MaterialSamplerType.SAMPLERTYPE_MASKS: "Masks",
        unreal.MaterialSamplerType.SAMPLERTYPE_DISTANCE_FIELD_FONT: "Distance Field Font",
        unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR: "Linear Color",
        unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_GRAYSCALE: "Linear Grayscale",
        unreal.MaterialSamplerType.SAMPLERTYPE_DATA: "Data",
        unreal.MaterialSamplerType.SAMPLERTYPE_EXTERNAL: "External",
        unreal.MaterialSamplerType.SAMPLERTYPE_VIRTUAL_COLOR: "Virtual Color",
        unreal.MaterialSamplerType.SAMPLERTYPE_VIRTUAL_GRAYSCALE: "Virtual Grayscale",
        unreal.MaterialSamplerType.SAMPLERTYPE_VIRTUAL_ALPHA: "Virtual Alpha",
        unreal.MaterialSamplerType.SAMPLERTYPE_VIRTUAL_NORMAL: "Virtual Normal",
        unreal.MaterialSamplerType.SAMPLERTYPE_VIRTUAL_MASKS: "Virtual Masks",
        unreal.MaterialSamplerType.SAMPLERTYPE_VIRTUAL_LINEAR_COLOR: "Virtual Linear Color",
        unreal.MaterialSamplerType.SAMPLERTYPE_VIRTUAL_LINEAR_GRAYSCALE: "Virtual Linear Grayscale"
    }
    return sampler_names.get(sampler_type, str(sampler_type))

print("=" * 80)
print("IDENTIFYING MATERIAL FUNCTIONS THAT USE VIRTUAL TEXTURES")
print("=" * 80)
print()

# Get asset registry
asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

# Find all Material Functions
print("Scanning for Material Functions...")
mf_filter = unreal.ARFilter(
    class_names=["MaterialFunction"],
    recursive_paths=True,
    package_paths=["/Game"]
)
mf_assets = asset_registry.get_assets(mf_filter)
print(f"Found {len(mf_assets)} Material Functions")
print()

# Track Material Functions that need fixing
functions_needing_fixes = {}

for mf_asset in mf_assets:
    mf_path = mf_asset.package_name
    material_function = unreal.load_asset(str(mf_path))

    if not material_function:
        continue

    # We can't access FunctionExpressions directly, but we can try to load and check outputs
    # Unfortunately, Material Functions don't have property inputs like Materials do
    # So we need a different approach - check the asset references

    # We can't access FunctionExpressions directly in UE 5.7
    # Skip this section - we'll use the reverse lookup approach below
    pass

print("=" * 80)
print("ALTERNATIVE APPROACH: FINDING VT TEXTURES AND THEIR MATERIAL FUNCTION USERS")
print("=" * 80)
print()

# Find all Virtual Textures
print("Scanning for Virtual Textures...")
texture_filter = unreal.ARFilter(
    class_names=["Texture2D"],
    recursive_paths=True,
    package_paths=["/Game"]
)
texture_assets = asset_registry.get_assets(texture_filter)

vt_textures = []
for tex_asset in texture_assets:
    tex_path = tex_asset.package_name
    texture = unreal.load_asset(str(tex_path))

    if texture and is_virtual_texture(texture):
        vt_textures.append((str(tex_path), texture))

print(f"Found {len(vt_textures)} Virtual Textures")
print()

# For each VT texture, find Material Functions that reference it
material_function_issues = {}

for tex_path, texture in vt_textures:
    # Find all assets that reference this texture
    referencers = unreal.EditorAssetLibrary.find_package_referencers_for_asset(tex_path)

    for ref_path in referencers:
        # The ref_path is a package path, try to get asset data from it
        # Extract package name (remove .AssetName part if it exists)
        package_path = ref_path.split('.')[0] if '.' in ref_path else ref_path

        # Get all assets in this package
        assets_in_package = asset_registry.get_assets_by_package_name(package_path)

        for asset_data in assets_in_package:
            # Check if this asset is a Material Function
            class_name = str(asset_data.asset_class_path.asset_name)
            if class_name == "MaterialFunction":
                mf_name = str(asset_data.asset_name)
                mf_path = str(asset_data.package_name)

                if mf_path not in material_function_issues:
                    material_function_issues[mf_path] = {
                        'name': mf_name,
                        'textures': []
                    }

                # Get expected sampler type
                expected_sampler = get_expected_sampler_type(texture)

                material_function_issues[mf_path]['textures'].append({
                    'texture_path': tex_path,
                    'texture_name': tex_path.split('/')[-1],
                    'expected_sampler': get_sampler_type_name(expected_sampler)
                })
                break  # Only need to process this package once

# Output results
if material_function_issues:
    print("=" * 80)
    print(f"FOUND {len(material_function_issues)} MATERIAL FUNCTIONS THAT USE VIRTUAL TEXTURES")
    print("=" * 80)
    print()

    # Sort by path for consistent output
    sorted_mfs = sorted(material_function_issues.items())

    for idx, (mf_path, info) in enumerate(sorted_mfs, 1):
        print(f"{idx}. {info['name']}")
        print(f"   Path: {mf_path}")
        print(f"   Virtual Textures Used ({len(info['textures'])}):")

        for tex_info in info['textures']:
            print(f"      - {tex_info['texture_name']}")
            print(f"        Expected Sampler Type: {tex_info['expected_sampler']}")

        print()
        print(f"   HOW TO FIX:")
        print(f"      1. Double-click to open: {mf_path}")
        print(f"      2. Find TextureSample nodes using the textures listed above")
        print(f"      3. Select each TextureSample node")
        print(f"      4. In Details panel, change 'Sampler Type' to the expected type")
        print(f"      5. Save the Material Function")
        print()
        print("-" * 80)
        print()

    print("=" * 80)
    print("SUMMARY")
    print("=" * 80)
    print(f"Total Material Functions needing manual fixes: {len(material_function_issues)}")
    print()
    print("You can use this list to systematically fix each Material Function.")
    print("The errors will clear once the sampler types match the Virtual Texture status.")

else:
    print("No Material Functions found that use Virtual Textures.")
    print("This might indicate that:")
    print("  1. No Material Functions use VT textures")
    print("  2. The dependency tracking approach has limitations")
    print()
    print("Try checking the VT textures directly in the editor to see their usage.")

print()
print("Script complete!")
