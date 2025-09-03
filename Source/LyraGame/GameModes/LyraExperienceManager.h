// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

//体验管理的引擎子系统
//主要负责多个PIE会议之间的协调工作
UCLASS(MinimalAPI)
class ULyraExperienceManager : public UEngineSubsystem
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	//在编辑器模块中的StartupModule（）进行调用，用于初始化GameFeaturePluginRequestCountMap
	LYRAGAME_API void OnPlayInEditorBegun();

	//通知有插件被激活了，增加计数，由LyraExperienceManagerComponent调用
	static void NotifyOfPluginActivation(const FString PluginURL);
	//通知有插件被关闭了，减少计数，由LyraExperienceManagerComponent调用
	static bool RequestToDeactivatePlugin(const FString PluginURL);
#else
	//运行时不需要相关功能
	static void NotifyOfPluginActivation(const FString PluginURL){}
	static bool RequestToDeactivatePlugin(const FString PluginURL){return true;}
#endif
	
private:
	//指定游戏功能插件的请求量与激活次数的关系图
	//（以便在PIE过程中实现【先进后出】的激活管理
	TMap<FString, int32> GameFeaturePluginRequestCountMap;
	
};
