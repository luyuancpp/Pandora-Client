// Copyright Xuanming. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class XuanmingClientTarget : TargetRules
{
	public XuanmingClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "Xuanming" });
	}
}
