#pragma once

#include "CoreMinimal.h"
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
	bool bIsOrbiting; 
	bool bIsPanning; 
	bool bIsZooming;
	
	float LockedDistance;     // 按下中键瞬间，相机到中心点的距离
	float CurrentOrbitYaw;    
	float CurrentOrbitPitch;  
	
	FVector OrbitPivot;
	FVector DynOffset;
	
	// 辅助函数
	FEditorViewportClient* GetLevelViewportClientUnderMouse(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) const;
};