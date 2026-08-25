#ifndef DRAW_HELPERS_MM
#define DRAW_HELPERS_MM

// ========== ПОДКЛЮЧАЕМ ВСЕ НЕОБХОДИМЫЕ ЗАГОЛОВКИ ==========
#import <string>
#import <vector>
#import <algorithm>
#import <UIKit/UIKit.h>
#include "IMGUI/imgui.h"          // Для ImVec2, ImU32, IM_COL32
#include "IL2CPP/Vector3.h"       // Для Vector3
#include "IL2CPP/Hooks.h"         // Для monoArray, Object_FindObjectsOfType
#include "ESPConfig.h"            // Для макросов CREATE_TYPE_STRING
// ===========================================================

// ========== ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ ДЛЯ ПРЕОБРАЗОВАНИЯ ЦВЕТА ==========
static inline UIColor* UIColorFromImU32(ImU32 color) {
    return [UIColor colorWithRed:((color >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f
                           green:((color >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f
                            blue:((color >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f
                           alpha:((color >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f];
}

// ========== АДАПТИРОВАННЫЕ ФУНКЦИИ РИСОВАНИЯ (CGContextRef) ==========

void DrawESPLine(CGContextRef context, ImVec2 start, ImVec2 end, ImU32 color, float thickness) {
    if (!context) return;
    
    // Чёрная обводка (тень)
    CGContextSetStrokeColorWithColor(context, [UIColor colorWithWhite:0.0 alpha:0.5].CGColor);
    CGContextSetLineWidth(context, thickness + 1.0f);
    CGContextMoveToPoint(context, start.x, start.y);
    CGContextAddLineToPoint(context, end.x, end.y);
    CGContextStrokePath(context);
    
    // Основная линия
    CGContextSetStrokeColorWithColor(context, UIColorFromImU32(color).CGColor);
    CGContextSetLineWidth(context, thickness);
    CGContextMoveToPoint(context, start.x, start.y);
    CGContextAddLineToPoint(context, end.x, end.y);
    CGContextStrokePath(context);
}

void DrawESPBox2D(CGContextRef context, ImVec2 min, ImVec2 max, ImU32 color, float thickness) {
    if (!context) return;
    
    CGRect rect = CGRectMake(min.x, min.y, max.x - min.x, max.y - min.y);
    
    // Чёрная обводка (тень)
    CGContextSetStrokeColorWithColor(context, [UIColor colorWithWhite:0.0 alpha:0.5].CGColor);
    CGContextSetLineWidth(context, thickness + 1.0f);
    CGContextStrokeRect(context, rect);
    
    // Основной прямоугольник
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
    
    // Левый верхний угол
    CGContextMoveToPoint(context, min.x, min.y);
    CGContextAddLineToPoint(context, min.x + length, min.y);
    CGContextMoveToPoint(context, min.x, min.y);
    CGContextAddLineToPoint(context, min.x, min.y + length);
    
    // Правый верхний угол
    CGContextMoveToPoint(context, max.x, min.y);
    CGContextAddLineToPoint(context, max.x - length, min.y);
    CGContextMoveToPoint(context, max.x, min.y);
    CGContextAddLineToPoint(context, max.x, min.y + length);
    
    // Левый нижний угол
    CGContextMoveToPoint(context, min.x, max.y);
    CGContextAddLineToPoint(context, min.x + length, max.y);
    CGContextMoveToPoint(context, min.x, max.y);
    CGContextAddLineToPoint(context, min.x, max.y - length);
    
    // Правый нижний угол
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
    
    // Рисуем текст с чёрной тенью
    NSDictionary *shadowAttributes = @{
        NSFontAttributeName: [UIFont systemFontOfSize:12 weight:UIFontWeightBold],
        NSForegroundColorAttributeName: [UIColor blackColor]
    };
    
    NSDictionary *textAttributes = @{
        NSFontAttributeName: [UIFont systemFontOfSize:12 weight:UIFontWeightBold],
        NSForegroundColorAttributeName: UIColorFromImU32(color)
    };
    
    CGPoint drawPoint = CGPointMake(position.x + 5, position.y - 12);
    
    // Тень
    [text drawAtPoint:CGPointMake(drawPoint.x + 1, drawPoint.y + 1) withAttributes:shadowAttributes];
    // Основной текст
    [text drawAtPoint:drawPoint withAttributes:textAttributes];
}

void DrawESPSkeleton(CGContextRef context, void* camera, void* gameObject, Vector3 cameraPosition, float distanceToCamera) {
    if (!context || !gameObject) return;
    
    // Используем правильные макросы из ESPConfig.h
    void* GameObjectType = Type_GetType(String_CreateString(CREATE_TYPE_STRING(GAMEOBJECT_CLASS_NAME, GAMEOBJECT_ASSEMBLY_NAME)));
    if (!GameObjectType) return;
    
    monoArray<void**>* allGameObjects = Object_FindObjectsOfType(GameObjectType);
    if (!allGameObjects) return;
    
    for (int i = 0; i < allGameObjects->getLength(); i++) {
        void* gameObject = allGameObjects->getPointer()[i];
        if (!gameObject || !GameObject_get_activeInHierarchy(gameObject)) continue;
        
        void* transform = GameObject_get_transform(gameObject);
        if (!transform) continue;
        
        Vector3 position = Transform_get_position(transform);
        if (position.x == 0 && position.y == 0 && position.z == 0) continue;
        
        float distanceToCamera = Vector3::Distance(cameraPosition, position);
        if (distanceToCamera > ESP_MAX_DISTANCE) continue;
        
        void* foundSkinned = nullptr;
        void* ComponentType = Type_GetType(String_CreateString(CREATE_TYPE_STRING(COMPONENT_CLASS_NAME, COMPONENT_ASSEMBLY_NAME)));
        monoArray<void**>* componentArrayForDraw = GameObject_GetComponentsInternal(gameObject, ComponentType, false, false, false, false, nullptr);
        
        if (componentArrayForDraw) {
            for (int jj = 0; jj < componentArrayForDraw->getLength(); jj++) {
                void* compPtr = componentArrayForDraw->getPointer()[jj];
                if (!compPtr) continue;
                Il2CppClassMetadata* meta = *(Il2CppClassMetadata**)compPtr;
                const char* klass = meta->name;
                const char* namespaze = meta->namespaze;
                std::string typeKey = (namespaze && strlen(namespaze) > 0) ? (std::string(namespaze) + "." + std::string(klass)) : std::string(klass);
                
                if (typeKey.find(SKINNED_MESH_RENDERER_CLASS_NAME) != std::string::npos) {
                    foundSkinned = compPtr;
                    break;
                }
            }
        }
        
        if (foundSkinned) {
            monoArray<void**>* bones = SkinnedMeshRenderer_get_bones(foundSkinned);
            if (bones && bones->getLength() > MIN_BONE_COUNT) {
                const int boneCount = bones->getLength();
                
                struct BoneInfo {
                    void* transform;
                    int parentIndex;
                    Vector3 worldPos;
                    ImVec2 screenPos;
                    bool isVisible;
                    bool isImportant;
                    float importance;
                };
                
                std::vector<BoneInfo> boneInfos;
                boneInfos.reserve(boneCount);
                
                for (int bi = 0; bi < boneCount; bi++) {
                    void* boneTr = bones->getPointer()[bi];
                    if (!boneTr) continue;
                    
                    BoneInfo info = {};
                    info.transform = boneTr;
                    info.worldPos = Transform_get_position(boneTr);
                    info.parentIndex = -1;
                    
                    void* parentTr = Transform_get_parent(boneTr);
                    if (parentTr) {
                        for (int pi = 0; pi < boneCount; pi++) {
                            if (bones->getPointer()[pi] == parentTr) {
                                info.parentIndex = pi;
                                break;
                            }
                        }
                    }
                    
                    Vector3 sp; bool vis;
                    WorldToScreen(camera, info.worldPos, sp, vis);
                    info.screenPos = ImVec2(sp.x, sp.y);
                    info.isVisible = vis;
                    
                    int childCount = Transform_get_childCount(boneTr);
                    if (childCount >= 3) info.importance = 1.0f;
                    else if (childCount == 2) info.importance = 0.8f;
                    else if (childCount == 1) info.importance = 0.6f;
                    else info.importance = 0.4f;
                    
                    info.isImportant = (info.importance >= 0.8f);
                    boneInfos.push_back(info);
                }
                
                for (size_t bi = 0; bi < boneInfos.size(); bi++) {
                    const BoneInfo& bone = boneInfos[bi];
                    
                    if (bone.parentIndex == -1 || bone.parentIndex >= (int)boneInfos.size()) continue;
                    if (!bone.isVisible) continue;
                    
                    const BoneInfo& parent = boneInfos[bone.parentIndex];
                    if (!parent.isVisible) continue;
                    
                    float boneDist = Vector3::Distance(bone.worldPos, parent.worldPos);
                    float maxDist = bone.isImportant ? MAX_BONE_DISTANCE_IMPORTANT : MAX_BONE_DISTANCE_NORMAL;
                    if (boneDist < 0.01f || boneDist > maxDist) continue;
                    
                    float finalThickness = ESP_SKELETON_THICKNESS;
                    
                    ImU32 boneColor;
                    if (bone.isImportant) {
                        boneColor = ESP_SKELETON_COLOR_IMPORTANT;
                    } else {
                        boneColor = ESP_SKELETON_COLOR_NORMAL;
                    }
                    
                    float distance = Vector3::Distance(cameraPosition, bone.worldPos);
                    float alpha = std::max(0.4f, 1.0f - (distance * 0.02f));
                    boneColor = IM_COL32(
                        (boneColor >> IM_COL32_R_SHIFT) & 0xFF,
                        (boneColor >> IM_COL32_G_SHIFT) & 0xFF,
                        (boneColor >> IM_COL32_B_SHIFT) & 0xFF,
                        (int)(255 * alpha)
                    );
                    
                    ImVec2 boneScreenPos = bone.screenPos;
                    ImVec2 parentScreenPos = parent.screenPos;
                    
                    // Рисуем линию через CGContext
                    CGContextSetStrokeColorWithColor(context, UIColorFromImU32(boneColor).CGColor);
                    CGContextSetLineWidth(context, finalThickness);
                    CGContextMoveToPoint(context, parentScreenPos.x, parentScreenPos.y);
                    CGContextAddLineToPoint(context, boneScreenPos.x, boneScreenPos.y);
                    CGContextStrokePath(context);
                }
            }
        }
    }
}

#endif // DRAW_HELPERS_MM
