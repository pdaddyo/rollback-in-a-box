// Copyright rollback-in-a-box. SPDX-License-Identifier: same as repository LICENSE.
using System.IO;
using UnrealBuildTool;

// The Unreal module is a thin adapter. All rollback/physics logic lives in the
// engine-neutral core (repo /core) and Box3D (repo /gdext/extern/box3d), which
// are built ONCE as static libraries by unreal/build_thirdparty.(sh|bat) and
// linked here. This is the same core the Godot GDExtension compiles — there is
// no second copy of the logic. See unreal/Box3DRollback/README.md.
public class Box3DRollback : ModuleRules
{
	public Box3DRollback(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		// UE 5.8 requires C++20 (C++17 is rejected). The neutral core is C++17
		// source, which is valid C++20, so it compiles cleanly here.
		CppStandard = CppStandardVersion.Cpp20;

		// The neutral core and Box3D use the C++ standard library (std::vector,
		// std::unordered_set), whose headers reference throw/exceptions; enable
		// exceptions in this module's translation units so they compile.
		bEnableExceptions = true;
		bUseRTTI = false;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
		});

		// unreal/Box3DRollback/Source/Box3DRollback -> plugin root -> repo root.
		string PluginRoot = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
		string RepoRoot = Path.GetFullPath(Path.Combine(PluginRoot, "..", ".."));

		string CoreInclude = Path.Combine(RepoRoot, "core", "include");
		string CoreIncludeFlat = Path.Combine(CoreInclude, "box3d_rollback");
		string Box3DInclude = Path.Combine(RepoRoot, "gdext", "extern", "box3d", "include");

		PublicIncludePaths.Add(CoreInclude);
		PublicIncludePaths.Add(CoreIncludeFlat); // for the unqualified "rollback_shim.h"
		PublicIncludePaths.Add(Box3DInclude);

		// Box3D's process-global state (worlds array, length-units global) must
		// exist exactly ONCE per process. This module re-exports the whole C API
		// from its DLL (see Private/Box3DApiExport.cpp); consumers that call the
		// raw API compile with BOX3D_EXPORT=__declspec(dllimport) and resolve
		// against this DLL instead of linking their own copy of the static lib.
		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PrivateDefinitions.Add("BOX3D_EXPORT=__declspec(dllexport)");
		}

		// Prebuilt static libs, one subdir per platform (see build_thirdparty).
		string LibDir = Path.Combine(PluginRoot, "Source", "ThirdParty",
			"Box3DRollbackLibrary", "lib", Target.Platform.ToString());

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PublicAdditionalLibraries.Add(Path.Combine(LibDir, "box3d_rollback_neutral.lib"));
			PublicAdditionalLibraries.Add(Path.Combine(LibDir, "box3d.lib"));
		}
		else
		{
			PublicAdditionalLibraries.Add(Path.Combine(LibDir, "libbox3d_rollback_neutral.a"));
			PublicAdditionalLibraries.Add(Path.Combine(LibDir, "libbox3d.a"));
		}
	}
}
