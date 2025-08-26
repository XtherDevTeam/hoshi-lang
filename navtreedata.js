/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "hoshi-lang", "index.html", [
    [ "The Hoshi-lang Programming Language", "index.html", "index" ],
    [ "Hoshi-lang Arrays", "md_docs_2Array.html", [
      [ "1. Fixed-Size Arrays", "md_docs_2Array.html#autotoc_md1", [
        [ "Declaration and Initialization", "md_docs_2Array.html#autotoc_md2", null ],
        [ "Element Access", "md_docs_2Array.html#autotoc_md3", null ]
      ] ],
      [ "2. Dynamic Arrays", "md_docs_2Array.html#autotoc_md4", [
        [ "Declaration and Initialization", "md_docs_2Array.html#autotoc_md5", null ]
      ] ],
      [ "3. The <tt>.length</tt> Property", "md_docs_2Array.html#autotoc_md6", null ]
    ] ],
    [ "Callable Objects & Lambda Expressions", "md_docs_2Callable_01_6_01Lambda.html", [
      [ "1. Callable Objects and <tt>func</tt> type", "md_docs_2Callable_01_6_01Lambda.html#autotoc_md8", [
        [ "<tt>func</tt> Type Syntax", "md_docs_2Callable_01_6_01Lambda.html#autotoc_md9", null ]
      ] ],
      [ "2. Lambda Expressions", "md_docs_2Callable_01_6_01Lambda.html#autotoc_md10", [
        [ "Syntax", "md_docs_2Callable_01_6_01Lambda.html#autotoc_md11", null ],
        [ "Example", "md_docs_2Callable_01_6_01Lambda.html#autotoc_md12", null ]
      ] ],
      [ "3. Under the Hood: Implementation Details", "md_docs_2Callable_01_6_01Lambda.html#autotoc_md13", null ]
    ] ],
    [ "直接赋值 (Direct assignment)", "md_docs_2Direct_01Assignment.html", null ],
    [ "接口设计", "md_docs_2Interface.html", [
      [ "new 语句设计", "md_docs_2Interface.html#autotoc_md16", null ]
    ] ],
    [ "The Hoshi-lang Intermediate Representation (IR) Handbook", "md_docs_2IR.html", [
      [ "1. Introduction", "md_docs_2IR.html#autotoc_md18", [
        [ "1.1. Purpose", "md_docs_2IR.html#autotoc_md19", null ],
        [ "1.2. Execution Model", "md_docs_2IR.html#autotoc_md20", null ]
      ] ],
      [ "2. Core Concepts", "md_docs_2IR.html#autotoc_md21", [
        [ "2.1. <tt>IRModule</tt>", "md_docs_2IR.html#autotoc_md22", null ],
        [ "2.2. <tt>IRValueType</tt>", "md_docs_2IR.html#autotoc_md23", null ]
      ] ],
      [ "3. Instruction Set Reference", "md_docs_2IR.html#autotoc_md25", [
        [ "3.1. Stack and Memory Operations", "md_docs_2IR.html#autotoc_md26", null ],
        [ "3.2. Arithmetic and Logical Operations", "md_docs_2IR.html#autotoc_md27", null ],
        [ "3.3. Control Flow", "md_docs_2IR.html#autotoc_md28", null ],
        [ "3.4. Object and Array Lifecycle", "md_docs_2IR.html#autotoc_md29", null ],
        [ "3.5. Type Operations", "md_docs_2IR.html#autotoc_md30", null ],
        [ "3.6. Function and Method Calls", "md_docs_2IR.html#autotoc_md31", null ],
        [ "1.2. Execution Model", "md_docs_2IR.html#autotoc_md32", null ]
      ] ],
      [ "2. Core Concepts", "md_docs_2IR.html#autotoc_md33", [
        [ "2.1. <tt>IRModule</tt>", "md_docs_2IR.html#autotoc_md34", null ],
        [ "2.2. <tt>IRFunctionDefinition</tt>", "md_docs_2IR.html#autotoc_md35", null ],
        [ "2.3. <tt>IRCodeBlock</tt>", "md_docs_2IR.html#autotoc_md36", null ],
        [ "2.4. <tt>IRValueType</tt>", "md_docs_2IR.html#autotoc_md37", null ],
        [ "2.5. <tt>IROperand</tt>", "md_docs_2IR.html#autotoc_md38", null ]
      ] ],
      [ "3. Instruction Set Reference", "md_docs_2IR.html#autotoc_md40", [
        [ "3.1. Stack and Memory Operations", "md_docs_2IR.html#autotoc_md41", null ],
        [ "3.2. Arithmetic and Logical Operations", "md_docs_2IR.html#autotoc_md42", null ],
        [ "3.3. Control Flow", "md_docs_2IR.html#autotoc_md43", null ],
        [ "3.4. Object Lifecycle", "md_docs_2IR.html#autotoc_md44", null ],
        [ "3.5. Function and Method Calls", "md_docs_2IR.html#autotoc_md45", null ]
      ] ],
      [ "4. Full Example", "md_docs_2IR.html#autotoc_md47", null ]
    ] ],
    [ "The implementation of null literal in hoshi-lang", "md_docs_2Null.html", [
      [ "Pointer Object in hoshi-lang", "md_docs_2Null.html#autotoc_md49", null ]
    ] ],
    [ "Nullable Check", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html", [
      [ "Raw Check", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md51", null ],
      [ "IRValueType 属性的增加和改写", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md52", null ]
    ] ],
    [ "Operator Overloading", "md_docs_2Operator_01Overloading.html", [
      [ "Overloadable Operators", "md_docs_2Operator_01Overloading.html#autotoc_md54", null ],
      [ "Defining Operator Overloads", "md_docs_2Operator_01Overloading.html#autotoc_md55", [
        [ "Binary Operators", "md_docs_2Operator_01Overloading.html#autotoc_md56", null ],
        [ "Unary Operators", "md_docs_2Operator_01Overloading.html#autotoc_md57", null ],
        [ "Callable Objects (operator())", "md_docs_2Operator_01Overloading.html#autotoc_md58", null ],
        [ "Subscript Operator (operator[])", "md_docs_2Operator_01Overloading.html#autotoc_md59", null ]
      ] ]
    ] ],
    [ "Hoshi-lang Language Specification", "md_docs_2Spec.html", [
      [ "1. Lexical Structure", "md_docs_2Spec.html#autotoc_md61", [
        [ "1.1. Identifiers", "md_docs_2Spec.html#autotoc_md62", null ],
        [ "1.2. Keywords", "md_docs_2Spec.html#autotoc_md63", null ],
        [ "1.3. Literals", "md_docs_2Spec.html#autotoc_md64", null ]
      ] ],
      [ "2. Types", "md_docs_2Spec.html#autotoc_md65", null ],
      [ "3. Structs", "md_docs_2Spec.html#autotoc_md66", null ],
      [ "4. Interfaces", "md_docs_2Spec.html#autotoc_md67", null ],
      [ "5. Functions", "md_docs_2Spec.html#autotoc_md68", null ],
      [ "6. Operator Overloading", "md_docs_2Spec.html#autotoc_md69", null ],
      [ "7. Interface Templates", "md_docs_2Spec.html#autotoc_md70", null ],
      [ "8. Standard Library", "md_docs_2Spec.html#autotoc_md71", null ]
    ] ],
    [ "Standard Library", "md_docs_2Standard_01Library.html", [
      [ "String (<tt>str</tt>)", "md_docs_2Standard_01Library.html#autotoc_md73", null ],
      [ "Vector (<tt>vec</tt>)", "md_docs_2Standard_01Library.html#autotoc_md74", null ]
    ] ],
    [ "Generic Programming with Templates", "md_docs_2Template.html", [
      [ "Function Templates", "md_docs_2Template.html#autotoc_md76", [
        [ "Implicit vs. Explicit Specialization", "md_docs_2Template.html#autotoc_md77", null ]
      ] ],
      [ "Struct Templates", "md_docs_2Template.html#autotoc_md78", null ],
      [ "Interface Templates", "md_docs_2Template.html#autotoc_md79", null ]
    ] ],
    [ "Export Wrapper - 实现 FFI 的必经之路", "md_docs_2Wrapper.html", null ],
    [ "Referenced third party codes", "md_THIRDPARTY.html", null ],
    [ "TODO List", "md_TODO.html", [
      [ "Known issues", "md_TODO.html#autotoc_md108", null ]
    ] ],
    [ "Deprecated List", "deprecated.html", null ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", "namespacemembers_dup" ],
        [ "Functions", "namespacemembers_func.html", "namespacemembers_func" ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", "functions_vars" ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Related Symbols", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"IRLinker_8cpp.html",
"classyoi_1_1BuiltinModuleBuilder.html",
"classyoi_1_1IR.html#a2112dbdb049f53df83b3ba6fcbf13dc7af196fe5188bbef3e23f9fd76b7d2cc00",
"classyoi_1_1IRFunctionDefinition.html#ae11ef1d0fd94f0034aa77d1664e2006f",
"classyoi_1_1IROptimizer.html#a35f75104c0b80b9a58f28db3216ecacf",
"classyoi_1_1IRVariableTable.html#a61fa8c172e7397d729af1135ec6076a6",
"classyoi_1_1addExpr.html#a9e0cf23a22b6af67b8b52a65f27ef7a2",
"classyoi_1_1externModuleAccessExpression.html#a9fdb49e7d01ff76d7a8d12e70c231aec",
"classyoi_1_1indexPool.html#ab9096678e21c77ed4b5e90d5fbad5afe",
"classyoi_1_1lexer.html#a62f5792763fde6e35ac89ab4fe5dbefc",
"classyoi_1_1subscriptExpr.html#a017162956e7cfdb52aaf1691b1930177",
"classyoi_1_1visitor.html#a9b92ec0821a2a26446904de21da87b4c",
"lexer_8cpp_source.html",
"namespacemagic__enum.html#ac3a448ce59bc0f085fcf9458dcf26c8e",
"namespaceyoi.html#ad69cb9eff54c3948f3f5f264f6a2432c",
"structYoiObject.html",
"structyoi_1_1IROptimizer_1_1SimulationStack.html#a42b8b38b8b19aeddb778a8c3350fab22",
"structyoi_1_1lexer_1_1token.html#a813647b1bf8cc722a45770b5bee77f32aa363b8d38ef47ae6bfc2f84653dc1a3b",
"whereami_8cpp.html#a39b37162eaf1ae5b907b22c1d0eaeb14"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';