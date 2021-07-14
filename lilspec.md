# LIL Is a Language – Specification

## Basic Values

### Numbers:

There are four kinds of integers: `i8`, `i16`, `i32`, `i64` and `i128`, which are whole numbers, with no decimals. The `i` stands for integer
and the number is how many bits wide the value is.

	var.i8 foo: 127;

There are three kinds of floating point numbers, `f16`, `f32` and `f64`, which are numbers with decimals. Like with integers, the `f` stands for
floating point and the number is how many bits.

	var.f32 bar: 12.45564;

When you write a number literal without any decimals in the soure code, it could either be an integer or floating point. The compiler will try
to determine what was meant and if it can't, it defaults to an integer number type which is large enough to hold most numbers. In this example the type will be `i64`:

	var length: 37; //defaults to i64
	var weight: 1.5; //defaults to f64

### Percentages:

In certain places, you can express a value as a percentage. The type is written the same as the other number 
types, but appending the percentage sign `%` at the end. The same sign needs to be at the end of a number literal
for it to be a percentage.

	width: 25%;
	
For example, storing a percentage in a var declaration where we force the type of the percentage to be a floating point, even though the literal
value doesn't seem to be:

	var.f64% myWidth: 100%;

### Booleans:

Booleans are expressed using the identifiers `true` and `false`, and represent the logical values of 1 and 0, respectively.

	var enabled: true;

### Strings:

You use single `''` or double `""` quotes around the text that is in your string.

	var hello: "Hello world";

Special symbols and escapes are written using a forward slash `\` in front of a special character or using the same quote type as the string.

There is no difference between using one kind of quotes or another, just use whatever needs less escaping, like `"Can't"` instead of `'Can\'t'`.

Here is a list of the characters and their meaning. It is not complete, look in further chapters for the whole list.

- `\\` the backslash itself
- `\n` new line
- `\t` tab
- `\b` backspace
- `\0` C string termination character

### String functions:

Use the percent character `%` followed by an identifier to insert another value into the string you are writing.

	var myText: "This is the content";
	var html: "<div>%myText</div>";

If you need to write out
something more complex than a simple identifier, you can use curly braces and everything between them will be escaped. In this example, the app
will print "Welcome Mr. Joe Lilamassa" to the standard output.

	var name: "Joe";
	var surname: "Lilamassa";
	fn makeName(var name; var surname) {
		return "%name %surname";
	}
	print "Welcome Mr. %{ makeName(name, surname) }."
	

### C Strings

Many times you have to interface with external code which expects old school character buffers like in the C language. To make things nicer,
there is the `cstr` type, which is a sequence of characters with `\0` at the end.

To write a literal c string in the source code, you can use the backtick symbols: `` `contents of string` ``.

Regular LIL strings have a `cstr` property that you can access which will give you the underlying character buffer.

	var.str myString: "this is my LIL string";
	functionWantingACString( myString.cstr );

### Identifiers:

Identifiers are plain words, without any symbols, like `foo`, `myThing` or `_x3`, for example. They have to start with an underscore or a
letter. They represent names of things like variables, properties or classes.

There are some reserved words that can't be used as an identifier, such as:

- var
- ivar
- vvar
- class
- alias
- type
- true
- false
- if
- else
- switch
- case
- default
- return
- finally
- for
- loop
- continue
- repeat
- break
- null


## Comments

Comments are used to add information to the source code which is intended to be read only by the humans reading
the code. They start either with two forward slashes `//` or a forward slash and a star `/*`.

In the case of the double slash, we call them single line comments, because it is interpreted that everything
between the slashes and the end of the line is the content of the comment. For example:

	//this is a comment and it ends here

On the other hand, if the slash and star is used, the source code that follows will be interpreted as a comment
until the oposite symbol is found, a star and a slash: `*/`. For example:

	/* this is the comment and
	it can span multiple
	lines until the end */ 

## Expressions

There are the basic math operations of sum, subtraction, multiplication and division, using the `+`, `-`, `*` and `/` symbols, respectively.
There is also the modulo operator `%`, which returns the remainder of a division.

	var.i64 myNum: 100 + 50;
	var.i64 myNum2: myNum * 2;

Expressions can be wrapped in parenthesis `(<lhs> <sign> <rhs>)`, in some cases to desambiguate, and others just to increase legibility.

## Functions

Functions are written using the `fn` keyword, followed by the name of the function and then optionally followed by variable declarations
wrapped in parenthesis `fn doStuff ( var myArg ) { <statements...>  }`, and then a list of commands inside a block delimited by curly braces `{ }`.

	fn sumOfTwoNumbers (var.i32 numA; var.i32 numB) {
		return numA + numB;
	};

Functions can also be written as values that are stored inside variables and take that name. For example, this is a function that we call `myFunc`,
which takes no parameters, and always prints hello to the standard output when called:

	var myFunc: fn {
		print "hello";
	}

## Classes

These are like templates to make identical, but separate, copies of various other values put together as a unit, which we call object instances.

They are written using the `class` keyword, followed by whitespace, then the "object symbol" `@`, and then the name of the class. A semicolon
afterwards is allowed but not required. For example:

	class @myClass {
		// more stuff here
	}

## Value paths

Value paths are used to access properties and methods of objects. They are written as a series of components of a larger unit:

First comes either a name or one of the special selector objects, like `@self` or `@root`.

If there are more components, you use a dot `.` as a separator and then the next piece. Whitespace between components and the dot is not
allowed.

For example:

	var myString: "hello";
	print myString.length;

Here we are calling the `length`method of the `myString` object

### Instantiating objects

## Variable declarations

A variable declaration defines a a space in memory that can hold a value. You declare it with a name which you will use later to access its
content. It can be initialized with a value or left empty and assigned later.

To declare a variable you write the `var` keyword, followed by some whitespace, followed by the name of the variable. Then a colon `:` and a
value or a semicolon to denote the end.

	var foo: 1;
	
You can provide the type of the variable, so no guessing needs to take place, by writing a dot `.` after the `var` keyword.

	var.i8 foo: 1;

## Assignments

In LIL the assignment is written with a colon `:`, not an equal sign. In the following example we declare a variable named `myString`, then check
if `foo` is equal to 1 (a single `=` is a comparator) and  then assign a different string depending on the case. The compiler will use type
inference to know that `myString` is a variable of the correct type.

	var myString;
	if foo = 1 {
		myString: "Hello world";
	} else {
		myString: "Foo was not one";
	}

### Fields

Since an object is a group of values, you have to specify each as a variable declaration. For example, the following class contains an integer,
a floating point value and another object of class `@string`:

	class @myClass2 {
		var.i64 id;
		var.f32 value;
		var.@string description;
	}

### Methods

When you put functions inside a class they are called methods. These are said to be "called on" an object instance. You typically use a value
path ending in a function call to invoke them. Inside the method the special selector object `@self` is available, which is a pointer to the
object instance itself.

The next example shows a class with a method which takes a boolean as argument and prints a message if `true` is passed:

	class @myClass3 {
		var.@string message;
		fn printMessage (var.bool areYouSure) {
			if areYouSure {
				print @self.message;
			}
		};
	}

	var myObjectInstance: @myClass3 { message: "This is the message" };
	myObjectInstance.printMessage(true);

## Comparisons

## Pointers

## Casts

## Unary expressions

## Multiple types

## Namespaces

## Alias, types and conversions

## Arrays

### Static arrays
### Array objects

## Flow control

### If, else
### If cast
### Switch
### Finally
### Loop
### For loop

## Flow control calls

### Return
### Repeat
### Continue
### Break

## The @root object

## Rules

## Selectors

## Filters

## Flags

## Combinators

## Instructions

### Needs
### Export
### Configure
### New
### Move
### Delete
### Colors

## Constants
### Arg

The `#arg` instruction is useful when you want to allow an user to pass arguments to the
compiler, allowing to customize the build by, for example, choosing which part of a file
gets compiled, or allowing the user to import a file with a custom filename.

The syntax is `#arg` followed by optional whitespace, then a block of curly braces, which
contain two assignments: `name` and `default`, to which we give the desired values.

	const.i64 theSize: #arg { name: "defaultSize"; default: 1024 };
	
	//invoke compiler with: --defaultSize:1024
	//e.g.:  lil main.lil --defaultSize:1024
	
Since the command line eats the quotes, if you want to use a string put the whole command
inside single quotes:
	
	lil main.lil '--appName:"My App"'

## Snippets

## Foreign languages
