// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AdvancedAIDemo : ModuleRules
{
	public AdvancedAIDemo(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate",
			"GameplayDebugger"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"AdvancedAIDemo",
			"AdvancedAIDemo/Variant_Platforming",
			"AdvancedAIDemo/Variant_Platforming/Animation",
			"AdvancedAIDemo/Variant_Combat",
			"AdvancedAIDemo/Variant_Combat/AI",
			"AdvancedAIDemo/Variant_Combat/Animation",
			"AdvancedAIDemo/Variant_Combat/Gameplay",
			"AdvancedAIDemo/Variant_Combat/Interfaces",
			"AdvancedAIDemo/Variant_Combat/UI",
			"AdvancedAIDemo/Variant_SideScrolling",
			"AdvancedAIDemo/Variant_SideScrolling/AI",
			"AdvancedAIDemo/Variant_SideScrolling/Gameplay",
			"AdvancedAIDemo/Variant_SideScrolling/Interfaces",
			"AdvancedAIDemo/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
