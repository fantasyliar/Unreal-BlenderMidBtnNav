#pragma once
#include "INavHandler.h"
#include "EditorViewportClient.h"

class FLevelNavHandler : public INavHandler
{
public:
	FLevelNavHandler();
	virtual ~FLevelNavHandler() = default;

	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent, TSharedPtr<SWidget> TargetWidget) override;
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual void ResetState() override;

private:
	bool bIsOrbiting;
	bool bIsPanning;
	bool bIsZooming;
    
	float LockedDistance;	// 按下中键瞬间，相机到中心点的距离
	float CurrentOrbitYaw;
	float CurrentOrbitPitch;
	FVector OrbitPivot;
	FVector DynOffset;

	// 缓存当前的 3D 视口客户端
	FEditorViewportClient* ViewportClient;
};
