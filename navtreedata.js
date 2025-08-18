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
    [ "Array in yoi-lang", "md_docs_2Array.html", [
      [ "Dynamic array and array", "md_docs_2Array.html#autotoc_md1", null ],
      [ "Array length by <tt>array_length</tt>", "md_docs_2Array.html#autotoc_md2", null ]
    ] ],
    [ "直接赋值 (Direct assignment)", "md_docs_2Direct_01Assignment.html", null ],
    [ "接口设计", "md_docs_2Interface.html", [
      [ "new 语句设计", "md_docs_2Interface.html#autotoc_md5", null ]
    ] ],
    [ "The Yoi-lang Intermediate Representation (IR) Handbook", "md_docs_2IR.html", [
      [ "1. Introduction", "md_docs_2IR.html#autotoc_md7", [
        [ "1.1. Purpose", "md_docs_2IR.html#autotoc_md8", null ],
        [ "1.2. Execution Model", "md_docs_2IR.html#autotoc_md9", null ]
      ] ],
      [ "2. Core Concepts", "md_docs_2IR.html#autotoc_md10", [
        [ "2.1. <tt>IRModule</tt>", "md_docs_2IR.html#autotoc_md11", null ],
        [ "2.2. <tt>IRFunctionDefinition</tt>", "md_docs_2IR.html#autotoc_md12", null ],
        [ "2.3. <tt>IRCodeBlock</tt>", "md_docs_2IR.html#autotoc_md13", null ],
        [ "2.4. <tt>IRValueType</tt>", "md_docs_2IR.html#autotoc_md14", null ],
        [ "2.5. <tt>IROperand</tt>", "md_docs_2IR.html#autotoc_md15", null ]
      ] ],
      [ "3. Instruction Set Reference", "md_docs_2IR.html#autotoc_md17", [
        [ "3.1. Stack and Memory Operations", "md_docs_2IR.html#autotoc_md18", null ],
        [ "3.2. Arithmetic and Logical Operations", "md_docs_2IR.html#autotoc_md19", null ],
        [ "3.3. Control Flow", "md_docs_2IR.html#autotoc_md20", null ],
        [ "3.4. Object Lifecycle", "md_docs_2IR.html#autotoc_md21", null ],
        [ "3.5. Function and Method Calls", "md_docs_2IR.html#autotoc_md22", null ]
      ] ],
      [ "4. Full Example", "md_docs_2IR.html#autotoc_md24", null ]
    ] ],
    [ "The implementation of null literal in hoshi-lang", "md_docs_2Null.html", [
      [ "Pointer Object in hoshi-lang", "md_docs_2Null.html#autotoc_md26", null ]
    ] ],
    [ "Hoshi Language Specification", "md_docs_2Spec.html", [
      [ "1. Introduction", "md_docs_2Spec.html#autotoc_md28", null ],
      [ "2. Lexical Structure", "md_docs_2Spec.html#autotoc_md29", [
        [ "2.1. Comments", "md_docs_2Spec.html#autotoc_md30", null ],
        [ "2.2. Keywords", "md_docs_2Spec.html#autotoc_md31", null ],
        [ "2.3. Identifiers", "md_docs_2Spec.html#autotoc_md32", null ],
        [ "2.4. Literals", "md_docs_2Spec.html#autotoc_md33", null ]
      ] ],
      [ "3. Types and Data Structures", "md_docs_2Spec.html#autotoc_md34", [
        [ "3.1. Primitive Types", "md_docs_2Spec.html#autotoc_md35", null ],
        [ "3.2. Structs", "md_docs_2Spec.html#autotoc_md36", null ],
        [ "3.3. Interfaces", "md_docs_2Spec.html#autotoc_md37", null ],
        [ "3.4. Implementations (<tt>impl</tt>)", "md_docs_2Spec.html#autotoc_md38", null ],
        [ "3.5. Arrays", "md_docs_2Spec.html#autotoc_md39", null ],
        [ "3.6. Generic Programming (Templates)", "md_docs_2Spec.html#autotoc_md40", null ]
      ] ],
      [ "4. Object Model and Lifecycle", "md_docs_2Spec.html#autotoc_md41", [
        [ "4.1. Everything is an Object", "md_docs_2Spec.html#autotoc_md42", null ],
        [ "4.2. Memory Management: Reference Counting", "md_docs_2Spec.html#autotoc_md43", null ],
        [ "4.3. Object Creation", "md_docs_2Spec.html#autotoc_md44", null ]
      ] ],
      [ "5. Yoi Intermediate Representation (IR)", "md_docs_2Spec.html#autotoc_md45", [
        [ "5.1. Key Components", "md_docs_2Spec.html#autotoc_md46", null ],
        [ "5.2. Execution Model", "md_docs_2Spec.html#autotoc_md47", null ],
        [ "5.3. Instruction Set Highlights", "md_docs_2Spec.html#autotoc_md48", null ]
      ] ],
      [ "6. Compilation and Linking", "md_docs_2Spec.html#autotoc_md49", null ],
      [ "7. Modules and Foreign Function Interface (FFI)", "md_docs_2Spec.html#autotoc_md50", [
        [ "7.1. Modules", "md_docs_2Spec.html#autotoc_md51", null ],
        [ "7.2. Foreign Function Interface (FFI)", "md_docs_2Spec.html#autotoc_md52", null ]
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
"classyoi_1_1BuiltinModuleBuilder.html#a176e180c9bd68c99c1846b7edc10d59a",
"classyoi_1_1IRBuilder.html#a37c226e842e50a94f5beee188cfecc9d",
"classyoi_1_1IRLinker.html#a559cad0c47bafda74cc4655844447752",
"classyoi_1_1IRTemplateBuilder.html#a9cac6707ec366dca31c9f0f5ce61e389",
"classyoi_1_1LLVMCodegen.html#ad13e3f9e9d0d63fca585da5553339ec7",
"classyoi_1_1enum__range.html#ac96fceb7a09e6c9b857b17a891cfef78",
"classyoi_1_1importDecl.html#a70e95930cc931b4923176f3306ec7cb1",
"classyoi_1_1leftExpr.html#a938276e038eb894371a14659e8364315",
"classyoi_1_1structDefInnerPair.html",
"classyoi_1_1visitor.html#a87b778b9418b7357fbda584bbca711d3",
"lexer_8hpp_source.html",
"namespacemagic__enum_1_1detail.html#a1c39f200e074038afe693c2231e0bca3",
"parser_8cpp.html#a1599eecf8781c9dba0e44baefb471abf",
"structmagic__enum_1_1detail_1_1is__scoped__enum.html",
"structyoi_1_1IROptimizer_1_1SimulationStack_1_1Item_1_1ContributedInstructionSet_1_1Iterator.html#a710b1a5c9f835b20b87a76ce12e4f305",
"structyoi_1_1lexer_1_1token.html#a813647b1bf8cc722a45770b5bee77f32ae1ed6480eefefc22b89d2fbe59a6f086"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';