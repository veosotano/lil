#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#include <simd/simd.h>

extern void LIL__init();
extern void LIL__addAppMenu();
extern void LIL__addMenus();
extern void LIL__nextFrame(void * vertexBuffer, long int * vertexCount);

static const MTLPixelFormat LILDepthPixelFormat = MTLPixelFormatDepth32Float;

typedef enum LILVertexInputIndex
{
    LILVertexInputIndexVertices = 0,
    LILVertexInputIndexUniforms = 1,
} LILVertexInputIndex;

typedef struct
{
    float x;
    float y;

    float red;
    float green;
    float blue;
    float alpha;
} LILVertex;

typedef struct
{
    float scale;
    vector_uint2 viewportSize;
} LILUniforms;

@interface LILMetalRenderer : NSObject

- (nonnull id)initWithMetalDevice:(nonnull id<MTLDevice>)device_ drawablePixelFormat:(MTLPixelFormat)drawablePixelFormat;
- (void)renderToMetalLayer:(nonnull CAMetalLayer*)metalLayer;
- (void *)getVertexBufferPointer;

@property(nonatomic, assign) MTLClearColor windowBgColor;
@property(nonatomic, assign) long int vertexCount;

@end

@implementation LILMetalRenderer
{
    id <MTLDevice> device;
    id <MTLCommandQueue> commandQueue;
    id <MTLRenderPipelineState> pipelineState;
    id <MTLBuffer> vertexBuffer;
    id <MTLTexture> depthTarget;
    MTLRenderPassDescriptor * drawableRenderDescriptor;
    vector_uint2 viewportSize;
    NSUInteger frameNum;
    MTLClearColor windowBgColor_;
    long int vertexCount_;
}

@synthesize windowBgColor = windowBgColor_;
@synthesize vertexCount = vertexCount_;

- (nonnull id)initWithMetalDevice:(nonnull id<MTLDevice>)device_ drawablePixelFormat:(MTLPixelFormat)drawablePixelFormat
{
    self = [super init];
    if (self)
    {
        device = device_;
        frameNum = 0;
        vertexCount_ = 0;
        self.windowBgColor = MTLClearColorMake(0., 0., 0., 1.);

        commandQueue = [device newCommandQueue];

        drawableRenderDescriptor = [MTLRenderPassDescriptor new];
        drawableRenderDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        drawableRenderDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;

        drawableRenderDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
        drawableRenderDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
        drawableRenderDescriptor.depthAttachment.clearDepth = 1.0;


        NSError *libraryError = NULL;
        NSString *libraryFile = [[NSBundle mainBundle] pathForResource:@"lil_shaders" ofType:@"metallib"];
        id <MTLLibrary> shaderLib = [device newLibraryWithFile:libraryFile error:&libraryError];
        if (!shaderLib) {
            NSLog(@"Library error: %@", libraryError.localizedDescription);
            return nil;
        }

        id <MTLFunction> vertexProgram = [shaderLib newFunctionWithName:@"vertexShader"];
        if(!vertexProgram) {
            NSLog(@">> ERROR: Couldn't load vertex function from default library");
            return nil;
        }

        id <MTLFunction> fragmentProgram = [shaderLib newFunctionWithName:@"fragmentShader"];
        if(!fragmentProgram) {
            NSLog(@" ERROR: Couldn't load fragment function from default library");
            return nil;
        }

        // Create a new empty vertex buffer
        vertexBuffer = [device newBufferWithLength:(sizeof(LILVertex)*1000) options:MTLResourceStorageModeShared];

        vertexBuffer.label = @"VertexBuffer";

        // Create a pipeline state descriptor to create a compiled pipeline state object
        MTLRenderPipelineDescriptor *pipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];

        pipelineDescriptor.label                           = @"MyPipeline";
        pipelineDescriptor.vertexFunction                  = vertexProgram;
        pipelineDescriptor.fragmentFunction                = fragmentProgram;
        pipelineDescriptor.colorAttachments[0].pixelFormat = drawablePixelFormat;

        pipelineDescriptor.depthAttachmentPixelFormat      = LILDepthPixelFormat;

        NSError *error;
        pipelineState = [device newRenderPipelineStateWithDescriptor:pipelineDescriptor error:&error];
        if(!pipelineState)
        {
            NSLog(@"ERROR: Failed aquiring pipeline state: %@", error);
            return nil;
        }
    }
    return self;
}

- (void)renderToMetalLayer:(nonnull CAMetalLayer*)metalLayer
{
    frameNum++;

    // Create a new command buffer for each render pass to the current drawable.
    id <MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];

    id<CAMetalDrawable> currentDrawable = [metalLayer nextDrawable];

    // If the current drawable is nil, skip rendering this frame
    if(!currentDrawable)
    {
        return;
    }

    drawableRenderDescriptor.colorAttachments[0].clearColor = self.windowBgColor;
    drawableRenderDescriptor.colorAttachments[0].texture = currentDrawable.texture;
    
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:drawableRenderDescriptor];


    [renderEncoder setRenderPipelineState:pipelineState];

    [renderEncoder setVertexBuffer:vertexBuffer offset:0 atIndex:LILVertexInputIndexVertices ];

    LILUniforms uniforms;
    //uniforms.scale = (0.7 + (0.2 * sin(frameNum * 0.1)));
    uniforms.scale = 1.0;
    uniforms.viewportSize = viewportSize;
    [renderEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:LILVertexInputIndexUniforms ];

    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:self.vertexCount];

    [renderEncoder endEncoding];

    [commandBuffer presentDrawable:currentDrawable];

    [commandBuffer commit];
}

- (void)drawableResize:(CGSize)drawableSize
{
    viewportSize.x = drawableSize.width;
    viewportSize.y = drawableSize.height;
    
    MTLTextureDescriptor *depthTargetDescriptor = [MTLTextureDescriptor new];
    depthTargetDescriptor.width = drawableSize.width;
    depthTargetDescriptor.height = drawableSize.height;
    depthTargetDescriptor.pixelFormat = LILDepthPixelFormat;
    depthTargetDescriptor.storageMode = MTLStorageModePrivate;
    depthTargetDescriptor.usage = MTLTextureUsageRenderTarget;

    depthTarget = [device newTextureWithDescriptor:depthTargetDescriptor];

    drawableRenderDescriptor.depthAttachment.texture = depthTarget;
}

- (void *)getVertexBufferPointer
{
    return vertexBuffer.contents;
}

@end

@interface LILMainView : NSView <CALayerDelegate>
@property(retain) LILMetalRenderer * renderer;
@end
@implementation LILMainView
{
    CVDisplayLinkRef displayLink;
    LILMetalRenderer * renderer_;
}

@synthesize renderer = renderer_;

- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    [self initialize];
    return self;
}

- (void)dealloc
{
    if(displayLink)
    {
        // Stop the display link BEFORE releasing anything in the view otherwise the display link
        // thread may call into the view and crash when it encounters something that no longer
        // exists
        CVDisplayLinkStop(displayLink);
        CVDisplayLinkRelease(displayLink);
    }
    
    [super dealloc];
}

- (void)initialize
{
    self.wantsLayer = YES;
    self.layer = [CAMetalLayer layer];
    self.layerContentsRedrawPolicy = NSViewLayerContentsRedrawDuringViewResize;
    self.layer.delegate = self;
}

- (void)drawRect:(NSRect)dirtyRect{
    NSLog(@"Draw rect");
    [super drawRect:dirtyRect];
}

- (void)render
{
    // Must synchronize if rendering on background thread to ensure resize operations from the
    // main thread are complete before rendering which depends on the size occurs.
    CAMetalLayer * metalLayer = (CAMetalLayer*)self.layer;
    @synchronized(metalLayer)
    {
        [self.renderer renderToMetalLayer:metalLayer];
    }
}

- (void)resizeDrawable:(CGFloat)scaleFactor
{
    CGSize newSize = self.bounds.size;
    newSize.width *= scaleFactor;
    newSize.height *= scaleFactor;

    if(newSize.width <= 0 || newSize.width <= 0)
    {
        return;
    }

    // All AppKit and UIKit calls which notify of a resize are called on the main thread.  Use
    // a synchronized block to ensure that resize notifications on the delegate are atomic
    CAMetalLayer * metalLayer = (CAMetalLayer*)self.layer;
    @synchronized(metalLayer)
    {
        if(newSize.width == metalLayer.drawableSize.width &&
           newSize.height == metalLayer.drawableSize.height)
        {
            return;
        }

        metalLayer.drawableSize = newSize;

        [self.renderer drawableResize:newSize];
    }
}

- (BOOL)acceptsFirstResponder {
    return YES;
}
- (void)mouseDown:(NSEvent *)anEvent {
    puts("mousedown!");
}
- (void)mouseDragged:(NSEvent *)anEvent {
    puts("mousemoved!");
}
- (void)keyDown:(NSEvent *)anEvent {
    puts("keydown!");
}

- (void)viewDidMoveToWindow
{
    [super viewDidMoveToWindow];
    
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();

    // Set the device for the layer so the layer can create drawable textures that can be rendered to
    // on this device.
    CAMetalLayer * metalLayer = (CAMetalLayer*)self.layer;
    metalLayer.device = device;

    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    self.renderer = [[LILMetalRenderer alloc] initWithMetalDevice:device drawablePixelFormat:metalLayer.pixelFormat];
    [self setupDisplayLink];

    [self resizeDrawable:self.window.screen.backingScaleFactor];
}

- (void)setupDisplayLink
{
    CVReturn cvReturn = CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    if(cvReturn != kCVReturnSuccess) {
        return;
    }
    cvReturn = CVDisplayLinkSetOutputCallback(displayLink, &LIL__dispatchRenderLoop, (__bridge void*)self);
    if(cvReturn != kCVReturnSuccess) {
        return;
    }
    CGDirectDisplayID viewDisplayID = (CGDirectDisplayID) [self.window.screen.deviceDescription[@"NSScreenNumber"] unsignedIntegerValue];
    cvReturn = CVDisplayLinkSetCurrentCGDisplay(displayLink, viewDisplayID);
    if(cvReturn != kCVReturnSuccess) {
        return;
    }
    CVDisplayLinkStart(displayLink);

    // Register to be notified when the window closes so that you can stop the display link
    NSNotificationCenter* notificationCenter = [NSNotificationCenter defaultCenter];
    [notificationCenter addObserver:self selector:@selector(windowWillClose:) name:NSWindowWillCloseNotification object:self.window];
}

- (void)windowWillClose:(NSNotification*)notification
{
    // Stop the display link when the window is closing since there
    // is no point in drawing something that can't be seen
    if(notification.object == self.window)
    {
        CVDisplayLinkStop(displayLink);
    }
}

static CVReturn LIL__dispatchRenderLoop(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    @autoreleasepool
    {
        LILMainView *theView = (__bridge LILMainView*)displayLinkContext;
        long int vertexCount = 0;
        LILMetalRenderer * renderer = theView.renderer;
        LIL__nextFrame([renderer getVertexBufferPointer], &vertexCount);
        renderer.vertexCount = vertexCount;
        [theView render];
    }
    return kCVReturnSuccess;
}

@end

@interface LILAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow * mainWindow;
    LILMainView * mainView;
    NSMenu * mainMenu;
    NSMutableArray * menuStack;
}
- (id)initWithWidth:(float) width height:(float) height;
- (void)populateMainMenu;
- (void)addMenu:(const char *) label;
- (void)addMenuItem:(const char *)label key:(const char *)key fnPtr:(void(*))fnPtr;
- (void)addMenuSeparator;
- (void)exitMenu;
- (void)menuItemSelected:(NSMenuItem *) menuItem;
- (void)setWindowBackgroundRed:(float)red green:(float)green blue:(float)blue alpha:(float)alpha;
@end
@implementation LILAppDelegate
- (id)initWithWidth:(float) width height:(float) height {
    if ( self = [super init] ) {
        if (width < 300.0) {
            width = 300.0;
        }
        if (height < 300.0) {
            height = 300.0;
        }
        NSRect screenRect = [[NSScreen mainScreen] frame];
        
        float x = (screenRect.size.width/2) - (width/2);
        float y = (screenRect.size.height/2) - (height/2);
        if (x < 0.0) {
            x = 0.0;
        }
        if (y < 0.0) {
            y = 0.0;
        }
        NSRect contentSize = NSMakeRect(x, y, width, height);

        mainWindow = [[NSWindow alloc]
            initWithContentRect: contentSize
            styleMask:
                NSWindowStyleMaskTitled
                | NSWindowStyleMaskClosable
                | NSWindowStyleMaskMiniaturizable
                | NSWindowStyleMaskResizable
            backing:NSBackingStoreBuffered
            defer:YES];
        
        mainView = [[LILMainView alloc] initWithFrame:contentSize];
        menuStack = [[NSMutableArray alloc] init];
    }
    return self;
}
- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    [mainWindow setContentView:mainView];
}
- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    //setup main menu item
    [self populateMainMenu];
    //add menu items for the app menu
    LIL__addAppMenu();
    //add the quit item
    NSMenu * currentMenu = [menuStack lastObject];
    NSString *applicationName = [[NSProcessInfo processInfo] processName];
    NSMenuItem * quitMenuItem = [currentMenu addItemWithTitle:[NSString stringWithFormat:@"%@ %@", NSLocalizedString(@"Quit", nil), applicationName] action:@selector(terminate:) keyEquivalent:@"q"];
    [quitMenuItem setTarget:NSApp];
    [self exitMenu];
    //add the rest of the menus
    LIL__addMenus();
    //set it as main menu
    [NSApp setMainMenu:mainMenu];
    [menuStack removeAllObjects];
    //show window
    [mainWindow makeKeyAndOrderFront:self];
    //call initializers
    LIL__init();
}
- (void)dealloc {
    // don’t forget to release allocated objects!
    [menuStack release];
    [mainMenu release];
    [mainView release];
    [mainWindow release];
    [super dealloc];
}
- (void)populateMainMenu {
    mainMenu = [[NSMenu alloc] initWithTitle:@"MainMenu"];
    NSMenuItem *menuItem;
    NSMenu *submenu;

    [menuStack addObject: mainMenu];

    menuItem = [mainMenu addItemWithTitle:@"Apple" action:NULL keyEquivalent:@""];
    submenu = [[NSMenu alloc] initWithTitle:@"Apple"];
    [NSApp performSelector:NSSelectorFromString(@"setAppleMenu:") withObject:submenu];
    [mainMenu setSubmenu:submenu forItem:menuItem];

    [menuStack addObject: submenu];
}

- (void)addMenu:(const char *)label {
    NSMenu * currentMenu = [menuStack lastObject];
    
    NSString * labelStr = [NSString stringWithUTF8String: label];
    NSMenuItem * menuItem = [currentMenu addItemWithTitle:labelStr action:NULL keyEquivalent:@""];
    NSMenu * submenu = [[NSMenu alloc] initWithTitle: labelStr];
    [mainMenu setSubmenu:submenu forItem:menuItem];
    [menuStack addObject: submenu];
}
- (void)addMenuItem:(const char *)label key:(const char *)key fnPtr:(void(*))fnPtr {
    NSMenu * currentMenu = [menuStack lastObject];

    NSString * labelStr = [NSString stringWithUTF8String: label];
    NSString * keyStr = [NSString stringWithUTF8String: key];
    NSMenuItem * menuItem = [currentMenu addItemWithTitle:labelStr  action:@selector(menuItemSelected:) keyEquivalent:keyStr];
    [menuItem setTag:(NSInteger)fnPtr];
    [menuItem setTarget: self];
}

- (void)addMenuSeparator
{
    NSMenuItem * separator = [NSMenuItem separatorItem];
    [[menuStack lastObject] addItem: separator];
}

- (void)exitMenu
{
    [menuStack removeLastObject];
}

- (void)menuItemSelected:(NSMenuItem *) menuItem
{
    void(*fnPtr)() = (void(*)())[menuItem tag];
    if ((NSUInteger)fnPtr == 0) {
        NSLog(@"Null pointer encountered");
        return;
    }
    fnPtr();
}

- (void)setWindowBackgroundRed:(float)red green:(float)green blue:(float)blue alpha:(float)alpha
{
    mainView.renderer.windowBgColor = MTLClearColorMake(red, green, blue, alpha);
}

@end

NSAutoreleasePool * LIL__autoreleasepool;
LILAppDelegate * LIL__applicationDelegate;

void LIL__run(float width, float height)
{
    LIL__autoreleasepool = [[NSAutoreleasePool alloc] init];

    // make sure the application singleton has been instantiated
    NSApplication * application = [NSApplication sharedApplication];

    // instantiate our application delegate
    LIL__applicationDelegate = [[[LILAppDelegate alloc] initWithWidth:width height: height] autorelease];
    
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
    
    // assign our delegate to the NSApplication
    [application setDelegate:LIL__applicationDelegate];

    // call the run method of our application
    [application run];
    
    // drain the autorelease pool
    [LIL__autoreleasepool drain];
}
