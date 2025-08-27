// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class LyraEditorTarget : TargetRules
{
	public LyraEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange(new string[] { "LyraGame", "LyraEditor" });

		// 用于控制是否构建所有模块或仅构建特定的模块。当该值为 false 时，通常会在条件语句中添加一些模块排除逻辑。
		if (!bBuildAllModules)
		{
			// NativePointerMemberBehaviorOverride是否允许使用原生指针成员的设置，如果不允许，则将出现 UHT 错误，并且应使用 TObjectPtr 成员代替。
			// PointerMemberBehavior指定 UnrealHeaderTool 应如何在 UCLASS 和 USTRUCT 中强制执行成员指针声明。
			// Disallow,原生指针成员将被禁止，并导致 UnrealHeaderTool 发出错误。
			// AllowSilently,原生指针成员将被允许，并且不会向日志或屏幕发送任何消息。
			// AllowAndLog,原生指针成员将被允许，但会向日志发送消息。
			NativePointerMemberBehaviorOverride = PointerMemberBehavior.Disallow;
		}

		LyraGameTarget.ApplySharedLyraTargetSettings(this);

		//增加一个插件，用于触屏设备开发，同时与“虚幻远程2“应用程序配合使用
		EnablePlugins.Add("RemoteSession");
	}
}