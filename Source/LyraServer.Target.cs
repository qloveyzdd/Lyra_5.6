// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LyraServerTarget : TargetRules
{
	public LyraServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;

		ExtraModuleNames.AddRange(new string[] { "LyraGame" });
		
		LyraGameTarget.ApplySharedLyraTargetSettings(this);

		//是否打开测试/发布版本的检查（断言）。
		bUseChecksInShipping = true;
	}
}