// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraExperienceActionSet.h"
#include "GameFeatureAction.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "LyraSystem"

ULyraExperienceActionSet::ULyraExperienceActionSet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#if WITH_EDITOR
EDataValidationResult ULyraExperienceActionSet::IsDataValid(class FDataValidationContext& Context) const
{
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context),
	                                                            EDataValidationResult::Valid);
	int32 EntryIndex = 0;
	for (const UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			EDataValidationResult ActionResult = Action->IsDataValid(Context);
			Result = CombineDataValidationResults(Result, ActionResult);
		}
		else
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(
				LOCTEXT("ActionEntryIsNull", "Null entry at index {0} in Action"), FText::AsNumber(EntryIndex)));
		}
		++EntryIndex;
	}
	return Result;
}
#endif

#if WITH_EDITORONLY_DATA
void ULyraExperienceActionSet::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	for (UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			// AddAdditionalAssetBundleData是UGameFeatureAction类中的一个虚函数，用于向资产包数据中添加额外的资产依赖项。
			// 这个方法允许游戏特性动作(Game Feature Actions)声明它们需要哪些额外的资产，确保这些资产在游戏特性激活时被正确加载。
			Action->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
}
#endif


#undef LOCTEXT_NAMESPACE
