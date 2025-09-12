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
      [ "Inter-functional call graph building and raw check", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md52", null ],
      [ "IRValueType 属性的增加和改写", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md53", null ]
    ] ],
    [ "Operator Overloading", "md_docs_2Operator_01Overloading.html", [
      [ "Overloadable Operators", "md_docs_2Operator_01Overloading.html#autotoc_md55", null ],
      [ "Defining Operator Overloads", "md_docs_2Operator_01Overloading.html#autotoc_md56", [
        [ "Binary Operators", "md_docs_2Operator_01Overloading.html#autotoc_md57", null ],
        [ "Unary Operators", "md_docs_2Operator_01Overloading.html#autotoc_md58", null ],
        [ "Callable Objects (operator())", "md_docs_2Operator_01Overloading.html#autotoc_md59", null ],
        [ "Subscript Operator (operator[])", "md_docs_2Operator_01Overloading.html#autotoc_md60", null ]
      ] ]
    ] ],
    [ "Hoshi-lang Language Specification", "md_docs_2Spec.html", [
      [ "1. Lexical Structure", "md_docs_2Spec.html#autotoc_md62", [
        [ "1.1. Identifiers", "md_docs_2Spec.html#autotoc_md63", null ],
        [ "1.2. Keywords", "md_docs_2Spec.html#autotoc_md64", null ],
        [ "1.3. Literals", "md_docs_2Spec.html#autotoc_md65", null ]
      ] ],
      [ "2. Types", "md_docs_2Spec.html#autotoc_md66", null ],
      [ "3. Structs", "md_docs_2Spec.html#autotoc_md67", null ],
      [ "4. Interfaces", "md_docs_2Spec.html#autotoc_md68", null ],
      [ "5. Functions", "md_docs_2Spec.html#autotoc_md69", null ],
      [ "6. Operator Overloading", "md_docs_2Spec.html#autotoc_md70", null ],
      [ "7. Interface Templates", "md_docs_2Spec.html#autotoc_md71", null ],
      [ "8. Standard Library", "md_docs_2Spec.html#autotoc_md72", null ]
    ] ],
    [ "Standard Library", "md_docs_2Standard_01Library.html", [
      [ "String (<tt>str</tt>)", "md_docs_2Standard_01Library.html#autotoc_md74", null ],
      [ "Vector (<tt>vec</tt>)", "md_docs_2Standard_01Library.html#autotoc_md75", null ]
    ] ],
    [ "Generic Programming with Templates", "md_docs_2Template.html", [
      [ "Function Templates", "md_docs_2Template.html#autotoc_md77", [
        [ "Implicit vs. Explicit Specialization", "md_docs_2Template.html#autotoc_md78", null ]
      ] ],
      [ "Struct Templates", "md_docs_2Template.html#autotoc_md79", null ],
      [ "Interface Templates", "md_docs_2Template.html#autotoc_md80", null ]
    ] ],
    [ "Export Wrapper - 实现 FFI 的必经之路", "md_docs_2Wrapper.html", null ],
    [ "Referenced third party codes", "md_THIRDPARTY.html", null ],
    [ "TODO List", "md_TODO.html", [
      [ "Known issues", "md_TODO.html#autotoc_md109", null ]
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
"classyoi_1_1AST.html#aae59fbfe596180f1547a5d365e6baca3",
"classyoi_1_1IR.html#a2112dbdb049f53df83b3ba6fcbf13dc7aea86840758f26cabf4f81d48c19677ae",
"classyoi_1_1IRFunctionDefinition.html#acf2fe838aef9496852dd242873f4b445a84a8921b25f505d0d2077aeb5db4bc16",
"classyoi_1_1IRModule.html#a257f8970d5f4221d78b4e493cb6ce659",
"classyoi_1_1IRValueType.html#ae512a76e6e645da2ab6f9153cba7831f",
"classyoi_1_1ObjectLinker.html#a804f6ada421031b9fef995fd82c8b0a3",
"classyoi_1_1enum__range_1_1iterator.html#a64e2407ef88f59ed9289485c12af21ca",
"classyoi_1_1inCodeBlockStmt.html#a8841073ebfcb23c592e02a483d1c8c9e",
"classyoi_1_1leftExpr.html#a660d12634bc6f49dbc492979216c1f58",
"classyoi_1_1returnStmt.html#a2ff862ac765b8a35c4bbec03030003e7",
"classyoi_1_1visitor.html#a52b461f1a6fcd065affbf392b8bfb254",
"functions_func_t.html",
"md_docs_2Operator_01Overloading.html",
"namespaceyoi.html#a4ca34fd416c2b4640aa019ed16026010",
"parser_8hpp.html#ad36c647b0695dda56ddc3eacbfbecdda",
"structyoi_1_1IRBuildConfig.html#a92616faabf3a5b7737e69f0237ec2772",
"structyoi_1_1LLVMCodegen_1_1StackValue.html",
"threading_8cpp.html#a444dba8591942ff8168348fad7a218fc"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';