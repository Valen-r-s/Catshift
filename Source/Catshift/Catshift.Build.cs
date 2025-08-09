// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Catshift : ModuleRules
{
	public Catshift(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
            "NavigationSystem",      
            "GameplayTasks"

        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Catshift",
			"Catshift/Variant_Platforming",
			"Catshift/Variant_Combat",
			"Catshift/Variant_Combat/AI",
			"Catshift/Variant_SideScrolling",
			"Catshift/Variant_SideScrolling/Gameplay",
			"Catshift/Variant_SideScrolling/AI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
