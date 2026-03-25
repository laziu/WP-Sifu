// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WP_Sifu : ModuleRules
{
	public WP_Sifu(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput",
			"AnimGraph", "AnimGraphRuntime", "Niagara",
			"UMG", "Slate", "SlateCore",
			"AIModule", "StateTreeModule", "GameplayStateTreeModule",
			"GameplayAbilities", "GameplayTags", "GameplayTasks", "NavigationSystem"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[]
		{
			"WP_Sifu/Public",
			"WP_Sifu/Public/Interface",
			"WP_Sifu/Public/Player",
			"WP_Sifu/Public/Data",
			"WP_Sifu/Public/Animation",
		});
	}
}