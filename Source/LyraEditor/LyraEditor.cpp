// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraEditor.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "LyraEditor"

// 用于在源文件中定义一个已经声明过的日志类别。
DEFINE_LOG_CATEGORY(LogLyraEditor)

class FLyraEditorModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
	}

	virtual void ShutdownModule() override
	{
	}
};


// 注册模块：这个宏用于向 Unreal Engine 的模块系统注册一个非主要模块（非 Primary Game Module）。
//
// 参数解释：
// 第一个参数 FLyraEditorModule：指定模块的实现类，这是一个自定义的模块类，负责实现该模块的功能。
// 第二个参数 LyraEditor：模块的名称，通常对应于模块的文件夹名称和项目结构。
// 与 IMPLEMENT_PRIMARY_GAME_MODULE 的区别：
//
// IMPLEMENT_PRIMARY_GAME_MODULE 用于主游戏模块（每个游戏只有一个）
// IMPLEMENT_MODULE 用于所有其他类型的模块（一个游戏可以有多个）
IMPLEMENT_MODULE(FLyraEditorModule, LyraEditor);

#undef LOCTEXT_NAMESPACE