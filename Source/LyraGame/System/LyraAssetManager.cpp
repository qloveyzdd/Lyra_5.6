// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraAssetManager.h"

#include "LyraAssetManagerStartupJob.h"
#include "LyraGameData.h"
#include "LyraLogChannels.h"
#include "Editor/Kismet/Internal/Blueprints/BlueprintDependencies.h"
#include "EditorState/EditorState.h"

//一个约定的Bundles的命名
const FName FLyraBundles::Equipped("Equipped");

//通过命令行调用这个方法 打印加载的资产
static FAutoConsoleCommand CVarDumpLoadedAssets(
	TEXT("Lyra,DumpLoadedAssets"),
	TEXT("Shows all assets that were loaded via the asset manager and are currently in memory."),
	FConsoleCommandDelegate::CreateStatic(ULyraAssetManager::DumpLoadedAssets)
);

//添加一个任务到容器里面，这个任务就是传递过来的JobFunc，并用Lambda包了一层
//入参是函数，函数权重
//在Lambda包了一层函数的函数名作为字符串传递作为任务名
#define STARTUP_JOB_WEIGHTED(JobFunc,JobWeight)\
StartupJobs.Add(\
	FLyraAssetManagerStartupJob(\
		#JobFunc,\
		[this](const FLyraAssetManagerStartupJob& StartupJob,TSharedPtr<FStreamableHandle>& LoadHandle){ JobFunc;},\
		JobWeight))

#define STARTUP_JOB(JobFunc) STARTUP_JOB_WEIGHTED(JobFunc,1.0f)


ULyraAssetManager::ULyraAssetManager()
{
	DefaultPawnData = nullptr;
}

ULyraAssetManager& ULyraAssetManager::Get()
{
	check(GEngine);

	if (ULyraAssetManager* Singleton = Cast<ULyraAssetManager>(GEngine->AssetManager))
	{
		return *Singleton;
	}
	UE_LOG(LogLyra, Fatal,
	       TEXT("Invalid AssetManagerClassName in DefaultEngine.ini. It must be set to LyraAssetManager!"));

	return *NewObject<ULyraAssetManager>();
}

void ULyraAssetManager::DumpLoadedAssets()
{
	UE_LOG(LogLyra, Log, TEXT("=========== Start Dumping Loaded Assets ==========="));

	for (const UObject* LoadedAsset : Get().LoadedAssets)
	{
		UE_LOG(LogLyra, Log, TEXT("	%s"), *GetNameSafe(LoadedAsset));
	}

	UE_LOG(LogLyra, Log, TEXT("... %d assets in loaded pool"), Get().LoadedAssets.Num());
	UE_LOG(LogLyra, Log, TEXT("=========== Finish Dumping Loaded Assets ==========="));
}

const ULyraPawnData* ULyraAssetManager::GetDefaultPawnData() const
{
	return GetAsset(DefaultPawnData);
}

UObject* ULyraAssetManager::SynchronousLoadAsset(const FSoftClassPath& AssetPath)
{
	if (AssetPath.IsValid())
	{
		TUniquePtr<FScopeLogTime> LogTimePtr;
		//通过命名行确认是否需要打印该日志
		if (ShouldLogAssetLoads())
		{
			LogTimePtr = MakeUnique<FScopeLogTime>(
				*FString::Printf(TEXT("Synchronously loaded asset [%s]"), *AssetPath.ToString()),
				nullptr, FScopeLogTime::ScopeLog_Seconds);
		}
		if (UAssetManager::IsInitialized())
		{
			return UAssetManager::GetStreamableManager().LoadSynchronous(AssetPath, false);
		}
	}
	return nullptr;
}

bool ULyraAssetManager::ShouldLogAssetLoads()
{
	static bool bLogAssetLoads = FParse::Param(FCommandLine::Get(),TEXT("LogAssetLoads"));
	return bLogAssetLoads;
}

void ULyraAssetManager::AddLoadedAsset(const UObject* Asset)
{
	if (ensureAlways(Asset))
	{
		FScopeLock LoadedAssetsLock(&LoadedAssetsCritical);
		LoadedAssets.Add(Asset);
	}
}

void ULyraAssetManager::StartInitialLoading()
{
	//专门用于UE启动阶段性能分析的工具，适合优化游戏启动事件
	SCOPED_BOOT_TIMING("ULyraAssetManager::StartInitialLoading");

	//这个会完成所有的扫描工作，即便加载操作被延迟，现在也需要执行此操作
	Super::StartInitialLoading();

	//申请一个任务 权重为1，去确认我们的GameplayCueManager是否正常
	// InitializeGameplayCueManager();
	//
	// auto Lambda = [this](const FLyraAssetManagerStartupJob&,TSharedPtr<FStreamableHandle>&)
	// {
	// 	this->InitializeGameplayCueManager();
	// };
	// FLyraAssetManagerStartupJob StartupJob(TEXT("TestJob"),Lambda,10.0f);
	// StartupJobs.Add(StartupJob);
	STARTUP_JOB(InitializeGameplayCueManager());

	//加载基础游戏数据文件
	STARTUP_JOB_WEIGHTED(GetGameData(), 25.0f);

	//执行所有已排队的启动任务
	DoAllStartupJobs();
}

void ULyraAssetManager::PreBeginPIE(bool bStartSimulate)
{
	Super::PreBeginPIE(bStartSimulate);

	{
		FScopedSlowTask SlowTask(0,NSLOCTEXT("LyraEditor","BeginLoadingPIEData","Loading PIE Data"));
		const bool bShowCancelButton = false;
		const bool bAllowInPIE = true;
		SlowTask.MakeDialog(bShowCancelButton,bAllowInPIE);

		//这里没有就崩溃
		const ULyraGameData& LocalGameDataCommon = GetGameData();

		//有意安排在获取游戏数据之后进行，以避免将游戏数据的处理时间计入此计时器
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("PreBegingPIE asset preloading complete"),nullptr);

		//可以预先加载我们在此处所使用体验所需的任何其他内容
		//可以参考我们在EditorEngine中的操作
	}
}

UPrimaryDataAsset* ULyraAssetManager::LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass,
                                                          const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath,
                                                          FPrimaryAssetType PrimaryAssetType)
{
	UPrimaryDataAsset* Asset = nullptr;

	//用作性能分析
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Loading GameData Object"), STAT_GameDate, STATGROUP_LoadTime);

	//加载路径不能为空
	if (!DataClassPath.IsNull())
	{
#if WITH_EDITOR
		//一个表示工作量的划分单元，该单元被划分为若干部分
		//在每一个函数的顶部使用一个范围块，以便向缓慢操作的用户提供准确的进度反馈
		FScopedSlowTask SlowTask(0, FText::Format(
			                         NSLOCTEXT("LyraEditor", "BeginLoadingGameDataTask", "Loading GameData {0}"),
			                         FText::FromName(DataClass->GetFName())));

		const bool bShowCancelButton = false;

		const bool bAllowInPIE = true;

		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);
#endif

		UE_LOG(LogLyra, Log, TEXT("Loading GameData: %s ..."), *DataClassPath.ToString());

		//指定计时打印日志的宏
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("	... GameData loaded!"), nullptr);

		//在编辑器中可以对该函数进行递归操作，因为它是根据需求从“PostLoad”阶段被调用的，所以在此情况下必须强制进行主资产的同步加载，并对其余部分进行异步加载。

		if (GIsEditor)
		{
			//先加载主资产
			Asset = DataClassPath.LoadSynchronous();

			//加载指定类型的全部资源，适用于烘培操作
			LoadPrimaryAssetsWithType(PrimaryAssetType);
		}
		else
		{
			TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(PrimaryAssetType);

			if (Handle.IsValid())
			{
				//直到所需资源加载完成才会停止，这会将所请求的资源推至优先级列表的首位
				//但不会清理所有异步加载操作，通常会比调用LoadObject函数完成的更快。
				Handle->WaitUntilComplete(0.0f, false);

				//返回所请求资产列表中的首个资产（若该资产已成功加载），若该资产加载失败，则此操作将失败
				Asset = Cast<UPrimaryDataAsset>(Handle->GetLoadedAsset());
			}
		}
	}

	if (Asset)
	{
		//添加到缓存中 Key值是类类型 value值是指针
		GameDataMap.Add(DataClass, Asset);
	}
	else
	{
		//任何游戏数据资产的加载都不得失败，否则将会导致一些不易诊断的软性故障
		UE_LOG(LogLyra, Fatal,
		       TEXT(
			       "Failed to load GameData asset at %s. Type %s. This is not recoverable and likely means you do not have the correct data to run %s."
		       ), *DataClassPath.ToString(), *PrimaryAssetType.ToString(), FApp::GetProjectName());
	}


	return Asset;
}

UPrimaryDataAsset* ULyraAssetManager::LoadGameDataOfClass(TSubclassOf<UPrimaryDataAsset> DataClass,
                                                          const TSoftObjectPtr<UPrimaryDataAsset>& DataClassPath,
                                                          FPrimaryAssetType PrimaryAssetType)
{
	UPrimaryDataAsset* Asset = nullptr;

	//用作性能分析
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("Loading GameData Object"), STAT_GameDate, STATGROUP_LoadTime);

	//加载路径不能为空
	if (!DataClassPath.IsNull())
	{
#if WITH_EDITOR
		//一个表示工作量的划分单元，该单元被划分为若干部分
		//在每一个函数的顶部使用一个范围块，以便向缓慢操作的用户提供准确的进度反馈
		FScopedSlowTask SlowTask(0, FText::Format(
			                         NSLOCTEXT("LyraEditor", "BeginLoadingGameDataTask", "Loading GameData {0}"),
			                         FText::FromName(DataClass->GetFName())));

		const bool bShowCancelButton = false;

		const bool bAllowInPIE = true;

		SlowTask.MakeDialog(bShowCancelButton, bAllowInPIE);
#endif

		UE_LOG(LogLyra, Log, TEXT("Loading GameData: %s ..."), *DataClassPath.ToString());

		//指定计时打印日志的宏
		SCOPE_LOG_TIME_IN_SECONDS(TEXT("	... GameData loaded!"), nullptr);

		//在编辑器中可以对该函数进行递归操作，因为它是根据需求从“PostLoad”阶段被调用的，所以在此情况下必须强制进行主资产的同步加载，并对其余部分进行异步加载。

		if (GIsEditor)
		{
			//先加载主资产
			Asset = DataClassPath.LoadSynchronous();

			//加载指定类型的全部资源，适用于烘培操作
			LoadPrimaryAssetsWithType(PrimaryAssetType);
		}
		else
		{
			TSharedPtr<FStreamableHandle> Handle = LoadPrimaryAssetsWithType(PrimaryAssetType);

			if (Handle.IsValid())
			{
				//直到所需资源加载完成才会停止，这会将所请求的资源推至优先级列表的首位
				//但不会清理所有异步加载操作，通常会比调用LoadObject函数完成的更快。
				Handle->WaitUntilComplete(0.0f, false);

				//返回所请求资产列表中的首个资产（若该资产已成功加载），若该资产加载失败，则此操作将失败
				Asset = Cast<UPrimaryDataAsset>(Handle->GetLoadedAsset());
			}
		}
	}

	if (Asset)
	{
		//添加到缓存中 Key值是类类型 value值是指针
		GameDataMap.Add(DataClass, Asset);
	}
	else
	{
		//任何游戏数据资产的加载都不得失败，否则将会导致一些不易诊断的软性故障
		UE_LOG(LogLyra, Fatal,
		       TEXT(
			       "Failed to load GameData asset at %s. Type %s. This is not recoverable and likely means you do not have the correct data to run %s."
		       ), *DataClassPath.ToString(), *PrimaryAssetType.ToString(), FApp::GetProjectName());
	}


	return Asset;
}

void ULyraAssetManager::DoAllStartupJobs()
{
	//专门用于UE启动阶段性能分析的工具，适合优化游戏启动事件
	SCOPED_BOOT_TIMING("ULyraAssetManager::DoAllStartupJobs");

	//记录所有任务的开始时间
	const double AllStartupJobStartTime = FPlatformTime::Seconds();

	//检查一下此可执行文件是否为独立服务器进程启动，且不应加载仅客户端使用的数据
	//可以通过在启动时使用“-server”参数来设置此选项为“真，但在单进程“PIE”模式下则为“假”
	//该功能不应用于游戏或网络用途，而应检查“NM_DedicatedServer"选项。

	//如果时运行在专属服务器中
	if (IsRunningDedicatedServer())
	{
		//无需定期提供进程更新，直接运行这些任务即可
		for (const FLyraAssetManagerStartupJob& StartupJob : StartupJobs)
		{
			StartupJob.DoJob();
		}
	}
	else
	{
		if (StartupJobs.Num() > 0)
		{
			//总的进度
			float TotalJobValue = 0.0f;
			for (const FLyraAssetManagerStartupJob& StartupJob : StartupJobs)
			{
				//权重相加
				TotalJobValue += StartupJob.JobWeight;
			}

			//累计推进的进度
			float AccumulatedJobValue = 0.0f;
			for (FLyraAssetManagerStartupJob& StartupJob : StartupJobs)
			{
				//拿到当前任务总权重
				const float JobValue = StartupJob.JobWeight;

				//绑定这个任务的进度更行
				StartupJob.SubstepProgressDelegate.BindLambda(
					[this,AccumulatedJobValue,JobValue,TotalJobValue](float NewProgress)
					{
						//当前任务的进度
						const float SubstepAdjustment = FMath::Clamp(NewProgress, 0.0f, 1.0f) * JobValue;

						//（之前已完成的任务权重+当前完成的权重）/总的任务权重
						const float OverallPercentWithSubstep = (AccumulatedJobValue + SubstepAdjustment) /
							TotalJobValue;

						UpdateInitialGameContentLoadPercent(OverallPercentWithSubstep);
					});

				// 执行任务，阻塞直到任务完成
				StartupJob.DoJob();

				StartupJob.SubstepProgressDelegate.Unbind();

				AccumulatedJobValue += JobValue;

				//更新界面
				UpdateInitialGameContentLoadPercent(AccumulatedJobValue / TotalJobValue);
			}
		}
		else
		{
			//更新界面
			UpdateInitialGameContentLoadPercent(1.0f);
		}
	}
	//清空任务容器
	StartupJobs.Empty();

	//日志，所有启动任务执行完毕
	UE_LOG(LogLyra, Display, TEXT("All startup jobs took %.2f seconds to complete"),
	       FPlatformTime::Seconds()-AllStartupJobStartTime);
}

void ULyraAssetManager::InitializeGameplayCueManager()
{
	//专门用于UE启动阶段性能分析的工具，适合优化游戏启动事件
	SCOPED_BOOT_TIMING("ULyraAssetManager::InitializeGameplayCueManager");
}

void ULyraAssetManager::UpdateInitialGameContentLoadPercent(float GameContentPercent)
{
	//可以将此内容转至早期启动加载界面
}

const class ULyraGameData& ULyraAssetManager::GetGameData()
{
	return GetOrLoadTypedGameData<ULyraGameData>(LyraGameDataPath);
}
