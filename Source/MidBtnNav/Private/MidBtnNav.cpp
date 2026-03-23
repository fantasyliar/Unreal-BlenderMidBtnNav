// MidBtnNav.cpp
#include "MidBtnNav.h"
#include "MidBtnNavProcessor.h" 
#include "Framework/Application/SlateApplication.h"

void FMidBtnNavModule::StartupModule()
{
    // 确保 Slate 已经初始化
    if (FSlateApplication::IsInitialized())
    {
        // 实例化处理器
        SceneInputProcessor = MakeShareable(new FMidBtnNavProcessor());
        
        // 注册
        FSlateApplication::Get().RegisterInputPreProcessor(SceneInputProcessor);
    }
}

void FMidBtnNavModule::ShutdownModule()
{
    // 卸载时注销
    if (FSlateApplication::IsInitialized() && SceneInputProcessor.IsValid())
    {
        FSlateApplication::Get().UnregisterInputPreProcessor(SceneInputProcessor);
        SceneInputProcessor.Reset();
    }
}

IMPLEMENT_MODULE(FMidBtnNavModule, MidBtnNav)