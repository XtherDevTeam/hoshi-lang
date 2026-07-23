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
        [ "Typedefs", "functions_type.html", "functions_type" ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", null ],
        [ "Related Symbols", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", "globals_dup" ],
        [ "Functions", "globals_func.html", "globals_func" ],
        [ "Variables", "globals_vars.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Macros", "globals_defs.html", "globals_defs" ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"IRLinker_8cpp.html",
"classbasic__json.html#a1b4fcb5b9927fe24d7831adbbd88c405",
"classbasic__json.html#adf2e047597ae1fdb8d0e501cbf79afa8",
"classdetail_1_1binary__writer.html#aac64bc201fda2938bbf6fec496d3c799",
"classdetail_1_1iteration__proxy__value.html#abc14888ae5a53293ccf3445724aa1585",
"classdetail_1_1json__sax__dom__parser.html#a74d4930870a831a12c34b92bf45e4860",
"classdetail_1_1parser.html#a0d441ad519e823ca28c69053cb174331",
"classlsp_1_1DocumentStore.html#a259c65337de62cf9a3448fc242741466",
"classyoi_1_1BuiltinModuleBuilder.html#a2580086e454bee20cf97f2c5acee25d5",
"classyoi_1_1Formatter.html#a7d263ff0a52c5ea7e504ee3ddb948918",
"classyoi_1_1IR.html#a2112dbdb049f53df83b3ba6fcbf13dc7a96ee5c0e2a7149613e7fd0b83316486e",
"classyoi_1_1IRConcept.html",
"classyoi_1_1IRFunctionOptimizer.html#aac1876ace76afb6ac3e0a28c6c3bae2d",
"classyoi_1_1IROperand.html#a36e44fa8cbaee4f943b6d62986b4d545",
"classyoi_1_1IRVariableTable.html#a23ef58e96cfbb021340b34923e1f9268",
"classyoi_1_1LLVMCodegen_1_1LLVMModuleContext.html#ac4cc9579c633f4e6815f1fc0ff61fd17",
"classyoi_1_1conceptStmt.html#aa10c9e8951b8ccf714a59ec321bdac5baa1a6657be79cc0fc1e9b23b9e108f043",
"classyoi_1_1globalStmt.html#a8841073ebfcb23c592e02a483d1c8c9ea723646ba2379f58847f411211799cfcf",
"classyoi_1_1indexTable_1_1iterator.html#a1251251e13f9f43f150a6567e761b433",
"classyoi_1_1lexer.html#ad2fc283e0d94fb9f8ea52974bc8140e0",
"classyoi_1_1structDefStmt.html#a726b3f8d771b30bcb075ff22577e77a1",
"classyoi_1_1visitor.html#a7e7fb2c39cc57152e5f6855aaae500fd",
"deprecated.html",
"globals_func_h.html",
"json_8hpp.html#a5563e5b0fbc2a9b525830081404d9208",
"json_8hpp.html#ae318b71889f1f0edeebd034de0bd056c",
"md_docs_2IR.html#autotoc_md64",
"namespacedetail.html#aafbf00b868b52296135404a1a6957a4a",
"namespacemagic__enum_1_1bitwise__operators.html#adfc25efbc82ccaba8466fbb898f4c927",
"namespaceyoi.html#a8f43718feda8ae338b496cac3a709be4",
"protocol_8h.html#a05812818de02fb4b202d7513ce50745da6f16a5f8ff5d75ab84c018adacdfcbb7",
"structYoiIntAndIntObject.html#a21197e49dd48d6f8dc580e42630ab047",
"structdetail_1_1has__to__json_3_01BasicJsonType_00_01T_00_01enable__if__t_3_01_9is__basic__json_3_01T_01_4_1_1value_01_4_01_4.html#a5b4ee4ae6cba6bb75200ba7af1910873",
"structdetail_1_1iterator__traits_3_01T_01_5_00_01enable__if__t_3_01std_1_1is__object_3_01T_01_4_1_1value_01_4_01_4.html#ab088798d28525c0befe3c707b95c5bc2",
"structlsp_1_1DocumentSymbol.html#a06f18b7e05b64c4ff0d03f07ae379a68",
"structordered__map.html#a2b340030e91cb2dcec290e5295b0bc5a",
"structyoi_1_1IRBuildConfig_1_1Builder.html#a5e48dafd436ebd63a183e750f57d7001",
"structyoi_1_1IRStructDefinition_1_1nameInfo.html#a5a23b037529794129c29b82304127dd0a06e3d36fa30cea095545139854ad1fb9",
"structyoi_1_1lexer_1_1token.html#a813647b1bf8cc722a45770b5bee77f32aa956161a69928cd130a889b88082fb6e",
"unionyoi_1_1conceptStmt_1_1ConceptStmtValue.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';