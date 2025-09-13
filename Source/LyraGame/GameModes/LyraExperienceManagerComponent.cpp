// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraExperienceManagerComponent.h"

#include "GameFeatureAction.h"
#include "GameFeaturesSubsystem.h"
#include "GameFeaturesSubsystemSettings.h"
#include "LyraExperienceActionSet.h"
#include "LyraExperienceManager.h"
#include "LyraLogChannels.h"
#include "Engine/AssetManager.h"
#include "Net/UnrealNetwork.h"

//这是一个测试Experience随机延迟的命令行参数，它可以通过命令行输入读取随机延迟时间的最小值与最大值，并通过这个GetExperienceLoadDelayDuration()去读取到一个随机测试值
namespace LyraConsoleVariables
{
	static float ExperienceLoadRandomDelayMin = 0.0f;

	static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayMin(
		TEXT("Lyra.chaos.ExperienceDelayLoad.MinSecs"),
		ExperienceLoadRandomDelayMin,
		TEXT(
			"This value (in seconds) will be added as a delay of load completion of the experience (along with the random value lyra.chaos.ExperienceDelayLoad.RandomSecs"),
		ECVF_Default
	);

	static float ExperienceLoadRandomDelayRange = 0.0f;

	static FAutoConsoleVariableRef CVarExperienceLoadRandomDelayRange(
		TEXT("Lyra.chaos.ExperienceDelayLoad.RandomSecs"),
		ExperienceLoadRandomDelayRange,
		TEXT(
			"A random amount of time between 0 and this value (in seconds) will be added as a delay of load completion of the experience (along with the fixed value Lyra.chaos.ExperienceDelayLoad.MinSecs)"),
		ECVF_Default
	);

	float GetExperienceLoadDelayDuration()
	{
		return FMath::Max(0.0f, ExperienceLoadRandomDelayMin + FMath::FRand() * ExperienceLoadRandomDelayRange);
	}
}


ULyraExperienceManagerComponent::ULyraExperienceManagerComponent(const FObjectInitializer& InObjectInitializer)
	: Super(InObjectInitializer)
{
	//开启默认网络同步
	SetIsReplicatedByDefault(true);
}

void ULyraExperienceManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULyraExperienceManagerComponent, CurrentExperience);
}

void ULyraExperienceManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//恢复取消加载的任何功能设置

	//关闭游戏特效插件
	for (const FString& PluginURL : GameFeaturePluginURLs)
	{
		//通过我们写的引擎子系统来确认这个插件确实已经所有依赖释放完毕，最终释放这个插件
		if (ULyraExperienceManager::RequestToDeactivatePlugin(PluginURL))
		{
			UGameFeaturesSubsystem::Get().DeactivateGameFeaturePlugin(PluginURL);
		}
	}

	if (LoadState == ELyraExperienceLoadedState::Loaded)
	{
		LoadState = ELyraExperienceLoadedState::Deactivating;

		//确保即便有人注册为暂停者但随即立刻执行操作，我们也不会过早完成转换过程
		NumExpectedPausers = INDEX_NONE;
		NumObservedPausers = 0;

		//反激活并卸载这些操作

		//在上下文中绑定Action结束时要进行的操作
		FGameFeatureDeactivatingContext Context(
			TEXT(""),
			[this](FStringView)
			{
				this->OnActionDeactivationCompleted();
			}
		);

		//制定上下文执行的世界
		const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());
		if (ExistingWorldContext)
		{
			Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
		}

		//取消Action的Lambda
		auto DeactivateListOfActions = [&Context](const TArray<UGameFeatureAction*>& ActionList)
		{
			for (UGameFeatureAction* Action : ActionList)
			{
				if (Action)
				{
					Action->OnGameFeatureDeactivating(Context);
					Action->OnGameFeatureUnregistering();
				}
			}
		};

		//取消Experience中的Actions
		DeactivateListOfActions(CurrentExperience->Actions);

		//取消Experience中的ActionSets的Action
		for (const TObjectPtr<ULyraExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
		{
			if (ActionSet != nullptr)
			{
				DeactivateListOfActions(ActionSet->Actions);
			}
		}

		NumExpectedPausers = Context.GetNumPausers();

		//现在还不支持异步去取消这些操作，所以此处应该是0
		if (NumExpectedPausers > 0)
		{
			UE_LOG(LogLyraExperience, Error,
			       TEXT("Actions that have asynchronous deactivation aren't fully supported yet in Lyra experiences"));
		}

		//所有的Actions都已观测到取消，这里是同步的，所以会执行
		if (NumExpectedPausers == NumObservedPausers)
		{
			OnAllActionsDeactivated();
		}
	}
}

bool ULyraExperienceManagerComponent::ShouldShowLoadingScreen(FString& OutReason) const
{
	if (LoadState != ELyraExperienceLoadedState::Loaded)
	{
		OutReason = TEXT("Experience still loading");
		return true;
	}
	else
	{
		return false;
	}
}

void ULyraExperienceManagerComponent::SetCurrentExperience(FPrimaryAssetId ExperienceId)
{
	//TODO:这里使用的是Lyra项目自定义的资产管理器
	//ULyraAssetManager& AssetManager = ULyraAssetManager::Get()
	UAssetManager& AssetManager = UAssetManager::Get();

	//获取指定主资产类型及名称的FSoftObjectPath.若未找到则返回无效值
	FSoftObjectPath AssetPath = AssetManager.GetPrimaryAssetObject(ExperienceId);

	//尝试加载该资源时，将调用“LoadObject”函数，该函数执行会非常缓慢
	TSubclassOf<ULyraExperienceDefinition> AssetClass = Cast<UClass>(AssetPath.TryLoad());

	check(AssetClass);

	//获取一个类的默认对象
	//在大多数情况下，类的默认对象不应被修改，因此，此放大返回的时一个不可变的指针，如果您需要修改默认对象，请使用GetMutableDefault替代。
	const ULyraExperienceDefinition* Experience = GetDefault<ULyraExperienceDefinition>(AssetClass);

	check(Experience!=nullptr);
	check(CurrentExperience == nullptr);

	//这里由服务器属性同步到客户端进行加载
	CurrentExperience = Experience;

	//开始加载
	StartExperienceLoad();
}

void ULyraExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_HighPriority(
	FOnLyraExperienceLoaded::FDelegate&& Delegate)
{
	//如果是已经加载了就直接执行代理即可
	//如果尚未加载完成，存到对应优先级级别的容器中，等待加载完成后统一按优先级顺序调用
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		//MoveTemp()强制转移数据
		OnExperienceLoaded_HighPriority.Add(MoveTemp(Delegate));
	}
}

void ULyraExperienceManagerComponent::CallOrRegister_OnExperienceLoaded(FOnLyraExperienceLoaded::FDelegate&& Delegate)
{
	//如果是已经加载了就直接执行代理即可
	//如果尚未加载完成，存到对应优先级级别的容器中，等待加载完成后统一按优先级顺序调用
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		//MoveTemp()强制转移数据
		OnExperienceLoaded.Add(MoveTemp(Delegate));
	}
}

void ULyraExperienceManagerComponent::CallOrRegister_OnExperienceLoaded_LowPriority(
	FOnLyraExperienceLoaded::FDelegate&& Delegate)
{
	//如果是已经加载了就直接执行代理即可
	//如果尚未加载完成，存到对应优先级级别的容器中，等待加载完成后统一按优先级顺序调用
	if (IsExperienceLoaded())
	{
		Delegate.Execute(CurrentExperience);
	}
	else
	{
		//MoveTemp()强制转移数据
		OnExperienceLoaded_LowPriority.Add(MoveTemp(Delegate));
	}
}

bool ULyraExperienceManagerComponent::IsExperienceLoaded() const
{
	//判断加载状态以及体验必须存在
	return LoadState == ELyraExperienceLoadedState::Loaded && CurrentExperience != nullptr;
}

void ULyraExperienceManagerComponent::OnRep_CurrentExperience()
{
	//从服务器同步过来，开启加载
	StartExperienceLoad();
}

void ULyraExperienceManagerComponent::StartExperienceLoad()
{
	//必须保证正确的初始化状态，不能是空指针，也不能多次加载，否则为流程错误
	check(CurrentExperience!=nullptr);
	check(LoadState==ELyraExperienceLoadedState::Unloaded);

	UE_LOG(LogLyraExperience, Log, TEXT("EXPERIENCE: StartExperienceLoad(CurrentExperience = %s, %s)"),
	       *CurrentExperience->GetPrimaryAssetId().ToString(),
	       *GetClientServerContextString(this));

	//切换到正在加载的状态
	LoadState = ELyraExperienceLoadedState::Loading;

	//TODO:这里使用的是Lyra项目自定义的资产管理器
	//ULyraAssetManager& AssetManager = ULyraAssetManager::Get()
	UAssetManager& AssetManager = UAssetManager::Get();

	//需要通过Bundlejin进行加载处理的资产,可以对资产做增加减少的操作
	TSet<FPrimaryAssetId> BundleAssetList;

	//只需要加载的资产
	TSet<FSoftObjectPath> RawAssetList;

	//添加我们正在使用的Experience的资产
	BundleAssetList.Add(CurrentExperience->GetPrimaryAssetId());

	//添加这个Experience所携带的ActionsSets，注意不是Actions
	for (const TObjectPtr<ULyraExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			BundleAssetList.Add(ActionSet->GetPrimaryAssetId());
		}
	}

	//加载与该体验相关的资源

	//需要传递的这些Bundles
	TArray<FName> BundlesToLoad;

	//TODO:这里添加了资产管理器俩民的一个全局变量，它是一个Bundle规则
	//BundlesToLoad.Add(FLyraBundles::Equipped)
	//暂时使用字符串代替
	BundlesToLoad.Add(TEXT("Equipped"));

	//TODO:将此客户端/服务器相关的内容集中到“LyraAssetManager”中。

	//获取当前的网络模式，是通过GameState的方式来获取
	const ENetMode OwnerNetMode = GetOwner()->GetNetMode();
	const bool bLoadClient = GIsEditor || OwnerNetMode != NM_DedicatedServer;
	const bool bLoadServer = GIsEditor || OwnerNetMode != NM_Client;

	if (bLoadClient)
	{
		//用于始终在客户端加载的配置/数据包
		//Client
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateClient);
	}
	if (bLoadServer)
	{
		//用于始终在专用服务器加载的配置/数据包
		//Client
		BundlesToLoad.Add(UGameFeaturesSubsystemSettings::LoadStateServer);
	}

	//一个用于同步或异步加载的句柄，只要该句柄处于激活状态，加载的资源就会保存在内存中
	TSharedPtr<FStreamableHandle> BundleLoadHandle = nullptr;
	if (BundleAssetList.Num() > 0)
	{
		//更改一组已加载的主资源的捆绑状态
		//等待返回的可流式请求完成，或根据需要进行轮询
		//如果没有需要执行的工作，返回句柄将为null，并会调用delegate方法
		//建议使用带有Param结构体的重载版本来编写新代码
		BundleLoadHandle = AssetManager.ChangeBundleStateForPrimaryAssets(
			BundleAssetList.Array(),
			BundlesToLoad,
			{},
			false,
			FStreamableDelegate(),
			FStreamableManager::AsyncLoadHighPriority);
	}

	//处理普通资产的句柄
	TSharedPtr<FStreamableHandle> RawLoadHandle = nullptr;
	if (RawAssetList.Num() > 0)
	{
		//使用主流式管理器加载非主要资源
		//该操作不会制动释放句柄，如有需要需要手动释放
		//对于新代码，建议使用带有参数结构体的重载版本
		RawLoadHandle = AssetManager.LoadAssetList(
			RawAssetList.Array(),
			FStreamableDelegate(),
			FStreamableManager::AsyncLoadHighPriority,
			TEXT("StartExperienceLoad()"));
	}

	//如果二个异步加载操作都在进行中，则将它们合并起来
	TSharedPtr<FStreamableHandle> Handle = nullptr;
	if (BundleLoadHandle.IsValid() && RawLoadHandle.IsValid())
	{
		//创建一个组合型句柄，该句柄会在其他句柄完成之前一直等待其完成，只要此句柄处于活跃状态，相关子句柄就会保持为强引用关系
		Handle = AssetManager.GetStreamableManager().CreateCombinedHandle({BundleLoadHandle, RawLoadHandle});
	}
	else
	{
		//使用其中一个可用的状态
		Handle = BundleLoadHandle.IsValid() ? BundleLoadHandle : RawLoadHandle;
	}

	//创建一个流式加载的代理
	FStreamableDelegate OnAssetsLoadedDelegate = FStreamableDelegate::CreateUObject(
		this, &ULyraExperienceManagerComponent::OnExperienceLoadComplete);

	if (!Handle.IsValid() || Handle->HasLoadCompleted())
	{
		// 资源已加载完成，现在调用委托函数即可
		FStreamableHandle::ExecuteDelegate(OnAssetsLoadedDelegate);
	}
	else
	{
		//绑定在加载完成时执行的委托函数，仅在加载过程有效，此操作会覆盖任何已绑定的委托函数
		Handle->BindCompleteDelegate(OnAssetsLoadedDelegate);

		//如果处理操作被取消，则会调用此绑定的委托，仅在加载过程有效，此操作会覆盖任何已绑定的委托函数
		Handle->BindCancelDelegate(FStreamableDelegate::CreateLambda(
			[OnAssetsLoadedDelegate]()
			{
				OnAssetsLoadedDelegate.ExecuteIfBound();
			}
		));
	}

	//这些资产会预先加载，但我们不会因此而阻止体验的开始
	TSet<FPrimaryAssetId> PreloadAssetList;
	//TODO:需要预先加载的资源（但是不需要进行阻塞式加载）
	if (PreloadAssetList.Num() > 0)
	{
		AssetManager.ChangeBundleStateForPrimaryAssets(PreloadAssetList.Array(), BundlesToLoad, {});
	}
}

void ULyraExperienceManagerComponent::OnExperienceLoadComplete()
{
	check(LoadState == ELyraExperienceLoadedState::Loading)

	check(CurrentExperience != nullptr);

	UE_LOG(LogLyraExperience, Log, TEXT("EXPERIENCE: OnExperienceLoadComplete(CurrentExperience = %s, %s)"),
	       *CurrentExperience->GetPrimaryAssetId().ToString(), *GetClientServerContextString(this));

	// 找出游戏功能插件网址，剔除重复项以及那些没有有效映射关系的网址
	GameFeaturePluginURLs.Reset();

	//搜集要使用的所有GameFeature插件
	auto CollectGameFeaturePluginURLs = [this](const UPrimaryDataAsset* Context,
	                                           const TArray<FString>& FeaturePluginList)
	{
		for (const FString& PluginName : FeaturePluginList)
		{
			FString PluginURL;
			//需要对这些插件名字和URL进行验证，因为有可能写错插件名导致找不到
			if (UGameFeaturesSubsystem::Get().GetPluginURLByName(PluginName,OUT PluginURL))
			{
				this->GameFeaturePluginURLs.AddUnique(PluginURL);
			}
			else
			{
				ensureMsgf(false,
				           TEXT(
					           "OnExperienceLoadComplete failed to find plugin URL from PluginName %s for experience %s - fix data, ignoring for this run"
				           ), *PluginName, *Context->GetPrimaryAssetId().ToString());
			}
		}
		//Add in our extra plugin
	};

	CollectGameFeaturePluginURLs(CurrentExperience, CurrentExperience->GameFeaturesToEnable);

	//把ActionsSets中的每一个ActionSet对应的所有GameFeatures插件都填进去
	for (const TObjectPtr<ULyraExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			CollectGameFeaturePluginURLs(ActionSet, ActionSet->GameFeaturesToEnable);
		}
	}

	//加载并启用各项功能

	//记录所有需要开启的游戏特性插件总数
	NumGameFeaturePluginsLoading = GameFeaturePluginURLs.Num();

	if (NumGameFeaturePluginsLoading > 0)
	{
		LoadState = ELyraExperienceLoadedState::LoadingGameFeatures;

		for (const FString& PluginURL : GameFeaturePluginURLs)
		{
			//增加使用计数
			ULyraExperienceManager::NotifyOfPluginActivation(PluginURL);

			//激活该插件，在该插件激活完毕后触发是否Experience完全加载的判定
			UGameFeaturesSubsystem::Get().LoadAndActivateGameFeaturePlugin(
				PluginURL, FGameFeaturePluginLoadComplete::CreateUObject(
					this, &ULyraExperienceManagerComponent::OnGameFeaturePluginLoadComplete));
		}
	}
	else
	{
		//如果没有。直接调用Experience充分加载这个函数
		OnExperienceFullloadCompleted();
	}
}

void ULyraExperienceManagerComponent::OnGameFeaturePluginLoadComplete(const UE::GameFeatures::FResult& Result)
{
	//减少正在加载的插件数量
	NumGameFeaturePluginsLoading--;

	if (NumGameFeaturePluginsLoading == 0)
	{
		OnExperienceFullloadCompleted();
	}
}

void ULyraExperienceManagerComponent::OnExperienceFullloadCompleted()
{
	check(LoadState == ELyraExperienceLoadedState::Loaded);

	//(如果已经配置）插入一段随机延迟以进行测试
	if (LoadState != ELyraExperienceLoadedState::LoadingChaosTestingDelay)
	{
		const float DelaySecs = LyraConsoleVariables::GetExperienceLoadDelayDuration();
		if (DelaySecs > 0.0f)
		{
			FTimerHandle DummyHandle;

			LoadState = ELyraExperienceLoadedState::LoadingChaosTestingDelay;
			GetWorld()->GetTimerManager().SetTimer(DummyHandle, this,
			                                       &ULyraExperienceManagerComponent::OnExperienceFullloadCompleted,
			                                       DelaySecs,/*bLooping=*/false);
			return;
		}
	}

	//切换状态到执行Actions
	LoadState = ELyraExperienceLoadedState::ExecutingActions;

	//执行这些操作
	FGameFeatureActivatingContext Context;

	//只有在设置的情况下才适用于我们特定的环境背景
	const FWorldContext* ExistingWorldContext = GEngine->GetWorldContextFromWorld(GetWorld());

	if (ExistingWorldContext)
	{
		Context.SetRequiredWorldContextHandle(ExistingWorldContext->ContextHandle);
	}

	//执行Action操作的Lambda，需要提供一个世界的上下文
	auto ActivateListOfActions = [&Context](const TArray<UGameFeatureAction*>& ActionList)
	{
		for (UGameFeatureAction* Action : ActionList)
		{
			//目前的这种行为与诸如游戏玩法标签这样的系统相匹配，在这些系统中，加载和注册操作适用于整个过程
			//但实际上，将这些结果运用于演员这一环节却受到特定环境的限制
			Action->OnGameFeatureRegistering();
			Action->OnGameFeatureLoading();
			Action->OnGameFeatureActivating(Context);
		}
	};

	ActivateListOfActions(CurrentExperience->Actions);

	//执行ActionSet的操作
	for (const TObjectPtr<ULyraExperienceActionSet>& ActionSet : CurrentExperience->ActionSets)
	{
		if (ActionSet != nullptr)
		{
			ActivateListOfActions(ActionSet->Actions);
		}
	}

	//到这里加载完成
	LoadState = ELyraExperienceLoadedState::Loaded;

	//呼叫执行各个级别的代理 这里通过优先级的控制 使得代理事件之间可以进行时序的区分
	OnExperienceLoaded_HighPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_HighPriority.Clear();

	OnExperienceLoaded.Broadcast(CurrentExperience);
	OnExperienceLoaded.Clear();

	OnExperienceLoaded_LowPriority.Broadcast(CurrentExperience);
	OnExperienceLoaded_LowPriority.Clear();

	//应用任何必要的扩展性设置
}

void ULyraExperienceManagerComponent::OnActionDeactivationCompleted()
{
	//对于正在退出的Action进行计数
	check(IsInGameThread());
	++NumObservedPausers;
	//所有的Action都已观测到取消，这里应该是异步的，但是代码给的日志，提示目前还不支持异步操作
	if (NumObservedPausers == NumExpectedPausers)
	{
		OnAllActionsDeactivated();
	}
}

void ULyraExperienceManagerComponent::OnAllActionsDeactivated()
{
	LoadState = ELyraExperienceLoadedState::Unloaded;

	CurrentExperience = nullptr;
}
