// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraAssetManager.h"

#include "LyraAssetManagerStartupJob.h"
#include "LyraGameData.h"
#include "LyraLogChannels.h"

//一个约定的Bundles的命名
const FName FLyraBundles::Equipped("Equipped");

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

const ULyraPawnData* ULyraAssetManager::GetDefaultPawnData() const
{
	return nullptr;
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
	return *NewObject<ULyraGameData>();
}
