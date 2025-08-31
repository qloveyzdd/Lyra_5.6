// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraWorldSettings.h"

#include "EngineUtils.h"
#include "LyraLogChannels.h"
#include "Engine/AssetManager.h"
#include "GameFramework/PlayerStart.h"
#include "Misc/UObjectToken.h"


ALyraWorldSettings::ALyraWorldSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ALyraWorldSettings::CheckForErrors()
{
	Super::CheckForErrors();

	// "MapCheck" 是虚幻引擎中预定义的一个特殊分类，专门用于地图验证过程中的消息记录。
	// 当开发者或设计师运行地图检查（Map Check）功能时，引擎会收集所有标记为 "MapCheck" 分类的消息，并在编辑器的消息日志窗口中显示。
	FMessageLog MapCheck("MapCheck");

	//检查一下是否使用的是Lyra自定义的顽疾起始点
	for (TActorIterator<APlayerStart> PlayerStartIt(GetWorld()); PlayerStartIt; ++PlayerStartIt)
	{
		APlayerStart* PlayerStart = *PlayerStartIt;
		//引擎原生的起始点
		if (IsValid(PlayerStart) && PlayerStart->GetClass() == APlayerStart::StaticClass())
		{
			// MapCheck.Warning()
			// MapCheck 是前面创建的 FMessageLog 对象
			// Warning() 方法表示创建一个警告级别的消息
			// 警告级别通常在编辑器中以黄色图标显示，表示需要注意但不是严重错误
			// Warning() 方法返回一个 TSharedRef<FTokenizedMessage> 对象，这是一个引用计数的智能指针，指向新创建的消息对象
			// AddToken() 方法用于向消息添加一个令牌(Token)
			// FUObjectToken::Create(PlayerStart) 创建一个对象引用令牌
			// 这个令牌包含对 PlayerStart 对象的引用
			// 在编辑器的消息日志中，这将显示为一个可点击的对象链接
			// 点击此链接会在编辑器中选中并聚焦到该 PlayerStart 对象
			// 这使得开发者可以直接从警告消息导航到有问题的对象
			MapCheck.Warning()
			        ->AddToken(FUObjectToken::Create(PlayerStart))
			        ->AddToken(FTextToken::Create(
				        FText::FromString("is a normal APlayerStart, replace with ALyraPlayerStart.")));
		}
	}
}

FPrimaryAssetId ALyraWorldSettings::GetDefaultGameplayExperience() const
{
	FPrimaryAssetId Result;

	if (DefaultGameplayExperience.IsNull())
	{
		//通过资产管理器加载软引用，价差是否可以获取到该资产
		Result = UAssetManager::Get().GetPrimaryAssetIdForPath(DefaultGameplayExperience.ToSoftObjectPath());

		if (!Result.IsValid())
		{
			UE_LOG(LogLyraExperience, Error,
			       TEXT(
				       "%s.DefaultGameplayExperience is %s but that failed to resolve into an asset ID (you might need to add a path to the Asset Rules in your game feature plugin or project settings"
			       ), *GetPathNameSafe(this), *DefaultGameplayExperience.ToString());
		}
	}
	return Result;
}
