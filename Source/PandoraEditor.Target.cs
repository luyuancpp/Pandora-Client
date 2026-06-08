// Copyright Pandora. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class PandoraEditorTarget : TargetRules
{
	public PandoraEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "Pandora" });
	}
}