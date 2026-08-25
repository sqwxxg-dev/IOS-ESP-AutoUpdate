#ifndef DRAW_HELPERS_MM
#define DRAW_HELPERS_MM

#import <UIKit/UIKit.h>
#include "IMGUI/imgui.h"
#include "IL2CPP/Vector3.h"
#include "ESPConfig.h"

// ========== ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ ==========
static inline UIColor* UIColorFromImU32(ImU32 color) {
    return [UIColor colorWithRed:((color >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f
                           green:((color >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f
                            blue:((color >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f
                           alpha:((color >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f];
}

// ========== РЕАЛИЗАЦИИ ФУНКЦИЙ ==========

void DrawESPLine(CGContextRef context, ImVec2 start, ImVec2 end, ImU32 color, float thickness) {
    if (!context) return;
    CGContextSetStrokeColorWithColor(context, [UIColor colorWithWhite:0.0 alpha:0.5].CGColor);
    CGContextSetLineWidth(context, thickness + 1.0f);
    CGContextMoveToPoint(context, start.x, start.y);
    CGContextAddLineToPoint(context, end.x, end.y);
    CGContextStrokePath(context);
    
    CGContextSetStrokeColorWithColor(context, UIColorFromImU32(color).CGColor);
    CGContextSetLineWidth(context, thickness);
    CGContextMoveToPoint(context, start.x, start.y);
    CGContextAddLineToPoint(context, end.x, end.y);
    CGContextStrokePath(context);
}

void DrawESPBox2D(CGContextRef context, ImVec2 min, ImVec2 max, ImU32 color, float thickness) {
    if (!context) return;
    CGRect rect = CGRectMake(min.x, min.y, max.x - min.x, max.y - min.y);
    CGContextSetStrokeColorWithColor(context, [UIColor colorWithWhite:0.0 alpha:0.5].CGColor);
    CGContextSetLineWidth(context, thickness + 1.0f);
    CGContextStrokeRect(context, rect);
    
    CGContextSetStrokeColorWithColor(context, UIColorFromImU32(color).CGColor);
    CGContextSetLineWidth(context, thickness);
    CGContextStrokeRect(context, rect);
}

void DrawESPBox3D(CGContextRef context, ImVec2 pts[8], bool visible[8], ImU32 color, float thickness) {
    if (!context) return;
    static int edges[12][2] = {{0,1}, {1,2}, {2,3}, {3,0}, {4,5}, {5,6}, {6,7}, {7,4}, {0,4}, {1,5}, {2,6}, {3,7}};
    CGContextSetStrokeColorWithColor(context, UIColorFromImU32(color).CGColor);
    CGContextSetLineWidth(context, thickness);
    for (int eidx = 0; eidx < 12; eidx++) {
        int a = edges[eidx][0], b = edges[eidx][1];
        if (visible[a] && visible[b]) {
            CGContextMoveToPoint(context, pts[a].x, pts[a].y);
            CGContextAddLineToPoint(context, pts[b].x, pts[b].y);
            CGContextStrokePath(context);
        }
    }
}

void DrawESPCorners(CGContextRef context, ImVec2 min, ImVec2 max, float length, ImU32 color, float thickness) {
    if (!context) return;
    CGContextSetStrokeColorWithColor(context, UIColorFromImU32(color).CGColor);
    CGContextSetLineWidth(context, thickness);
    
    CGContextMoveToPoint(context, min.x, min.y);
    CGContextAddLineToPoint(context, min.x + length, min.y);
    CGContextMoveToPoint(context, min.x, min.y);
    CGContextAddLineToPoint(context, min.x, min.y + length);
    
    CGContextMoveToPoint(context, max.x, min.y);
    CGContextAddLineToPoint(context, max.x - length, min.y);
    CGContextMoveToPoint(context, max.x, min.y);
    CGContextAddLineToPoint(context, max.x, min.y + length);
    
    CGContextMoveToPoint(context, min.x, max.y);
    CGContextAddLineToPoint(context, min.x + length, max.y);
    CGContextMoveToPoint(context, min.x, max.y);
    CGContextAddLineToPoint(context, min.x, max.y - length);
    
    CGContextMoveToPoint(context, max.x, max.y);
    CGContextAddLineToPoint(context, max.x - length, max.y);
    CGContextMoveToPoint(context, max.x, max.y);
    CGContextAddLineToPoint(context, max.x, max.y - length);
    
    CGContextStrokePath(context);
}

void DrawESPDistance(CGContextRef context, ImVec2 position, float distance, ImU32 color) {
    if (!context) return;
    char distBuf[32];
    snprintf(distBuf, sizeof(distBuf), "%dM", (int)distance);
    NSString *text = [NSString stringWithUTF8String:distBuf];
    
    NSDictionary *shadowAttributes = @{
        NSFontAttributeName: [UIFont systemFontOfSize:12 weight:UIFontWeightBold],
        NSForegroundColorAttributeName: [UIColor blackColor]
    };
    NSDictionary *textAttributes = @{
        NSFontAttributeName: [UIFont systemFontOfSize:12 weight:UIFontWeightBold],
        NSForegroundColorAttributeName: UIColorFromImU32(color)
    };
    CGPoint drawPoint = CGPointMake(position.x + 5, position.y - 12);
    [text drawAtPoint:CGPointMake(drawPoint.x + 1, drawPoint.y + 1) withAttributes:shadowAttributes];
    [text drawAtPoint:drawPoint withAttributes:textAttributes];
}

void DrawESPSkeleton(CGContextRef context, void* camera, void* gameObject, Vector3 cameraPosition, float distanceToCamera) {
    // Пустая реализация, так как скелетон рисуется прямо в ImGuiDrawView.mm
    // Эта функция оставлена для совместимости
}

#endif // DRAW_HELPERS_MM
