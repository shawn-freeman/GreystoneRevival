import unreal
import re

print("="*70)
print("Force Recompile All Materials and Find Sampler Errors")
print("="*70)

# Clear the log before we start
unreal.log("="*70)
unreal.log("MATERIAL SAMPLER ERROR SCAN - START")
unreal.log("="*70)

asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

# Get all materials
filter_materials = unreal.ARFilter(class_names=["Material"], recursive_paths=True)
material_assets = asset_registry.get_assets(filter_materials)

materials_with_errors = []

print(f"Recompiling {len(material_assets)} materials to check for errors...")
print("This may take a few minutes...\n")

for i, asset_data in enumerate(material_assets):
  material_name = str(asset_data.asset_name)

  if i % 50 == 0:
	  print(f"Progress: {i}/{len(material_assets)} materials checked...")

  try:
	  material = unreal.load_asset(asset_data.package_name)
	  if not material:
		  continue

	  # Force recompile by marking dirty and updating
	  material.post_edit_change()

	  # Check if material has compilation errors
	  # The errors will be in the log, but we can also check statistics
	  try:
		  stats = unreal.MaterialEditingLibrary.get_statistics(material)
		  # If we can get stats, the material compiled
		  # But we need to check the actual error state

		  # Log this material so we can find it in the output log
		  unreal.log(f"CHECKING: {material_name}")

	  except Exception as e:
		  # If get_statistics fails, there might be an error
		  materials_with_errors.append(material_name)
		  unreal.log_warning(f"FAILED: {material_name} - {e}")

  except Exception as e:
	  materials_with_errors.append(material_name)
	  unreal.log_error(f"ERROR: {material_name} - {e}")
	  continue

print(f"\n{'='*70}")
print("IMPORTANT: Check the Output Log!")
print(f"{'='*70}")
print("\nWindow → Developer Tools → Output Log")
print("\nSearch for 'Sampler type' to find all sampler errors")
print("Errors will show which materials/functions need fixing")
print(f"\n{'='*70}")

if materials_with_errors:
  print(f"\nMaterials that failed to compile: {len(materials_with_errors)}")
  for mat in materials_with_errors[:20]:  # Show first 20
	  print(f"  - {mat}")

"But actually, there is a BETTER approach - check the existing log file:"

import re

print("="*70)
print("Parsing Unreal Log for Sampler Type Errors")
print("="*70)

log_path = "C:/Users/magus/OneDrive/Documents/Unreal Projects/Greystone/Saved/Logs/Greystone.log"

try:
  with open(log_path, 'r', encoding='utf-8', errors='ignore') as f:
	  log_content = f.read()

  # Find all sampler type errors
  # Pattern: "Sampler type is X, should be Y for /Path/To/Texture"
  pattern = r'Sampler type is (\w+), should be Virtual (\w+) for ([^\s]+)'

  matches = re.findall(pattern, log_content)

  # Also find which materials/functions these errors are in
  # Pattern: (Function XXXX) or material name before the error
  function_pattern = r'\(Function ([^)]+)\)'

  # Parse the log line by line to get context
  errors_by_asset = {}

  lines = log_content.split('\n')
  current_material = None

  for line in lines:
	  # Check if this line mentions a material being compiled
	  if 'Material' in line and ('Compiling' in line or 'Error' in line):
		  # Try to extract material name
		  mat_match = re.search(r'/Game/[^\s]+', line)
		  if mat_match:
			  current_material = mat_match.group(0)

	  # Check for sampler type errors
	  if 'Sampler type is' in line and 'should be Virtual' in line:
		  # Extract function name if present
		  func_match = re.search(r'\(Function ([^)]+)\)', line)
		  # Extract texture path
		  tex_match = re.search(r'for (/Game/[^\s]+)', line)
		  # Extract sampler types
		  sampler_match = re.search(r'Sampler type is (\w+), should be Virtual (\w+)', line)

		  if sampler_match:
			  current_type = sampler_match.group(1)
			  should_be = sampler_match.group(2)

			  key = "Unknown"
			  if func_match:
				  key = f"MaterialFunction: {func_match.group(1)}"
			  elif current_material:
				  key = f"Material: {current_material.split('/')[-1].split('.')[0]}"

			  if key not in errors_by_asset:
				  errors_by_asset[key] = []

			  error_info = f"{current_type} → Virtual {should_be}"
			  if tex_match:
				  tex_name = tex_match.group(1).split('/')[-1].split('.')[0]
				  error_info += f" ({tex_name})"

			  errors_by_asset[key].append(error_info)

  print(f"\nFound errors in {len(errors_by_asset)} assets:\n")
  print("="*70)

  if errors_by_asset:
	  for asset, errors in sorted(errors_by_asset.items()):
		  print(f"\n{asset}:")
		  for error in set(errors):  # Remove duplicates
			  print(f"  - {error}")
  else:
	  print("No sampler type errors found in log!")
	  print("\nThis means either:")
	  print("1. All materials are already fixed")
	  print("2. Materials haven't been compiled recently")
	  print("\nTry opening a material and checking for errors manually")

  print("\n" + "="*70)

except FileNotFoundError:
  print(f"Log file not found at: {log_path}")
except Exception as e:
  print(f"Error reading log: {e}")