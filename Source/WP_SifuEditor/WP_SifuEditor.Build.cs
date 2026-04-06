// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WP_SifuEditor : ModuleRules
{
	public WP_SifuEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AnimationModifiers",
			"AnimationBlueprintLibrary",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
