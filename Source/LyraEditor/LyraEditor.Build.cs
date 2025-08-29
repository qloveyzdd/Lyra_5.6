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

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				// UE 基础类型（字符串、容器等）。
				"Core",
				// UObject 系统（反射、序列化）。
				"CoreUObject",
				// 引擎核心功能（材质、粒子等）。
				"Engine",
				// 编辑器扩展框架（自定义资产编辑器等）。
				"EditorFramework",
				// 虚幻编辑器核心功能（关卡编辑、细节面板等）。
				"UnrealEd",
				// 物理系统基础支持。
				"PhysicsCore",
				// 游戏标签（GameplayTags）的编辑器支持（如标签管理界面）。
				"GameplayTagsEditor",
				// 游戏任务系统的编辑器工具。
				"GameplayTasksEditor",
				// 游戏技能系统（GA）的运行时逻辑。
				"GameplayAbilities",
				// GA 系统的编辑器工具（如技能蓝图编辑）。
				"GameplayAbilitiesEditor",
				// 开发工作室的遥测数据收集（用于分析开发行为）。
				"StudioTelemetry",
				// Lyra 游戏本身的运行时模块（依赖游戏逻辑）。
				"LyraGame",
			});

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

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				// 输入设备（键盘、鼠标）支持。
				"InputCore",
				// UE 的 UI 框架（编辑器界面元素）。
				"Slate",
				"SlateCore",
				// 编辑器工具栏和菜单扩展。
				"ToolMenus",
				// 编辑器UI样式（图标、字体等）。
				"EditorStyle",
				// 数据校验工具（防止无效资产）。
				"DataValidation",
				// 编辑器日志系统。
				"MessageLog",
				// 项目管理（插件、游戏路径等）。
				"Projects",
				// 开发者工具配置。
				"DeveloperToolSettings",
				// 资产集合管理（分类、标签等）。
				"CollectionManager",
				// 版本控制集成（Git、Perforce等）。
				"SourceControl",
				// Chaos 物理引擎支持。
				"Chaos",

				"HttpServer",
			});

		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			});

		//外部RPC框架的基本设置
		//在正式发布版本中，框架中的功能将被删除，以消除潜在的安全漏洞
		PrivateDependencyModuleNames.Add("ExternalRpcRegistry");
		if (Target.Configuration == UnrealTargetConfiguration.Shipping)
		{
			PublicDefinitions.Add("WITH_RPC_REGISTRY=0");
			PublicDefinitions.Add("WITH_HTTPSERVER_LISTENERS=0");
		}
		else
		{
			// HTTP 服务器功能（仅开发/测试版启用）。
			PrivateDependencyModuleNames.Add("HTTPServer");
			PublicDefinitions.Add("WITH_RPC_REGISTRY=1");
			PublicDefinitions.Add("WITH_HTTPSERVER_LISTENERS=1");
		}

		// 若在测试版本或正式版本中使用DrawDebug函数，则会生成编译错误
		PublicDefinitions.Add("SHIPPING_DRAW_DEBUG_ERROR=1");

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}