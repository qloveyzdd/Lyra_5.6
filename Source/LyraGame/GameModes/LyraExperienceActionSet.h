// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LyraExperienceActionSet.generated.h"


//作为进入某一体验过程中所需执行的一系列动作的定义
// NotBlueprintable - 表示该类不能被蓝图继承或创建
UCLASS(BlueprintType, NotBlueprintable)
class ULyraExperienceActionSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	ULyraExperienceActionSet(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

#if WITH_EDITORONLY_DATA
	virtual void UpdateAssetBundleData() override;
#endif

public:
	//表示当某个游戏功能被激活时应采取的操作
	// Instanced 说明符指示引擎将引用的对象作为"子对象"(Sub-object)处理，而不是普通的对象引用。这意味着被引用的对象将成为拥有者对象的一部分，与拥有者共享生命周期。
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Actions")
	TArray<TObjectPtr<class UGameFeatureAction>> Actions;

	//用于构建此体验的附加操作集列表
	UPROPERTY(EditDefaultsOnly, Category="Actions")
	TArray<FString> GameFeaturesToEnable;
};
