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
    [ "The design of yoi-lang programming language", "index.html", "index" ],
    [ "Array in yoi-lang", "md_docs_2Array.html", null ],
    [ "直接赋值 (Direct assignment)", "md_docs_2Direct_01Assignment.html", null ],
    [ "接口设计", "md_docs_2Interface.html", [
      [ "new 语句设计", "md_docs_2Interface.html#autotoc_md3", null ]
    ] ],
    [ "The Yoi-lang Intermediate Representation (IR) Handbook", "md_docs_2IR.html", [
      [ "1. Introduction", "md_docs_2IR.html#autotoc_md5", [
        [ "1.1. Purpose", "md_docs_2IR.html#autotoc_md6", null ],
        [ "1.2. Execution Model", "md_docs_2IR.html#autotoc_md7", null ]
      ] ],
      [ "2. Core Concepts", "md_docs_2IR.html#autotoc_md8", [
        [ "2.1. <tt>IRModule</tt>", "md_docs_2IR.html#autotoc_md9", null ],
        [ "2.2. <tt>IRFunctionDefinition</tt>", "md_docs_2IR.html#autotoc_md10", null ],
        [ "2.3. <tt>IRCodeBlock</tt>", "md_docs_2IR.html#autotoc_md11", null ],
        [ "2.4. <tt>IRValueType</tt>", "md_docs_2IR.html#autotoc_md12", null ],
        [ "2.5. <tt>IROperand</tt>", "md_docs_2IR.html#autotoc_md13", null ]
      ] ],
      [ "3. Instruction Set Reference", "md_docs_2IR.html#autotoc_md15", [
        [ "3.1. Stack and Memory Operations", "md_docs_2IR.html#autotoc_md16", null ],
        [ "3.2. Arithmetic and Logical Operations", "md_docs_2IR.html#autotoc_md17", null ],
        [ "3.3. Control Flow", "md_docs_2IR.html#autotoc_md18", null ],
        [ "3.4. Object Lifecycle", "md_docs_2IR.html#autotoc_md19", null ],
        [ "3.5. Function and Method Calls", "md_docs_2IR.html#autotoc_md20", null ]
      ] ],
      [ "4. Full Example", "md_docs_2IR.html#autotoc_md22", null ]
    ] ],
    [ "Hoshi Language Specification", "md_docs_2Spec.html", [
      [ "1. Introduction", "md_docs_2Spec.html#autotoc_md24", null ],
      [ "2. Lexical Structure", "md_docs_2Spec.html#autotoc_md25", [
        [ "2.1. Comments", "md_docs_2Spec.html#autotoc_md26", null ],
        [ "2.2. Keywords", "md_docs_2Spec.html#autotoc_md27", null ],
        [ "2.3. Identifiers", "md_docs_2Spec.html#autotoc_md28", null ],
        [ "2.4. Literals", "md_docs_2Spec.html#autotoc_md29", null ]
      ] ],
      [ "3. Types and Data Structures", "md_docs_2Spec.html#autotoc_md30", [
        [ "3.1. Primitive Types", "md_docs_2Spec.html#autotoc_md31", null ],
        [ "3.2. Structs", "md_docs_2Spec.html#autotoc_md32", null ],
        [ "3.3. Interfaces", "md_docs_2Spec.html#autotoc_md33", null ],
        [ "3.4. Implementations (<tt>impl</tt>)", "md_docs_2Spec.html#autotoc_md34", null ],
        [ "3.5. Arrays", "md_docs_2Spec.html#autotoc_md35", null ],
        [ "3.6. Generic Programming (Templates)", "md_docs_2Spec.html#autotoc_md36", null ]
      ] ],
      [ "4. Object Model and Lifecycle", "md_docs_2Spec.html#autotoc_md37", [
        [ "4.1. Everything is an Object", "md_docs_2Spec.html#autotoc_md38", null ],
        [ "4.2. Memory Management: Reference Counting", "md_docs_2Spec.html#autotoc_md39", null ],
        [ "4.3. Object Creation", "md_docs_2Spec.html#autotoc_md40", null ]
      ] ],
      [ "5. Yoi Intermediate Representation (IR)", "md_docs_2Spec.html#autotoc_md41", [
        [ "5.1. Key Components", "md_docs_2Spec.html#autotoc_md42", null ],
        [ "5.2. Execution Model", "md_docs_2Spec.html#autotoc_md43", null ],
        [ "5.3. Instruction Set Highlights", "md_docs_2Spec.html#autotoc_md44", null ]
      ] ],
      [ "6. Compilation and Linking", "md_docs_2Spec.html#autotoc_md45", null ],
      [ "7. Modules and Foreign Function Interface (FFI)", "md_docs_2Spec.html#autotoc_md46", [
        [ "7.1. Modules", "md_docs_2Spec.html#autotoc_md47", null ],
        [ "7.2. Foreign Function Interface (FFI)", "md_docs_2Spec.html#autotoc_md48", null ]
      ] ]
    ] ],
    [ "模板隐式特化的实现思路", "md_docs_2Template.html", null ],
    [ "Export Wrapper - 实现 FFI 的必经之路", "md_docs_2Wrapper.html", null ],
    [ "Referenced third party codes", "md_THIRDPARTY.html", null ],
    [ "TODO", "md_TODO.html", null ],
    [ "Deprecated List", "deprecated.html", null ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", "namespacemembers_dup" ],
        [ "Functions", "namespacemembers_func.html", null ],
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
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"IRLinker_8cpp.html",
"classyoi_1_1BuiltinModuleBuilder.html#a28c9e3567386a7ce1499749ae4b4091b",
"classyoi_1_1IRBuilder.html#a694ae8e9b02f2fd1b4411f04f7533be5",
"classyoi_1_1IRModule.html#a5ff575527eac5139bb55573da0f2869b",
"classyoi_1_1IRValueType.html#a6a54d8929e41805f4a28e13ed08d4e85ad235f60d8182b02ba4fd1afb93ede3b9",
"classyoi_1_1ObjectLinker.html#ad9cf9cac8117c250ae07ae54bc8db517",
"classyoi_1_1forEachStmt.html#a07f9633ac52baaaf0c76bd380f688487",
"classyoi_1_1indexTable.html#a2172ac23ccace7bbe941af751ae3cd74",
"classyoi_1_1lexer.html#a9cff40af0d1128b9faf8081323556836",
"classyoi_1_1symbolTable.html#a6b7cdf2baba5384998a10f5f80a6cda8",
"debug_8h.html#a86adaf5269d06b1ce6c63afb48f8c7ee",
"main_8cpp.html",
"namespaceyoi.html#a4a59d5413d6da04facbe9ac48e2f7fff",
"runtime_8h.html#a3302890ca27fb35f72095c2e63cfae7d",
"structyoi_1_1IRInterfaceInstanceDefinition_1_1Builder.html#a9d24a0275ba98a7726bfe14c798f9850",
"structyoi_1_1lexer_1_1token.html#a813647b1bf8cc722a45770b5bee77f32a9fbbaa4cc515bc46e0c12e82a31df736"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';