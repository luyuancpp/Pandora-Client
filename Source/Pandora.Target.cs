// Copyright Pandora. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class PandoraTarget : TargetRules
{
	public PandoraTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "Pandora" });
	}
}