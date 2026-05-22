// Copyright Xuanming. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class XuanmingEditorTarget : TargetRules
{
	public XuanmingEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "Xuanming" });
	}
}
