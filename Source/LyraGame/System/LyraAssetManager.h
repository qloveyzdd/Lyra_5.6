// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Character/LyraPawnData.h"
#include "Engine/AssetManager.h"
#include "LyraAssetManager.generated.h"


//一个约定的Bundles的命名
struct FLyraBundles
{
	static const FName Equipped;
};

//资产管理器的实现，该实现会覆盖原有功能并存储游戏特定类型的数据。
//预计大多数游戏都会希望提供过重写AssetManager类，因为它为游戏特定的加载逻辑提供了一个理想的位置。
//此类通过在DefaultEngine.ini中设置AssetManagerClassName来使用。
UCLASS(MinimalAPI, Config=Game)
class ULyraAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	//构造函数 初始化PawnData
	LYRAGAME_API ULyraAssetManager();

	//返回资产管理器的单例对象
	static LYRAGAME_API ULyraAssetManager& Get();

	//获取游戏数据
	LYRAGAME_API const class ULyraGameData& GetGameData();

	//获取默认的玩家数据
	LYRAGAME_API const ULyraPawnData* GetDefaultPawnData() const;

protected:
	//开始初始加载，由”初始化对象引用“函数调用
	virtual void StartInitialLoading() override;

	//在PIE开始之前被调用，会刷新资源目录，并且可以在此处重写以实现预加载资源
#if WITH_EDITOR
	LYRAGAME_API virtual void PreBeginPIE(bool bStartSimulate) override;
#endif

protected:
	//所需的全局游戏数据资源
	//这里是通过ini配置
	//不好好配置会崩溃
	UPROPERTY(Config)
	TSoftObjectPtr<class ULyraGameData> LyraGameDataPath;

	//已加载的游戏数据版本
	UPROPERTY(Transient)
	TMap<TObjectPtr<UClass>, TObjectPtr<UPrimaryDataAsset>> GameDataMap;

	//当玩家状态中未设置相关数据时，用于生成玩家兵卒的数量
	//这里是通过ini配置
	//不好好配置会崩溃
	UPROPERTY(Config)
	TSoftObjectPtr<class ULyraPawnData> DefaultPawnData;

private:
	//清理“启动任务”数组，处理所有启动工作
	LYRAGAME_API void DoAllStartupJobs();

	//设置能力系统
	LYRAGAME_API void InitializeGameplayCueManager();

	//在加载过程中会定期调用此函数，可用于将状态信息传递给加载界面
	LYRAGAME_API void UpdateInitialGameContentLoadPercent(float GameContentPercent);

	//启动时要执行的任务列表，用于跟踪启动过程的进度。
	TArray<class FLyraAssetManagerStartupJob> StartupJobs;
};
