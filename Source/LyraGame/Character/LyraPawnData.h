// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LyraPawnData.generated.h"

// MinimalAPI：
// 这个参数限制了类的导出功能，只导出最小必要的API函数，减少编译时间和最终二进制文件大小。
// 适用于那些不需要完全导出到其他模块的类。
// Meta：
// 为类添加元数据，提供额外信息。
// 这里设置了两个元数据项：
// DisplayName = "Lyra Pawn Data"：在编辑器UI中显示的友好名称。
// ShortTooltip = "Data asset used to define a Pawn."：当鼠标悬停在编辑器中的此类型上时显示的简短提示。
//一种不可变的数据资产，其中包含用于定义棋子的属性。
UCLASS(MinimalAPI, BlueprintType, Const,
	Meta = (DisplayName = "Lyra Pawn Data", ShortTooltip = "Data asset used to define a Pawn."))
class ULyraPawnData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	// FObjectInitializer是虚幻引擎中用于初始化对象的特殊类，提供了一种标准化的方式来创建和初始化UObject派生类。
	LYRAGAME_API ULyraPawnData(const FObjectInitializer& ObjectInitializer);

public:
	//用于创建此兵种实例的类（通常应派生自ALyraPawn或ALyraCharacter）
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Lyra|Pawn")
	TSubclassOf<APawn> PawnClass;
	
	// //为该兵种赋予的能力组
	// UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Lyra|Abilities")
	// TArray<TSubclassOf<ULyraAbilitySet>> AbilitySets;
};
