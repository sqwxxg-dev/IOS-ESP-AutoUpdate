// ESPRenderer.h
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

@interface ESPRenderer : NSObject

+ (instancetype)sharedInstance;
- (void)setupOverlayWindow;
- (void)renderESP:(void (^)(CGContextRef context))drawBlock;

@end
