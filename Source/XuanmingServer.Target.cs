// Copyright Xuanming. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class XuanmingServerTarget : TargetRules
{
	public XuanmingServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "Xuanming" });

		// Dedicated Server 优化
		bUseLoggingInShipping = true;
	}
}
