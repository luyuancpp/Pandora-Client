// Copyright Pandora. All Rights Reserved.

using UnrealBuildTool;

public class Pandora : ModuleRules
{
	public Pandora(ReadOnlyTargetRules Target) : base(Target)
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
			"HTTP",
			"UMG",
			"Slate",
			"SlateCore",
			"FieldNotification",
			"ModelViewViewModel",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"OnlineSubsystemNull"
		});

		// 启用网络回放支持
		PublicDefinitions.Add("WITH_NETWORK_REPLAY=1");
	}
}
