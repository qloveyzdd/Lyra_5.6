// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraExperienceDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "LyraSystem"

ULyraExperienceDefinition::ULyraExperienceDefinition()
{
}

#if WITH_EDITOR
EDataValidationResult ULyraExperienceDefinition::IsDataValid(class FDataValidationContext& Context) const
{
	// CombineDataValidationResults：
	// 这是一个辅助函数，用于组合两个验证结果
	// 通常的实现逻辑是：如果任一结果是Invalid，则返回Invalid；否则返回Valid
	// 这确保了只有当所有验证都通过时，最终结果才是有效的
	EDataValidationResult Result = CombineDataValidationResults(Super::IsDataValid(Context),
	                                                            EDataValidationResult::Valid);

	int32 EntryIndex = 0;
	for (const UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			EDataValidationResult ChildResult = Action->IsDataValid(Context);
			Result = CombineDataValidationResults(Result, ChildResult);
		}
		else
		{
			Result = EDataValidationResult::Invalid;
			Context.AddError(FText::Format(
				LOCTEXT("ActionEntryIsNull", "Null entry at index {0} in Actions"), FText::AsNumber(EntryIndex)));
		}
		++EntryIndex;
	}

	//确保用户没有从本业务包的基类派生出新的类（这种情况是正常的且预期的，即在一个业务包中只能派生一次子类，不能二次派生
	// IsNative() 这是UClass类的一个方法，用于判断该类是否是"原生"类
	// 在UE中，"原生"(Native)类指的是用C++编写的类，而非蓝图创建的类
	// 返回一个布尔值：如果是C++类则返回true，如果是蓝图类则返回false
	if (!GetClass()->IsNative())
	{
		const UClass* ParentClass = GetClass()->GetSuperClass();
		const UClass* FirstNativeParent = ParentClass;
		while (FirstNativeParent != nullptr && !FirstNativeParent->IsNative())
		{
			FirstNativeParent = FirstNativeParent->GetSuperClass();
		}
		//如果原始父类和父类不是同一个，证明发生了多次继承，这是不合理的
		if (FirstNativeParent != ParentClass)
		{
			Context.AddError(FText::Format(LOCTEXT("ExperienceInheritenceIsUnsupported",
			                                       "Blueprint subclasses of Blueprint experiences is not currently supported (use composition via ActionSets instead). Parent class was {0} but should be {1}."),
			                                       FText::AsCultureInvariant(GetPathNameSafe(ParentClass)),
			                                       FText::AsCultureInvariant(GetPathNameSafe(FirstNativeParent))
			                                       ));
			Result = EDataValidationResult::Invalid;
		}
	}
	return Result;
}
#endif

#if WITH_EDITORONLY_DATA
void ULyraExperienceDefinition::UpdateAssetBundleData()
{
	Super::UpdateAssetBundleData();

	for (UGameFeatureAction* Action : Actions)
	{
		if (Action)
		{
			Actor->AddAdditionalAssetBundleData(AssetBundleData);
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE
