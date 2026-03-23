// MidBtnNav.h
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

// 前向声明处理器类
class FMidBtnNavProcessor;

class FMidBtnNavModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	// 智能指针
	TSharedPtr<FMidBtnNavProcessor> SceneInputProcessor;
};