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
    [ "Console I/O in Hoshi-lang", "md_docs_2Console.html", [
      [ "<tt>print</tt> Function", "md_docs_2Console.html#autotoc_md15", [
        [ "Overloads", "md_docs_2Console.html#autotoc_md16", null ],
        [ "Example", "md_docs_2Console.html#autotoc_md17", null ]
      ] ],
      [ "<tt>input</tt> Function", "md_docs_2Console.html#autotoc_md18", [
        [ "Overloads", "md_docs_2Console.html#autotoc_md19", null ],
        [ "Example", "md_docs_2Console.html#autotoc_md20", null ]
      ] ]
    ] ],
    [ "直接赋值 (Direct assignment)", "md_docs_2Direct_01Assignment.html", null ],
    [ "File System in Hoshi-lang", "md_docs_2File_01System.html", [
      [ "<tt>file</tt> Module", "md_docs_2File_01System.html#autotoc_md23", [
        [ "<tt>AbstractFile</tt> Interface", "md_docs_2File_01System.html#autotoc_md24", null ]
      ] ],
      [ "<tt>fs</tt> Module", "md_docs_2File_01System.html#autotoc_md25", [
        [ "Functions", "md_docs_2File_01System.html#autotoc_md26", null ],
        [ "<tt>OSFile</tt> Struct", "md_docs_2File_01System.html#autotoc_md27", null ]
      ] ],
      [ "<tt>io</tt> Module", "md_docs_2File_01System.html#autotoc_md28", [
        [ "<tt>ArrayBuffer</tt> Struct", "md_docs_2File_01System.html#autotoc_md29", null ],
        [ "Example", "md_docs_2File_01System.html#autotoc_md30", null ]
      ] ]
    ] ],
    [ "Finalizers in Hoshi-lang", "md_docs_2Finalizers.html", [
      [ "Syntax", "md_docs_2Finalizers.html#autotoc_md32", null ],
      [ "Execution", "md_docs_2Finalizers.html#autotoc_md33", null ]
    ] ],
    [ "HashMap in Hoshi-lang", "md_docs_2HashMap.html", [
      [ "<tt>HashMap<K, V></tt> Struct", "md_docs_2HashMap.html#autotoc_md35", [
        [ "Methods", "md_docs_2HashMap.html#autotoc_md36", null ],
        [ "Example", "md_docs_2HashMap.html#autotoc_md37", null ]
      ] ]
    ] ],
    [ "接口设计", "md_docs_2Interface.html", [
      [ "new 语句设计", "md_docs_2Interface.html#autotoc_md39", null ]
    ] ],
    [ "The Hoshi-lang Intermediate Representation (IR) Handbook", "md_docs_2IR.html", [
      [ "1. Introduction", "md_docs_2IR.html#autotoc_md41", [
        [ "1.1. Purpose", "md_docs_2IR.html#autotoc_md42", null ],
        [ "1.2. Execution Model", "md_docs_2IR.html#autotoc_md43", null ]
      ] ],
      [ "2. Core Concepts", "md_docs_2IR.html#autotoc_md44", [
        [ "2.1. <tt>IRModule</tt>", "md_docs_2IR.html#autotoc_md45", null ],
        [ "2.2. <tt>IRValueType</tt>", "md_docs_2IR.html#autotoc_md46", null ]
      ] ],
      [ "3. Instruction Set Reference", "md_docs_2IR.html#autotoc_md48", [
        [ "3.1. Stack and Memory Operations", "md_docs_2IR.html#autotoc_md49", null ],
        [ "3.2. Arithmetic and Logical Operations", "md_docs_2IR.html#autotoc_md50", null ],
        [ "3.3. Control Flow", "md_docs_2IR.html#autotoc_md51", null ],
        [ "3.4. Object and Array Lifecycle", "md_docs_2IR.html#autotoc_md52", null ],
        [ "3.5. Type Operations", "md_docs_2IR.html#autotoc_md53", null ],
        [ "3.6. Function and Method Calls", "md_docs_2IR.html#autotoc_md54", null ],
        [ "1.2. Execution Model", "md_docs_2IR.html#autotoc_md55", null ]
      ] ],
      [ "2. Core Concepts", "md_docs_2IR.html#autotoc_md56", [
        [ "2.1. <tt>IRModule</tt>", "md_docs_2IR.html#autotoc_md57", null ],
        [ "2.2. <tt>IRFunctionDefinition</tt>", "md_docs_2IR.html#autotoc_md58", null ],
        [ "2.3. <tt>IRCodeBlock</tt>", "md_docs_2IR.html#autotoc_md59", null ],
        [ "2.4. <tt>IRValueType</tt>", "md_docs_2IR.html#autotoc_md60", null ],
        [ "2.5. <tt>IROperand</tt>", "md_docs_2IR.html#autotoc_md61", null ]
      ] ],
      [ "3. Instruction Set Reference", "md_docs_2IR.html#autotoc_md63", [
        [ "3.1. Stack and Memory Operations", "md_docs_2IR.html#autotoc_md64", null ],
        [ "3.2. Arithmetic and Logical Operations", "md_docs_2IR.html#autotoc_md65", null ],
        [ "3.3. Control Flow", "md_docs_2IR.html#autotoc_md66", null ],
        [ "3.4. Object Lifecycle", "md_docs_2IR.html#autotoc_md67", null ],
        [ "3.5. Function and Method Calls", "md_docs_2IR.html#autotoc_md68", null ]
      ] ],
      [ "4. Full Example", "md_docs_2IR.html#autotoc_md70", null ]
    ] ],
    [ "JSON in Hoshi-lang", "md_docs_2JSON.html", [
      [ "<tt>parse</tt> Function", "md_docs_2JSON.html#autotoc_md72", null ],
      [ "<tt>JSONValue</tt> Interface", "md_docs_2JSON.html#autotoc_md73", null ],
      [ "Example", "md_docs_2JSON.html#autotoc_md74", null ]
    ] ],
    [ "Macros in Hoshi-lang", "md_docs_2Macros.html", [
      [ "Syntax", "md_docs_2Macros.html#autotoc_md76", null ],
      [ "Pre-defined Macros", "md_docs_2Macros.html#autotoc_md77", null ],
      [ "Custom Macros", "md_docs_2Macros.html#autotoc_md78", null ],
      [ "Example", "md_docs_2Macros.html#autotoc_md79", null ]
    ] ],
    [ "Math in Hoshi-lang", "md_docs_2Math.html", [
      [ "Functions", "md_docs_2Math.html#autotoc_md81", null ],
      [ "Example", "md_docs_2Math.html#autotoc_md82", null ]
    ] ],
    [ "The implementation of null literal in hoshi-lang", "md_docs_2Null.html", [
      [ "Pointer Object in hoshi-lang", "md_docs_2Null.html#autotoc_md84", null ]
    ] ],
    [ "Nullable Check", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html", [
      [ "Raw Check", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md86", null ],
      [ "Inter-functional call graph building and raw check", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md87", null ],
      [ "IRValueType 属性的增加和改写", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md88", null ]
    ] ],
    [ "Operator Overloading", "md_docs_2Operator_01Overloading.html", [
      [ "Overloadable Operators", "md_docs_2Operator_01Overloading.html#autotoc_md90", null ],
      [ "Defining Operator Overloads", "md_docs_2Operator_01Overloading.html#autotoc_md91", [
        [ "Binary Operators", "md_docs_2Operator_01Overloading.html#autotoc_md92", null ],
        [ "Unary Operators", "md_docs_2Operator_01Overloading.html#autotoc_md93", null ],
        [ "Callable Objects (operator())", "md_docs_2Operator_01Overloading.html#autotoc_md94", null ],
        [ "Subscript Operator (operator[])", "md_docs_2Operator_01Overloading.html#autotoc_md95", null ]
      ] ]
    ] ],
    [ "Result Type in Hoshi-lang", "md_docs_2Result.html", [
      [ "<tt>Result<T, E></tt> Struct", "md_docs_2Result.html#autotoc_md97", [
        [ "Methods", "md_docs_2Result.html#autotoc_md98", null ],
        [ "Static Methods", "md_docs_2Result.html#autotoc_md99", null ],
        [ "Example", "md_docs_2Result.html#autotoc_md100", null ]
      ] ]
    ] ],
    [ "Runtime in Hoshi-lang", "md_docs_2Runtime.html", [
      [ "Functions", "md_docs_2Runtime.html#autotoc_md102", null ]
    ] ],
    [ "Hoshi-lang Language Specification", "md_docs_2Spec.html", [
      [ "1. Lexical Structure", "md_docs_2Spec.html#autotoc_md104", [
        [ "1.1. Identifiers", "md_docs_2Spec.html#autotoc_md105", null ],
        [ "1.2. Keywords", "md_docs_2Spec.html#autotoc_md106", null ],
        [ "1.3. Literals", "md_docs_2Spec.html#autotoc_md107", null ]
      ] ],
      [ "2. Types", "md_docs_2Spec.html#autotoc_md108", null ],
      [ "3. Structs", "md_docs_2Spec.html#autotoc_md109", null ],
      [ "4. Interfaces", "md_docs_2Spec.html#autotoc_md110", null ],
      [ "5. Functions", "md_docs_2Spec.html#autotoc_md111", null ],
      [ "6. Operator Overloading", "md_docs_2Spec.html#autotoc_md112", null ],
      [ "7. Interface Templates", "md_docs_2Spec.html#autotoc_md113", null ],
      [ "8. Standard Library", "md_docs_2Spec.html#autotoc_md114", null ]
    ] ],
    [ "String in Hoshi-lang", "md_docs_2String.html", [
      [ "<tt>Str</tt> Struct", "md_docs_2String.html#autotoc_md116", [
        [ "Methods", "md_docs_2String.html#autotoc_md117", null ],
        [ "Static Methods", "md_docs_2String.html#autotoc_md118", null ]
      ] ],
      [ "<tt>Stringable</tt> Interface", "md_docs_2String.html#autotoc_md119", null ],
      [ "<tt>format</tt> Function", "md_docs_2String.html#autotoc_md120", null ]
    ] ],
    [ "Structured Bindings in Hoshi-lang", "md_docs_2Structured_01Bindings.html", [
      [ "Array Destructuring", "md_docs_2Structured_01Bindings.html#autotoc_md122", null ],
      [ "Struct Destructuring", "md_docs_2Structured_01Bindings.html#autotoc_md123", null ]
    ] ],
    [ "Generic Programming with Templates", "md_docs_2Template.html", [
      [ "Function Templates", "md_docs_2Template.html#autotoc_md125", [
        [ "Implicit vs. Explicit Specialization", "md_docs_2Template.html#autotoc_md126", null ]
      ] ],
      [ "Struct Templates", "md_docs_2Template.html#autotoc_md127", null ],
      [ "Interface Templates", "md_docs_2Template.html#autotoc_md128", null ]
    ] ],
    [ "The Optimization Strategy of Interface Allocation and Virtual Invocation Reduction 接口分配和虚函数调用消除优化策略", "md_docs_2The_01Optimization_01Strategy_01of_01Interface_01Allocation_01and_01Virtual_01Invocation_01Reduction.html", null ],
    [ "Threading in Hoshi-lang", "md_docs_2Threading.html", [
      [ "<tt>Thread</tt> Struct", "md_docs_2Threading.html#autotoc_md131", [
        [ "Methods", "md_docs_2Threading.html#autotoc_md132", null ],
        [ "Example", "md_docs_2Threading.html#autotoc_md133", null ]
      ] ],
      [ "<tt>Mutex</tt> Struct", "md_docs_2Threading.html#autotoc_md134", [
        [ "Methods", "md_docs_2Threading.html#autotoc_md135", null ],
        [ "Example", "md_docs_2Threading.html#autotoc_md136", null ]
      ] ],
      [ "<tt>current_tid()</tt>", "md_docs_2Threading.html#autotoc_md137", null ]
    ] ],
    [ "Type Aliases in Hoshi-lang", "md_docs_2Type_01Aliases.html", [
      [ "Syntax", "md_docs_2Type_01Aliases.html#autotoc_md139", null ],
      [ "Example", "md_docs_2Type_01Aliases.html#autotoc_md140", null ]
    ] ],
    [ "Vector in Hoshi-lang", "md_docs_2Vector.html", [
      [ "<tt>Vec<T></tt> Struct", "md_docs_2Vector.html#autotoc_md142", [
        [ "Methods", "md_docs_2Vector.html#autotoc_md143", null ],
        [ "Example", "md_docs_2Vector.html#autotoc_md144", null ]
      ] ]
    ] ],
    [ "Export Wrapper - 实现 FFI 的必经之路", "md_docs_2Wrapper.html", null ],
    [ "Referenced third party codes", "md_THIRDPARTY.html", null ],
    [ "TODO List", "md_TODO.html", [
      [ "Known issues", "md_TODO.html#autotoc_md168", null ]
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
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"IRLinker_8cpp.html",
"classyoi_1_1AST.html#a357495ad29c8d099eaa2962fcc73210e",
"classyoi_1_1IR.html#a2112dbdb049f53df83b3ba6fcbf13dc7ad8d049e16ed571a7074c9b6e20938907",
"classyoi_1_1IRExternEntry.html#a5f8ceb788ee36d45815bf2a6a499eed7ac1c425268e68385d1ab5074c17a94f14",
"classyoi_1_1IRInterfaceInstanceDefinition.html#a7ba1180139ce7d7b69bc45f87eeb5621",
"classyoi_1_1IRValueType.html#a0a8161a9919991b3b5d4199f96536c29",
"classyoi_1_1LLVMCodegen.html#a7ffc2d88378f6a1270662c1be20f7db0",
"classyoi_1_1compilerContext.html#aadfa1ff9bc52b3e274d10c0c929ea7fa",
"classyoi_1_1globalStmt.html#ade670788993aac3544d4123d723d0af1",
"classyoi_1_1indexTable_1_1iterator.html#a605a99690a67b7400cce5943cb7868ab",
"classyoi_1_1logicalAndExpr.html#a9e0cf23a22b6af67b8b52a65f27ef7a2",
"classyoi_1_1templateArgSpec.html",
"classyoi_1_1visitor.html#ad8cc77d97249d99c80f9d5194dc58135",
"magic__enum_8h.html#a0e1cc9614698479a137e998ffc983447",
"memory_8cpp.html#aadd6dcb4d367e405a3c4d02e248c4220",
"namespaceyoi.html#a74b8801052181bccaf80f0c08e7f561e",
"rtti_8h.html",
"structyoi_1_1IRBuildConfig.html#ac349892b20a306b6c2a89db26d24c010a334c4a4c42fdb79d7ebc3e73b517e6f8",
"structyoi_1_1IRTemplateBuilder_1_1Argument.html#a60960128d6c4567b98053924e484bc59",
"structyoi_1_1lexer_1_1token.html#a813647b1bf8cc722a45770b5bee77f32ada78de4127377591c515888bad472367"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';