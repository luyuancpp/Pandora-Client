// Copyright Pandora. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class PandoraServerTarget : TargetRules
{
	public PandoraServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[] { "Pandora" });

		// Dedicated Server 优化
		bUseLoggingInShipping = true;
	}
}