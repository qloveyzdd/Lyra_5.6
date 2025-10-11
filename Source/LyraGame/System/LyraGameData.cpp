// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameData.h"

#include "LyraAssetManager.h"

ULyraGameData::ULyraGameData()
{
}

const ULyraGameData& ULyraGameData::Get()
{
	//调用资产管理的方法即可
	return ULyraAssetManager::Get().GetGameData();
}
