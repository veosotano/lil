#import <Cocoa/Cocoa.h>
#import <AudioToolbox/AudioToolbox.h>
#import <IOKit/hid/IOHIDLib.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#include <simd/simd.h>
#include <math.h>

extern void LIL__init();
extern void LIL__addAppMenu();
extern void LIL__addMenus();
extern void LIL__nextFrame(void * vertexBuffer, long int * vertexCount);
extern void LIL__setKeyDown(int keyCode);
extern void LIL__setKeyUp(int keyCode);

extern void LIL__audioInit();
extern void LIL__audioFree();

extern long int LIL__gamepadConnected(long int vendorID, long int productID);
extern void LIL__setGamepadButtonState(long int gamepadId, long int buttonId, bool value);
extern void LIL__setGamepadX(long int gamepadId, double value);
extern void LIL__setGamepadY(long int gamepadId, double value);

typedef struct LIL__audioDescriptorStruct {
    AudioComponentInstance * audioUnit;
    size_t bufferSize;
    char * data;
    UInt64 playCursor;
    UInt32 bitsPerSample;
    UInt32 bytesPerFrame;
    UInt32 freq;
    UInt32 samplesPerSecond;
} LIL__audioDescriptorStruct;

//this will be populated after calling LIL__audioInit()
extern LIL__audioDescriptorStruct LIL__audioDescriptor;

float LIL__sineValue = 0.f;

OSStatus LIL__renderSineWave(void * inData, AudioUnitRenderActionFlags * flags, const AudioTimeStamp * timestamp, UInt32 busNumber, UInt32 frames, AudioBufferList *ioData)
{
    short int * channel = (short int * )ioData->mBuffers[0].mData;
    UInt32 freq = LIL__audioDescriptor.freq;
    UInt32 period = 48000 / freq;
    float volume = 32000.0f;

    for (UInt32 i = 0; i < frames; i++) {
        float sineValue = sinf(LIL__sineValue);
        short int sampleValue = (short int)(sineValue * volume);
        *channel = sampleValue;
        channel += 1;
        *channel = sampleValue;
        channel += 1;
        LIL__sineValue += 2.0f * M_PI / (float)period;
        if (LIL__sineValue > 2.0f * M_PI) {
            LIL__sineValue -= 2.0f * M_PI;
        }
    }
    
    return noErr;
}

// OSStatus LIL__renderAudio(void * inData, AudioUnitRenderActionFlags * flags, const AudioTimeStamp * timestamp, UInt32 busNumber, UInt32 frames, AudioBufferList *ioData)
// {
//     short int * dstBuffer = (short int * )ioData->mBuffers[0].mData;
//     short int * srcBuffer = (short int *)inData;
//     UInt32 bytesPerFrame = 4;
//     UInt32 bufferLength = frames * bytesPerFrame;
//     memcpy(dstBuffer, srcBuffer, bufferLength);
//     return noErr;
// }

void LIL__setupAudio()
{
    //this needs to be called so that the LIL__audioDescriptor
    //global variable contains values from the config system
    LIL__audioInit();

    AudioComponentDescription acd;
    acd.componentType = kAudioUnitType_Output;
    acd.componentSubType = kAudioUnitSubType_DefaultOutput;
    acd.componentManufacturer = kAudioUnitManufacturer_Apple;
    acd.componentFlags = 0;
    acd.componentFlagsMask = 0;
    
    AudioComponentInstance au;
    LIL__audioDescriptor.audioUnit = &au;

    AudioComponent out = AudioComponentFindNext(NULL, &acd);
    OSStatus status = AudioComponentInstanceNew(out, LIL__audioDescriptor.audioUnit);

    if (status != noErr) {
        NSLog(@"Error setting up audio component");
        return;
    }
    
    AudioStreamBasicDescription asbd;
    asbd.mSampleRate = LIL__audioDescriptor.samplesPerSecond;
    asbd.mFormatID = kAudioFormatLinearPCM;
    asbd.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    asbd.mFramesPerPacket = 1;
    asbd.mChannelsPerFrame = 2;
    asbd.mBitsPerChannel = LIL__audioDescriptor.bitsPerSample;
    asbd.mBytesPerFrame = LIL__audioDescriptor.bytesPerFrame;
    asbd.mBytesPerPacket = LIL__audioDescriptor.bytesPerFrame;
    status = AudioUnitSetProperty(*LIL__audioDescriptor.audioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &asbd, sizeof(asbd));

    if (status != noErr) {
        NSLog(@"Error setting up audio output stream");
        return;
    }
    
    AURenderCallbackStruct callback;
    callback.inputProc = LIL__renderSineWave;

    status = AudioUnitSetProperty(*LIL__audioDescriptor.audioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Global, 0, &callback, sizeof(callback));
    
    if (status != noErr) {
        NSLog(@"Error setting up audio callback");
        return;
    }

    AudioUnitInitialize(*LIL__audioDescriptor.audioUnit);
    AudioOutputUnitStart(*LIL__audioDescriptor.audioUnit);
}

void LIL__gamepadInputListener(void* ctxt, IOReturn status, void *sender, IOHIDValueRef value)
{
    if (status != kIOReturnSuccess) {
        return;
    }

    long int gamepadId = (long int)ctxt;

    IOHIDElementRef Element = IOHIDValueGetElement(value);
    unsigned int usagePage = IOHIDElementGetUsagePage(Element);
    unsigned int usage = IOHIDElementGetUsage(Element);
    if (usagePage == kHIDPage_Button) {
        unsigned int buttonState = IOHIDValueGetIntegerValue(value);
        LIL__setGamepadButtonState(gamepadId, usage, !(buttonState == 0));
    } else if (usagePage == kHIDPage_GenericDesktop) {
        unsigned int direction = IOHIDValueGetIntegerValue(value);
        switch (usage) {
            case kHIDUsage_GD_X:
            {
                double xValue = -((128.0 - (double)direction) / 128.0);
                LIL__setGamepadX(gamepadId, xValue);
                break;
            }
            case kHIDUsage_GD_Y:
            {
                double yValue = (128.0 - (double)direction) / 128.0;
                LIL__setGamepadY(gamepadId, yValue);
                break;
            }
            case kHIDUsage_GD_Z:
                //NSLog(@"X2: %i\n", direction);
                break;
            case kHIDUsage_GD_Rz:
                //NSLog(@"Y2: %i\n", direction);
                break;

            case kHIDUsage_GD_Hatswitch:
            {
                double xValue = 0.0, yValue = 0.0;
                switch (direction) {
                    case 0:  xValue = 0.0;  yValue = 1.0; break;
                    case 1:  xValue = 1.0;  yValue = 1.0; break;
                    case 2:  xValue = 1.0;  yValue = 0.0; break;
                    case 3:  xValue = 1.0;  yValue = -1.0; break;
                    case 4:  xValue = 0.0;  yValue = -1.0; break;
                    case 5:  xValue = -1.0; yValue = -1.0; break;
                    case 6:  xValue = -1.0; yValue = 0.0; break;
                    case 7:  xValue = -1.0; yValue = 1.0; break;
                    default: xValue = 0.0;  yValue = 0.0; break;
                }
                LIL__setGamepadX(gamepadId, xValue);
                LIL__setGamepadY(gamepadId, yValue);
                break;
            }
            default:
                NSLog(@"Usage: %i\n", usage);
                break;
        }
    }
}

void LIL__gamepadConnectedListener(void* ctxt, IOReturn status, void* sender, IOHIDDeviceRef device)
{
    if (status != kIOReturnSuccess) {
        return;
    }
    NSUInteger vendorID = [(__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDVendorIDKey)) unsignedIntegerValue];
    NSUInteger productID = [(__bridge NSNumber *)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey)) unsignedIntegerValue];

    long int gamepadId = LIL__gamepadConnected(vendorID, productID);
    
    IOHIDDeviceRegisterInputValueCallback(device, LIL__gamepadInputListener, (void*)gamepadId);
    IOHIDDeviceSetInputValueMatchingMultiple(device, (__bridge CFArrayRef)@[
        @{@(kIOHIDElementUsagePageKey): @(kHIDPage_Button)},
        @{@(kIOHIDElementUsagePageKey): @(kHIDPage_GenericDesktop)},
    ]);
}

void LIL__setupGamepads()
{
    //second params mean "no options"
    IOHIDManagerRef mgr = IOHIDManagerCreate(kCFAllocatorDefault, 0);
    IOReturn status = IOHIDManagerOpen(mgr, 0);
    if (status != kIOReturnSuccess) {
        NSLog(@"Error setting up HID manager");
        return;
    }
    IOHIDManagerRegisterDeviceMatchingCallback(mgr, LIL__gamepadConnectedListener, NULL);

    //we're interested in gamepads with thumbsticks
    IOHIDManagerSetDeviceMatchingMultiple(mgr, (__bridge CFArrayRef)@[
        @{@(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop), @(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_GamePad)},
        @{@(kIOHIDDeviceUsagePageKey): @(kHIDPage_GenericDesktop), @(kIOHIDDeviceUsageKey): @(kHIDUsage_GD_MultiAxisController)},
    ]);

    //run on the main thread
    IOHIDManagerScheduleWithRunLoop(mgr, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
}

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
    LIL__setKeyDown(anEvent.keyCode);
}
- (void)keyUp:(NSEvent *)anEvent {
    LIL__setKeyUp(anEvent.keyCode);
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
    //call initializers
    LIL__init();
    //show window
    [mainWindow makeKeyAndOrderFront:self];
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
    
    LIL__setupAudio();
    LIL__setupGamepads();

    // call the run method of our application
    [application run];

    LIL__audioFree();

    // drain the autorelease pool
    [LIL__autoreleasepool drain];
}
