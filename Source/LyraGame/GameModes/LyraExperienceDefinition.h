// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

//游戏体验的定义

// Const作用：将该类标记为"常量"类。
// 具体含义：
// 该类的实例被视为只读对象
// 类中的所有UPROPERTY默认都是只读的
// 编辑器中不能修改该类实例的属性
// 通常用于定义配置数据、常量数据或引用数据的类
// 限制：
// 标记为Const的类通常不应该有修改其内部状态的方法
// 如果需要修改属性，必须显式地标记为非Const
UCLASS(BlueprintType, Const)
class ULyraExperienceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	ULyraExperienceDefinition();

#if WITH_EDITOR
	//用于在变更列表验证等过程中验证对象的通用函数
	//Context 包含验证警告/错误的信息区域
	//如果此对象已设置数据验证规则且该对象的数据有效，则返回有效；否则返回无效。规则，如果此对象未设置任何规则，则返回“NotValidated”。
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif

#if WITH_EDITORONLY_DATA
	//此函数会遍历该类中的资产属性，查找资产包的元数据，并使用“InitializeAssetBundlesFromMetadata”方法来初始化AssetBundleData对象
	virtual void UpdateAssetBundleData() override;
#endif
	

public:
	//此体验所需的已激活的游戏功能插件列表
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TArray<FString> GameFeaturesToEnable;

	//用于生成玩家默认兵种的兵种类
	UPROPERTY(EditDefaultsOnly, Category=Gameplay)
	TObjectPtr<const ULyraPawnData> DefaultPawnData;

	//当此体验被加载/激活/停用/卸载时要执行的操作列表

	//表示当某个游戏功能被激活时应采取的操作
	// Instanced 说明符指示引擎将引用的对象作为"子对象"(Sub-object)处理，而不是普通的对象引用。这意味着被引用的对象将成为拥有者对象的一部分，与拥有者共享生命周期。
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Actions")
	TArray<TObjectPtr<UGameFeatureAction>> Actions;

	//用于构建此体验的附加操作集列表
	UPROPERTY(EditDefaultsOnly, Instanced, Category="Actions")
	TArray<TObjectPtr<ULyraExperienceActionSet>> ActionSets;
};
