// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Engine/StreamableManager.h"


DECLARE_DELEGATE_OneParam(FLyraAssetManagerStartupJobSubstepProgress, float);


//处理来自可流式句柄的进度报告
struct FLyraAssetManagerStartupJob
{
	//进度代理
	FLyraAssetManagerStartupJobSubstepProgress SubstepProgressDelegate;

	TFunction<void(const FLyraAssetManagerStartupJob&, TSharedPtr<FStreamableHandle>&)> JobFunc;

	//任务名称
	FString JobName;

	//任务权重
	float JobWeight;

	mutable double LastUpdate = 0;

	//简单的同步型任务
	//任务名，任务函数，权重
	FLyraAssetManagerStartupJob(const FString& InJobName,
	                            const TFunction<
		                            void(const FLyraAssetManagerStartupJob&,
		                                 TSharedPtr<FStreamableHandle>&)>& InJobFunc,
	                            float InJobWeight)
		: JobFunc(InJobFunc), JobName(InJobName), JobWeight(InJobWeight)
	{
	}

	//执行实际加载操作，如果创建了处理对象则会返回该处理对象句柄
	TSharedPtr<FStreamableHandle> DoJob() const;

	//更新进度，未使用,因为字串管理器中没有注册复杂任务
	void UpdataSubstepProgress(float NewProgress)const
	{
		SubstepProgressDelegate.ExecuteIfBound(NewProgress);
	}

	//根据流式加载句柄 没有使用 因为字串管理器中没有注册复杂任务
	void UpdataSubstepProgressFormStreamable(TSharedRef<FStreamableHandle> StreamableHandle)const
	{
		//先判断是否绑定
		if (SubstepProgressDelegate.IsBound())
		{
			double Now = FPlatformTime::Seconds();
			//比较时间，每16ms去获取下进度并传递出去
			if (LastUpdate - Now > 1.0f/60.0f)
			{
				SubstepProgressDelegate.Execute(StreamableHandle->GetProgress());
				LastUpdate = Now;
			}
		}
	}
};
