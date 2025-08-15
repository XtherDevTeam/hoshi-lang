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
        [ "Enumerations", "globals_enum.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"IRLinker_8cpp.html",
"classyoi_1_1BuiltinModuleBuilder.html#a2580086e454bee20cf97f2c5acee25d5",
"classyoi_1_1IRBuilder.html#a693c76bf4ded3879028e83f8286ed1be",
"classyoi_1_1IRModule.html#a02df2d9d7dc7d8d5a35488c94c980882",
"classyoi_1_1IRValueType.html#a6a54d8929e41805f4a28e13ed08d4e85a84f79a71b165bab23e1bfe95d54f91a0",
"classyoi_1_1ObjectLinker.html#a804f6ada421031b9fef995fd82c8b0a3",
"classyoi_1_1exportDecl.html#a726b6789532ce6f35a0b7d5b941989f4",
"classyoi_1_1indexPool.html",
"classyoi_1_1lexer.html#a72993309a1242bc81500cc79f61b7e7b",
"classyoi_1_1symbol.html#adbced06f2a0b52ca205e82de1a1f4171a3c1aac82863ed9e5a9aca8ce687f711d",
"classyoi_1_1whileStmt.html#a4f12d9ae58f8b4d08debfc33b8042adf",
"magic__enum_8h.html#acb3aed292286cf34a905529025ba4827",
"namespaceyoi.html#a258c2f25b921c6fed0c5436b8f3c64d1",
"parser_8hpp.html#add54fcea8dc58f89d04c81c9e671bdfa",
"structyoi_1_1IRFunctionDefinition_1_1Builder.html#a127ac7aab5c87bd7df242644885320ac",
"structyoi_1_1lexer_1_1token.html#a813647b1bf8cc722a45770b5bee77f32a465289687a70db7aa7217cc240c29f0f",
"unionyoi_1_1lexer_1_1token_1_1vBasicValue.html#a22dcd3dcdb72b540bca86cd2fee0f88d"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';