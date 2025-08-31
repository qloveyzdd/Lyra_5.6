// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Editor/UnrealEdEngine.h"
#include "LyraEditorEngine.generated.h"

UCLASS()
class ULyraEditorEngine : public UUnrealEdEngine
{
	GENERATED_BODY()

public:
	ULyraEditorEngine(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void Init(IEngineLoop* InEngineLoop) override;
	virtual void Start() override;
	virtual void Tick(float DeltaSeconds, bool bIdleMode) override;

	// 这是一个虚幻引擎中游戏实例类的虚函数，用于在创建 PIE (Play In Editor，在编辑器中游戏) 实例之前进行准备工作。这个函数在多人游戏测试和编辑器中游戏功能中扮演着重要角色。
	// 返回类型: FGameInstancePIEResult
	// FGameInstancePIEResult 是一个结构体，用于返回 PIE 实例创建前的准备结果，通常包含：
	// 是否成功
	// 可能的错误信息
	// 其他 PIE 会话相关的设置或状态信息
	// 参数解析
	// const bool bAnyBlueprintErrors
	// 指示当前项目中是否存在蓝图错误
	// 如果为 true，表示存在蓝图错误，可能需要决定是否继续启动 PIE
	// const bool bStartInSpectatorMode
	// 指示是否以观察者模式启动 PIE
	// 观察者模式下，玩家不会直接控制角色，而是以自由相机形式观察游戏世界
	// const float PIEStartTime
	// PIE 会话的启动时间戳
	// 用于计时和日志记录，以及可能的性能分析
	// const bool bSupportsOnlinePIE
	// 指示当前项目是否支持在线多人 PIE 模式
	// 如果为 true，表示可以创建多个 PIE 实例来模拟网络多人游戏
	// int32& InNumOnlinePIEInstances
	// 传入/传出参数，表示要创建的在线 PIE 实例数量
	// 函数可以修改这个值，影响最终创建的 PIE 实例数量
	// PreCreatePIEInstances 在 PIE 启动流程中的位置：
	// 用户在编辑器中点击"Play"按钮
	// 编辑器准备启动 PIE 会话
	// 调用 GameInstance 的 PreCreatePIEInstances
	// 如果成功，创建请求的 PIE 实例
	// 初始化游戏世界和玩家控制器
	// 开始游戏循环
	virtual FGameInstancePIEResult PreCreatePIEInstances(const bool bAnyBlueprintErrors,
	                                                     const bool bStartInSpectatorMode, const float PIEStartTime,
	                                                     const bool bSupportsOnlinePIE,
	                                                     int32& InNumOnlinePIEInstances) override;

private:
	void FirstTickSetup();

	bool bFirstTickSetup = false;
};
