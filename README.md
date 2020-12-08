# LIL is a Language

Hi there! LIL is a new programming language targeted at creating native apps and games, on desktop and mobile platforms, maybe even on consoles.

Programming in LIL will be somewhat familiar to developers coming from the web development world, since the original inspiration for its design was
to provide a better alternative to CSS, but then grew into a full blown language of its own, taking inspirations from many sources.

## Mini tutorial

LIL aims to be "batteries included". That means that by default it will provide the infrastructure necessary to render your app into a window or screen.
You have a full 2D/3D rendering engine at your disposal that will take care of such details for you, so you can focus on what makes your app or game special.

At the topmost of the visual hierarchy of a frame there is the `@root` object, which you can use in a selector chain, like so:

	//add 10 points of padding
	@root {
		padding: 10;
	}

Let's say we want to add a button to the app:

	@root {
		padding: 10;

		#new myButton {
			width: 200;
			height: 60;
			content: "Press me";
			font.color: #F;
			background: #C;
			shape: 10; //rounded corners
		}
	}

Now let's add a hover state:

	@root {
		padding: 10;

		#new myButton {
			width: 200;
			height: 60;
			content: "Press me";
			font.color: #F;
			background: #C;
			shape: 10; //rounded corners
			
			@this::hover {
				background: #0;
			}
		}
	}

Now we have a button we can interact with. But we would like to reuse this button style, so we put it into a variable:

	var buttonStyle: @container {
		width: 200;
		height: 60;
		font.color: #F;
		background: #C;
		shape: 10; //rounded corners

		@this::hover {
			background: #0;
		}
	};

	@root {
		padding: 10;

		//apply the styles with the isA property
		#new myButton {
			isA: buttonStyle;
			content: "Press me";
		}
		//we can make another one 
		#new myButton2 {
			isA: buttonStyle;
			content: "No thank you";
		}
	}

But a button would be not be a real button if it doesn't do anything. Let's change that:

	fn button1Pressed {
		//just put some text into the terminal output
		print "Button 1 pressed";
	};
	fn aPress(var.i64 number) {
		print "Pressed button %number";
	};
	var buttonStyle: @container {
		width: 200;
		height: 60;
		font.color: #F;
		background: #C;
		shape: 10; //rounded corners

		@this::hover {
			background: #0;
		}
	};
	@root {
		padding: 10;

		//apply the styles with the isA property
		#new myButton {
			isA: buttonStyle;
			content: "Press me";
			on.click: button1Pressed();
		}
		//we can make another one 
		#new myButton2 {
			isA: buttonStyle;
			content: "No thank you";
			on.click: aPress(2);
		}
	}

Now if you click the first button, it will print the text "Button 1 pressed" to the console, without the quotes, and if you press the second one it
will output "Pressed button 2", since we passed `2` as the argument to the function. Notice that the call is not executed when it is declared but
as a result of the user interaction. This is because it was expressed inside of a rule, and the compiler knows that it needs to create an anonymous
function containing your code and calls it appropriately when needed.

Obviously this is nowhere enough to make a full app. I'll be adding some more complete tutorials in the future, stay tuned! Also take a look at the
specification, it is just the beginning right now, but this will all come together soon.

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
