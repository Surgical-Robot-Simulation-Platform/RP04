// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RP04 : ModuleRules
{
	public RP04(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new[] { "AnimGraph" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "Sockets", "Networking", "AnimGraphRuntime", "Json",
			"JsonUtilities"
		});
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}