import unreal

# Get the static mesh editor subsystem
mesh_editor_subsystem = unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)

# List all tree meshes to convert
tree_mesh_paths = [
    # Full trees
    "/Game/PN_interactiveSpruceForest/Meshes/full/low/spruce_full_01_low",
    "/Game/PN_interactiveSpruceForest/Meshes/full/low/spruce_full_02_low",
    "/Game/PN_interactiveSpruceForest/Meshes/full/low/spruce_full_03_low",
    # Half trees
    "/Game/PN_interactiveSpruceForest/Meshes/half/low/spruce_half_01_low",
    "/Game/PN_interactiveSpruceForest/Meshes/half/low/spruce_half_02_low",
    "/Game/PN_interactiveSpruceForest/Meshes/half/low/spruce_half_03_low",
    "/Game/PN_interactiveSpruceForest/Meshes/half/low/spruce_half_04_low",
    # Small trees
    "/Game/PN_interactiveSpruceForest/Meshes/small/spruce_small_03",
    "/Game/PN_interactiveSpruceForest/Meshes/small/spruce_small_04",
    "/Game/PN_interactiveSpruceForest/Meshes/small/spruce_small_05",
    "/Game/PN_interactiveSpruceForest/Meshes/small/spruce_small_06",
    "/Game/PN_interactiveSpruceForest/Meshes/small/spruce_small_07",
]

unreal.log("=" * 70)
unreal.log("ENABLING NANITE ON TREE MESHES")
unreal.log("=" * 70)

success_count = 0
fail_count = 0
skipped_count = 0

for mesh_path in tree_mesh_paths:
    mesh_name = mesh_path.split('/')[-1]
    
    try:
        # Load the mesh
        static_mesh = unreal.load_asset(mesh_path)
        
        if not static_mesh:
            unreal.log_warning(f"Could not load: {mesh_name}")
            fail_count += 1
            continue
        
        # Get current Nanite settings
        current_settings = mesh_editor_subsystem.get_nanite_settings(static_mesh)
        
        if current_settings.enabled:
            unreal.log(f"SKIP: {mesh_name} - Already has Nanite enabled")
            skipped_count += 1
            continue
        
        # Create new Nanite settings with enabled = True
        new_settings = unreal.MeshNaniteSettings()
        new_settings.enabled = True
        
        # Apply Nanite settings
        mesh_editor_subsystem.set_nanite_settings(static_mesh, new_settings, apply_changes=True)
        
        # Verify it was applied
        verify_settings = mesh_editor_subsystem.get_nanite_settings(static_mesh)
        
        if verify_settings.enabled:
            unreal.log(f"SUCCESS: {mesh_name} - Nanite enabled")
            success_count += 1
        else:
            unreal.log_warning(f"VERIFY FAILED: {mesh_name}")
            fail_count += 1
            
    except Exception as e:
        unreal.log_error(f"ERROR: {mesh_name} - {str(e)}")
        fail_count += 1

unreal.log("\n" + "=" * 70)
unreal.log("NANITE CONVERSION SUMMARY")
unreal.log("=" * 70)
unreal.log(f"Successfully enabled: {success_count}")
unreal.log(f"Already enabled: {skipped_count}")
unreal.log(f"Failed: {fail_count}")
unreal.log(f"Total processed: {len(tree_mesh_paths)}")

if success_count > 0:
    unreal.log("\nIMPORTANT: Regenerate your PCG to see Nanite in effect!")
