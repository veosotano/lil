#import <Cocoa/Cocoa.h>
#import <AudioToolbox/AudioToolbox.h>
#import <IOKit/hid/IOHIDLib.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include <simd/simd.h>
#include <math.h>
#include <mach/mach_time.h>

typedef struct LIL__audioDescriptorStruct {
    AudioComponentInstance * audioUnit;
    size_t bufferSize;
    char * data;
    UInt64 playCursor;
    UInt32 bitsPerSample;
    UInt32 bytesPerFrame;
    UInt32 freq;
    float volume;
    UInt32 samplesPerSecond;
} LIL__audioDescriptorStruct;

typedef struct LIL__resourceStruct {
    char path[1024];
    void * data;
} LIL__resourceStruct;

extern void LIL__init();
extern void LIL__addAppMenu();
extern void LIL__addMenus();
extern void LIL__nextFrame(double deltaTime);
extern void LIL__makeBoxVertices(void * vertexBuffer, long int * vertexCount);
extern void LIL__makeTextureVertices(void * vertexBuffer, long int * vertexCount);
extern void LIL__makeShapeVertices(void * vertexBuffer, long int * vertexCount, void * indexBuffer, long int * indexCount);
extern long int LIL__getResourceCount();
extern LIL__resourceStruct * LIL__getResorceById(long int id);
extern void LIL__setTextureSize(long int imgId, float width, float height);
extern void LIL__setMouseDown(long int buttonNumber, double x, double y);
extern void LIL__setMouseDragged(long int buttonNumber, double x, double y);
extern void LIL__setMouseUp(long int buttonNumber, double x, double y, long int clickCount);
extern void LIL__setKeyDown(int keyCode);
extern void LIL__setKeyUp(int keyCode);
extern CGSize LIL__getWindowSize();

extern LIL__audioDescriptorStruct * LIL__audioInit();
extern void LIL__audioFree();

extern long int LIL__gamepadConnected(long int vendorID, long int productID);
extern void LIL__setGamepadButtonState(long int gamepadId, long int buttonId, bool value);
extern void LIL__setGamepadX(long int gamepadId, double value);
extern void LIL__setGamepadY(long int gamepadId, double value);
extern void LIL__setGamepadX2(long int gamepadId, double value);
extern void LIL__setGamepadY2(long int gamepadId, double value);

extern bool LIL__automaticFullScreen();

extern void msgEmit(char * name, void * data);
OSStatus LIL__renderAudio(void * inData, AudioUnitRenderActionFlags * flags, const AudioTimeStamp * timestamp, UInt32 busNumber, UInt32 frames, AudioBufferList *ioData)
{
    LIL__audioDescriptorStruct * audioDescriptor = (LIL__audioDescriptorStruct *)inData;
    
    char * dstBuffer = (char * )ioData->mBuffers[0].mData;
    UInt32 bytesToOutput = frames * audioDescriptor->bytesPerFrame;
    
    UInt32 region1Size = bytesToOutput;
    UInt32 region2Size = 0;

    if (audioDescriptor->playCursor + bytesToOutput > audioDescriptor->bufferSize) {
        region1Size = audioDescriptor->bufferSize - audioDescriptor->playCursor;
        region2Size = bytesToOutput - region1Size;
    }

    memcpy(dstBuffer, audioDescriptor->data + audioDescriptor->playCursor, region1Size);
    if (region2Size > 0) {
        memcpy(dstBuffer+region1Size, audioDescriptor->data, region2Size);
    }
    audioDescriptor->playCursor = (audioDescriptor->playCursor + bytesToOutput) % audioDescriptor->bufferSize;
    return noErr;
}

void LIL__setupAudio()
{
    LIL__audioDescriptorStruct * audioDescriptor = LIL__audioInit();

    AudioComponentDescription acd;
    acd.componentType = kAudioUnitType_Output;
    acd.componentSubType = kAudioUnitSubType_DefaultOutput;
    acd.componentManufacturer = kAudioUnitManufacturer_Apple;
    acd.componentFlags = 0;
    acd.componentFlagsMask = 0;
    
    AudioComponentInstance au;
    audioDescriptor->audioUnit = &au;

    AudioComponent out = AudioComponentFindNext(NULL, &acd);
    OSStatus status = AudioComponentInstanceNew(out, audioDescriptor->audioUnit);

    if (status != noErr) {
        NSLog(@"Error setting up audio component");
        return;
    }
    
    AudioStreamBasicDescription asbd;
    asbd.mSampleRate = audioDescriptor->samplesPerSecond;
    asbd.mFormatID = kAudioFormatLinearPCM;
    asbd.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    asbd.mFramesPerPacket = 1;
    asbd.mChannelsPerFrame = 2;
    asbd.mBitsPerChannel = audioDescriptor->bitsPerSample;
    asbd.mBytesPerFrame = audioDescriptor->bytesPerFrame;
    asbd.mBytesPerPacket = audioDescriptor->bytesPerFrame;
    status = AudioUnitSetProperty(*(audioDescriptor->audioUnit), kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &asbd, sizeof(asbd));

    if (status != noErr) {
        NSLog(@"Error setting up audio output stream");
        return;
    }
    
    AURenderCallbackStruct callback;
    callback.inputProcRefCon = (void *)audioDescriptor;
    callback.inputProc = LIL__renderAudio;

    status = AudioUnitSetProperty(*(audioDescriptor->audioUnit), kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Global, 0, &callback, sizeof(callback));
    
    if (status != noErr) {
        NSLog(@"Error setting up audio callback");
        return;
    }

    AudioUnitInitialize(*(audioDescriptor->audioUnit));
    AudioOutputUnitStart(*(audioDescriptor->audioUnit));
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
            {
                double xValue = -((128.0 - (double)direction) / 128.0);
                LIL__setGamepadX2(gamepadId, xValue);
                break;
            }
            case kHIDUsage_GD_Rz:
            {
                double yValue = (128.0 - (double)direction) / 128.0;
                LIL__setGamepadY2(gamepadId, yValue);
                break;
            }

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
                //NSLog(@"Usage: %i\n", usage);
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
    
    float textureX;
    float textureY;
} LILVertex;

typedef struct
{
    float scale;
    vector_uint2 viewportSize;
} LILUniforms;

@interface LILMetalRenderer : NSObject

- (nonnull id)initWithMetalDevice:(nonnull id<MTLDevice>)device_ drawablePixelFormat:(MTLPixelFormat)drawablePixelFormat;
- (void)loadTextureForFile:(NSString *)filename index:(unsigned int)idx;
- (void)unloadTextureForIndex:(unsigned int)idx;
- (void)renderToMetalLayer:(nonnull CAMetalLayer*)metalLayer;
- (void *)getVertexBufferPointer;
- (void *)getIndexBufferPointer;

@property(nonatomic, assign) MTLClearColor windowBgColor;
@property(nonatomic, assign) long int boxVertexCount;
@property(nonatomic, assign) long int textureVertexCount;
@property(nonatomic, assign) long int textureCount;
@property(nonatomic, assign) long int shapeVertexCount;
@property(nonatomic, assign) long int shapeIndexCount;

@end

@implementation LILMetalRenderer
{
    id <MTLDevice> device;
    id <MTLCommandQueue> commandQueue;
    id <MTLRenderPipelineState> boxPipeline;
    id <MTLBuffer> vertexBuffer;
	id <MTLBuffer> indexBuffer;
    id <MTLTexture> depthTarget;
    id <MTLTexture> textures[256];
    id <MTLTexture> msaaTexture_;
	id <MTLTexture> msaaDepthTexture_;
    id <MTLRenderPipelineState> texturePipelines[256];
	id <MTLRenderPipelineState> shapePipeline;
    id <MTLFunction> vertexShaderFn;
    id <MTLFunction> fragmentShaderFn;
    id <MTLFunction> textureShaderFn;
    MTLPixelFormat drawablePixelFormat_;
    MTLRenderPassDescriptor * drawableRenderDescriptor;
    vector_uint2 viewportSize;
    NSUInteger frameNum;
    MTLClearColor windowBgColor_;
    long int boxVertexCount_;
    long int textureVertexCount_;
    long int textureCount_;
    long int shapeVertexCount_;
	long int shapeIndexCount_;
}

@synthesize windowBgColor = windowBgColor_;
@synthesize boxVertexCount = boxVertexCount_;
@synthesize textureVertexCount = textureVertexCount_;
@synthesize textureCount = textureCount_;
@synthesize shapeVertexCount = shapeVertexCount_;
@synthesize shapeIndexCount = shapeIndexCount_;

- (nonnull id)initWithMetalDevice:(nonnull id<MTLDevice>)device_ drawablePixelFormat:(MTLPixelFormat)drawablePixelFormat
{
    self = [super init];
    if (self)
    {
        device = device_;
        frameNum = 0;
        boxVertexCount_ = 0;
        textureVertexCount_ = 0;
        textureCount_ = 0;
		shapeVertexCount_ = 0;
		shapeIndexCount_ = 0;
        self.windowBgColor = MTLClearColorMake(0., 0., 0., 1.);
        drawablePixelFormat_ = drawablePixelFormat;

        commandQueue = [device newCommandQueue];

        drawableRenderDescriptor = [MTLRenderPassDescriptor new];
        drawableRenderDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
        drawableRenderDescriptor.colorAttachments[0].storeAction = MTLStoreActionMultisampleResolve;
        drawableRenderDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
        drawableRenderDescriptor.depthAttachment.storeAction = MTLStoreActionMultisampleResolve;
        drawableRenderDescriptor.depthAttachment.clearDepth = 1.0;

        NSError *libraryError = NULL;
        NSString *libraryFile = [[NSBundle mainBundle] pathForResource:@"lil_shaders" ofType:@"metallib"];
        id <MTLLibrary> shaderLib = [device newLibraryWithFile:libraryFile error:&libraryError];
        if (!shaderLib) {
            NSLog(@"Library error: %@", libraryError.localizedDescription);
            return nil;
        }

        vertexShaderFn = [shaderLib newFunctionWithName:@"vertexShader"];
        if(!vertexShaderFn) {
            NSLog(@">> ERROR: Couldn't load vertex function from default library");
            return nil;
        }

        fragmentShaderFn = [shaderLib newFunctionWithName:@"fragmentShader"];
        if(!fragmentShaderFn) {
            NSLog(@" ERROR: Couldn't load fragment function from default library");
            return nil;
        }
        
        textureShaderFn = [shaderLib newFunctionWithName:@"textureShader"];
        if(!textureShaderFn) {
            NSLog(@" ERROR: Couldn't load texture function from default library");
            return nil;
        }

        //create a new empty vertex buffer
        vertexBuffer = [device newBufferWithLength:(sizeof(LILVertex)*1000) options:MTLResourceStorageModeShared];
        vertexBuffer.label = @"vertexBuffer";
        //create a new empty index buffer
        indexBuffer = [device newBufferWithLength:(sizeof(UInt16)*8000) options:MTLResourceStorageModeShared];
        indexBuffer.label = @"indexBuffer";

        NSError *error;

        //create pipeline state descriptors to create a compiled pipeline state object
        //box
        MTLRenderPipelineDescriptor *boxPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
		boxPipelineDescriptor.rasterSampleCount               = 4;
        boxPipelineDescriptor.label                           = @"boxPipeline";
        boxPipelineDescriptor.vertexFunction                  = vertexShaderFn;
        boxPipelineDescriptor.fragmentFunction                = fragmentShaderFn;
        boxPipelineDescriptor.colorAttachments[0].pixelFormat = drawablePixelFormat_;
        boxPipelineDescriptor.depthAttachmentPixelFormat      = LILDepthPixelFormat;

        boxPipeline = [device newRenderPipelineStateWithDescriptor:boxPipelineDescriptor error:&error];
        if(!boxPipeline)
        {
            NSLog(@"ERROR: Failed aquiring box pipeline state: %@", error);
            return nil;
        }
		
        //shape
        MTLRenderPipelineDescriptor *shapePipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
		shapePipelineDescriptor.rasterSampleCount				= 4;
        shapePipelineDescriptor.label                           = @"shapePipeline";
        shapePipelineDescriptor.vertexFunction                  = vertexShaderFn;
        shapePipelineDescriptor.fragmentFunction                = fragmentShaderFn;
        shapePipelineDescriptor.colorAttachments[0].pixelFormat = drawablePixelFormat_;
        shapePipelineDescriptor.depthAttachmentPixelFormat      = LILDepthPixelFormat;

        shapePipeline = [device newRenderPipelineStateWithDescriptor:shapePipelineDescriptor error:&error];
        if(!shapePipeline)
        {
            NSLog(@"ERROR: Failed aquiring shape pipeline state: %@", error);
            return nil;
        }
    }
    return self;
}

- (void)loadTextureForFile:(NSString *)filename index:(unsigned int)idx
{
    NSURL * textureUrl;
	if ([filename isAbsolutePath]){
		textureUrl = [NSURL fileURLWithPath:filename];
	} else {
		textureUrl = [[NSBundle mainBundle] URLForResource:filename withExtension:nil];
	}
	
    if (textureUrl != nil) {
        MTKTextureLoader * loader = [[MTKTextureLoader alloc] initWithDevice: device];

        NSDictionary * textureOptions = @{
            MTKTextureLoaderOptionSRGB : @NO
        };
        textures[idx] = [loader newTextureWithContentsOfURL: textureUrl options: textureOptions error: nil ];
        if(textures[idx])
        {
			id<MTLTexture> theTexture = textures[idx];
			LIL__setTextureSize(idx, theTexture.width, theTexture.height);
        }
        else
        {
            NSLog(@"Failed to create the texture from %@", textureUrl.absoluteString);
            return;
        }
        
        NSError *error;
        
        MTLRenderPipelineDescriptor *texturePipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        texturePipelineDescriptor.rasterSampleCount               = 4;
		texturePipelineDescriptor.label                           = @"texturePipeline";
        texturePipelineDescriptor.vertexFunction                  = vertexShaderFn;
        texturePipelineDescriptor.fragmentFunction                = textureShaderFn;
        MTLRenderPipelineColorAttachmentDescriptor *attachment = texturePipelineDescriptor.colorAttachments[0];
        attachment.pixelFormat = drawablePixelFormat_;
        attachment.blendingEnabled = YES;
        attachment.rgbBlendOperation = MTLBlendOperationAdd;
        attachment.alphaBlendOperation = MTLBlendOperationAdd;
        attachment.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        attachment.sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
        attachment.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        attachment.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        texturePipelineDescriptor.depthAttachmentPixelFormat      = LILDepthPixelFormat;

        texturePipelines[idx] = [device newRenderPipelineStateWithDescriptor:texturePipelineDescriptor error:&error];
        if(!texturePipelines[idx])
        {
            NSLog(@"ERROR: Failed aquiring texture pipeline state: %@", error);
            return;
        }
        if (self.textureCount <= idx) {
            self.textureCount = idx + 1;
        }
    }
}

- (void)unloadTextureForIndex:(unsigned int)idx
{
    if (self.textureCount == idx + 1) {
        self.textureCount = idx;
    } else {
        NSLog(@"ERROR: Can't unload texture index %i because it was not at the end.", idx);
    }
}

- (void)renderToMetalLayer:(nonnull CAMetalLayer*)metalLayer
{
    @autoreleasepool {
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
        drawableRenderDescriptor.colorAttachments[0].resolveTexture = currentDrawable.texture;
    
        id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:drawableRenderDescriptor];

        //box
        [renderEncoder setRenderPipelineState:boxPipeline];

        [renderEncoder setVertexBuffer:vertexBuffer offset:0 atIndex:LILVertexInputIndexVertices ];

        LILUniforms uniforms;
        uniforms.scale = 1.0;
        uniforms.viewportSize = viewportSize;
        [renderEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:LILVertexInputIndexUniforms ];

        [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:self.boxVertexCount];

        for (long int i = 0; i < self.textureCount; i+=1) {
            //texture
            if(texturePipelines[i] == nil) {
                NSLog(@"Error loading pipeline state nr %li\n", i);
            }
            [renderEncoder setRenderPipelineState:texturePipelines[i]];
            [renderEncoder setVertexBuffer:vertexBuffer offset:(sizeof(LILVertex) * self.boxVertexCount) atIndex:LILVertexInputIndexVertices ];
            [renderEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:LILVertexInputIndexUniforms ];
            [renderEncoder setFragmentTexture:textures[i] atIndex:0];

            long int vtxStart = i * 6;

            [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vtxStart vertexCount:6];
        }

        //shape
        if (self.shapeIndexCount > 0) {
            long int shapeOffset = (sizeof(LILVertex) * (self.boxVertexCount + self.textureVertexCount));
            [renderEncoder setRenderPipelineState:shapePipeline];
            [renderEncoder setVertexBuffer:vertexBuffer offset:shapeOffset atIndex:LILVertexInputIndexVertices ];
            LILUniforms shapeUniforms;
            shapeUniforms.scale = 1.0;
            shapeUniforms.viewportSize = viewportSize;
            [renderEncoder setVertexBytes:&shapeUniforms length:sizeof(shapeUniforms) atIndex:LILVertexInputIndexUniforms ];
            [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:self.shapeIndexCount indexType:MTLIndexTypeUInt32 indexBuffer:indexBuffer indexBufferOffset:0];
        }
	
        [renderEncoder endEncoding];
        [commandBuffer presentDrawable:currentDrawable];
        [commandBuffer commit];
    }
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
	drawableRenderDescriptor.depthAttachment.resolveTexture = depthTarget;
	
	MTLTextureDescriptor * msaaTextureDescriptor = [MTLTextureDescriptor new];
	msaaTextureDescriptor.textureType = MTLTextureType2DMultisample;
	msaaTextureDescriptor.sampleCount = 4;
	msaaTextureDescriptor.pixelFormat = drawablePixelFormat_;
	msaaTextureDescriptor.width = drawableSize.width;
	msaaTextureDescriptor.height = drawableSize.height;
	msaaTextureDescriptor.storageMode = MTLStorageModePrivate;
	msaaTextureDescriptor.usage = MTLTextureUsageRenderTarget;
	
	msaaTexture_ = [device newTextureWithDescriptor:msaaTextureDescriptor];
	drawableRenderDescriptor.colorAttachments[0].texture = msaaTexture_;
	
    MTLTextureDescriptor *msaaDepthTextureDescriptor = [MTLTextureDescriptor new];
	msaaDepthTextureDescriptor.textureType = MTLTextureType2DMultisample;
	msaaDepthTextureDescriptor.sampleCount = 4;
    msaaDepthTextureDescriptor.width = drawableSize.width;
    msaaDepthTextureDescriptor.height = drawableSize.height;
    msaaDepthTextureDescriptor.pixelFormat = LILDepthPixelFormat;
    msaaDepthTextureDescriptor.storageMode = MTLStorageModePrivate;
    msaaDepthTextureDescriptor.usage = MTLTextureUsageRenderTarget;

    msaaDepthTexture_ = [device newTextureWithDescriptor:msaaDepthTextureDescriptor];
	drawableRenderDescriptor.depthAttachment.texture = msaaDepthTexture_;
}

- (void *)getVertexBufferPointer
{
    return vertexBuffer.contents;
}

- (void *)getIndexBufferPointer
{
    return indexBuffer.contents;
}

@end


mach_timebase_info_data_t LIL__machTimebase;
long int LIL__lastFrameTime = 0;

void LIL__timeInit() {
    mach_timebase_info(&LIL__machTimebase);
}
long int LIL__ticksTonanoseconds(long int ticks) {
    return (double)ticks * (double)LIL__machTimebase.numer / (double)LIL__machTimebase.denom;
}

@interface LILMainView : NSView <CALayerDelegate,NSWindowDelegate>
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

- (void)resizeDrawable:(NSSize) bounds scaleFactor:(CGFloat)scaleFactor
{
    NSSize newSize = bounds;
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
	NSPoint loc = [anEvent locationInWindow];
	CGFloat scaleFactor = self.window.screen.backingScaleFactor;
    LIL__setMouseDown(
		0,
		loc.x * scaleFactor,
		loc.y * scaleFactor
	);
}
- (void)mouseDragged:(NSEvent *)anEvent {
	NSPoint loc = [anEvent locationInWindow];
	CGFloat scaleFactor = self.window.screen.backingScaleFactor;
    LIL__setMouseDragged(
		0,
		loc.x * scaleFactor,
		loc.y * scaleFactor
	);
}
- (void)mouseUp:(NSEvent *)anEvent {
	NSPoint loc = [anEvent locationInWindow];
	CGFloat scaleFactor = self.window.screen.backingScaleFactor;
    LIL__setMouseUp(
		0,
		loc.x * scaleFactor,
		loc.y * scaleFactor,
		[anEvent clickCount]
	);
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
	[self.window setDelegate: self];
    
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();

    // Set the device for the layer so the layer can create drawable textures that can be rendered to
    // on this device.
    CAMetalLayer * metalLayer = (CAMetalLayer*)self.layer;
    metalLayer.device = device;

    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    self.renderer = [[LILMetalRenderer alloc] initWithMetalDevice:device drawablePixelFormat:metalLayer.pixelFormat];
    [self setupDisplayLink];

    [self resizeDrawable:self.bounds.size scaleFactor:self.window.screen.backingScaleFactor];
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
    // Stop the display link BEFORE releasing anything in the view otherwise the display link
    // thread may call into the view and crash when it encounters something that no longer
    // exists
    CVDisplayLinkStop(displayLink);
    //CVDisplayLinkRelease(displayLink);
}

- (NSSize)windowWillResize:(NSWindow *)sender 
                    toSize:(NSSize)frameSize
{
	[self resizeDrawable:frameSize scaleFactor:sender.screen.backingScaleFactor];
	return frameSize;
}

static CVReturn LIL__dispatchRenderLoop(CVDisplayLinkRef displayLink, const CVTimeStamp* now, const CVTimeStamp* outputTime, CVOptionFlags flagsIn, CVOptionFlags* flagsOut, void* displayLinkContext)
{
    LILMainView *theView = (__bridge LILMainView*)displayLinkContext;
    long int vertexCount = 0;
    long int textureVertexCount = 0;
    LILMetalRenderer * renderer = theView.renderer;
    long int currentTime = LIL__ticksTonanoseconds(outputTime->hostTime);
    double deltaTime;
    if (LIL__lastFrameTime == 0) {
        deltaTime = 0.0166666666666666666666666;
    } else {
        deltaTime = ((double)(currentTime - LIL__lastFrameTime) * 0.000000001);
    }
    LIL__lastFrameTime = currentTime;

    @synchronized(renderer)
    {
        LIL__nextFrame(deltaTime);
        char * vertexBufferPointer = [renderer getVertexBufferPointer];
        LIL__makeBoxVertices((void *)vertexBufferPointer, &vertexCount);
        renderer.boxVertexCount = vertexCount;
        LIL__makeTextureVertices((void *)(vertexBufferPointer + (vertexCount * sizeof(LILVertex))), &textureVertexCount);
    	vertexCount += textureVertexCount;
        renderer.textureVertexCount = textureVertexCount;
    	long int shapeVertexCount = 0;
    	long int shapeIndexCount = 0;
    	char * indexBufferPointer = [renderer getIndexBufferPointer];
        LIL__makeShapeVertices((void *)(vertexBufferPointer + (vertexCount * sizeof(LILVertex))), &shapeVertexCount, (void *)indexBufferPointer, &shapeIndexCount);
        renderer.shapeVertexCount = shapeVertexCount;
    	renderer.shapeIndexCount = shapeIndexCount;
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
- (id)init;
- (void)initializeMainWindowWithWidth:(float) width height:(float) height;
- (void)populateMainMenu;
- (void)addMenu:(const char *) label;
- (void)addMenuItem:(const char *)label key:(const char *)key fnPtr:(void(*))fnPtr;
- (void)addMenuSeparator;
- (void)exitMenu;
- (void)menuItemSelected:(NSMenuItem *) menuItem;
- (void)setWindowBackgroundRed:(float)red green:(float)green blue:(float)blue alpha:(float)alpha;
- (void)loadTextureForFile:(NSString *)path index:(unsigned int)idx;
- (void)unloadTextureForIndex:(unsigned int)idx;
- (LILMainView *)getMainView;
- (void)quit;
- (void)showOpenPanelWithSuccessCallback:(void(*)(const char *))onSuccess cancelCallback:(void(*)())onCancel;
- (void)showSavePanelWithSuccessCallback:(void(*)(const char *))onSuccess cancelCallback:(void(*)())onCancel;
@end
@implementation LILAppDelegate
- (id)init {
    if ( self = [super init] ) {
        menuStack = [[NSMutableArray alloc] init];
    }
    return self;
}
- (void)initializeMainWindowWithWidth:(float) width height:(float) height {
    NSScreen * mainScreen = [NSScreen mainScreen];
    NSRect screenRect = [mainScreen frame];

    width /= [mainScreen backingScaleFactor];
    height /= [mainScreen backingScaleFactor];
    
    if (width < 300.0) {
        width = 300.0;
    }
    if (height < 300.0) {
        height = 300.0;
    }
    
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
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    [mainWindow setContentView:mainView];
    
    long int count = LIL__getResourceCount();
    for(long int i = 0; i<count; i+=1) {
        LIL__resourceStruct * res = LIL__getResorceById(i);
        NSString * path = [[NSString alloc] initWithUTF8String: res->path];
        [self loadTextureForFile: path index:i];
    }
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
    if (LIL__automaticFullScreen()) {
        [mainWindow toggleFullScreen:self];
    }
    msgEmit("onFinishLaunching", NULL);
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    msgEmit("onTerminate", NULL);
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
    void(*fnPtr)(void *) = (void(*)())[menuItem tag];
    if ((NSUInteger)fnPtr == 0) {
        NSLog(@"Null pointer encountered");
        return;
    }
    fnPtr(NULL);
}

- (void)setWindowBackgroundRed:(float)red green:(float)green blue:(float)blue alpha:(float)alpha
{
    mainView.renderer.windowBgColor = MTLClearColorMake(red, green, blue, alpha);
}

- (void)loadTextureForFile:(NSString *)path index:(unsigned int)idx
{
	LILMetalRenderer * renderer = mainView.renderer;
    //avoid race conditions with anyone touching the renderer
    @synchronized(renderer)
    {
        [renderer loadTextureForFile:path index:idx];
    }
}
- (void)unloadTextureForIndex:(unsigned int)idx
{
	LILMetalRenderer * renderer = mainView.renderer;
    //avoid race conditions with anyone touching the renderer
    @synchronized(renderer)
    {
        [renderer unloadTextureForIndex:idx];
    }
}

- (LILMainView *)getMainView
{
    return mainView;
}

- (void)quit
{
    [[NSApplication  sharedApplication] terminate:self];
}

- (void)showOpenPanelWithSuccessCallback:(void(*)(const char *))onSuccess cancelCallback:(void(*)())onCancel
{
	NSOpenPanel* panel = [NSOpenPanel openPanel];

	// This method displays the panel and returns immediately.
	// The completion handler is called when the user selects an
	// item or cancels the panel.
	[panel beginWithCompletionHandler:^(NSInteger result){
		if (result == NSModalResponseOK) {
			NSURL*  theDoc = [[panel URLs] objectAtIndex:0];

			onSuccess([theDoc fileSystemRepresentation]);
		} else {
			onCancel();
		}
	}];
}

- (void)showSavePanelWithSuccessCallback:(void(*)(const char *))onSuccess cancelCallback:(void(*)())onCancel
{
	NSSavePanel* panel = [NSSavePanel savePanel];

	// This method displays the panel and returns immediately.
	// The completion handler is called when the user selects an
	// item or cancels the panel.
	[panel beginWithCompletionHandler:^(NSInteger result){
		if (result == NSModalResponseOK) {
			NSURL*  theDoc = [panel URL];

			onSuccess([theDoc fileSystemRepresentation]);
		} else {
			onCancel();
		}
	}];
}

@end

LILAppDelegate * LIL__applicationDelegate;

void LIL__run()
{
    // make sure the application singleton has been instantiated
    NSApplication * application = [NSApplication sharedApplication];

    // instantiate our application delegate
    LIL__applicationDelegate = [[LILAppDelegate alloc] init];
    
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];
    
    // assign our delegate to the NSApplication
    [application setDelegate:LIL__applicationDelegate];
    
    LIL__setupAudio();
    LIL__setupGamepads();
    LIL__timeInit();
    LIL__init();

    CGSize windowSize = LIL__getWindowSize();
    [LIL__applicationDelegate initializeMainWindowWithWidth: windowSize.width height: windowSize.height];

    // call the run method of our application
    [application run];

    LIL__audioFree();
}
