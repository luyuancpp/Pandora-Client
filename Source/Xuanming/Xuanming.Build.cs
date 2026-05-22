// Copyright Xuanming. All Rights Reserved.

using UnrealBuildTool;

public class Xuanming : ModuleRules
{
	public Xuanming(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"NetCore",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"OnlineSubsystemNull"
		});

		// 启用网络回放支持
		PublicDefinitions.Add("WITH_NETWORK_REPLAY=1");
	}
}
