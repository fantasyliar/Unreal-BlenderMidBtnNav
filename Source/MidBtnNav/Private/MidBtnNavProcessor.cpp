#include "MidBtnNavProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "EditorViewportClient.h"
#include "Editor.h"
#include "LevelNavHandler.h"
#include "GraphNavHandler.h"

FMidBtnNavProcessor::FMidBtnNavProcessor()
    : ActiveHandler(nullptr)
{
    // 初始化子处理器
    LevelHandler = MakeShared<FLevelNavHandler>();
    GraphHandler = MakeShared<FGraphNavHandler>();
}

void FMidBtnNavProcessor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) {}

// 获取鼠标下方 Widget
TSharedPtr<SWidget> FMidBtnNavProcessor::GetWidgetUnderMouse(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent, FString& OutWidgetType)
{
    FWidgetPath WidgetPath = SlateApp.LocateWindowUnderMouse(MouseEvent.GetScreenSpacePosition(), SlateApp.GetInteractiveTopLevelWindows());
    if (WidgetPath.IsValid())
    {
        for (int32 i = WidgetPath.Widgets.Num() - 1; i >= 0; --i)
        {
            FString Type = WidgetPath.Widgets[i].Widget->GetTypeAsString();
            if (Type == TEXT("SLevelViewport") || Type == TEXT("SGraphPanel"))
            {
                OutWidgetType = Type;
                return WidgetPath.Widgets[i].Widget;
            }
        }
    }
    return nullptr;
}

bool FMidBtnNavProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    //只处理中键
    if (MouseEvent.GetEffectingButton() != EKeys::MiddleMouseButton) return false;

    FString WidgetType;
    if (TSharedPtr<SWidget> TargetWidget = GetWidgetUnderMouse(SlateApp, MouseEvent, WidgetType))
    {
        if (WidgetType == TEXT("SLevelViewport"))
        {
            if (LevelHandler->HandleMouseButtonDownEvent(SlateApp, MouseEvent, TargetWidget))
            {
                ActiveHandler = LevelHandler.Get();
                return true;
            }
        }
        else if (WidgetType == TEXT("SGraphPanel"))
        {
            if (GraphHandler->HandleMouseButtonDownEvent(SlateApp, MouseEvent, TargetWidget))
            {
                ActiveHandler = GraphHandler.Get();
                return true;
            }
        }
    }
    return false;
}

bool FMidBtnNavProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    if (ActiveHandler)
    {
        return ActiveHandler->HandleMouseMoveEvent(SlateApp, MouseEvent);
    }
    return false;
}

bool FMidBtnNavProcessor::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton && ActiveHandler)
    {
        bool bHandled = ActiveHandler->HandleMouseButtonUpEvent(SlateApp, MouseEvent);
        ActiveHandler = nullptr; // 清除指针
        return bHandled;
    }
    return false;
}