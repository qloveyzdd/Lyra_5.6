// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

// 这个类继承自 TargetRules，用于配置 Lyra 游戏项目的构建目标。每个 Unreal Engine 项目通常会有多个目标文件，如游戏目标、编辑器目标、服务器目标等。
public class LyraGameTarget : TargetRules
{
	public LyraGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_3;

		// 添加 "LyraGame" 作为此目标需要构建的额外模块
		// 	这表示 LyraGame 是主游戏模块，包含游戏的核心功能
		// 	当构建这个目标时，会自动包含 LyraGame 模块
		ExtraModuleNames.Add("LyraGame");
	}
}