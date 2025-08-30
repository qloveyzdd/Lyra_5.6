// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// LYRAGAME_API 是一个宏，用于控制符号的导出/导入。这是虚幻引擎模块系统的一部分，用于：
// 在构建 DLL 时，将标记的类、函数或变量导出，使其他模块可以使用它们
// 在使用 DLL 时，正确导入这些符号
// 与普通的 DECLARE_LOG_CATEGORY_EXTERN 相比，添加 LYRAGAME_API 的主要区别是：
// 模块间可见性：使日志类别可以在不同模块间共享
// DLL导出/导入：确保在使用动态链接库时正确处理符号
// 项目范围使用：允许在整个项目中使用同一个日志类别
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyra, Log, All);
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraExperience, Log, All);
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraAbilitySystem, Log, All);
LYRAGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLyraTeams, Log, All);

//传递一个上下文对象，来判断当前的世界是客户端还是服务端
//该函数在LyraExperienceManagerComponent.h中调用，用来打印Experience的加载日志，传递的对象是GameState，该对象具有网络同步的属性
FString GetClientServerContextString(UObject* ContextObject = nullptr);