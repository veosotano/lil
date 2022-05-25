# LIL is a Language

WARNING: Work In Progress!!!

LIL aims to be a programming language which:

- has a nice syntax, trying to be intuitive, readable and consistent
- is very low level but still feels quite high level
- focuses on performance to create 60fps games
- gives high control over memory management
- is aimed at two audiences: general users and library implementors
- is an all-in-one solution: build system, compiler and shader language all in the same language

## Mini tutorial

LIL aims to be "batteries included". That means that by default it will provide the infrastructure necessary to render your game into a window or screen.
You have a full 2D/3D rendering engine at your disposal that will take care of such details for you, so you can focus on what makes your app or game special.

At the topmost of the visual hierarchy of a frame there is the `@root` object, which you can use to target the main view itself, like so:

	//set the size of the window and background color
	@root {
		width: 1440;
		height: 900;
		background: #C;
	}

Let's add something to it:

	@root {
		width: 1440;
		height: 900;
		background: #C;

		#new @container test {
			width: 200;
			height: 200;
			background: #9;
			x: 620;
			y: 350;
		}
	}

Instead of just a square, let's make it an image:

	@root {
		width: 1440;
		height: 900;
		background: #C;

		#new @image test {
			width: 200;
			height: 200;
			src: "myimage.png";
			x: 620;
			y: 350;
		}
	}

We already have some rudimentary physics (more to come), so let's play with velocities:

	@root {
		width: 1440;
		height: 900;
		background: #C;

		#new @image test {
			width: 200;
			height: 200;
			src: "myimage.png";
			x: 620;
			y: 350;
			velocity: @vel {
				x: 5;
				y: 3;
			};
		}
	}

Let's set some configuration values. We give the app a name, we automatically enter fullscreen mode and we enable the onUpdate function and do some work on every frame.

	#configure {
    	name: "Professor";
    	automaticFullScreen: true;
    	onUpdateFn: true;
	}
	fn onUpdate(var.f64 deltaTime) {
		//cheating here, no selection system yet
		//also, suppose the following functions exist:
		var img: @image { id: 1 };
		applyUserInput(img);
		limitSpeed(img);
		updateAI(img);
		print "done with the frame :)";
	}
	@root {
		width: 1440;
		height: 900;
		background: #C;

		#new @image test {
			width: 200;
			height: 200;
			src: "myimage.png";
			x: 620;
			y: 350;
			velocity: @vel {
				x: 5;
				y: 3;
			};
		}
	}

To be continued...

## Please contribute
This is an open source project which aims to bring a great new technology to use for the general public, to make it fun to create apps and games.
To be easy to learn and read, while at the same time be powerful enough to not feel limiting or having to resort to hacks to get things done.

If you want to help bring that idea to reality, please get in touch. Any help a any skill level is appreciated.

## General information

This is the main repository of the LIL language. I chose the MIT license to get started, but this is all up for discussion.

The file lilspec.md contains the specification of the language. It's not a tutorial, but it's quite easy to read. Warning: WORK IN PROGRESS!!

In the cpp folder resides the compiler that is a custom front-end that interfaces with LLVM to generate native machine code. Warning: WORK IN PROGRESS!!

The lil_cli folder contains the command line interface to the compiler. Also written in C++. Warning: WORK IN PROGRESS!!

Please don't judge the quality of this project by the existing code: there is sooooo much yet to come. The examples are just basic stubs that need to be
replaced with some good informative ones.

Hit me up on Twitter if you want to contribute, discuss or just say hi: https://twitter.com/veosotano

Peace
