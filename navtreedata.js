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
    [ "直接赋值 (Direct assignment)", "md_docs_2Direct_01Assignment.html", null ],
    [ "接口设计", "md_docs_2Interface.html", [
      [ "new 语句设计", "md_docs_2Interface.html#autotoc_md9", null ]
    ] ],
    [ "The Hoshi-lang Intermediate Representation (IR) Handbook", "md_docs_2IR.html", [
      [ "1. Introduction", "md_docs_2IR.html#autotoc_md11", [
        [ "1.1. Purpose", "md_docs_2IR.html#autotoc_md12", null ],
        [ "1.2. Execution Model", "md_docs_2IR.html#autotoc_md13", null ]
      ] ],
      [ "2. Core Concepts", "md_docs_2IR.html#autotoc_md14", [
        [ "2.1. <tt>IRModule</tt>", "md_docs_2IR.html#autotoc_md15", null ],
        [ "2.2. <tt>IRValueType</tt>", "md_docs_2IR.html#autotoc_md16", null ]
      ] ],
      [ "3. Instruction Set Reference", "md_docs_2IR.html#autotoc_md18", [
        [ "3.1. Stack and Memory Operations", "md_docs_2IR.html#autotoc_md19", null ],
        [ "3.2. Arithmetic and Logical Operations", "md_docs_2IR.html#autotoc_md20", null ],
        [ "3.3. Control Flow", "md_docs_2IR.html#autotoc_md21", null ],
        [ "3.4. Object and Array Lifecycle", "md_docs_2IR.html#autotoc_md22", null ],
        [ "3.5. Type Operations", "md_docs_2IR.html#autotoc_md23", null ],
        [ "3.6. Function and Method Calls", "md_docs_2IR.html#autotoc_md24", null ],
        [ "1.2. Execution Model", "md_docs_2IR.html#autotoc_md25", null ]
      ] ],
      [ "2. Core Concepts", "md_docs_2IR.html#autotoc_md26", [
        [ "2.1. <tt>IRModule</tt>", "md_docs_2IR.html#autotoc_md27", null ],
        [ "2.2. <tt>IRFunctionDefinition</tt>", "md_docs_2IR.html#autotoc_md28", null ],
        [ "2.3. <tt>IRCodeBlock</tt>", "md_docs_2IR.html#autotoc_md29", null ],
        [ "2.4. <tt>IRValueType</tt>", "md_docs_2IR.html#autotoc_md30", null ],
        [ "2.5. <tt>IROperand</tt>", "md_docs_2IR.html#autotoc_md31", null ]
      ] ],
      [ "3. Instruction Set Reference", "md_docs_2IR.html#autotoc_md33", [
        [ "3.1. Stack and Memory Operations", "md_docs_2IR.html#autotoc_md34", null ],
        [ "3.2. Arithmetic and Logical Operations", "md_docs_2IR.html#autotoc_md35", null ],
        [ "3.3. Control Flow", "md_docs_2IR.html#autotoc_md36", null ],
        [ "3.4. Object Lifecycle", "md_docs_2IR.html#autotoc_md37", null ],
        [ "3.5. Function and Method Calls", "md_docs_2IR.html#autotoc_md38", null ]
      ] ],
      [ "4. Full Example", "md_docs_2IR.html#autotoc_md40", null ]
    ] ],
    [ "The implementation of null literal in hoshi-lang", "md_docs_2Null.html", [
      [ "Pointer Object in hoshi-lang", "md_docs_2Null.html#autotoc_md42", null ]
    ] ],
    [ "Hoshi Language Specification", "md_docs_2Spec.html", [
      [ "1. Introduction", "md_docs_2Spec.html#autotoc_md44", null ],
      [ "2. Lexical Structure", "md_docs_2Spec.html#autotoc_md45", [
        [ "2.1. Comments", "md_docs_2Spec.html#autotoc_md46", null ],
        [ "2.2. Keywords", "md_docs_2Spec.html#autotoc_md47", null ]
      ] ],
      [ "3. Types and Data Structures", "md_docs_2Spec.html#autotoc_md48", [
        [ "3.1. Primitive Types", "md_docs_2Spec.html#autotoc_md49", null ],
        [ "3.2. Structs", "md_docs_2Spec.html#autotoc_md50", null ],
        [ "3.3. Interfaces", "md_docs_2Spec.html#autotoc_md51", null ],
        [ "3.4. Implementations (<tt>impl</tt>)", "md_docs_2Spec.html#autotoc_md52", null ],
        [ "3.5. Arrays and Dynamic Arrays", "md_docs_2Spec.html#autotoc_md53", [
          [ "Fixed-Size Arrays", "md_docs_2Spec.html#autotoc_md54", null ],
          [ "Dynamic Arrays", "md_docs_2Spec.html#autotoc_md55", null ],
          [ "The <tt>.length</tt> Property", "md_docs_2Spec.html#autotoc_md56", null ]
        ] ],
        [ "3.6. Generic Programming (Templates)", "md_docs_2Spec.html#autotoc_md57", null ]
      ] ],
      [ "4. Object Model and Lifecycle", "md_docs_2Spec.html#autotoc_md58", [
        [ "4.1. Everything is an Object", "md_docs_2Spec.html#autotoc_md59", null ],
        [ "4.2. Memory Management: Reference Counting", "md_docs_2Spec.html#autotoc_md60", null ],
        [ "4.3. Object Creation", "md_docs_2Spec.html#autotoc_md61", null ]
      ] ],
      [ "5. Type Introspection and Casting", "md_docs_2Spec.html#autotoc_md62", [
        [ "5.1. <tt>typeid</tt> Operator", "md_docs_2Spec.html#autotoc_md63", null ],
        [ "5.2. <tt>interfaceof</tt> Operator", "md_docs_2Spec.html#autotoc_md64", null ],
        [ "5.3. Dynamic Casting (<tt>dyn_cast</tt>)", "md_docs_2Spec.html#autotoc_md65", null ]
      ] ],
      [ "6. Compilation and Linking", "md_docs_2Spec.html#autotoc_md66", null ],
      [ "7. Modules and Foreign Function Interface (FFI)", "md_docs_2Spec.html#autotoc_md67", [
        [ "7.1. Modules", "md_docs_2Spec.html#autotoc_md68", null ],
        [ "7.2. Foreign Function Interface (FFI)", "md_docs_2Spec.html#autotoc_md69", null ]
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
"classyoi_1_1BuiltinModuleBuilder.html#a170a7036e9359575d23ed4cd95ce7c6b",
"classyoi_1_1IRBuilder.html#a2c0487a0411af457d2a23859b8344375",
"classyoi_1_1IRLinker.html#a20223d6c01f045c7e8c38e81cd1eed2e",
"classyoi_1_1IRStructTemplate_1_1Builder.html#ad72b478f57f4d51a3c2f010afb03056f",
"classyoi_1_1LLVMCodegen.html#ac4cc9579c633f4e6815f1fc0ff61fd17",
"classyoi_1_1definitionArguments.html#a52f208ab3c950a48d57c4fa4d368c3be",
"classyoi_1_1implInnerPair.html#afbcc34ca2ea0fda2f5706726a6589b63",
"classyoi_1_1interfaceDefStmt.html#a38b5e7dde5836d07400f66289e29ca24",
"classyoi_1_1returnStmt.html",
"classyoi_1_1visitor.html#a628aac51b352074768d7d4b2d4d3dd65",
"functions_vars_s.html",
"moduleContext_8h_source.html",
"namespaceyoi.html#aaed6e0f29c663d428e7051d3b479db29",
"structYoiBooleanObject.html#a0376be5904d0dd864b7d97c9ce1295ab",
"structyoi_1_1IRInterfaceInstanceDefinition_1_1Builder.html#a7595ec51b788725a74d2d928890383d8",
"structyoi_1_1lexer_1_1token.html#a813647b1bf8cc722a45770b5bee77f32a95da522d005dab6fff2c0e2f6d1ce400"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';