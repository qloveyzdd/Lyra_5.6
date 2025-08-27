// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LyraGame : ModuleRules
{
	public LyraGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;


// 添加公共包含路径：将 "LyraGame" 目录添加到公共包含路径列表中
// 公共包含路径的特点：
// 这些路径中的头文件可以被依赖于此模块的其他模块访问
// 当其他模块包含此模块的公共头文件时，这些路径会被自动添加到包含路径中
// 适用于需要对外暴露的API和接口

		PublicIncludePaths.AddRange(
			new string[]
			{
				"LyraGame"
			}
		);


// 添加私有包含路径：这里是一个空数组，表示没有添加任何私有包含路径
// 私有包含路径的特点：
// 这些路径仅在当前模块内部可见
// 其他依赖此模块的模块无法访问这些路径中的头文件
// 适用于模块内部实现细节，不需要对外暴露的代码

		PrivateIncludePaths.AddRange(
			new string[]
			{
			}
		);

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}