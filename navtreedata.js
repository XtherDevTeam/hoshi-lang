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
    [ "Nullable Check", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html", [
      [ "Raw Check", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md44", null ],
      [ "IRValueType 属性的增加和改写", "md_docs_2Nullable_01Check_01_6_01Raw_01Check.html#autotoc_md45", null ]
    ] ],
    [ "Operator Overloading", "md_docs_2Operator_01Overloading.html", [
      [ "Overloadable Operators", "md_docs_2Operator_01Overloading.html#autotoc_md47", null ],
      [ "Defining Operator Overloads", "md_docs_2Operator_01Overloading.html#autotoc_md48", [
        [ "Binary Operators", "md_docs_2Operator_01Overloading.html#autotoc_md49", null ],
        [ "Unary Operators", "md_docs_2Operator_01Overloading.html#autotoc_md50", null ],
        [ "Callable Objects (operator())", "md_docs_2Operator_01Overloading.html#autotoc_md51", null ],
        [ "Subscript Operator (operator[])", "md_docs_2Operator_01Overloading.html#autotoc_md52", null ]
      ] ]
    ] ],
    [ "Hoshi-lang Language Specification", "md_docs_2Spec.html", [
      [ "1. Lexical Structure", "md_docs_2Spec.html#autotoc_md54", [
        [ "1.1. Identifiers", "md_docs_2Spec.html#autotoc_md55", null ],
        [ "1.2. Keywords", "md_docs_2Spec.html#autotoc_md56", null ],
        [ "1.3. Literals", "md_docs_2Spec.html#autotoc_md57", null ]
      ] ],
      [ "2. Types", "md_docs_2Spec.html#autotoc_md58", null ],
      [ "3. Structs", "md_docs_2Spec.html#autotoc_md59", null ],
      [ "4. Interfaces", "md_docs_2Spec.html#autotoc_md60", null ],
      [ "5. Functions", "md_docs_2Spec.html#autotoc_md61", null ],
      [ "6. Operator Overloading", "md_docs_2Spec.html#autotoc_md62", null ],
      [ "7. Interface Templates", "md_docs_2Spec.html#autotoc_md63", null ],
      [ "8. Standard Library", "md_docs_2Spec.html#autotoc_md64", null ]
    ] ],
    [ "Standard Library", "md_docs_2Standard_01Library.html", [
      [ "String (<tt>str</tt>)", "md_docs_2Standard_01Library.html#autotoc_md66", null ],
      [ "Vector (<tt>vec</tt>)", "md_docs_2Standard_01Library.html#autotoc_md67", null ]
    ] ],
    [ "Generic Programming with Templates", "md_docs_2Template.html", [
      [ "Function Templates", "md_docs_2Template.html#autotoc_md69", [
        [ "Implicit vs. Explicit Specialization", "md_docs_2Template.html#autotoc_md70", null ]
      ] ],
      [ "Struct Templates", "md_docs_2Template.html#autotoc_md71", null ],
      [ "Interface Templates", "md_docs_2Template.html#autotoc_md72", null ]
    ] ],
    [ "Export Wrapper - 实现 FFI 的必经之路", "md_docs_2Wrapper.html", null ],
    [ "Referenced third party codes", "md_THIRDPARTY.html", null ],
    [ "TODO List", "md_TODO.html", [
      [ "Known issues", "md_TODO.html#autotoc_md101", null ]
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
"classyoi_1_1BuiltinModuleBuilder.html#a170a7036e9359575d23ed4cd95ce7c6b",
"classyoi_1_1IRBuilder.html#a0e82125bac8069a5ee90e607478781a0",
"classyoi_1_1IRInterfaceImplementationTemplate.html#ac2821dbb7405311f25a5b578e8e2bef7",
"classyoi_1_1IROptimizer.html#ab7ab4e42dfb8a8195027f81a0ef47d10",
"classyoi_1_1LLVMCodegen.html#a2e61cae680796ff5a75ae45d70fd4b9f",
"classyoi_1_1compilerContext.html#a01671892f454a6b5e9c001a6c2343dbb",
"classyoi_1_1globalStmt.html",
"classyoi_1_1indexTable_1_1iterator.html#a1251251e13f9f43f150a6567e761b433",
"classyoi_1_1logicalOrExpr.html#a67b76affb3b5d35fa419ac234144038b",
"classyoi_1_1typeSpec.html#a8bfa3bf6753afc1b913227f7087508e4",
"debug_8h.html#ae8247f718dd0d710b7fb5d221f93a2ec",
"magic__enum_8h.html#ad1e8ca7399ef090ef09549302d9d7d40a4e5868d676cb634aa75b125a0f741abf",
"namespacemembers_m.html",
"parser_8hpp.html#a20fb5f7b02960fa3b9b90d8bad0b538c",
"structyoi_1_1IRBuildConfig.html#a7dc2a1e0a369fb339abb8c28dbb71593a123fead50246387983ee340507115ef4",
"structyoi_1_1LLVMCodegen_1_1StackValue.html#a4aecd6896ac1b6503a4e0a19dec00928",
"unionyoi_1_1IROperand_1_1operandValue.html#a42bccff23eea4fd68ca80b3d85cb29f6"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';