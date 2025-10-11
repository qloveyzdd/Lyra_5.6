// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "LyraGameData.generated.h"

//不可变的数据资产，其中包含全局游戏数据

UCLASS(MinimalAPI,BlueprintType,Const,meta=(DisplayName = "Lyra Game Data",ShortTooltip = "Data asset containing global game data."))
class ULyraGameData:public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	LYRAGAME_API ULyraGameData();

	//返回已加载的游戏数据
	LYRAGAME_API static const ULyraGameData& Get();

public:
	//用于造成伤害的游戏效果，其伤害值由调用者设定。
	//比如跌落超出高度
	UPROPERTY(EditDefaultsOnly,Category="Default Gameplay Effects",meta=(DisplayName = "Damage Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<class UGameplayEffect> DamageGameplayEffect_SetByCaller;

	//用于施加治疗效果的玩法特效，治疗量由调用者设定
	UPROPERTY(EditDefaultsOnly,Category="Default Gameplay Effects",meta=(DisplayName = "Heal Gameplay Effect (SetByCaller)"))
	TSoftClassPtr<class UGameplayEffect> HealGameplayEffect_SetByCaller;

	//用于添加和移除动态标签的游戏玩法效果
	UPROPERTY(EditDefaultsOnly,Category="Default Gameplay Effects")
	TSoftClassPtr<class UGameplayEffect> DynamicTagGameplayEffect;
	
};
