// ESPRenderer.h
#import <UIKit/UIKit.h>

@interface ESPRenderer : NSObject

+ (instancetype)sharedInstance;
- (void)setupOverlayWindow;
- (UIWindow *)getESPWindow;
- (void)renderESP:(void (^)(CGContextRef context))drawBlock;

@end
