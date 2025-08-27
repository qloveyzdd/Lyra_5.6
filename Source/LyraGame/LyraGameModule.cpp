// Copyright Epic Games, Inc. All Rights Reserved.

#include "Modules/ModuleManager.h"


// FLyraGameModule
//  
// FDefaultGameModuleImpl 是 Unreal Engine 提供的基础游戏模块实现类
// 模块系统允许游戏代码被组织成可加载/卸载的单元
// 两个重写的虚函数：
//
// StartupModule() - 当模块被加载时调用，用于初始化模块资源
// ShutdownModule() - 当模块被卸载时调用，用于清理资源
class FLyraGameModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};

// 声明主游戏模块：在 Unreal Engine 项目中，必须有一个主游戏模块，这个宏用来指定哪个模块是主模块。
//
// 参数解释：
// 第一个参数 FDefaultGameModuleImpl：指定模块的实现类。在这种情况下，使用的是自定义的 FLyraGameModule。
// 第二个参数 LyraGame：指定模块名称。
// 第三个参数 "LyraGame"：模块的字符串名称，通常用于日志和调试。
//
// 功能实现：
// 这个宏会生成必要的代码来注册模块
// 创建模块实例
// 处理模块的加载和卸载
// 将模块与 Unreal Engine 的模块系统集成

IMPLEMENT_PRIMARY_GAME_MODULE(FLyraGameModule, LyraGame, "LyraGame");
