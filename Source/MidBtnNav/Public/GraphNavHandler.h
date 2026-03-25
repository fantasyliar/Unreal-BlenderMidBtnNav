#pragma once
#include "INavHandler.h"
#include "SGraphPanel.h"

class FGraphNavHandler : public INavHandler
{
public:
	FGraphNavHandler();
	virtual ~FGraphNavHandler() = default;
	
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent, TSharedPtr<SWidget> TargetWidget) override;
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	
	virtual void ResetState() override;

private:
	bool bIsGraphPanning;
	bool bIsGraphZooming;
	float MouseAccumulator = 0.0f;
	
	TWeakPtr<SGraphPanel> ActivePanel;
	
};