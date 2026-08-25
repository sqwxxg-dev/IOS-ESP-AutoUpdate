#ifndef DRAW_HELPERS_MM
#define DRAW_HELPERS_MM

#import <UIKit/UIKit.h>
#include "IMGUI/imgui.h"      // Только для типов ImVec2, ImU32, IM_COL32
#include "IL2CPP/Vector3.h"   // Для Vector3

// ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ (ОСТАВЛЯЕМ)
static inline UIColor* UIColorFromImU32(ImU32 color) {
    return [UIColor colorWithRed:((color >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f
                           green:((color >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f
                            blue:((color >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f
                           alpha:((color >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f];
}

// ============================================================
// ВНИМАНИЕ! Здесь только ОБЪЯВЛЕНИЯ функций, но нет их тела.
// Это нужно, чтобы компилятор знал, что они существуют,
// но реализация будет взята из ImGuiDrawView.mm.
// Это решает проблему "duplicate symbols".
// ============================================================

void DrawESPLine(CGContextRef context, ImVec2 start, ImVec2 end, ImU32 color, float thickness);
void DrawESPBox2D(CGContextRef context, ImVec2 min, ImVec2 max, ImU32 color, float thickness);
void DrawESPBox3D(CGContextRef context, ImVec2 pts[8], bool visible[8], ImU32 color, float thickness);
void DrawESPCorners(CGContextRef context, ImVec2 min, ImVec2 max, float length, ImU32 color, float thickness);
void DrawESPDistance(CGContextRef context, ImVec2 position, float distance, ImU32 color);
void DrawESPSkeleton(CGContextRef context, void* camera, void* gameObject, Vector3 cameraPosition, float distanceToCamera);

#endif // DRAW_HELPERS_MM
