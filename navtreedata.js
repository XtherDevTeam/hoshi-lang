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
"classmagic__enum_1_1detail_1_1static__str_3_010_01_4.html#ad06e5969bdb6a5cef8a02da11dee11de",
"classyoi_1_1Formatter.html#a8ff527b86d669bb35b00c216eba88f7c",
"classyoi_1_1IR.html#a2112dbdb049f53df83b3ba6fcbf13dc7acfbe39a1b1f72923af053e14dda0527b",
"classyoi_1_1IREnumerationType_1_1Builder.html#a63b14fb7d5c5b5e6b98e93c8c0c53e4e",
"classyoi_1_1IRInterfaceImplementationDefinition.html#af301c03951a1e64d1d649b696865714b",
"classyoi_1_1IRStructDefinition.html#a1c0dad859c2a9feb09ecf53f8280538f",
"classyoi_1_1LLVMCodegen.html#a2bbbfd685e3fb848cfb60855ad433e33",
"classyoi_1_1addExpr.html#a9e0cf23a22b6af67b8b52a65f27ef7a2",
"classyoi_1_1equalityExpr.html#a67b76affb3b5d35fa419ac234144038b",
"classyoi_1_1importDecl.html#a00e8a649f83f70fd95dcf0628ba09299",
"classyoi_1_1lambdaExpr.html#a07f9633ac52baaaf0c76bd380f688487",
"classyoi_1_1primary.html#a436d924fe7de14180235e4eef8740894a2b44ce34841af051cca259c8f9c5587d",
"classyoi_1_1visitor.html#a22560c4340ac4dade215406386c15cd3",
"debug_8cpp.html#af6c723974a9c6298d9310c004e125b16",
"globals_vars.html",
"md_docs_2IR.html#autotoc_md72",
"namespacemembers_w.html",
"parser_8cpp.html#af6edf372460ef70c6cc567de91d912f4",
"structYoiIntegerObject.html#a8a850078f95585c20c624755a05d61b9",
"structyoi_1_1IRBuildConfig.html#a984823ab6e576ff2b3ecbd5e086bed71",
"structyoi_1_1IRMetadata.html#abf7d149967b8665bd54810d8b4b1c6b3",
"structyoi_1_1lexer_1_1token.html#a813647b1bf8cc722a45770b5bee77f32a801ae6277601f775ca52dc2d47c0ab23",
"unionyoi_1_1IROperand_1_1operandValue.html#a98dfeee6c7fcbe8f2adf39d0c10b06ec"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';