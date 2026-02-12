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
    [ "Data struct in hoshi-lang", "md_docs_2Datastruct.html", [
      [ "Syntax", "md_docs_2Datastruct.html#autotoc_md22", null ],
      [ "Memory Layout", "md_docs_2Datastruct.html#autotoc_md23", null ],
      [ "Underlying mechanism", "md_docs_2Datastruct.html#autotoc_md24", null ],
      [ "The integration with existing object optimization system", "md_docs_2Datastruct.html#autotoc_md25", null ],
      [ "The validation of the performance of data struct", "md_docs_2Datastruct.html#autotoc_md26", null ]
    ] ],
    [ "直接赋值 (Direct assignment)", "md_docs_2Direct_01Assignment.html", null ],
    [ "File System in Hoshi-lang", "md_docs_2File_01System.html", [
      [ "<tt>file</tt> Module", "md_docs_2File_01System.html#autotoc_md29", [
        [ "<tt>AbstractFile</tt> Interface", "md_docs_2File_01System.html#autotoc_md30", null ]
      ] ],
      [ "<tt>fs</tt> Module", "md_docs_2File_01System.html#autotoc_md31", [
        [ "Functions", "md_docs_2File_01System.html#autotoc_md32", null ],
        [ "<tt>OSFile</tt> Struct", "md_docs_2File_01System.html#autotoc_md33", null ]
      ] ],
      [ "<tt>io</tt> Module", "md_docs_2File_01System.html#autotoc_md34", [
        [ "<tt>ArrayBuffer</tt> Struct", "md_docs_2File_01System.html#autotoc_md35", null ],
        [ "Example", "md_docs_2File_01System.html#autotoc_md36", null ]
      ] ]
    ] ],
    [ "Finalizers in Hoshi-lang", "md_docs_2Finalizers.html", [
      [ "Syntax", "md_docs_2Finalizers.html#autotoc_md38", null ],
      [ "Execution", "md_docs_2Finalizers.html#autotoc_md39", null ]
    ] ],
    [ "HashMap in Hoshi-lang", "md_docs_2HashMap.html", [
      [ "<tt>HashMap<K, V></tt> Struct", "md_docs_2HashMap.html#autotoc_md41", [
        [ "Methods", "md_docs_2HashMap.html#autotoc_md42", null ],
        [ "Example", "md_docs_2HashMap.html#autotoc_md43", null ]
      ] ]
    ] ],
    [ "接口设计", "md_docs_2Interface.html", [
      [ "new 语句设计", "md_docs_2Interface.html#autotoc_md45", null ]
    ] ],
    [ "The Hoshi-lang Intermediate Representation (IR) Handbook", "md_docs_2IR.html", [
      [ "1. Introduction", "md_docs_2IR.html#autotoc_md47", [
        [ "1.1. Purpose", "md_docs_2IR.html#autotoc_md48", null ],
        [ "1.2. Execution Model", "md_docs_2IR.html#autotoc_md49", null ]
      ] ],
      [ "2. Core Concepts", "md_docs_2IR.html#autotoc_md50", [
        [ "2.1. <tt>IRModule</tt>", "md_docs_2IR.html#autotoc_md51", null ],
        [ "2.2. <tt>IRValueType</tt>", "md_docs_2IR.html#autotoc_md52", null ]
      ] ],
      [ "3. Instruction Set Reference", "md_docs_2IR.html#autotoc_md54", [
        [ "3.1. Stack and Memory Operations", "md_docs_2IR.html#autotoc_md55", null ],
        [ "3.2. Arithmetic and Logical Operations", "md_docs_2IR.html#autotoc_md56", null ],
        [ "3.3. Control Flow", "md_docs_2IR.html#autotoc_md57", null ],
        [ "3.4. Object and Array Lifecycle", "md_docs_2IR.html#autotoc_md58", null ],
        [ "3.5. Type Operations", "md_docs_2IR.html#autotoc_md59", null ],
        [ "3.6. Function and Method Calls", "md_docs_2IR.html#autotoc_md60", null ],
        [ "1.2. Execution Model", "md_docs_2IR.html#autotoc_md61", null ]
      ] ],
      [ "2. Core Concepts", "md_docs_2IR.html#autotoc_md62", [
        [ "2.1. <tt>IRModule</tt>", "md_docs_2IR.html#autotoc_md63", null ],
        [ "2.2. <tt>IRFunctionDefinition</tt>", "md_docs_2IR.html#autotoc_md64", null ],
        [ "2.3. <tt>IRCodeBlock</tt>", "md_docs_2IR.html#autotoc_md65", null ],
        [ "2.4. <tt>IRValueType</tt>", "md_docs_2IR.html#autotoc_md66", null ],
        [ "2.5. <tt>IROperand</tt>", "md_docs_2IR.html#autotoc_md67", null ]
      ] ],
      [ "3. Instruction Set Reference", "md_docs_2IR.html#autotoc_md69", [
        [ "3.1. Stack and Memory Operations", "md_docs_2IR.html#autotoc_md70", null ],
        [ "3.2. Arithmetic and Logical Operations", "md_docs_2IR.html#autotoc_md71", null ],
        [ "3.3. Control Flow", "md_docs_2IR.html#autotoc_md72", null ],
        [ "3.4. Object Lifecycle", "md_docs_2IR.html#autotoc_md73", null ],
        [ "3.5. Function and Method Calls", "md_docs_2IR.html#autotoc_md74", null ]
      ] ],
      [ "4. Full Example", "md_docs_2IR.html#autotoc_md76", null ]
    ] ],
    [ "JSON in Hoshi-lang", "md_docs_2JSON.html", [
      [ "<tt>parse</tt> Function", "md_docs_2JSON.html#autotoc_md78", null ],
      [ "<tt>JSONValue</tt> Interface", "md_docs_2JSON.html#autotoc_md79", null ],
      [ "Example", "md_docs_2JSON.html#autotoc_md80", null ]
    ] ],
    [ "Macros in Hoshi-lang", "md_docs_2Macros.html", [
      [ "Syntax", "md_docs_2Macros.html#autotoc_md82", null ],
      [ "Pre-defined Macros", "md_docs_2Macros.html#autotoc_md83", null ],
      [ "Custom Macros", "md_docs_2Macros.html#autotoc_md84", null ],
      [ "Example", "md_docs_2Macros.html#autotoc_md85", null ]
    ] ],
    [ "Math in Hoshi-lang", "md_docs_2Math.html", [
      [ "Functions", "md_docs_2Math.html#autotoc_md87", null ],
      [ "Example", "md_docs_2Math.html#autotoc_md88", null ]
    ] ],
    [ "The implementation of null literal in hoshi-lang", "md_docs_2Null.html", [
      [ "Pointer Object in hoshi-lang", "md_docs_2Null.html#autotoc_md90", null ]
    ] ],
    [ "Nullable Check", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html", [
      [ "Raw Check", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md92", null ],
      [ "Inter-functional call graph building and raw check", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md93", null ],
      [ "IRValueType 属性的增加和改写", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md94", null ]
    ] ],
    [ "Operator Overloading", "md_docs_2Operator_01Overloading.html", [
      [ "Overloadable Operators", "md_docs_2Operator_01Overloading.html#autotoc_md96", null ],
      [ "Defining Operator Overloads", "md_docs_2Operator_01Overloading.html#autotoc_md97", [
        [ "Binary Operators", "md_docs_2Operator_01Overloading.html#autotoc_md98", null ],
        [ "Unary Operators", "md_docs_2Operator_01Overloading.html#autotoc_md99", null ],
        [ "Callable Objects (operator())", "md_docs_2Operator_01Overloading.html#autotoc_md100", null ],
        [ "Subscript Operator (operator[])", "md_docs_2Operator_01Overloading.html#autotoc_md101", null ]
      ] ]
    ] ],
    [ "Result Type in Hoshi-lang", "md_docs_2Result.html", [
      [ "<tt>Result<T, E></tt> Struct", "md_docs_2Result.html#autotoc_md103", [
        [ "Methods", "md_docs_2Result.html#autotoc_md104", null ],
        [ "Static Methods", "md_docs_2Result.html#autotoc_md105", null ],
        [ "Example", "md_docs_2Result.html#autotoc_md106", null ]
      ] ]
    ] ],
    [ "Runtime in Hoshi-lang", "md_docs_2Runtime.html", [
      [ "Functions", "md_docs_2Runtime.html#autotoc_md108", null ]
    ] ],
    [ "Hoshi-lang Language Specification", "md_docs_2Spec.html", [
      [ "1. Lexical Structure", "md_docs_2Spec.html#autotoc_md110", [
        [ "1.1. Identifiers", "md_docs_2Spec.html#autotoc_md111", null ],
        [ "1.2. Keywords", "md_docs_2Spec.html#autotoc_md112", null ],
        [ "1.3. Literals", "md_docs_2Spec.html#autotoc_md113", null ]
      ] ],
      [ "2. Types", "md_docs_2Spec.html#autotoc_md114", null ],
      [ "3. Structs", "md_docs_2Spec.html#autotoc_md115", null ],
      [ "4. Interfaces", "md_docs_2Spec.html#autotoc_md116", null ],
      [ "5. Functions", "md_docs_2Spec.html#autotoc_md117", null ],
      [ "6. Operator Overloading", "md_docs_2Spec.html#autotoc_md118", null ],
      [ "7. Interface Templates", "md_docs_2Spec.html#autotoc_md119", null ],
      [ "8. Standard Library", "md_docs_2Spec.html#autotoc_md120", null ]
    ] ],
    [ "String in Hoshi-lang", "md_docs_2String.html", [
      [ "<tt>Str</tt> Struct", "md_docs_2String.html#autotoc_md122", [
        [ "Methods", "md_docs_2String.html#autotoc_md123", null ],
        [ "Static Methods", "md_docs_2String.html#autotoc_md124", null ]
      ] ],
      [ "<tt>Stringable</tt> Interface", "md_docs_2String.html#autotoc_md125", null ],
      [ "<tt>format</tt> Function", "md_docs_2String.html#autotoc_md126", null ]
    ] ],
    [ "Structured Bindings in Hoshi-lang", "md_docs_2Structured_01Bindings.html", [
      [ "Array Destructuring", "md_docs_2Structured_01Bindings.html#autotoc_md128", null ],
      [ "Struct Destructuring", "md_docs_2Structured_01Bindings.html#autotoc_md129", null ]
    ] ],
    [ "Generic Programming with Templates", "md_docs_2Template.html", [
      [ "Function Templates", "md_docs_2Template.html#autotoc_md131", [
        [ "Implicit vs. Explicit Specialization", "md_docs_2Template.html#autotoc_md132", null ]
      ] ],
      [ "Struct Templates", "md_docs_2Template.html#autotoc_md133", null ],
      [ "Interface Templates", "md_docs_2Template.html#autotoc_md134", null ]
    ] ],
    [ "The Optimization Strategy of Interface Allocation and Virtual Invocation Reduction 接口分配和虚函数调用消除优化策略", "md_docs_2The_01Optimization_01Strategy_01of_01Interface_01Allocation_01and_01Virtual_01Invocation_01Reduction.html", null ],
    [ "Threading in Hoshi-lang", "md_docs_2Threading.html", [
      [ "<tt>Thread</tt> Struct", "md_docs_2Threading.html#autotoc_md137", [
        [ "Methods", "md_docs_2Threading.html#autotoc_md138", null ],
        [ "Example", "md_docs_2Threading.html#autotoc_md139", null ]
      ] ],
      [ "<tt>Mutex</tt> Struct", "md_docs_2Threading.html#autotoc_md140", [
        [ "Methods", "md_docs_2Threading.html#autotoc_md141", null ],
        [ "Example", "md_docs_2Threading.html#autotoc_md142", null ]
      ] ],
      [ "<tt>current_tid()</tt>", "md_docs_2Threading.html#autotoc_md143", null ]
    ] ],
    [ "Type Aliases in Hoshi-lang", "md_docs_2Type_01Aliases.html", [
      [ "Syntax", "md_docs_2Type_01Aliases.html#autotoc_md145", null ],
      [ "Example", "md_docs_2Type_01Aliases.html#autotoc_md146", null ]
    ] ],
    [ "Vector in Hoshi-lang", "md_docs_2Vector.html", [
      [ "<tt>Vec<T></tt> Struct", "md_docs_2Vector.html#autotoc_md148", [
        [ "Methods", "md_docs_2Vector.html#autotoc_md149", null ],
        [ "Example", "md_docs_2Vector.html#autotoc_md150", null ]
      ] ]
    ] ],
    [ "Export Wrapper - 实现 FFI 的必经之路", "md_docs_2Wrapper.html", null ],
    [ "Referenced third party codes", "md_THIRDPARTY.html", null ],
    [ "TODO List", "md_TODO.html", [
      [ "Known issues", "md_TODO.html#autotoc_md174", null ]
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
"classutf8Unicode.html",
"classyoi_1_1IR.html#a2112dbdb049f53df83b3ba6fcbf13dc7a8a89df5082e48abdc267afde37f07e78",
"classyoi_1_1IRCodeBlock.html#a1ce9d2325368dd3c3cffacd0f82e3bf9",
"classyoi_1_1IRFunctionOptimizer.html#ab1457933fbe38b8dbb36f3898f9e81e2",
"classyoi_1_1IROperand.html#a871fdb7683d8f38cb75e7f3ed1727c91",
"classyoi_1_1IRVariableTable.html#a61fa8c172e7397d729af1135ec6076a6",
"classyoi_1_1ObjectLinker.html#a827e8525695536d557f854edffadbf58",
"classyoi_1_1enum__range.html#a9164df3225e65afc486c637377db0dd1",
"classyoi_1_1implInner.html#a241c2283596937007e2e27157d3e4e8d",
"classyoi_1_1innerMethodDef.html#ad073861ccdab59de05eb8d34e9f0a34f",
"classyoi_1_1moduleContext.html#affc03367afdb2b7984d698b1af375424",
"classyoi_1_1visitor.html#a027fbc7de87978eea4c23d96f9374829",
"codegenObjectCache_8hpp.html#a5f935aed7bd36627d500906ef52180d1",
"globals_l.html",
"md_docs_2IR.html#autotoc_md67",
"namespacemembers_v.html",
"parser_8hpp.html#a03f268b61aace319b820cd20ee161624",
"structYoiResultUnsignedAndIntObject.html",
"structyoi_1_1IRBuildConfig_1_1Builder.html#a2c8321663f9f80d832a940ba78027260",
"structyoi_1_1IRStructDefinition_1_1nameInfo.html",
"structyoi_1_1lexer_1_1token.html#a813647b1bf8cc722a45770b5bee77f32ab5152fa9f8596245f455b7f939f7e9fd",
"unionyoi_1_1inCodeBlockStmt_1_1vValue.html#add9af9569af79ec26dd741fb226b38ba"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';