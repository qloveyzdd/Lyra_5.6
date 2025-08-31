// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraEditorEngine.h"

#include "Framework/Notifications/NotificationManager.h"
#include "GameModes/LyraWorldSettings.h"
#include "Settings/ContentBrowserSettings.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "LyraEditor"

ULyraEditorEngine::ULyraEditorEngine(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
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
                                                                const bool bStartInSpectatorMode,
                                                                const float PIEStartTime, const bool bSupportsOnlinePIE,
                                                                int32& InNumOnlinePIEInstances)
{
	// 这段代码的目的是确保前端部分在编辑器中运行时使用独立网络模式，而不是其他网络模式（如客户端-服务器模式）。
	// 如果检测到使用了非独立模式，它会显示一个通知，告知用户系统正在强制切换到独立模式。
	if (const ALyraWorldSettings* LyraWorldSettings = Cast<ALyraWorldSettings>(EditorWorld->GetWorldSettings()))
	{
		if (LyraWorldSettings->ForceStandaloneNetMode)
		{
			//网络模式的枚举值
			EPlayNetMode OutPlayNetMode;
			// PlaySessionRequest（或完整名称可能是FRequestPlaySessionParams）是一个包含了启动游戏会话所需的各种参数和设置的结构体或类。
			// 当你在虚幻编辑器中点击"Play"按钮时，引擎会创建一个这样的请求对象来配置游戏会话的运行方式。
			PlaySessionRequest->EditorPlaySettings->GetPlayNetMode(OutPlayNetMode);
			if (OutPlayNetMode != PIE_Standalone)
			{
				PlaySessionRequest->EditorPlaySettings->SetPlayNetMode(OutPlayNetMode);

				// FNotificationInfo 是虚幻引擎(Unreal Engine)中用于创建和配置通知消息的类。
				// 这些通知通常显示在编辑器界面的右下角，用于向用户提供各种信息、警告或错误消息。
				// 创建了一个通知，内容为 "Forcing NetMode: Standalone for the Frontend"
				// 设置通知显示时间为 2 秒
				// 使用 FSlateNotificationManager 将通知添加到界面上显示
				FNotificationInfo Info(LOCTEXT("ForcingStandaloneForFrontend",
				                               "Forcing NetMode: Standalone for the Frontend"));
				Info.ExpireDuration = 2.0f;
				FSlateNotificationManager::Get().AddNotification(Info);
			}
		}
	}

	FGameInstancePIEResult Result = Super::PreCreatePIEInstances(bAnyBlueprintErrors, bStartInSpectatorMode,
	                                                             PIEStartTime, bSupportsOnlinePIE,
	                                                             InNumOnlinePIEInstances);
	return Result;
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


#undef LOCTEXT_NAMESPACE
