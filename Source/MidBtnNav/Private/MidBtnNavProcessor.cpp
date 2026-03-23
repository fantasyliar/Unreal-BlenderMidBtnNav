#include "MidBtnNavProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "SLevelViewport.h"
#include "EditorViewportClient.h"
#include "Editor.h"
#include "Selection.h"

FMidBtnNavProcessor::FMidBtnNavProcessor()
    : bIsOrbiting(false), bIsPanning(false), bIsZooming(false), LockedDistance(0), CurrentOrbitYaw(0),CurrentOrbitPitch(0)
{
}

static FVector GetSelectionCenter()
{
    if (!GEditor)
        return FVector::ZeroVector;

    USelection* SelectedActors = GEditor->GetSelectedActors();
    if (!SelectedActors || SelectedActors->Num() == 0)
        return FVector::ZeroVector;
    
    //取包围盒中心
    FBox Bounds(ForceInit);
    for (FSelectionIterator It(*SelectedActors); It; ++It)
    {
        if (AActor* Actor = Cast<AActor>(*It))
        {
            // 使用组件的边界框累加，获取几何中心
            Bounds += Actor->GetComponentsBoundingBox(true);
        }
    }

    return Bounds.GetCenter();
}
void FMidBtnNavProcessor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) {}

bool FMidBtnNavProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() != EKeys::MiddleMouseButton)
    {
        return false; // 除了鼠标中键都不拦截
    }
    if (FEditorViewportClient* ViewportClient = GetLevelViewportClientUnderMouse(SlateApp, MouseEvent))
    {
        // 根据修饰键判断进入哪种模式
        if (MouseEvent.IsControlDown()) bIsZooming = true;
        else if (MouseEvent.IsShiftDown()) bIsPanning = true;
        else
        {
            bIsOrbiting = true;
            
            FRotator CurrentRot = ViewportClient->GetViewRotation();
            FVector ViewLoc = ViewportClient->GetViewLocation();
            
            // 获取轴心
            OrbitPivot = GetSelectionCenter();
            if (OrbitPivot.IsZero()) OrbitPivot = ViewportClient->GetLookAtLocation();

            // 计算相机到球心的向量
            FVector ToPivot = OrbitPivot - ViewLoc;
            LockedDistance = ToPivot.Size();
           
            CurrentOrbitYaw = CurrentRot.Yaw;
            CurrentOrbitPitch = CurrentRot.Pitch;
            // 判断是否倒置：检查 Up 向量与世界 Z 的点积
            FVector CameraUp = FRotationMatrix(CurrentRot).GetScaledAxis(EAxis::Z);
            if ((CameraUp | FVector::UpVector) < 0.f)// 如果倒立，CurrentRot将Yaw翻转，Pitch限制在+-90
            {
                CurrentOrbitYaw -= 180.0f; 
                CurrentOrbitPitch = 180.0f - CurrentOrbitPitch;
            }
            
            // dyn_ofs：pivot 在相机局部空间的位置
            FQuat ViewQuat = CurrentRot.Quaternion();
            DynOffset = ViewQuat.Inverse().RotateVector(ToPivot);
        }
        return true; // 拦截事件，屏蔽原有功能，以最先注册的插件为准
    }
    return false;
}

bool FMidBtnNavProcessor::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    if (MouseEvent.GetEffectingButton() == EKeys::MiddleMouseButton)
    {
        if (bIsOrbiting || bIsPanning || bIsZooming)
        {
            bIsOrbiting = bIsPanning = bIsZooming = false;
            return true;
        }
    }
    return false;
}

bool FMidBtnNavProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
    if (bIsOrbiting || bIsPanning || bIsZooming)
    {
        FEditorViewportClient* ViewportClient = GetLevelViewportClientUnderMouse(SlateApp, MouseEvent);
        if (ViewportClient)
        {
            const FVector2D CursorDelta = MouseEvent.GetCursorDelta();
        
            // 获取基础速度
            float BaseSpeed = ViewportClient->GetCameraSpeed(); 
            // 计算基于距离的缩放比例：获取相机当前位置和焦点位置之间的距离
            float DistanceToFocus = (ViewportClient->GetLookAtLocation() - ViewportClient->GetViewLocation()).Size();
            // 将距离转化为倍率：距离 1000cm 时倍率为 1。Clamp 限制范围，防止太近太慢，或太远太快
            float CustomSpeedScale = FMath::Clamp(DistanceToFocus / 1000.0f, 0.05f, 50.0f);
            // 综合灵敏度设置
            float FinalSensitivity = BaseSpeed * CustomSpeedScale * 0.1f;

            FVector CombinedMoveDelta = FVector::ZeroVector;
            FRotator CombinedRotateDelta = FRotator::ZeroRotator;
            
            if (bIsOrbiting)
            {
                // 更新经纬度累加
                constexpr float RotationSpeed = 0.25f;
                CurrentOrbitYaw += CursorDelta.X * RotationSpeed;
                CurrentOrbitPitch -= CursorDelta.Y * RotationSpeed;

                // 计算球面坐标的 Forward 向量
                // FRotator会钳制pinch的范围，在+-90°将无法继续旋转
                float CP, SP, CY, SY;
                FMath::SinCos(&SP, &CP, FMath::DegreesToRadians(CurrentOrbitPitch));
                FMath::SinCos(&SY, &CY, FMath::DegreesToRadians(CurrentOrbitYaw));
                FVector ForwardDir(CP * CY, CP * SY, SP);

                // 动态参考向量处理
                // Pitch 在 90~270 度之间，相机z指向世界z负向。动态Up应该反向才能维持旋转。不反向会导致叉乘反号。
                // 使用余弦值 CP 的符号来判断相机是否处于倒置半球
                float UpSign = (CP >= 0) ? 1.0f : -1.0f;
                FVector DynamicUp = FVector::UpVector * UpSign;

                // 使用 MakeFromXZ 构造旋转矩阵
                // Z轴参考 = DynamicUp，与X叉乘得到的Y轴垂直于世界Z轴，保证绝对水平
                // 最终的Z向量不是DynamicUp，而是先算好Y轴再叉乘得到的Z
                // 如果恰好90°，叉乘结果不会是零向量，因为MakeFromXZ把世界Z换成世界X了，但还是会突变，只是很难恰好等于这个值
                FMatrix RotMatrix = FRotationMatrix::MakeFromXZ(ForwardDir, DynamicUp);
    
                // 计算新位置，dyn_ofs 位置补偿
                FVector NewLoc = OrbitPivot - RotMatrix.TransformVector(DynOffset);
                
                // 更新视口
                ViewportClient->SetViewLocation(NewLoc);
                ViewportClient->SetViewRotation(RotMatrix.Rotator());
                ViewportClient->SetLookAtLocation(OrbitPivot);

                return true;
            }
            else
            {
                const FRotator CameraRot = ViewportClient->GetViewRotation();
                const FVector Forward = CameraRot.Vector();
                const FVector Right = FRotationMatrix(CameraRot).GetScaledAxis(EAxis::Y);
                const FVector Up = FRotationMatrix(CameraRot).GetScaledAxis(EAxis::Z);

                if (bIsPanning)
                {
                    CombinedMoveDelta = (Right * -CursorDelta.X + Up * CursorDelta.Y) * FinalSensitivity * 0.25f;
                }
                else if (bIsZooming)
                {
                    CombinedMoveDelta = Forward * (CursorDelta.Y * FinalSensitivity);
                }
            }

            ViewportClient->MoveViewportCamera(CombinedMoveDelta, CombinedRotateDelta, true);
            ViewportClient->Invalidate();
            return true;
        }
    }
    return false;
}

FEditorViewportClient* FMidBtnNavProcessor::GetLevelViewportClientUnderMouse(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) const
{
    FWidgetPath WidgetPath = SlateApp.LocateWindowUnderMouse(MouseEvent.GetScreenSpacePosition(), SlateApp.GetInteractiveTopLevelWindows());
    
    if (WidgetPath.IsValid())
    {
        
        for (int32 i = WidgetPath.Widgets.Num() - 1; i >= 0; --i)
        {
            if (WidgetPath.Widgets[i].Widget->GetTypeAsString() == TEXT("SLevelViewport"))
            {
                TSharedRef<SLevelViewport> LevelViewport = StaticCastSharedRef<SLevelViewport>(WidgetPath.Widgets[i].Widget);
                return LevelViewport->GetViewportClient().Get();
            }
        }
    }
    return nullptr;
}

