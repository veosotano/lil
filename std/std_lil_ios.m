#import <UIKit/UIKit.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#include <simd/simd.h>
#include <math.h>

typedef struct LIL__resourceStruct {
	char path[1024];
	void * data;
} LIL__resourceStruct;


extern void LIL__init();
extern void LIL__nextFrame(double deltaTime);
extern void LIL__makeBoxVertices(void * vertexBuffer, long int * vertexCount);
extern void LIL__makeTextureVertices(void * vertexBuffer, long int * vertexCount);
extern void LIL__makeShapeVertices(void * vertexBuffer, long int * vertexCount, void * indexBuffer, long int * indexCount);
extern long int LIL__getResourceCount();
extern LIL__resourceStruct * LIL__getResorceById(long int id);
extern void LIL__setTextureSize(long int imgId, double width, double height);
extern void msgEmit(char * name, void * data);
extern void LIL__notifyChange(long int theId, const char * value);

extern void LIL__setMouseDown(long int buttonNumber, double x, double y);
extern void LIL__setMouseDragged(long int buttonNumber, double x, double y);
extern void LIL__setMouseUp(long int buttonNumber, double x, double y, long int clickCount);


static long int LIL__roundUp(long int numToRound, long int multiple) {
	assert(multiple && ((multiple & (multiple - 1)) == 0));
	return (numToRound + multiple - 1) & -multiple;
}

// =========== renderer =============
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
@property (nonatomic) double scale;

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
#if TARGET_OS_SIMULATOR
	id <MTLTexture> renderTarget;
#else
	id <MTLTexture> msaaTexture_;
	id <MTLTexture> msaaDepthTexture_;
#endif
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
	double scale_;
}

@synthesize windowBgColor = windowBgColor_;
@synthesize boxVertexCount = boxVertexCount_;
@synthesize textureVertexCount = textureVertexCount_;
@synthesize textureCount = textureCount_;
@synthesize shapeVertexCount = shapeVertexCount_;
@synthesize shapeIndexCount = shapeIndexCount_;
@synthesize scale = scale_;

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
		scale_ = 1.0;

		commandQueue = [device newCommandQueue];

		drawableRenderDescriptor = [MTLRenderPassDescriptor new];
		drawableRenderDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
#if TARGET_OS_SIMULATOR
		drawableRenderDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
		drawableRenderDescriptor.depthAttachment.storeAction = MTLStoreActionDontCare;
#else
		drawableRenderDescriptor.colorAttachments[0].storeAction = MTLStoreActionMultisampleResolve;
		drawableRenderDescriptor.depthAttachment.storeAction = MTLStoreActionMultisampleResolve;
#endif
		drawableRenderDescriptor.depthAttachment.loadAction = MTLLoadActionClear;
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
#if !TARGET_OS_SIMULATOR
		boxPipelineDescriptor.rasterSampleCount			   = 4;
#endif
		boxPipelineDescriptor.label						   = @"boxPipeline";
		boxPipelineDescriptor.vertexFunction				  = vertexShaderFn;
		boxPipelineDescriptor.fragmentFunction				= fragmentShaderFn;
		boxPipelineDescriptor.colorAttachments[0].pixelFormat = drawablePixelFormat_;
		boxPipelineDescriptor.depthAttachmentPixelFormat	  = LILDepthPixelFormat;

		boxPipeline = [device newRenderPipelineStateWithDescriptor:boxPipelineDescriptor error:&error];
		if(!boxPipeline)
		{
			NSLog(@"ERROR: Failed aquiring box pipeline state: %@", error);
			return nil;
		}
		
		//shape
		MTLRenderPipelineDescriptor *shapePipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
#if !TARGET_OS_SIMULATOR
		shapePipelineDescriptor.rasterSampleCount				= 4;
#endif
		shapePipelineDescriptor.label						   = @"shapePipeline";
		shapePipelineDescriptor.vertexFunction				  = vertexShaderFn;
		shapePipelineDescriptor.fragmentFunction				= fragmentShaderFn;
		shapePipelineDescriptor.colorAttachments[0].pixelFormat = drawablePixelFormat_;
		shapePipelineDescriptor.depthAttachmentPixelFormat	  = LILDepthPixelFormat;

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
#if !TARGET_OS_SIMULATOR
		texturePipelineDescriptor.rasterSampleCount			   = 4;
#endif
		texturePipelineDescriptor.label						   = @"texturePipeline";
		texturePipelineDescriptor.vertexFunction				  = vertexShaderFn;
		texturePipelineDescriptor.fragmentFunction				= textureShaderFn;
		MTLRenderPipelineColorAttachmentDescriptor *attachment = texturePipelineDescriptor.colorAttachments[0];
		attachment.pixelFormat = drawablePixelFormat_;
		attachment.blendingEnabled = YES;
		attachment.rgbBlendOperation = MTLBlendOperationAdd;
		attachment.alphaBlendOperation = MTLBlendOperationAdd;
		attachment.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
		attachment.sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
		attachment.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
		attachment.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
		texturePipelineDescriptor.depthAttachmentPixelFormat	  = LILDepthPixelFormat;

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
#if TARGET_OS_SIMULATOR
		drawableRenderDescriptor.colorAttachments[0].texture = currentDrawable.texture;
#else
		drawableRenderDescriptor.colorAttachments[0].resolveTexture = currentDrawable.texture;
#endif
	
		id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:drawableRenderDescriptor];

		//box
		[renderEncoder setRenderPipelineState:boxPipeline];

		[renderEncoder setVertexBuffer:vertexBuffer offset:0 atIndex:LILVertexInputIndexVertices ];

		LILUniforms uniforms;
		uniforms.scale = scale_;
		uniforms.viewportSize = viewportSize;
		[renderEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:LILVertexInputIndexUniforms ];

		[renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:self.boxVertexCount];

		long int textureOffset = 0;
		long int shapeOffset = 0;

		for (long int i = 0; i < self.textureCount; i+=1) {
			//texture
			if(texturePipelines[i] == nil) {
				NSLog(@"Error loading pipeline state nr %li\n", i);
			}
			[renderEncoder setRenderPipelineState:texturePipelines[i]];
			textureOffset = LIL__roundUp(sizeof(LILVertex) * self.boxVertexCount, 256);
			[renderEncoder setVertexBuffer:vertexBuffer offset:textureOffset atIndex:LILVertexInputIndexVertices ];
			[renderEncoder setVertexBytes:&uniforms length:sizeof(uniforms) atIndex:LILVertexInputIndexUniforms ];
			[renderEncoder setFragmentTexture:textures[i] atIndex:0];

			long int vtxStart = i * 6;

			[renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:vtxStart vertexCount:6];
		}

		//shape
		if (self.shapeIndexCount > 0) {
			shapeOffset = LIL__roundUp(textureOffset + (sizeof(LILVertex) * self.textureVertexCount), 256);
			[renderEncoder setRenderPipelineState:shapePipeline];
			[renderEncoder setVertexBuffer:vertexBuffer offset:shapeOffset atIndex:LILVertexInputIndexVertices ];
			LILUniforms shapeUniforms;
			shapeUniforms.scale = scale_;
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
#if TARGET_OS_SIMULATOR
	MTLTextureDescriptor *renderTargetDescriptor = [MTLTextureDescriptor new];
	renderTargetDescriptor.width = drawableSize.width;
	renderTargetDescriptor.height = drawableSize.height;
	renderTargetDescriptor.pixelFormat = drawablePixelFormat_;
	renderTargetDescriptor.storageMode = MTLStorageModePrivate;
	renderTargetDescriptor.usage = MTLTextureUsageRenderTarget;
	
	renderTarget = [device newTextureWithDescriptor:renderTargetDescriptor];
	drawableRenderDescriptor.colorAttachments[0].texture = renderTarget;
	drawableRenderDescriptor.depthAttachment.texture = depthTarget;
#else
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
#endif
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


// =========== main view =============
@interface LILMainView : UIView <CALayerDelegate> {	
}
@property(nonatomic, nonnull, readonly) LILMetalRenderer * renderer;
@property (nonatomic, nonnull, readonly) CAMetalLayer *metalLayer;
@property (nonatomic, getter=isPaused) BOOL paused;

- (void)initCommon;
- (void)resizeDrawable:(CGFloat)scaleFactor;
- (void)stopRenderLoop;
- (void)render;

@end
@implementation LILMainView {
	LILMetalRenderer * _renderer;
	CADisplayLink * _displayLink;
	NSThread * _renderThread;
	BOOL _continueRunLoop;
}

@synthesize renderer = _renderer;

+ (Class) layerClass
{
	return [CAMetalLayer class];
}

- (instancetype) initWithFrame:(CGRect)frame
{
	self = [super initWithFrame:frame];
	if(self)
	{
		[self initCommon];
	}
	return self;
}

- (instancetype) initWithCoder:(NSCoder *)aDecoder
{
	self = [super initWithCoder:aDecoder];
	if(self)
	{
		[self initCommon];
	}
	return self;
}

- (void)initCommon
{
	_paused = NO;
	_metalLayer = (CAMetalLayer*) self.layer;

	id<MTLDevice> device = MTLCreateSystemDefaultDevice();
	_metalLayer.device = device;
	_metalLayer.delegate = self;

	_metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
	_renderer = [[LILMetalRenderer alloc] initWithMetalDevice:device drawablePixelFormat:_metalLayer.pixelFormat];
}

- (void)resizeDrawable:(CGFloat)scaleFactor
{
	CGSize newSize = self.bounds.size;
	newSize.width *= scaleFactor;
	newSize.height *= scaleFactor;

	_renderer.scale = scaleFactor;
	NSLog(@"scaleFactor: %lf", scaleFactor);

	if(newSize.width <= 0 || newSize.width <= 0)
	{
		return;
	}

	// All AppKit and UIKit calls which notify of a resize are called on the main thread.  Use
	// a synchronized block to ensure that resize notifications on the delegate are atomic
	@synchronized(_metalLayer)
	{
		if(newSize.width == _metalLayer.drawableSize.width &&
			newSize.height == _metalLayer.drawableSize.height)
		{
			return;
		}

		_metalLayer.drawableSize = newSize;

		[_renderer drawableResize:newSize];
	}
}

- (void)render
{
	long int vertexCount = 0;
	long int textureVertexCount = 0;

	// long int currentTime = LIL__ticksTonanoseconds(outputTime->hostTime);
	double deltaTime = 0.0166666666666666666666666;
	// if (LIL__lastFrameTime == 0) {
	// 	deltaTime = 0.0166666666666666666666666;
	// } else {
	// 	deltaTime = ((double)(currentTime - LIL__lastFrameTime) * 0.000000001);
	// }
	// LIL__lastFrameTime = currentTime;

	@synchronized(_renderer)
	{
		LIL__nextFrame(deltaTime);
		char * vertexBufferPointer = [_renderer getVertexBufferPointer];
		LIL__makeBoxVertices((void *)vertexBufferPointer, &vertexCount);
		_renderer.boxVertexCount = vertexCount;
		char * textureVtxBufferPtr = (char *)LIL__roundUp((long int)(vertexBufferPointer + (vertexCount * sizeof(LILVertex))), 256);
		LIL__makeTextureVertices((void *)textureVtxBufferPtr, &textureVertexCount);
		_renderer.textureVertexCount = textureVertexCount;
		long int shapeVertexCount = 0;
		long int shapeIndexCount = 0;
		char * indexBufferPointer = [_renderer getIndexBufferPointer];
		char * shapeVtxBufferPtr = (char *)LIL__roundUp((long int)textureVtxBufferPtr + (textureVertexCount * sizeof(LILVertex)), 256);
		LIL__makeShapeVertices((void *)shapeVtxBufferPtr, &shapeVertexCount, (void *)indexBufferPointer, &shapeIndexCount);
		_renderer.shapeVertexCount = shapeVertexCount;
		_renderer.shapeIndexCount = shapeIndexCount;
	}
	// Must synchronize if rendering on background thread to ensure resize operations from the
	// main thread are complete before rendering which depends on the size occurs.
	@synchronized(_metalLayer)
	{
		[_renderer renderToMetalLayer:_metalLayer];
	}
}



- (void)didMoveToWindow
{
	[super didMoveToWindow];

	if(self.window == nil)
	{
		// If moving off of a window destroy the display link.
		[_displayLink invalidate];
		_displayLink = nil;
		return;
	}

	//load textures
	long int count = LIL__getResourceCount();
	for(long int i = 0; i<count; i+=1) {
		LIL__resourceStruct * res = LIL__getResorceById(i);
		NSString * path = [[NSString alloc] initWithUTF8String: res->path];
		[_renderer loadTextureForFile: path index:i];
	}

	[self setupCADisplayLinkForScreen:self.window.screen];

	// Protect _continueRunLoop with a `@synchronized` block since it is accessed by the seperate
	// animation thread
	@synchronized(self)
	{
		// Stop animation loop allowing the loop to complete if it's in progress.
		_continueRunLoop = NO;
	}

	// Create and start a secondary NSThread which will have another run runloop.  The NSThread
	// class will call the 'runThread' method at the start of the secondary thread's execution.
	_renderThread =  [[NSThread alloc] initWithTarget:self selector:@selector(runThread) object:nil];
	_continueRunLoop = YES;
	[_renderThread start];

	// Perform any actions which need to know the size and scale of the drawable.  When UIKit calls
	// didMoveToWindow after the view initialization, this is the first opportunity to notify
	// components of the drawable's size
	[self resizeDrawable:self.window.screen.scale];
}

- (void)setPaused:(BOOL)paused
{
	_paused = paused;

	_displayLink.paused = paused;
}

- (void)setupCADisplayLinkForScreen:(UIScreen*)screen
{
	[self stopRenderLoop];

	_displayLink = [screen displayLinkWithTarget:self selector:@selector(render)];
	_displayLink.paused = self.paused;
	_displayLink.preferredFramesPerSecond = 60;
}

- (void)didEnterBackground:(NSNotification*)notification
{
	self.paused = YES;
}

- (void)willEnterForeground:(NSNotification*)notification
{
	self.paused = NO;
}

- (void)stopRenderLoop
{
	[_displayLink invalidate];
}

- (void)runThread
{
	// Set the display link to the run loop of this thread so its call back occurs on this thread
	NSRunLoop *runLoop = [NSRunLoop currentRunLoop];
	[_displayLink addToRunLoop:runLoop forMode:@"AAPLDisplayLinkMode"];

	// The '_continueRunLoop' ivar is set outside this thread, so it must be synchronized.  Create a
	// 'continueRunLoop' local var that can be set from the _continueRunLoop ivar in a @synchronized block
	BOOL continueRunLoop = YES;

	// Begin the run loop
	while (continueRunLoop)
	{
		// Create autorelease pool for the current iteration of loop.
		@autoreleasepool
		{
			// Run the loop once accepting input only from the display link.
			[runLoop runMode:@"AAPLDisplayLinkMode" beforeDate:[NSDate distantFuture]];
		}

		// Synchronize this with the _continueRunLoop ivar which is set on another thread
		@synchronized(self)
		{
			// Anything accessed outside the thread such as the '_continueRunLoop' ivar
			// is read inside the synchronized block to ensure it is fully/atomically written
			continueRunLoop = _continueRunLoop;
		}
	}
}

// Override all methods which indicate the view's size has changed

- (void)setContentScaleFactor:(CGFloat)contentScaleFactor
{
	[super setContentScaleFactor:contentScaleFactor];
	[self resizeDrawable:self.window.screen.scale];
}

- (void)layoutSubviews
{
	[super layoutSubviews];
	[self resizeDrawable:self.window.screen.scale];
}

- (void)setFrame:(CGRect)frame
{
	[super setFrame:frame];
	[self resizeDrawable:self.window.screen.scale];
}

- (void)setBounds:(CGRect)bounds
{
	[super setBounds:bounds];
	[self resizeDrawable:self.window.screen.scale];
}

- (void)touchesBegan:(NSSet<UITouch *> *)touches  withEvent:(UIEvent *)event
{

}

- (void)touchesMoved:(NSSet<UITouch *> *)touches  withEvent:(UIEvent *)event
{
	NSLog(@"Touches moved");
}

- (void)touchesEnded:(NSSet<UITouch *> *)touches  withEvent:(UIEvent *)event
{
	//location in window
	if ([touches count] == 1) {
		CGRect viewBounds = [self bounds];
		UITouch * touch = [touches anyObject];
		CGPoint loc = [touch locationInView:nil];
		LIL__setMouseUp(
			0,
			loc.x,
			viewBounds.size.height - loc.y,
			[touch tapCount]
		);
	}
}

- (void)touchesCancelled:(NSSet<UITouch *> *)touches  withEvent:(UIEvent *)event
{
	NSLog(@"Touches cancelled");
}

@end



// =========== app delegate =============
@interface LILAppDelegate : UIResponder <UIApplicationDelegate> {
	LILMainView * mainView;
}
- (void)setMainView:(LILMainView *)theView;
- (void)setWindowBackgroundRed:(double)red green:(double)green blue:(double)blue alpha:(double)alpha;
- (CGSize)getUIWindowSize;
- (LILMainView *)getMainView;
@property (strong, nonatomic) UIWindow *window;

@end


LILAppDelegate * LIL__applicationDelegate;

@implementation LILAppDelegate
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
	LIL__applicationDelegate = self;
	return YES;
}

- (void)setMainView:(LILMainView *)theView {
	mainView = theView;
}

- (void)setWindowBackgroundRed:(double)red green:(double)green blue:(double)blue alpha:(double)alpha {
	mainView.renderer.windowBgColor = MTLClearColorMake(red, green, blue, alpha);
}
- (void)applicationWillResignActive:(UIApplication *)application {
	// Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
	// Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
	// Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
	// If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
	// Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
	// Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application {
	// Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

- (CGSize)getUIWindowSize {
	return mainView.frame.size;
}

- (LILMainView *)getMainView
{
	return mainView;
}

@end


// =========== view controller =============
@interface LILViewController : UIViewController {
}

@end

@implementation LILViewController {
	
}

- (void)viewDidLoad {
	[super viewDidLoad];
	LILMainView * mainView = (LILMainView *)self.view;
	[LIL__applicationDelegate setMainView:mainView];

	LIL__init();
}


@end


// ============ main function ==============
int main(int argc, char * argv[]) {
	NSString * appDelegateClassName;
	@autoreleasepool {
		// Setup code that might create autoreleased objects goes here.
		appDelegateClassName = NSStringFromClass([LILAppDelegate class]);
	}
	return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
