// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using UnrealBuildTool;
using System.Collections.Generic;
using System.IO;
using EpicGames.Core;
using Microsoft.Extensions.Logging;
using UnrealBuildBase;

// 这个类继承自 TargetRules，用于配置 Lyra 游戏项目的构建目标。每个 Unreal Engine 项目通常会有多个目标文件，如游戏目标、编辑器目标、服务器目标等。
public class LyraGameTarget : TargetRules
{
	public LyraGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		// 添加 "LyraGame" 作为此目标需要构建的额外模块
		// 	这表示 LyraGame 是主游戏模块，包含游戏的核心功能
		// 	当构建这个目标时，会自动包含 LyraGame 模块
		// ExtraModuleNames.Add("LyraGame");
		ExtraModuleNames.AddRange(new string[] { "LyraGame" });

		ApplySharedLyraTargetSettings(this);
	}

	//自定义变量，对重复提示做修正
	private static bool bHasWarnedAboutShared = false;

	// C# 的 internal: 同一程序集内可访问
	internal static void ApplySharedLyraTargetSettings(TargetRules Target)
	{
		// ILogger 是一个接口，用于处理构建过程中的日志记录功能。通过这个日志记录器，可以在构建脚本中输出信息、警告和错误消息，帮助开发者了解构建过程中发生的事情。
		// 从 Target 对象中获取 Logger 属性
		// 将其赋值给一个类型为 ILogger 的局部变量 logger
		ILogger logger = Target.Logger;

		Target.DefaultBuildSettings = BuildSettingsVersion.V5;
		Target.IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;

		//判断当前配置是否是测试配置
		//Configuration,正在构建的配置
		//UnrealTargetConfiguration,包含着可构建的配置类型，Debug/Development/Test/Shipping等
		bool bIsTest = Target.Configuration == UnrealTargetConfiguration.Test;
		//判断当前配置是否是发行配置
		bool bIsShipping = Target.Configuration == UnrealTargetConfiguration.Shipping;

		//判断当前编译目标是否为专属服务器
		//TargetType,编译选项,Game/Editor/Client/Server/Program(独立程序)等
		bool bIsDedicatedServer = Target.Type == UnrealBuildTool.TargetType.Server;

		//指定是否与其他项目共享引擎二进制文件和中间文件，或创建项目特定版本。自己的理解：有一些文件做了特殊的修改，会与原引擎版本不同，将包含在当前工程中
		//Shared,引擎二进制文件和中间文件将输出到引擎文件夹。针对引擎构建环境的特定目标修改将被忽略。
		//Unique,引擎二进制文件和中间体特定于此目标
		if (Target.BuildEnvironment == TargetBuildEnvironment.Unique) //使用特定版本
		{
			//将目标做出的警告级别的输出显示为错误警告(调整警告级别）
			Target.CppCompileWarningSettings.ShadowVariableWarningLevel = WarningLevel.Error;
			//是否打开测试/发布版本的检查（断言）。
			Target.bUseChecksInShipping = true;
			// 是否在测试配置中跟踪 RHI 资源的所有者（资产名称）。此功能对“列表着色器映射”和“列表着色器库”命令非常有用。
			Target.bTrackRHIResourceInfoForTest = true;

			//发行模式且非专属服务器
			if (bIsShipping && !bIsDedicatedServer)
			{
				//是否允许引擎配置确定我们是否可以加载未经验证的证书。
				Target.bDisableUnverifiedCertificates = true;

				// Uncomment these lines to lock down the command line processing
				// 取消这些行的注释即可锁定命令行处理过程
				// This will only allow the specified command line arguments to be parsed
				// 这将仅允许对指定的命令行参数进行解析。
				//Target.GlobalDefinitions.Add("UE_COMMAND_LINE_USES_ALLOW_LIST=1");
				//Target.GlobalDefinitions.Add("UE_OVERRIDE_COMMAND_LINE_ALLOW_LIST=\"-space -separated -list -of -commands\"");

				// Uncomment this line to filter out sensitive command line arguments that you
				// 取消注释这一行，以便过滤掉那些敏感的命令行参数。您需要这样做。
				// don't want to go into the log file (e.g., if you were uploading logs)
				// 不想查看日志文件（例如，如果您正在上传日志的话）
				//Target.GlobalDefinitions.Add("FILTER_COMMANDLINE_LOGGING=\"-some_connection_id -some_other_arg\"");
			}

			//发行模式或测试模式下
			if (bIsShipping || bIsTest)
			{
				//这些设置的主要目的是在发行版和测试版中限制配置文件的使用，使游戏更加稳定和安全。
				//通过禁用某些配置文件的加载，可以防止用户或测试人员通过修改这些文件来改变游戏的预期行为.
				//同时也可能提高性能，因为需要处理的配置文件更少。
				//是否在编译版本中加载生成的 ini 文件（无论哪种方式都会加载 GameUserSettings.ini）
				Target.bAllowGeneratedIniWhenCooked = false;
				//是否在编译后加载非 ufs ini 文件（无论哪种方式都会加载 GameUserSettings.ini）
				Target.bAllowNonUFSIniWhenCooked = false;
			}

			if (Target.Type != TargetType.Editor)
			{
				// 我们在运行时并不使用路径追踪功能，仅用于制作精美的效果图，而且这个动态链接库的体积相当大。
				Target.DisablePlugins.Add("OpenImageDenoise");
				// 减少资产注册表中始终加载数据所占用的内存，但增加一些计算资源消耗较大的查询操作
				Target.GlobalDefinitions.Add("UE_ASSETREGISTRY_INDIRECT_ASSETDATA_POINTERS=1");
			}

			ConfigureGameFeaturePlugins(Target);
		}
		else //不使用特殊版本
		{
			//仅在编辑模式下有效
			if (Target.Type == TargetType.Editor)
			{
				ConfigureGameFeaturePlugins(Target);
			}
			else
			{
				// 共享的单体式构建无法启停插件或更改任何选项，因为其会尝试复用已安装的引擎二进制文件。
				if (!bHasWarnedAboutShared)
				{
					bHasWarnedAboutShared = true;
					logger.LogWarning(
						"LyraGameEOS and dynamic target options are disabled when packaging from an installed version of the engine");
				}
			}
		}
	}

	static public bool ShouldEnableAllGameFeaturePlugins(TargetRules Target)
	{
		if (Target.Type == TargetType.Editor)
		{
			// 若设置为“true”，编辑器将构建所有游戏功能插件，但这些插件是否会被全部加载则取决于具体情况。
			// 这样您就可以在编辑器中启用插件，而无需编译代码。
		}

		// 检查当前的构建环境是否是自动化构建机器。
		// Environment.GetEnvironmentVariable("IsBuildMachine") - 这部分代码从系统环境变量中获取名为 "IsBuildMachine" 的变量值。
		// 将获取到的环境变量值与字符串 "true" 进行比较，如果相等，则表达式结果为 true，否则为 false。
		// 将比较结果赋值给布尔变量 bIsBuildMachine。
		// 在 Unreal Engine 构建系统中的作用：
		// 在自动化构建环境（如持续集成/持续部署系统、构建服务器等）中，通常会设置 "IsBuildMachine" 环境变量为 "true"。
		bool bIsBuildMachine = Environment.GetEnvironmentVariable("IsBuildMachine") == "true";

		if (bIsBuildMachine)
		{
			//这可以用于为构建机器启用所有插件
		}

		//默认情况下，将使用插件浏览器在编辑器中设置的默认插件规则
		return false;
	}

	//一个插件名和它的JsonObject的对象，用于获取插件描述信息
	// 这个字典很可能用于缓存已解析的插件配置文件（通常是 .uplugin 文件的 JSON 内容）
	// 使用字典可以通过插件名称快速查找对应的 JSON 配置，避免重复解析同一个插件文件 
	// 静态变量意味着这个缓存在整个构建过程中都可用，可以跨多个目标或模块共享
	// 这种缓存机制在处理大量插件时特别有用，可以显著提高构建系统的性能，因为解析 JSON 文件是一个相对耗时的操作，
	// 尤其是在大型项目中可能有几十甚至上百个插件。
	private static Dictionary<string, JsonObject> AllPluginRootJsonObjectsByName = new Dictionary<string, JsonObject>();

	// 用于配置我们希望启用哪些游戏功能插件
	// 这是一种相对简单的实现方式，但您也可以根据当前分支的目标发布版本来构建不同的插件。
	// 例如，在主分支中启用正在进行中的功能，而在当前发布分支中则禁用这些功能。
	static public void ConfigureGameFeaturePlugins(TargetRules Target)
	{
		ILogger logger = Target.Logger;

		// 打印当前分支
		Log.TraceInformationOnce("Compiling GameFeaturePlugins in branch {0}", Target.Version.BranchName);

		//获取是否构建所有的GameFeature插件
		bool bBuildAllGameFeaturePlugins = ShouldEnableAllGameFeaturePlugins(Target);

		//加载所有游戏特性插件描述器

		//创建一个FileReference的容器
		// FileReference 是列表中元素的类型，表示文件引用对象
		// 在 Unreal Engine 构建系统中，FileReference 通常是一个表示文件路径的类，比普通字符串路径提供更多功能
		// = new List<FileReference>() - 初始化这个列表为一个新的空列表实例。
		// 在 Unreal Engine 构建系统中的作用：
		// 这个列表很可能用于收集项目中所有需要处理的插件文件的引用
		// 这可能是从多个来源（如引擎插件、项目插件、第三方插件等）合并而来的插件列表
		// 列表中的每个 FileReference 对象指向一个 .uplugin 文件的路径
		// 这种列表在构建过程中非常有用，因为：
		// 它允许构建系统跟踪所有需要考虑的插件
		// 可以遍历这个列表来处理每个插件（如解析其配置、确定是否启用、添加其模块到构建等）
		// 使用 FileReference 而不是简单的字符串路径提供了更好的路径操作和验证功能
		List<FileReference> CombinedPluginList = new List<FileReference>();

		// 创建一个 DirectoryReference 类型的列表，命名为 GameFeaturePluginRoots
		// 使用 Unreal.GetExtensionDirs() 方法获取特定路径下的目录引用
		// 方法参数：
		// 第一个参数 Target.ProjectFile.Directory：当前项目文件的目录路径
		// 	第二个参数 Path.Combine("plugins", "GameFeatures")：将 "plugins" 和 "GameFeatures" 组合成一个路径（即 "plugins/GameFeatures"）
		// 这行代码的目的是获取项目中所有 GameFeatures 插件的根目录列表。
		List<DirectoryReference> GameFeaturePluginRoots =
			Unreal.GetExtensionDirs(Target.ProjectFile.Directory, Path.Combine("plugins", "GameFeatures"));

		foreach (DirectoryReference SearchDir in GameFeaturePluginRoots)
		{
			//填充容器，获取到所有GameFeatures插件并记录它们的.uplugin信息
			CombinedPluginList.AddRange(PluginsBase.EnumeratePlugins(SearchDir));
		}

		if (CombinedPluginList.Count > 0)
		{
			//记录所有引用到的插件，使用List是因为插件还有其外部依赖关系
			Dictionary<string, List<string>> AllPluginReferencesByName = new Dictionary<string, List<string>>();

			foreach (FileReference PluginFile in CombinedPluginList)
			{
				// PluginFile != null：检查 PluginFile 变量不为空（null）这是为了避免在空引用上调用方法导致的空引用异常
				// FileReference.Exists(PluginFile)：检查由 PluginFile 引用的文件是否实际存在于文件系统中
				// FileReference.Exists() 是 Unreal Engine 构建系统中的一个方法，用于检查指定的文件引用是否对应到实际存在的文件
				// 整个表达式的含义是：只有当 PluginFile 变量不为空，并且它引用的文件确实存在时，整个条件才为真。
				// 这种检查在处理插件文件时非常常见，通常用于：
				// 在尝试加载或处理插件之前验证插件文件的存在性
				// 在构建过程中决定是否包含特定插件
				// 防止因为引用不存在的文件而导致的错误
				if (PluginFile != null && FileReference.Exists(PluginFile))
				{
					//是否使用
					bool bEnabled = false;
					//强制不使用
					bool bForceDisabled = false;

					try
					{
						// 声明一个 JsonObject 类型的变量 RawObject，用于存储插件的 JSON 配置内容
						// 尝试从 AllPluginRootJsonObjectsByName 字典中获取插件的 JSON 对象：
						// PluginFile.GetFileNameWithoutExtension() 获取插件文件的名称（不包含扩展名）作为键
						// TryGetValue 方法尝试从字典中获取对应的值，并将结果存储在 RawObject 中
						// 如果字典中不存在该键，TryGetValue 返回 false
						// 如果字典中没有找到对应的 JSON 对象（条件为真），则：
						// 使用 JsonObject.Read(PluginFile) 从插件文件中读取 JSON 内容
						// 将读取的 JSON 对象添加到 AllPluginRootJsonObjectsByName 字典中，键为插件文件名（不含扩展名）
						// 这段代码实现了一个简单的缓存机制：
						// 首先检查是否已经读取过该插件的 JSON 配置
						// 如果已读取过，直接使用缓存的结果
						// 如果未读取过，则从文件读取并缓存结果
						// 这种方式可以避免重复读取相同的插件文件，提高构建系统的效率，特别是在多次需要访问同一插件配置的场景下。
						JsonObject RawObject;
						if (!AllPluginRootJsonObjectsByName.TryGetValue(PluginFile.GetFileNameWithoutExtension(),
							    out RawObject))
						{
							RawObject = JsonObject.Read(PluginFile);
							AllPluginRootJsonObjectsByName.Add(PluginFile.GetFileNameWithoutExtension(), RawObject);
						}

						// 确认所有游戏功能插件默认均已禁用
						// 如果 EnabledByDefault 为真且某个插件处于禁用状态，则该插件的名称将被嵌入到可执行文件中
						// 如果出现此问题，请启用此警告，并将游戏功能编辑插件模板修改为在新插件中禁用 EnabledByDefault 参数
						// RawObject.TryGetBoolField("EnabledByDefault", out bEnabledByDefault)：
						// 尝试从 JSON 对象中获取名为 "EnabledByDefault" 的布尔字段值。
						// 如果字段不存在或不是布尔类型，TryGetBoolField 返回 false
						bool bEnabledByDefault = false;
						if (!RawObject.TryGetBoolField("EnabledByDefault", out bEnabledByDefault) ||
						    bEnabledByDefault == true)
						{
							//Log.TraceWarning("GameFeaturePlugin {0}, does not set EnabledByDefault to false. This is required for built-in GameFeaturePlugins.", PluginFile.GetFileNameWithoutExtension());
						}

						// 确认所有游戏功能插件均已设置为“明确加载”状态
						// 这点非常重要，因为游戏功能插件需要在项目启动后才进行加载
						bool bExplicitlyLoaded = false;
						if (!RawObject.TryGetBoolField("ExplicitlyLoaded", out bExplicitlyLoaded) ||
						    bExplicitlyLoaded == false)
						{
							logger.LogWarning(
								"GameFeaturePlugin {0}, does not set ExplicitlyLoaded to true. This is required for GameFeaturePlugins.",
								PluginFile.GetFileNameWithoutExtension());
						}

						// You could read an additional field here that is project specific, e.g.,
						// 您在此处还可以添加一个项目特定的额外字段，例如，
						//string PluginReleaseVersion;
						//if (RawObject.TryGetStringField("MyProjectReleaseVersion", out PluginReleaseVersion))
						//{
						//      bEnabled = SomeFunctionOf(PluginReleaseVersion, CurrentReleaseVersion) || bBuildAllGameFeaturePlugins;
						//}

						if (bBuildAllGameFeaturePlugins)
						{
							// 我们目前处于这样一种状态：我们需要所有的游戏功能插件，但不包括那些我们无法加载或编译的插件。
							bEnabled = true;
						}

						// 防止在非编辑器版本中使用仅适用于编辑器的功能插件
						bool bEditorOnly = false;
						if (!RawObject.TryGetBoolField("EditorOnly", out bEditorOnly))
						{
							if (bEditorOnly && Target.Type != TargetType.Editor && !bBuildAllGameFeaturePlugins)
							{
								//该插件适用于编辑器，构建非编辑器版本时，插件被禁用
								bForceDisabled = true;
							}
						}
						else
						{
							//编辑器下专用
						}

						//  有些插件仅应在特定分支中可用
						string RestrictToBranch;
						if (RawObject.TryGetStringField("RestrictToBranch", out RestrictToBranch))
						{
							// 比较当前目标的分支名称 Target.Version.BranchName 与插件指定的分支名称 RestrictToBranch
							// 比较时忽略大小写（StringComparison.OrdinalIgnoreCase）
							if (!Target.Version.BranchName.Equals(RestrictToBranch, StringComparison.OrdinalIgnoreCase))
							{
								// 该插件是针对特定分支设计的，而这里并非该分支。
								bForceDisabled = true;
								logger.LogDebug(
									"GameFeaturePlugin {Name} was marked as restricted to other branches. Disabling.",
									PluginFile.GetFileNameWithoutExtension());
							}
							else
							{
								logger.LogDebug(
									"GameFeaturePlugin {Name} was marked as restricted to this branch. Leaving enabled.",
									PluginFile.GetFileNameWithoutExtension());
							}
						}

						// 可以将插件标记为“从不编译”，这将覆盖上述设置。
						bool bNeverBuild = false;
						if (RawObject.TryGetBoolField("NeverBuild", out bNeverBuild) && bNeverBuild)
						{
							// 此插件已被标记为永远不进行编译，所以请勿进行编译操作。
							bForceDisabled = true;
							logger.LogDebug("GameFeaturePlugin {Name} was marked as NeverBuild, disabling.",
								PluginFile.GetFileNameWithoutExtension());
						}

						//记录插件的引用信息，以便后续进行验证操作
						JsonObject[] PluginReferencesArray;
						if (RawObject.TryGetObjectArrayField("Plugins", out PluginReferencesArray))
						{
							foreach (JsonObject ReferenceObject in PluginReferencesArray)
							{
								bool bRefEnabled = false;
								if (ReferenceObject.TryGetBoolField("Enabled", out bRefEnabled) && bRefEnabled == true)
								{
									string PluginReferenceName;
									if (ReferenceObject.TryGetStringField("Name", out PluginReferenceName))
									{
										string ReferenceName = PluginFile.GetFileNameWithoutExtension();
										//确定 Dictionary<TKey,TValue> 是否包含指定的键。
										if (!AllPluginReferencesByName.ContainsKey(ReferenceName))
										{
											AllPluginReferencesByName[ReferenceName] = new List<string>();
										}

										AllPluginReferencesByName[ReferenceName].Add(PluginReferenceName);
									}
								}
							}
						}
					}
					catch (Exception ParseException)
					{
						//如何有任何错误就强制关闭
						logger.LogWarning("Failed to parse GameFeaturePlugin file {Name}, disabling. Exception: {1}",
							PluginFile.GetFileNameWithoutExtension(), ParseException.Message);
						bForceDisabled = true;
					}

					//禁用状态优先于启用状态
					if (bForceDisabled)
					{
						bEnabled = false;
					}

					// 输出此插件的最终决策结果
					logger.LogDebug("ConfigureGameFeaturePlugins() has decided to {Action} feature {Name}",
						bEnabled ? "enable" : (bForceDisabled ? "disable" : "ignore"),
						PluginFile.GetFileNameWithoutExtension());

					//启用或禁用分组
					if (bEnabled)
					{
						Target.EnablePlugins.Add(PluginFile.GetFileNameWithoutExtension());
					}
					else if (bForceDisabled)
					{
						Target.DisablePlugins.Add(PluginFile.GetFileNameWithoutExtension());
					}
				}
			}

			// 如果您使用的是某个发布版本，请考虑进行参考性验证，以确保那些发布版本较早的插件不会依赖于发布版本较晚的内容。
		}
	}
}