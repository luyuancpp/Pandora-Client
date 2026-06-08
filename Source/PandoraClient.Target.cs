// Copyright Pandora. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class PandoraClientTarget : TargetRules
{
	public PandoraClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "Pandora" });
	}
}