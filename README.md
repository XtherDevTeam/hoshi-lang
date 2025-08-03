# The design of yoi-lang programming language



## Intro

I've made dynamic-typed language for serval years, now I'm trying to make something new. I'm going to make a static-typed, strong-typed programming language which supports modern programming design like `Generic Programming` `Object Oriented Programming`.

**Design Ideas: **Concise, Strong, Fast.

## Syntax

```rust
use io "std/io";

interface hello {
  say() : null
}

struct s_a {
  a: i32,
  b: deci,
  constructor(),
  constructor(a: int, b: deci),
}

impl s_a {
  constructor() {
    this.a = 0
    this.b = 0.0
  },
  constructor(a: int, b: deci) {
    this.a = a
    this.b = b
  }
}

impl hello for s_a {
	say() : null {
      io.println("fuck you!")
      return null
    }
}

func main(argv: Vec<rstr>) : int {
  let a = s_a(114514, 1919.810)
  let sayer = hello(a)
  sayer.say()
  io.println("Hello, world!")
  return 0;
}
```

Here is an example.



## OOP

我他妈直接快进

OOP为组合模式，傻逼继承，谁写继承模式谁傻逼。`yoi-lang` 的 OOP 由三个部分组成，`interface` 接口、`struct` 结构体、`impl` 实现接口。

`interface` 是抽象接口，包含一组方法和成员，可以被 `struct` 实现。`interface` 里只包含方法的声明，不包含实现，实现由 `struct` 完成。

`struct` 可以使用 `impl :` 结构实现接口。语法如 `impl [interface] : [struct] { implmentation }`。

## Ref

引用可以理解为与原来对象具有相同内存地址的对象。使用 `&object` 来获取一个对象的引用。函数传递参数时可以使用引用来传递，E.g. `func a(arg: s_a&)` 这行声明的参数 `arg` 传递 `s_a` 的引用，而不会复制构造一个新的对象。

## Objects

在此处，**所有数据类型**均为对象，除了 `integerRaw` 等原始类型在 push 时被转换为 object，在 `integerObject` 参与运算时解引用获得 `raw` 类型之外，在 LLVM IR 生成过程中栈上的**均为指向对应空间的指针类型**。
在 push 系列命令中，所有字面值将全部转换为对应的 integerObject、booleanObject、decimalObject。在 llvmCodegen 中可创建对应的 helper function 用于生成指定字面值的对象，在 llvmCodegen 初始化时完成基础类型对应 object 的 llvm struct 和 gc_x 函数创建工作。

**所有**对象（基础数据类型如 `int` `deci` `bool` `char`、接口、结构体）在创建过程中，或作为右值被传递时，均会更新引用计数（减小原对象（若存在）引用计数，增加新对象引用计数）。
同理，在销毁过程中，作为右值参与运算结束，会减小原对象引用计数。
当引用计数变为 0 时，自动销毁。

基础数据类型对象创建时调用 `basic_[data type name]_gc_refcount_increase` 进行增加引用计数调用 `basic_[data type name]_gc_refcount_decrease` 减小引用计数。
structObjecct 对象创建时调用 `struct_[moduleIndex]_[structIndex]_gc_refcount_increase` 进行增加引用计数调用 `struct_[moduleIndex]_[structIndex]_gc_refcount_decrease` 减小引用计数。
interfaceObject 对象创建时调用 `interface_gc_refcount_increase` 进行增加引用计数调用 `interface_gc_refcount_decrease` 减小引用计数，其通过对应该函数执行 this 指针上对应的虚函数完成相应逻辑。

所有对象均为 llvmCodegen 过程中的 struct 类型，对于基础类型，可在初始化 llvmCodegen 上下文过程中完成类型的创建和对应 `gc` 系列函数的定义。
所有对象在创建时，均调用签名为 `void* gc_object_alloc(unsigned long long sizeOfObject)` 的运行时函数完成内存开辟。

对于 `gc_refcount_increase` / `gc_refcount_decrease` 系列函数，其签名应为 `void gc_function_name(objectType* ptr);`，

执行流程如下：

1. 接受有且仅有一个 this 指针参数
2. 操作位于对象头部的 `gc_refcount` 递增或递减
3. 若引用计数小于或等于0，则调用提前声明的 `runtime_finalize_object(void* objectPtr)` 进行销毁

对于 structObject， `gc_refcount_increase` / `gc_refcount_decrease` 系列函数应在 llvmCodegen 创建数据类型时一并创建，并实现对应功能，在 llvmCodegen 实现可创建对应 helper function 模块化该过程。

## GC

GC的实现有点清奇，基本为引用计数模式，当对象创建时，会更新计数，引用被创建时也会更新计数，当对象或引用离开当前作用域时，就会减少计数。当最后一次减少计数操作发生时，便会调用 `runtime` 提供的函数销毁对象。

更新计数和减少计数可以通过在对象生成一个名为 `gc_refcount_increase` 和 `gc_refcount_decrease` 的方法来完成，更新计数时调用 `gc_refcount_increase` 来完成，减少计数时调用 `gc_refcount_decrease` 来完成。这样可以确保更新到对象里面的对象指针。

因为结构体无法创建一个没有声明的结构体实例，所以不需要担心循环引用的问题。

对于接口的GC，在 `Interface.md` 中讲到，我们的 `interfaceObject` 在实例化后为存储`this`指针和虚函数集合的结构体，同时 `interfaceImpl` 在 llvmCodegen 时应该将对应的 GC 函数生成为 `interfaceImpl#interfaceModuleIdx#interfaceIdx#structModuleIdx#structIdx#gc_refcount_increase` 和 `interfaceImpl#interfaceModuleIdx#interfaceIdx#structModuleIdx#structIdx#gc_refcount_decrease` 的虚函数，用于增加和减少指向 `struct` 的 this 指针的引用计数。

一个接口任意 `impl` 的通用 `interfaceObject` 对象的 `gc_refcount_increase` 和 `gc_refcount_decrease` 将会调用如上所述的两个虚函数，进行对应引用计数操作。

接口在 llvmCodegen 中的 struct 结构如下

| offset | data |
| --- | --- |
| 0~7 byte | gc_refcount `unsigned long long` |
| 8~15 byte | `this` pointer to the struct |
| 16~23 byte | virtual method pointer to `interfaceImpl#interfaceModuleIdx#interfaceIdx#structModuleIdx#structIdx#gc_refcount_increase` |
| 24~31 byte | virtual method pointer to `interfaceImpl#interfaceModuleIdx#interfaceIdx#structModuleIdx#structIdx#gc_refcount_decrease` |
| 31 byte ~ ... | virtual method pointers to methods defined in `interfaceObject` |

## Module

`yoi-lang` 提供了将项目模块化的功能，使用 `use` 语句即可导入模块。

如 `use io "std/io"` 就是从 `hoshiModulesPath` 中寻找 `std/io` 这个模块并使用 `io` 这个名字导入。

一个模块可以是一个 `yoi` 源文件，也可以是一个包含一组模块的目录。

在引入目录形式的模块时会导入目录下的所有 `yoi` 源文件。

当检测到重复模块加载时，会使用已经处理完成的 `AST` 挂载到 `use` 语句，`yoi-lang` 编译器内部会维护一个序号 `hoshiModuleId` 在加载模组时会使用当前序号当作当前导入模组的ID，将其十六进制化后添加到符号名头部。主包不作处理。

`use` 语句会先使用指定的 `prefix` 编译指定的模块，然后在符号表加入模块的别名。

## Generic

### Template arguments

`Generic Programming` 将会是 `yoi-lang` 的一大重要特性，`yoi-lang` 主要使用 `template` 来实现 `GP`，`template` 即模板，在类或函数声明时的标识符后加上 `<>` 符号来声明一个模板类或函数的模板参数。

E.g.

```rust
interface hello {
  say() : null
};
struct s_a<T> {
  i: T,
  fuck<T1>(a: T1) : null,
};
impl s_a {
  fuck<T1>(a: T1) : null {
    io.print(a);
    return i;
  }
};
impl hello for s_a {
  say() : null {
    io.println("114514");
    return;
  }
}
```



## Variadic arguments

`Variadic arguments (变长参数)` 是 `GP` 的一个附带特性，在模板函数中，`vaArgs` 是变长参数的模板类型，可放于函数参数尾部接收变长参数。

E.g.

```rust
func a<fT>(f: fT, args: vaArgs) {
  return fT(args);
}
```



上例是使用变长参数 `args` 调用函数 `f` 的例子。

此外，当使用变长参数去调用只含有单参数的函数时，会自动展开为多次函数调用；当变长参数对象被放置在调用参数尾部进行调用时，会自动添加这些参数到调用参数尾部。



## Syntax definition

```java
basicLiterals ::= TOK_string | TOK_integer | TOK_decimal | TOK_boolean | TOK_char | "null"
identifier ::= TOK_identifier
identifierWithTypeSpec ::= identifier ":" typeSpec
defTemplateArgSpec ::= identifier
					 | identifier "impl" externModuleAccessExpression # TODO
defTemplateArg ::= "<" [ { defTemplateArgSpec "," } defTemplateArgSpec ] ">"
templateArgSpec ::= typeSpec
templateArg ::= "<" [ { templateArgSpec "," } templateArgSpec ] ">"
invocationArguments ::= "(" [ { rExpr "," } rExpr ] ")"
definitionArguments ::= "(" [ { identifierWithTypeSpec "," } identifierWithTypeSpec ] ")"
funcTypeSpecArgs ::= "(" [ { typeSpec "," } typeSpec ] ")"
funcTypeSpec ::= "func" definitionArguments ":" typeSpec
typeSpec ::= externModuleAccessExpression
           | funcTypeSpec
           | "null"
subscript ::= "[" rExpr "]"
identifierWithTemplateArg ::= identifier
                            | identifier TemplateArg
identifierWithDefTemplateArg ::= identifier
                               | identifier defTemplateArg
externModuleAccessExpression ::= identifier { "." identifierWithTemplateArg }
lambdaDefinition ::= "(" [ memberExpression "as" identifier ] ")" "=>" definitionArguments codeBlock  
subscriptTypeSpec ::= externModuleAccessExpression { "[" TOK_integer "]" }
subscriptExpression ::= identifierWithTemplateArg
                      | identifierWithTemplateArg invocationArguments
                      | identifierWithTemplateArg subscript
memberExpression ::= subscriptExpression { "." subscriptExpression }
primary ::= memberExpression | basicLiterals | "(" rExpr ")"
uniqueExpr ::= ( "++" | "--" | "!" | "~" | "-" | "&" ) primary
             | primary
leftExpr ::= uniqueExpr { ( "=" | "+=" | "-=" | "*=" | "/=" | "%=" ) uniqueExpr }
mulExpr ::= uniqueExpr { ( "*" | "/" | "%" ) uniqueExpr }
addExpr ::= mulExpr { ( "+" | "-" ) mulExpr }
shiftExpr ::= addExpr { ( "<<" | ">>" ) addExpr }
relationalExpr ::= shiftExpr { ( "<" | ">" | "<=" | ">=" ) shiftExpr }
equalityExpr ::= relationalExpr { ( "==" | "!=" ) relationalExpr }
andExpr ::= equalityExpr { "&" equalityExpr }
exclusiveExpr ::= andExpr { "^" andExpr }
inclusiveExpr ::= exclusiveExpr { "|" exclusiveExpr }
logicalAndExpr ::= inclusiveExpr { "&&" inclusiveExpr }
logicalOrExpr ::= logicalAndExpr { "||" logicalAndExpr }
rExpr ::= logicalOrExpr
useStmt ::= "use" identifier TOK_string
funcDefStmt ::= "func" identifierWithDefTemplateArg definitionArguments ":" typeSpec codeBlock
innerMethodDecl ::= identifierWithDefTemplateArg definitionArguments ":" typeSpec
innerMethodDef ::= identifierWithDefTemplateArg definitionArguments ":" typeSpec codeBlock
constructorDecl ::= "constructor" definitionArguments
constructorDef ::= "constructor" definitionArguments codeBlock
interfaceDefInnerPair ::= identifierWithTypeSpec
                        | innerMethodDecl
interfaceDefInner ::= "{" [ interfaceDefInnerPair { "," interfaceDefInnerPair } ] "}"
interfaceDefStmt ::= "interface" identifier interfaceDefInner
structDefInnerPair ::= identifierWithTypeSpec
                     | innerMethodDecl
                     | constructDecl
structDefInner ::= "{" [ structDefInnerPair { "," structDefInnerPair } ] "}"
structDefStmt ::= "struct" identifierWithDefTemplateArg structDefInner
implInnerPair ::= identifierWithDefTemplateArg definitionArguments ":" typeSpec codeBlock
                | constructorDef
implInner ::= "{" [ implInnerPair { "," implInnerPair } ] "}"
implStmt ::= "impl" externModuleAccessExpression implInner
           | "impl" externModuleAccessExpression ":" identifierWithDefTemplateArg implInner
letAssignmentPair ::= identifier "=" rExpr
letStmt ::= "let" letAssignmentPair { "," letAssignmentPair }
globalStmt ::= useStmt | interfaceDefStmt | structDefStmt | implStmt | letStmt
hoshiModule ::= { globalStmt }
ifStmt ::= "if" "(" rExpr ")" codeBlock [ "elif" codeBlock ] [ "else" codeBlock ]
whileStmt ::= "while" "(" rExpr ")" codeBlock
forStmt ::= "for" "(" inCodeBlockStmt ";" rExpr ";" inCodeBlockStmt ")" codeBlock
forEachStmt ::= "forEach" "(" identifier ":" rExpr ")" codeBlock
returnStmt ::= "return" rExpr
continueStmt ::= "continue"
breakStmt ::= "break"
inCodeBlockStmt ::= ifStmt | whileStmt | forEachStmt | returnStmt | continueStmt | breakStmt | letStmt | codeBlock | rExpr
codeBlock ::= "{" { inCodeBlockStmt } "}"
exportDecl ::= "export" externModuleAccessExpression "as" identifier
importInner ::= innerMethodDecl
              | structDefStmt
importDecl ::= "import" importInner "from" TOK_string
```