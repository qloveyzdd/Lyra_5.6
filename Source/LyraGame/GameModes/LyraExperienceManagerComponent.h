// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "LoadingProcessInterface.h"
#include "LyraExperienceDefinition.h"
#include "Components/GameStateComponent.h"

//Experience 各个阶段的加载代理，是一个多播
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLyraExperienceLoaded, const ULyraExperienceDefinition* /*Experience*/)

//用来表明Experience的加载状态的枚举，整个状态比较复杂，同时包含异步和同步的过程
enum class ELyraExperienceLoadedState
{
	Unloaded,
	Loading,
	LoadingGameFeatures,
	LoadingChaosTestingDelay,
	ExecutingActions,
	Loaded,
	Deactivating
};

//管理体验的游戏状态组件，非常重要
//它在GameState的构造函数种创建，开启了网络同步的功能用来传递Experience
// final - 表示这个类不能被进一步继承。
UCLASS(MinimalAPI)
class ULyraExperienceManagerComponent final : public UGameStateComponent, public ILoadingProcessInterface
{
	GENERATED_BODY()

public:
	LYRAGAME_API ULyraExperienceManagerComponent(
		const FObjectInitializer& InObjectInitializer = FObjectInitializer::Get());

	//结束此组件的游戏进程
	//仅在bHasBegunPlay为真时，从AActor::EndPlay中调用此函数
	LYRAGAME_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	//检查该对象是否实现了该接口，如果实现了则询问是否当前应显示加载界面
	LYRAGAME_API virtual bool ShouldShowLoadingScreen(FString& OutReason) const override;

	//尝试设置当前的体验，可以是用户界面体验，也可以是游戏体验
	LYRAGAME_API void SetCurrentExperience(FPrimaryAssetId ExperienceId);
	
	//确保在体验加载完成后调用该委托函数。
	//而不会在其他函数被嗲用之前再调用它。
	//但若体验已加载完成，则会立即调用该委托函数
	LYRAGAME_API void CallOrRegister_OnExperienceLoaded_HighPriority(FOnLyraExperienceLoaded::FDelegate&& Delegate);
	//确保在体验加载完成后调用委托函数
	//但若体验已加载完成，则会立即调用该委托函数
	LYRAGAME_API void CallOrRegister_OnExperienceLoaded(FOnLyraExperienceLoaded::FDelegate&& Delegate);
	//确保在体验加载完成后调用委托函数
	//但若体验已加载完成，则会立即调用该委托函数
	LYRAGAME_API void CallOrRegister_OnExperienceLoaded_LowPriority(FOnLyraExperienceLoaded::FDelegate&& Delegate);

	//若体验已完全加载，则返回true
	LYRAGAME_API bool IsExperienceLoaded() const;

private:
	//由网络同步过来的Experience从而启动加载，这是客户端的Experience加载启动
	UFUNCTION()
	void OnRep_CurrentExperience();

	//开始加载
	void StartExperienceLoad();

	//加载完成
	void OnExperienceLoadComplete();

private:
	//当前正在使用的体验
	UPROPERTY(ReplicatedUsing=OnRep_CurrentExperience)
	TObjectPtr<const ULyraExperienceDefinition> CurrentExperience;

	//目前体验的工作状态
	ELyraExperienceLoadedState LoadState = ELyraExperienceLoadedState::Unloaded;

	//正在加载的游戏特性插件数
	int32 NumGameFeaturePluginsLoading = 0;

	//游戏特性插件对应的URL数组
	TArray<FString> GameFeaturePluginURLs;

	//观察到的停留数，用于Action计数
	int32 NumObservedPausers = 0;
	//期望的停留数，用于Action计数
	int32 NumExpectedPausers = 0;

	//当体验在其他部分加载完成之前就已经完成加载时会触发此代理
	//例如，那些为常规游戏流程做准备的子系统
	FOnLyraExperienceLoaded OnExperienceLoaded_HighPriority;

	//当体验加载完成时所调用的委托函数
	FOnLyraExperienceLoaded OnExperienceLoaded;

	//当体验加载完成时所调用的委托函数
	FOnLyraExperienceLoaded OnExperienceLoaded_LowPriority;
};
