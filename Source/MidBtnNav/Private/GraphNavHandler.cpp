#include "GraphNavHandler.h"

FGraphNavHandler::FGraphNavHandler()
    : bIsGraphPanning(false), bIsGraphZooming(false)
{
}

void FGraphNavHandler::ResetState()
{
    bIsGraphPanning = bIsGraphZooming = false;
    ActivePanel.Reset();
}

bool FGraphNavHandler::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent, TSharedPtr<SWidget> TargetWidget)
{
    ActivePanel = StaticCastSharedPtr<SGraphPanel>(TargetWidget);
    if (!ActivePanel.IsValid()) return false;
    if (MouseEvent.IsControlDown()) 
        { 
            bIsGraphZooming = true; 
            return true; 
        }
    else
    {
        bIsGraphPanning = true;
    }
    return true; 
}

bool FGraphNavHandler::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    if (!bIsGraphPanning && !bIsGraphZooming) return false;

    if (TSharedPtr<SGraphPanel> GraphPanel = ActivePanel.Pin())
    {
        const FVector2D CursorDelta = MouseEvent.GetCursorDelta();
        
        //有一点延迟
        if (bIsGraphPanning)
        {
            const FGeometry& CachedGeom = GraphPanel->GetCachedGeometry();
            
            FVector2D ScreenPos = MouseEvent.GetScreenSpacePosition();
            // 通过当前屏幕坐标和真实的 Delta 反推上一个准确的屏幕坐标
            FVector2D ScreenLastPos = ScreenPos - MouseEvent.GetCursorDelta(); 
            
            // 转换到局部坐标，消除 DPI 和缩放误差
            FVector2D LocalCurrentPos = CachedGeom.AbsoluteToLocal(ScreenPos);
            FVector2D LocalLastPos = CachedGeom.AbsoluteToLocal(ScreenLastPos);
            FVector2D LocalDelta = LocalCurrentPos - LocalLastPos;
            
            // 应用偏移
            FVector2D CurrentViewOffset = GraphPanel->GetViewOffset();
            float CurrentZoom = GraphPanel->GetZoomAmount();
            
            FVector2D NewViewOffset = CurrentViewOffset - (LocalDelta / CurrentZoom);
            GraphPanel->RestoreViewSettings(NewViewOffset, CurrentZoom);
        }
        
        else if (bIsGraphZooming)
        {
            if (!GraphPanel->GetCachedGeometry().IsUnderLocation(MouseEvent.GetScreenSpacePosition()))
                return false;
            
            
            MouseAccumulator += CursorDelta.Y;
            constexpr float MovementThreshold = 30.0f; 
            if (FMath::Abs(MouseAccumulator) >= MovementThreshold)
            {
                float WheelDelta = MouseAccumulator < 0 ? 1.0f : -1.0f;
                // 构造假的滚轮事件
                FPointerEvent WheelEvent(
                    MouseEvent.GetPointerIndex(),
                    MouseEvent.GetScreenSpacePosition(),
                    MouseEvent.GetLastScreenSpacePosition(),
                    MouseEvent.GetPressedButtons(),
                    EKeys::MouseWheelAxis,
                    WheelDelta,
                    MouseEvent.GetModifierKeys()
                );

                GraphPanel->OnMouseWheel(GraphPanel->GetCachedGeometry(), WheelEvent);
                
                MouseAccumulator = 0.0f;
            }
        }
        return true;
    }
    return false;
}

bool FGraphNavHandler::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    if (bIsGraphPanning || bIsGraphZooming)
    {
        ResetState();
        return true;
    }
    return false;
}