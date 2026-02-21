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
    [ "The hoshi-lang Programming Language", "index.html", "index" ],
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
    [ "The purposal of introducing data fields in legacy struct", "md_docs_2Data_01Fields_01in_01Legacy_01Struct.html", [
      [ "The syntax of data fields in legacy struct", "md_docs_2Data_01Fields_01in_01Legacy_01Struct.html#autotoc_md22", null ],
      [ "Underlying mechanisms", "md_docs_2Data_01Fields_01in_01Legacy_01Struct.html#autotoc_md23", null ]
    ] ],
    [ "Data struct in hoshi-lang", "md_docs_2Datastruct.html", [
      [ "Syntax", "md_docs_2Datastruct.html#autotoc_md25", null ],
      [ "Memory Layout", "md_docs_2Datastruct.html#autotoc_md26", null ],
      [ "Underlying mechanism", "md_docs_2Datastruct.html#autotoc_md27", null ],
      [ "The integration with existing object optimization system", "md_docs_2Datastruct.html#autotoc_md28", null ]
    ] ],
    [ "直接赋值 (Direct assignment)", "md_docs_2Direct_01Assignment.html", null ],
    [ "File System in Hoshi-lang", "md_docs_2File_01System.html", [
      [ "<tt>file</tt> Module", "md_docs_2File_01System.html#autotoc_md31", [
        [ "<tt>AbstractFile</tt> Interface", "md_docs_2File_01System.html#autotoc_md32", null ]
      ] ],
      [ "<tt>fs</tt> Module", "md_docs_2File_01System.html#autotoc_md33", [
        [ "Functions", "md_docs_2File_01System.html#autotoc_md34", null ],
        [ "<tt>OSFile</tt> Struct", "md_docs_2File_01System.html#autotoc_md35", null ]
      ] ],
      [ "<tt>io</tt> Module", "md_docs_2File_01System.html#autotoc_md36", [
        [ "<tt>ArrayBuffer</tt> Struct", "md_docs_2File_01System.html#autotoc_md37", null ],
        [ "Example", "md_docs_2File_01System.html#autotoc_md38", null ]
      ] ]
    ] ],
    [ "Finalizers in Hoshi-lang", "md_docs_2Finalizers.html", [
      [ "Syntax", "md_docs_2Finalizers.html#autotoc_md40", null ],
      [ "Execution", "md_docs_2Finalizers.html#autotoc_md41", null ]
    ] ],
    [ "HashMap in Hoshi-lang", "md_docs_2HashMap.html", [
      [ "<tt>HashMap<K, V></tt> Struct", "md_docs_2HashMap.html#autotoc_md43", [
        [ "Methods", "md_docs_2HashMap.html#autotoc_md44", null ],
        [ "Example", "md_docs_2HashMap.html#autotoc_md45", null ]
      ] ]
    ] ],
    [ "接口设计", "md_docs_2Interface.html", [
      [ "new 语句设计", "md_docs_2Interface.html#autotoc_md47", null ]
    ] ],
    [ "The Hoshi-lang Intermediate Representation (IR) Handbook", "md_docs_2IR.html", [
      [ "1. Introduction", "md_docs_2IR.html#autotoc_md49", [
        [ "1.1. Purpose", "md_docs_2IR.html#autotoc_md50", null ],
        [ "1.2. Execution Model", "md_docs_2IR.html#autotoc_md51", null ]
      ] ],
      [ "2. Core Concepts", "md_docs_2IR.html#autotoc_md52", [
        [ "2.1. <tt>IRModule</tt>", "md_docs_2IR.html#autotoc_md53", null ],
        [ "2.2. <tt>IRValueType</tt>", "md_docs_2IR.html#autotoc_md54", null ]
      ] ],
      [ "3. Instruction Set Reference", "md_docs_2IR.html#autotoc_md56", [
        [ "3.1. Stack and Memory Operations", "md_docs_2IR.html#autotoc_md57", null ],
        [ "3.2. Arithmetic and Logical Operations", "md_docs_2IR.html#autotoc_md58", null ],
        [ "3.3. Control Flow", "md_docs_2IR.html#autotoc_md59", null ],
        [ "3.4. Object and Array Lifecycle", "md_docs_2IR.html#autotoc_md60", null ],
        [ "3.5. Type Operations", "md_docs_2IR.html#autotoc_md61", null ],
        [ "3.6. Function and Method Calls", "md_docs_2IR.html#autotoc_md62", null ],
        [ "1.2. Execution Model", "md_docs_2IR.html#autotoc_md63", null ]
      ] ],
      [ "2. Core Concepts", "md_docs_2IR.html#autotoc_md64", [
        [ "2.1. <tt>IRModule</tt>", "md_docs_2IR.html#autotoc_md65", null ],
        [ "2.2. <tt>IRFunctionDefinition</tt>", "md_docs_2IR.html#autotoc_md66", null ],
        [ "2.3. <tt>IRCodeBlock</tt>", "md_docs_2IR.html#autotoc_md67", null ],
        [ "2.4. <tt>IRValueType</tt>", "md_docs_2IR.html#autotoc_md68", null ],
        [ "2.5. <tt>IROperand</tt>", "md_docs_2IR.html#autotoc_md69", null ]
      ] ],
      [ "3. Instruction Set Reference", "md_docs_2IR.html#autotoc_md71", [
        [ "3.1. Stack and Memory Operations", "md_docs_2IR.html#autotoc_md72", null ],
        [ "3.2. Arithmetic and Logical Operations", "md_docs_2IR.html#autotoc_md73", null ],
        [ "3.3. Control Flow", "md_docs_2IR.html#autotoc_md74", null ],
        [ "3.4. Object Lifecycle", "md_docs_2IR.html#autotoc_md75", null ],
        [ "3.5. Function and Method Calls", "md_docs_2IR.html#autotoc_md76", null ]
      ] ],
      [ "4. Full Example", "md_docs_2IR.html#autotoc_md78", null ]
    ] ],
    [ "JSON in Hoshi-lang", "md_docs_2JSON.html", [
      [ "<tt>parse</tt> Function", "md_docs_2JSON.html#autotoc_md80", null ],
      [ "<tt>JSONValue</tt> Interface", "md_docs_2JSON.html#autotoc_md81", null ],
      [ "Example", "md_docs_2JSON.html#autotoc_md82", null ]
    ] ],
    [ "Macros in Hoshi-lang", "md_docs_2Macros.html", [
      [ "Syntax", "md_docs_2Macros.html#autotoc_md84", null ],
      [ "Pre-defined Macros", "md_docs_2Macros.html#autotoc_md85", null ],
      [ "Custom Macros", "md_docs_2Macros.html#autotoc_md86", null ],
      [ "Example", "md_docs_2Macros.html#autotoc_md87", null ]
    ] ],
    [ "Math in Hoshi-lang", "md_docs_2Math.html", [
      [ "Functions", "md_docs_2Math.html#autotoc_md89", null ],
      [ "Example", "md_docs_2Math.html#autotoc_md90", null ]
    ] ],
    [ "The implementation of null literal in hoshi-lang", "md_docs_2Null.html", [
      [ "Pointer Object in hoshi-lang", "md_docs_2Null.html#autotoc_md92", null ]
    ] ],
    [ "Nullable Check", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html", [
      [ "Raw Check", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md94", null ],
      [ "Inter-functional call graph building and raw check", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md95", null ],
      [ "IRValueType 属性的增加和改写", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md96", null ]
    ] ],
    [ "Operator Overloading", "md_docs_2Operator_01Overloading.html", [
      [ "Overloadable Operators", "md_docs_2Operator_01Overloading.html#autotoc_md98", null ],
      [ "Defining Operator Overloads", "md_docs_2Operator_01Overloading.html#autotoc_md99", [
        [ "Binary Operators", "md_docs_2Operator_01Overloading.html#autotoc_md100", null ],
        [ "Unary Operators", "md_docs_2Operator_01Overloading.html#autotoc_md101", null ],
        [ "Callable Objects (operator())", "md_docs_2Operator_01Overloading.html#autotoc_md102", null ],
        [ "Subscript Operator (operator[])", "md_docs_2Operator_01Overloading.html#autotoc_md103", null ]
      ] ]
    ] ],
    [ "Result Type in Hoshi-lang", "md_docs_2Result.html", [
      [ "<tt>Result<T, E></tt> Struct", "md_docs_2Result.html#autotoc_md105", [
        [ "Methods", "md_docs_2Result.html#autotoc_md106", null ],
        [ "Static Methods", "md_docs_2Result.html#autotoc_md107", null ],
        [ "Example", "md_docs_2Result.html#autotoc_md108", null ]
      ] ]
    ] ],
    [ "Runtime in Hoshi-lang", "md_docs_2Runtime.html", [
      [ "Functions", "md_docs_2Runtime.html#autotoc_md110", null ]
    ] ],
    [ "Hoshi-lang Language Specification", "md_docs_2Spec.html", [
      [ "1. Lexical Structure", "md_docs_2Spec.html#autotoc_md112", [
        [ "1.1. Identifiers", "md_docs_2Spec.html#autotoc_md113", null ],
        [ "1.2. Keywords", "md_docs_2Spec.html#autotoc_md114", null ],
        [ "1.3. Literals", "md_docs_2Spec.html#autotoc_md115", null ]
      ] ],
      [ "2. Types", "md_docs_2Spec.html#autotoc_md116", null ],
      [ "3. Structs", "md_docs_2Spec.html#autotoc_md117", null ],
      [ "4. Interfaces", "md_docs_2Spec.html#autotoc_md118", null ],
      [ "5. Functions", "md_docs_2Spec.html#autotoc_md119", null ],
      [ "6. Operator Overloading", "md_docs_2Spec.html#autotoc_md120", null ],
      [ "7. Interface Templates", "md_docs_2Spec.html#autotoc_md121", null ],
      [ "8. Standard Library", "md_docs_2Spec.html#autotoc_md122", null ]
    ] ],
    [ "String in Hoshi-lang", "md_docs_2String.html", [
      [ "<tt>Str</tt> Struct", "md_docs_2String.html#autotoc_md124", [
        [ "Methods", "md_docs_2String.html#autotoc_md125", null ],
        [ "Static Methods", "md_docs_2String.html#autotoc_md126", null ]
      ] ],
      [ "<tt>Stringable</tt> Interface", "md_docs_2String.html#autotoc_md127", null ],
      [ "<tt>format</tt> Function", "md_docs_2String.html#autotoc_md128", null ]
    ] ],
    [ "Structured Bindings in Hoshi-lang", "md_docs_2Structured_01Bindings.html", [
      [ "Array Destructuring", "md_docs_2Structured_01Bindings.html#autotoc_md130", null ],
      [ "Struct Destructuring", "md_docs_2Structured_01Bindings.html#autotoc_md131", null ]
    ] ],
    [ "Generic Programming with Templates", "md_docs_2Template.html", [
      [ "Function Templates", "md_docs_2Template.html#autotoc_md133", [
        [ "Implicit vs. Explicit Specialization", "md_docs_2Template.html#autotoc_md134", null ]
      ] ],
      [ "Struct Templates", "md_docs_2Template.html#autotoc_md135", null ],
      [ "Interface Templates", "md_docs_2Template.html#autotoc_md136", null ]
    ] ],
    [ "The Optimization Strategy of Interface Allocation and Virtual Invocation Reduction 接口分配和虚函数调用消除优化策略", "md_docs_2The_01Optimization_01Strategy_01of_01Interface_01Allocation_01and_01Virtual_01Invocation_01Reduction.html", null ],
    [ "Threading in Hoshi-lang", "md_docs_2Threading.html", [
      [ "<tt>Thread</tt> Struct", "md_docs_2Threading.html#autotoc_md139", [
        [ "Methods", "md_docs_2Threading.html#autotoc_md140", null ],
        [ "Example", "md_docs_2Threading.html#autotoc_md141", null ]
      ] ],
      [ "<tt>Mutex</tt> Struct", "md_docs_2Threading.html#autotoc_md142", [
        [ "Methods", "md_docs_2Threading.html#autotoc_md143", null ],
        [ "Example", "md_docs_2Threading.html#autotoc_md144", null ]
      ] ],
      [ "<tt>current_tid()</tt>", "md_docs_2Threading.html#autotoc_md145", null ]
    ] ],
    [ "Type Aliases in Hoshi-lang", "md_docs_2Type_01Aliases.html", [
      [ "Syntax", "md_docs_2Type_01Aliases.html#autotoc_md147", null ],
      [ "Example", "md_docs_2Type_01Aliases.html#autotoc_md148", null ]
    ] ],
    [ "Vector in Hoshi-lang", "md_docs_2Vector.html", [
      [ "<tt>Vec<T></tt> Struct", "md_docs_2Vector.html#autotoc_md150", [
        [ "Methods", "md_docs_2Vector.html#autotoc_md151", null ],
        [ "Example", "md_docs_2Vector.html#autotoc_md152", null ]
      ] ]
    ] ],
    [ "Export Wrapper - 实现 FFI 的必经之路", "md_docs_2Wrapper.html", null ],
    [ "Referenced third party codes", "md_THIRDPARTY.html", null ],
    [ "TODO List", "md_TODO.html", [
      [ "Known issues", "md_TODO.html#autotoc_md180", null ]
    ] ],
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
        [ "All", "globals.html", "globals_dup" ],
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
"classmagic__enum_1_1detail_1_1static__str_3_010_01_4.html#ac727e8dced788957117cbae18d6bffdc",
"classyoi_1_1Formatter.html#a8d2d073a3a7c00db070a1c5bea6ddd05",
"classyoi_1_1IR.html#a2112dbdb049f53df83b3ba6fcbf13dc7aba2787e1cdc508060eba1fa78e3bbb7c",
"classyoi_1_1IREnumerationType.html#aaaf91a8097d88fcf9daa98a42270ab41a5aef4e3ea379fa0eb2bf42d979443902",
"classyoi_1_1IRFunctionTemplate_1_1Builder.html#addb6b15125e0f2e6b94ac1e2f652f603",
"classyoi_1_1IROptimizer.html#aee239745b9245cd62923a29f011623d2",
"classyoi_1_1LLVMCodegen.html#a0e6ad8c75e0ec585347b9ed29af2b0cb",
"classyoi_1_1ObjectLinker.html#aa41dd1fc48736fb2d1513f7ade7db1a7",
"classyoi_1_1enum__range.html#a9164df3225e65afc486c637377db0dd1",
"classyoi_1_1implInner.html#a241c2283596937007e2e27157d3e4e8d",
"classyoi_1_1innerMethodDef.html#a726b6789532ce6f35a0b7d5b941989f4",
"classyoi_1_1moduleContext.html#aef836205cda58b0b2eb84c5a29ce5245",
"classyoi_1_1uniqueExpr.html#a5d80b4b3ed0db9c87f45d15ed42dd3ce",
"classyoi_1_1visitor.html#af8d16f3d9aa85256ff201cfd81d9ef0e",
"functions_k.html",
"md_docs_2Array.html#autotoc_md6",
"namespacemagic__enum_1_1detail.html#aa3831777332881653b3b7b4f21ec8f0e",
"namespaceyoi.html#aed7adb79d19f7e7ab303ece0047d8654",
"string_8cpp.html#a5a01f705cc7b4a7317a57f6bf41aef25",
"structyoi_1_1FormatOption.html#a26d53020eefb93178889aa20ebb799fa",
"structyoi_1_1IRFunctionOptimizer_1_1SimulationStack_1_1Item_1_1ContributedInstructionSet_1_1Iterator.html",
"structyoi_1_1lexer_1_1token.html#a813647b1bf8cc722a45770b5bee77f32a13ee0eb09df761a766580fc5e2b7efe2",
"time_8cpp.html#a5fe91b646fc7b03ade4cc9def4c9912b"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';