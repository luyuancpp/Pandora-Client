// Copyright Xuanming. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class XuanmingTarget : TargetRules
{
	public XuanmingTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "Xuanming" });
	}
}
