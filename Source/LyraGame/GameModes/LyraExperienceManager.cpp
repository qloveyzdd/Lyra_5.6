// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraExperienceManager.h"

#if WITH_EDITOR
void ULyraExperienceManager::OnPlayInEditorBegun()
{
	// ensure() 是虚幻引擎(Unreal Engine)中一个非常重要的调试和验证宏，用于检查条件并报告错误，
	// 但与 check() 不同，它不会导致程序崩溃。
	ensure(GameFeaturePluginRequestCountMap.IsEmpty());
	GameFeaturePluginRequestCountMap.Empty();
}

void ULyraExperienceManager::NotifyOfPluginActivation(const FString PluginURL)
{
	if (GIsEditor)
	{
		ULyraExperienceManager* ExperienceManagerSubsystem = GEngine->GetEngineSubsystem<ULyraExperienceManager>();
		check(ExperienceManagerSubsystem);

		//记录激活此插件的请求者的数量，由于可以处理并发请求，因此允许多次加载/激活操作
		int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindOrAdd(PluginURL);

		++Count;
	}
}

bool ULyraExperienceManager::RequestToDeactivatePlugin(const FString PluginURL)
{
	if (GIsEditor)
	{
		ULyraExperienceManager* ExperienceManagerSubsystem = GEngine->GetEngineSubsystem<ULyraExperienceManager>();
		check(ExperienceManagerSubsystem);
		//只允许最后提出请求的用户能够继续进行这一步，并且由其来解除该插件的激活状态
		//FindChecked,查找的值
		int32& Count = ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.FindChecked(PluginURL);
		--Count;
		
		if (Count == 0)
		{
			ExperienceManagerSubsystem->GameFeaturePluginRequestCountMap.Remove(PluginURL);
			return true;
		}
		//如果计数未归零，则不应该移除插件
		return false;
	}
	return true;
}

#endif
