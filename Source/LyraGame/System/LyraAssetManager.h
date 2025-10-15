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

	//TSoftObjectPtr是通用FSoftObjectPtr的模板化封装类，可用于UProperties中，

	//返回由TSoftObjectPtr引用的资产，如果该资产尚未加载，则会同步加载该资产。
	template <typename AssetType>
	static AssetType* GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);

	//返回由TSoftClassPtr指向的子类。如果该资产尚未加载，则会同步进行加载操作。

	//返回由TSoftClassPtr指向的子类，如果该资产尚未加载，则会同步进行加载操作。
	template <typename AssetType>
	static TSubclassOf<AssetType>* GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory = true);


	//记录当前由资产管理器加载并跟踪的所有资产信息
	//可以通过命令行调用
	static LYRAGAME_API void DumpLoadedAssets();

	
	//获取游戏数据
	LYRAGAME_API const class ULyraGameData& GetGameData();

	//获取默认的玩家数据
	const ULyraPawnData* GetDefaultPawnData() const;

protected:
	//获取或加载指定的游戏数据
	template <typename GameDataClass>
	const GameDataClass& GetOrLoadTypedGameData(const TSoftObjectPtr<GameDataClass>& DataPath)
	{
		//如果已经缓存了直接读取
		if (const TObjectPtr<UPrimaryDataAsset>* pResult = GameDataMap.Find(GameDataClass::StaticClass()))
		{
			return *CastChecked<GameDataClass>(*pResult);
		}
		//如有需要则进行阻塞式加载
		return *CastChecked<GameDataClass>(LoadGameDataOfClass(
			GameDataClass::StaticClass(),
			DataPath,
			GameDataClass::StaticClass()->GetFName()));
	}

	//同步加载资产
	static LYRAGAME_API UObject* SynchronousLoadAsset(const FSoftClassPath& AssetPath);

	//读取命令行参数，是否应该打印资产加载日志
	static LYRAGAME_API bool ShouldLogAssetLoads();

	//一种线程安全的添加已加载资源到内存中的方法
	LYRAGAME_API void AddLoadedAsset(const UObject* Asset);

	//开始初始加载，由”初始化对象引用“函数调用
	virtual void StartInitialLoading() override;

	//在PIE开始之前被调用，会刷新资源目录，并且可以在此处重写以实现预加载资源
#if WITH_EDITOR
	LYRAGAME_API virtual void PreBeginPIE(bool bStartSimulate) override;
#endif

	//一种主要的资产类型，内部以FName格式表示，并且可以自动进行双向转换
	//此设置的存在是为了让蓝图API能够明白者不是一个普通的FName类型
	LYRAGAME_API UPrimaryDataAsset* LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass,
	                                                    const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath,
	                                                    FPrimaryAssetType PrimaryAssetType);

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
	TArray<struct FLyraAssetManagerStartupJob> StartupJobs;

private:
	//资源已由资源管理器加载并进行跟踪
	UPROPERTY()
	TSet<TObjectPtr<const UObject>> LoadedAssets;

	//用于在修改加载资源列表时进行范围锁定
	FCriticalSection LoadedAssetsCritical;
};

template <typename AssetType>
AssetType* ULyraAssetManager::GetAsset(const TSoftObjectPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	AssetType* LoadedAsset = nullptr;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedAsset = Cast<AssetType>(SynchronousLoadAsset(AssetPath.ToString()));
		ensureAlwaysMsgf(LoadedAsset, TEXT("Failed to load asset [%s]"), *AssetPath.ToString());
	}

	if (LoadedAsset && bKeepInMemory)
	{
		//已添加至已加载资源列表
		Get().AddLoadedAsset(Cast<UObject>(LoadedAsset));
	}

	return LoadedAsset;
}

template <typename AssetType>
TSubclassOf<AssetType>* ULyraAssetManager::GetSubclass(const TSoftClassPtr<AssetType>& AssetPointer, bool bKeepInMemory)
{
	TSubclassOf<AssetType> LoadedSubclass;

	const FSoftObjectPath& AssetPath = AssetPointer.ToSoftObjectPath();

	if (AssetPath.IsValid())
	{
		LoadedSubclass = AssetPointer.Get();
		if (!LoadedSubclass)
		{
			LoadedSubclass = Cast<AssetType>(SynchronousLoadAsset(AssetPath.ToString()));
			ensureAlwaysMsgf(LoadedSubclass, TEXT("Failed to load asset class [%s]"), *AssetPath.ToString());
		}

		if (LoadedSubclass && bKeepInMemory)
		{
			//已添加至已加载资源列表
			Get().AddLoadedAsset(Cast<UObject>(LoadedSubclass));
		}
	}

	return LoadedSubclass;
}
