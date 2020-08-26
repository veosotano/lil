# LIL is a Language – Specification

## Basic Values

### Numbers:

There are four kinds of integers: `i8`, `i16`, `i32` and `i64`, which are whole numbers, with no decimals. The `i` stands for integer and the number is how many bits wide the value is.

There are two kinds of floating point numbers, `f32` and `f64`, which are numbers with decimals. Like with integers, the `f` stands for floating point and the number is how many bits.

When you write a number literal without any decimals in the soure code, it could either be an integer or floating point. The compiler will try to determine what was meant and if it can't, it defaults to `i64`.

### Booleans:

Booleans are expressed using the identifiers `true` and `false`, and represent the logical values of 1 and 0, respectively.

### Strings:

You use single `''` or double `""` quotes around the text that is in your string.

Special symbols and escapes are written using a forward slash `\` in front of a special character or using the same quote type as the string.

There is no difference between using one kind of quotes or another, just use whatever needs less escaping, like `"Can't"` instead of `'Can\'t'`.

Here is a list of the characters and their meaning. It is not complete, look in further chapters for the whole list.

- `\\` the backslash itself
- `\n` new line
- `\t` tab
- `\b` backspace
- `\0` C string termination character

### C Strings

Many times you have to interface with external code which expects old school character buffers like in the C language. To make things nicer, there is the `cstr` type, which is a sequence of characters with `\0` at the end.

To write a literal c string in the source code, you can use the backtick symbols: `` `contents of string` ``.

Regular LIL strings have a `cstr` method that you can call which will give you the underlying character buffer.

```
var.str myString: "this is my LIL string";
functionWantingACString( myString.cstr );
```

### Identifiers:

Identifiers are plain words, without any symbols, like `foo`, `myThing` or `_x3`, for example. They have to start with an underscore or a letter. They represent names of things like variables, properties or classes.

There are some reserved words that can't be used as an identifier, such as:

- var
- ivar
- prop
- class
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


## Variables

In LIL, all variables need to be declared before they can be used. To do so, you use the `var` keyword (optionally annotaded with the type of the variable using a dot and then the name of the type), followed by whitespace, followed by an identifier to define the name of the variable.

The declaration can end there or it can have an initial value. You write a colon `:`, some whitespace and then a value, like a number, a path to a value or even a function call.

```
var.i64 myEmptyVariable;
var foo: 10;
var bar: doStuff(foo);
```

## Expressions

There are the basic math operations of sum, subtraction, multiplication and division, using the `+`, `-`, `*` and `/` symbols, respectively.

```
var.i64 myNum: 100 + 50;
var.i64 myNum2: myNum * 2;
```

Expressions can be wrapped in parenthesis `(<lhs> <sign> <rhs>)`, in some cases to desambiguate, and others just to increase legibility.

## Functions

Functions are written as values that are stored inside variables and take that name. You use the `fn` keyword, optionally followed by variable declarations wrapped in parenthesis `fn ( var myArg ) { <statements...>  }`, and then a list of commands inside a block delimited by curly braces `{ }`. For example:

```
var myFunc: fn {
	print "hello";
}
```

This is a function that we call `myFoo`, which takes no parameters, and always prints hello to the standard output when called.

## Value paths

When you need to pass some existing value somewhere you use a value path. They are written as a series of components of a larger unit. First comes either a name or one of the special selector objects, like `@self` or `@root`.

If there are more components, you use a dot `.` as a separator and then the next piece. Whitespace between components and the dot is not allowed.

For example:

```
var myString: "hello";
print myString.length;
```

Here we are calling the `length`method of the `myString` object
