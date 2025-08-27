// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LyraEditor : ModuleRules
{
	public LyraEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				"LyraEditor"
			}
		);

		PrivateIncludePaths.AddRange(
			new string[]
			{
			}
		);


		// 添加公共依赖模块：声明该模块依赖于 "Core"、"CoreUObject"、"Engine" 和 "LyraGame" 这几个模块。
		//
		// 公共依赖的特点：
		// 这些依赖关系会传递给依赖于当前模块的其他模块
		// 任何依赖于当前模块的模块也会自动依赖这些公共依赖
		// 适用于需要在模块的公共接口中使用的类型和功能
		//
		// 具体依赖说明：
		// Core：提供基础功能，如内存管理、字符串处理等
		// CoreUObject：Unreal 对象系统的核心
		// Engine：游戏引擎的主要功能
		// LyraGame：Lyra 游戏的主要游戏模块，包含游戏逻辑

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "LyraGame" });

		// 添加私有依赖模块：声明该模块私下依赖于 "InputCore"、"Slate" 和 "SlateCore" 这几个模块。
		//
		// 私有依赖的特点：
		// 这些依赖关系不会传递给依赖于当前模块的其他模块
		// 仅在当前模块的实现中使用
		// 适用于模块内部实现需要但不需要在公共接口中暴露的功能
		//
		// 具体依赖说明：
		// InputCore：处理输入相关的功能
		// Slate：Unreal 的 UI 系统
		// SlateCore：Slate UI 系统的核心组件

		PrivateDependencyModuleNames.AddRange(new string[] { "InputCore", "Slate", "SlateCore" });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}