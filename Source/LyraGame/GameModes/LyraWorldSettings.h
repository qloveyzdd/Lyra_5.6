// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LyraWorldSettings.generated.h"

//默认的世界设置对象，主要用于设置在该地图上游玩时所使用的默认游戏体验设置

UCLASS(MinimalAPI)
class ALyraWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	LYRAGAME_API ALyraWorldSettings(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	//该函数在“Map_Check"内部被调用，以便此角色能够检查自身是否存在任何潜在错误，并将这些错误通过”地图检查对话框“进行记录
	LYRAGAME_API virtual void CheckForErrors() override;
#endif

public:
	//返回服务器在打开此地图时所使用的默认体验设置，若该设置未被用户界面中的体验所覆盖则使用此默认设置
	// FPrimaryAssetId：这将对象标识为“主要”资产，可由资产管理器搜索并在各种工具中使用
	LYRAGAME_API FPrimaryAssetId GetDefaultGameplayExperience() const;

protected:
	//当服务器打开此地图时（若用户界面体验未对其进行覆盖），所采用的默认体验方式
	UPROPERTY(EditDefaultsOnly, Category=GameMode)
	TSoftClassPtr<class ULyraExperienceDefinition> DefaultGameplayExperience;

public:
#if WITH_EDITORONLY_DATA
	//设置这个级别属于前端部分还是独立的独立体验的一部分
	//如果设置了此项，那在编辑器中点击“播放”时，网络模式将强制切换为“独立模式”
	UPROPERTY(EditDefaultsOnly, Category = PIE)
	bool ForceStandaloneNetMode = false;
#endif
};
