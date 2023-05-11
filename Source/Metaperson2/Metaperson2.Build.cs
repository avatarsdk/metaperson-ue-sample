// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Metaperson2 : ModuleRules
{
	public Metaperson2(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
