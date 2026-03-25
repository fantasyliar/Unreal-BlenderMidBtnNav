#pragma once
#include "CoreMinimal.h"
#include "Input/Events.h"
#include "Widgets/SWidget.h"

class FSlateApplication;

class INavHandler
{
public:
	virtual ~INavHandler() = default;

	// Processor 捕获当前窗口指针 TargetWidget
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent, TSharedPtr<SWidget> TargetWidget) = 0;
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) = 0;
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) = 0;
    
	// 异常中断时重置状态
	virtual void ResetState() = 0; 
};