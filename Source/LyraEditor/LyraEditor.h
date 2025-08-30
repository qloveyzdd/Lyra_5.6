// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

//用于在头文件中声明一个日志类别，使其可以在多个源文件中使用。
// LogLyraEditor: 日志类别的名称
// Log: 运行时默认的日志级别
// All: 编译时的日志级别（高于此级别的日志会在编译时被移除）

// Fatal: 致命错误，会导致程序崩溃
// Error: 错误，但不会导致程序崩溃
// Warning: 警告信息
// Display: 总是显示的重要信息
// Log: 一般日志信息
// Verbose: 详细日志
// VeryVerbose: 非常详细的日志
// All: 所有日志级别
DECLARE_LOG_CATEGORY_EXTERN(LogLyraEditor, Log, All);