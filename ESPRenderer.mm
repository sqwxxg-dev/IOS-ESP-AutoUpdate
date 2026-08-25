// ESPRenderer.mm
#import "ESPRenderer.h"

@interface ESPRenderer ()
@property (nonatomic, strong) UIWindow *espWindow;
@property (nonatomic, strong) UIView *espContainer;
@end

@implementation ESPRenderer

+ (instancetype)sharedInstance {
    static ESPRenderer *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[ESPRenderer alloc] init];
    });
    return instance;
}

- (void)setupOverlayWindow {
    dispatch_async(dispatch_get_main_queue(), ^{
        // Создаём окно поверх игры
        self.espWindow = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
        self.espWindow.windowLevel = UIWindowLevelStatusBar + 1;
        self.espWindow.backgroundColor = [UIColor clearColor];
        self.espWindow.userInteractionEnabled = YES; // Для кнопки меню
        self.espWindow.hidden = NO;
        
        // Контейнер для ESP и кнопки
        self.espContainer = [[UIView alloc] initWithFrame:self.espWindow.bounds];
        self.espContainer.backgroundColor = [UIColor clearColor];
        self.espContainer.userInteractionEnabled = YES;
        [self.espWindow addSubview:self.espContainer];
        
        NSLog(@"[ESP] Overlay window setup complete");
    });
}

- (void)renderESP:(void (^)(CGContextRef))drawBlock {
    if (!drawBlock || !self.espContainer) return;
    
    UIGraphicsBeginImageContextWithOptions(self.espContainer.bounds.size, NO, 0);
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    if (context) {
        CGContextClearRect(context, self.espContainer.bounds);
        drawBlock(context);
    }
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    dispatch_async(dispatch_get_main_queue(), ^{
        self.espContainer.layer.contents = (__bridge id)image.CGImage;
    });
}

@end
