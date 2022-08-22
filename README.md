## Goals

- [ ] Object-oriented programming (inherit, up-case, down-case, interface)
- [ ] Closure support
- [ ] Multi-threading support
- [ ] Generic programming (template support)

## Syntax

```dart
use Hoshi::IO;

func main () -> void {
    IO.println("Hello, Hoshi.");
    return;
}
```

## Design

### Basic types

There are 6 kinds of basic types in hoshi-lang.

They are `(int) Integer`, `(uint) Unsigned integer`, `(float) Decimal`, `(char) Character`, `(bool) Boolean`, `(rstr) Read-only string literal`.

### Type-casting

You must specify the target type explicitly in the casting statement.

Reference expression: `as expression`

Like these following code:

```dart
use Hoshi::IO;
use Hoshi::Str;

func main () -> void {
    var i : int = 128;
    var j : uint = i as uint;
    IO.println(Str.fromInt(i));
    return;
}
```

### String literals

String literals are read-only in hoshi-lang. They are `rstr` type. String literals can be joined to a `Str` object by `Str.join()` method.

### Expressions

Different expressions will have different priority.

- MemberExpressions : `.`
- StaticMemberAccessExpressions : `::`
- FunctionInvokingExpressions : `()`
- IndexExpressions : `[]`
- SingleExpressions : `+() !() -() ~() ++ --`
- MultipleExpressions : `* / %`
- AdditionExpressions : `+ -`
- BinaryMoveExpressions : `<< >>`
- LogicComparingExpressions : `< <= > >= instanceOf implemented`
- LogicEqualExpressions : `== !=`
- BinaryExpressions : `& ^ |`
- BooleanExpressions : `&& ||`
- AsExpression : `as`
- AssignmentExpressions : `= += -= &= /= %= &= |= ^= ~= <<= >>=`

### Classes

```dart
use Hoshi::IO;
use Hoshi::Str;

interface Printable {
    func toString () -> Str {
        return "";
    }
};

class Foo {
    var name : Str;
    
    // new expression will invoke this method with `this` pointer without any other arguments
    // the default constructor will be the constructor without any arguments
    // if there's no default constructor, compiler will throw a exception.
    func constructor () -> void {
        // you must access members with `this`.
        this.name = "Foo";
        return;
    }
    
    func say () -> void {
        IO.println( ("Watashino namae ha: " as Str) + name );
        return;
    }
};

class Bar extends Foo {
    public name;
    public say;
	func constructor () -> void {
        this.constructor();
        this.name = "Bar";
        return;
    }
}

func main () -> void {
    // compiler will push the object to heap without `new` expression
    var s : Foo = Foo();
    s.say();
    return;
}
```

- **Inheritance**

  Classes in hoshi-lang can be inherited by `extends`.

  Compiler will create a `super` member for parent class.

  You can't access any member or method in parent class in outside. To make members or methods in parent class public, you can use `public <method name>;` to declare it in child class. After that, you can access `this.super.blahblah` by `this.blahblah` directly.

- **Interface**

  Interface is a method table that means that a class implemented some feature, and they can return a result just like the interface.

  You can use `interface` to create an interface. Use `class <classname> impl <interface_name>` to make compiler know what interfaces are this class implemented.

  Use `<class_name> as <interface_name>` to covert a class to an interface.

- **Up-casting**

  Up-casting means covert a child class to the parent class.

  

- 

### Generic programming (Template)

```dart
use Hoshi::IO;
use Hoshi::Str;

template <typename T>
class Foo {
    var a : T;
    func constructor (a : T) -> void {
        this.a = a;
        return;
    }
}
```

### Exceptions

