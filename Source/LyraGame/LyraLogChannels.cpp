// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraLogChannels.h"

#include "LandscapeGizmoActiveActor.h"

DEFINE_LOG_CATEGORY(LogLyra);
DEFINE_LOG_CATEGORY(LogLyraExperience);
DEFINE_LOG_CATEGORY(LogLyraAbilitySystem);
DEFINE_LOG_CATEGORY(LogLyraTeams);

FString GetClientServerContextString(UObject* ContextObject)
{
	// ENetRole 是虚幻引擎中的一个枚举类型，用于定义网络对象（通常是Actor）在网络游戏中的角色或权限级别。这个枚举定义了对象在网络复制系统中的行为方式。
	// ENetRole 枚举通常包含以下值：

	// ROLE_None：
	// 值为 0
	// 表示对象没有网络角色
	// 通常用于初始化或非网络对象

	// ROLE_SimulatedProxy：
	// 值为 1
	// 表示对象是一个模拟代理
	// 这种角色只接收服务器的更新，不能控制对象
	// 通常用于客户端上的其他玩家对象

	// ROLE_AutonomousProxy：
	// 值为 2
	// 表示对象是一个自主代理
	// 这种角色可以发送输入到服务器
	// 通常用于客户端上的本地玩家控制的对象

	// ROLE_Authority：
	// 值为 3
	// 表示对象具有权威性
	// 这种角色可以做出最终决定
	// 通常用于服务器上的所有对象
	ENetRole Role = ROLE_None;

	if (AActor* Actor = Cast<AActor>(ContextObject))
	{
		// 	GetLocalRole() 函数返回Actor在当前执行机器上的网络角色。这个角色决定了Actor在网络复制系统中的行为：
		// 在服务器上，大多数Actor的 GetLocalRole() 返回 ROLE_Authority
		// 在客户端上，本地控制的Actor的 GetLocalRole() 通常返回 ROLE_AutonomousProxy
		// 在客户端上，其他玩家控制的Actor的 GetLocalRole() 通常返回 ROLE_SimulatedProxyetLocalRole() 是 AActor 类的一个成员函数，用于获取Actor在当前执行环境（本地机器）中的网络角色。这个函数返回一个 ENetRole 类型的值。
		Role = Actor->GetLocalRole();
	}
	else if (UActorComponent* Component = Cast<UActorComponent>(ContextObject))
	{
		// GetOwnerRole() 是 UActorComponent 类的一个成员函数，用于获取组件所属Actor的网络角色。这个函数返回一个 ENetRole 类型的值。
		// 获取组件所有者的网络角色：组件本身不直接参与网络复制角色系统，而是继承其所属Actor的网络角色
		// 简化组件级别的网络逻辑：让组件可以根据其所属Actor的网络角色来决定行为
		Role = Component->GetOwnerRole();
	}

	if (Role != ROLE_None)
	{
		return Role == ROLE_Authority ? TEXT("Server") : TEXT("Client");
	}
	else
	{
#if WITH_EDITOR
		if (GIsEditor)
		{
			//在“编辑器中游戏”（PIE）模式下切换不同的游戏场景时会启用此调试辅助工具集

			// extern ENGINE_API FString GPlayInEditorContextString;这行声明了一个外部变量 GPlayInEditorContextString：
			// extern：表示这个变量在其他地方定义，这里只是声明
			// ENGINE_API：这是虚幻引擎的宏，用于标记可以从引擎模块导出的符号
			// FString：虚幻引擎的字符串类型

			// GPlayInEditorContextString：变量名，存储PIE模式的上下文信息
			// return GPlayInEditorContextString;这行代码返回PIE模式的上下文字符串，这个字符串包含了当前PIE会话的相关信息。

			// GPlayInEditorContextString 的作用和内容
			// GPlayInEditorContextString 是虚幻引擎在PIE模式下生成的一个特殊字符串，主要用于以下目的：
			// 区分不同的PIE实例：当你在编辑器中启动多人游戏测试时，可能会同时运行多个PIE实例（服务器+多个客户端）
			// 网络连接标识：在多人游戏测试中，帮助不同PIE实例相互识别和连接
			// 保存/加载状态：确保每个PIE实例有自己独立的保存数据
			// 日志区分：在日志中区分来自不同PIE实例的输出
			extern ENGINE_API FString GPlayInEditorContextString;
			return GPlayInEditorContextString;
		}
#endif
	}

	return TEXT("[]");
}
