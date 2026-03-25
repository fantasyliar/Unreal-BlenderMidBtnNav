#pragma once

#include "CoreMinimal.h"
#include "INavHandler.h"
#include "Framework/Application/IInputProcessor.h"

class FEditorViewportClient; // 前向声明

class FMidBtnNavProcessor : public IInputProcessor
{
public:
	FMidBtnNavProcessor();
	virtual ~FMidBtnNavProcessor() override = default;

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent,const FPointerEvent* InGestureEvent) override { return false; }
    
	virtual const TCHAR* GetDebugName() const override { return TEXT("CtrlScaleNavProcessor"); }

private:
	// 辅助函数
	static TSharedPtr<SWidget> GetWidgetUnderMouse(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent, FString& OutWidgetType);

	// 持有具体的逻辑处理器
	TSharedPtr<INavHandler> LevelHandler;
	TSharedPtr<INavHandler> GraphHandler;
	// 指向当前正在接管输入的处理器
	INavHandler* ActiveHandler;
};