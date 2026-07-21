// Copyright rollback-in-a-box. SPDX-License-Identifier: same as repository LICENSE.
using System.IO;
using UnrealBuildTool;

// Private implementation libraries for the Box3DRollback runtime module.
// Do not depend on this module from raw-API consumers: Box3D owns process-global
// state and must be linked into exactly one dylib in modular targets.
public class Box3DRollbackLibrary : ModuleRules
{
	public Box3DRollbackLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		string LibDir = Path.Combine(ModuleDirectory, "lib", Target.Platform.ToString());
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
