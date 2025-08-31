// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraEditorEngine.h"

#include "Settings/ContentBrowserSettings.h"

ULyraEditorEngine::ULyraEditorEngine(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void ULyraEditorEngine::Init(IEngineLoop* InEngineLoop)
{
	Super::Init(InEngineLoop);
}

void ULyraEditorEngine::Start()
{
	Super::Start();
}

void ULyraEditorEngine::Tick(float DeltaSeconds, bool bIdleMode)
{
	Super::Tick(DeltaSeconds, bIdleMode);
	FirstTickSetup();
}

FGameInstancePIEResult ULyraEditorEngine::PreCreatePIEInstances(const bool bAnyBlueprintErrors,
	const bool bStartInSpectatorMode, const float PIEStartTime, const bool bSupportsOnlinePIE,
	int32& InNumOnlinePIEInstances)
{
	return Super::PreCreatePIEInstances(bAnyBlueprintErrors, bStartInSpectatorMode, PIEStartTime, bSupportsOnlinePIE,
	                                    InNumOnlinePIEInstances);
}

void ULyraEditorEngine::FirstTickSetup()
{
	if (bFirstTickSetup)return;

	bFirstTickSetup = true;
	
	// 1. GetMutableDefault<UContentBrowserSettings>()
	// GetMutableDefault<T>() 是虚幻引擎中的一个模板函数，用于获取指定UObject类型的默认对象的可修改引用。
	// GetMutableDefault：获取可修改的默认对象实例
	// UContentBrowserSettings：内容浏览器设置类，继承自UObject，包含了内容浏览器的各种配置选项
	// 与之相关的还有 GetDefault<T>()，区别在于：
	// GetDefault<T>() 返回只读引用，不能修改设置
	// GetMutableDefault<T>() 返回可修改的引用，可以更改设置
	// 2. SetDisplayPluginFolders(true)
	// 这是 UContentBrowserSettings 类的一个方法，用于设置内容浏览器是否显示插件文件夹。
	// 参数 true：启用插件文件夹显示
	// 参数 false：禁用插件文件夹显示
	// 功能和作用
	// 执行这行代码后，内容浏览器会显示项目中所有插件的内容文件夹。这些文件夹通常包含插件自带的资源，比如模型、材质、蓝图等。
	// 默认行为
	// 默认情况下，虚幻引擎的内容浏览器可能不会显示所有插件文件夹，特别是引擎自带的插件。这是为了保持界面整洁，避免显示过多可能不相关的内容。
	// 启用后的变化
	// 执行这行代码后，内容浏览器会显示：
	// 项目中安装的所有第三方插件的内容
	// 引擎自带的插件内容
	// 自己创建的插件内容
	GetMutableDefault<UContentBrowserSettings>()->SetDisplayPluginFolders(true);
}
