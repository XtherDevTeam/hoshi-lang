diff --git a/compiler/ir/IR.cpp b/compiler/ir/IR.cpp
index a35e2f9..de7133a 100644
--- a/compiler/ir/IR.cpp
+++ b/compiler/ir/IR.cpp
@@ -543,7 +543,7 @@ namespace yoi {
     IRFunctionDefinition::IRFunctionDefinition(
         const yoi::wstr &name,
         const yoi::vec<std::pair<yoi::wstr, std::shared_ptr<IRValueType>>> &argumentTypes,
-        const std::shared_ptr<IRValueType> &returnType, const yoi::vec<std::shared_ptr<IRCodeBlock>> &codeBlock, const yoi::vec<FunctionAttrs> &attrs, const IRDebugInfo &debugInfo)
+        const std::shared_ptr<IRValueType> &returnType, const yoi::vec<std::shared_ptr<IRCodeBlock>> &codeBlock, const yoi::vec<FunctionAttrs> &attrs, const IRDebugInfo &debugInfo) 
         : name(name), returnType(returnType), variableTable(), codeBlock(), debugInfo(debugInfo), attrs(attrs) {
         variableTable.createScope();
         for (auto &i : argumentTypes) {
@@ -783,16 +783,16 @@ namespace yoi {
 
     yoi::wstr IRInterfaceImplementationDefinition::to_string(yoi::indexT indent) {
         yoi::wstr r;
-        r += yoi::wstr(indent, L' ') 
-            + L"impl " 
-            + name 
-            + L" for " 
-            + yoi::string2wstring(std::string{magic_enum::enum_name(std::get<0>(implStructIndex))}) 
-            + L"#" 
-            + std::to_wstring(std::get<1>(implStructIndex)) 
-            + L"#" 
+        r += yoi::wstr(indent, L' ')
+            + L"impl "
+            + name
+            + L" for "
+            + yoi::string2wstring(std::string{magic_enum::enum_name(std::get<0>(implStructIndex))})
+            + L"#"
+            + std::to_wstring(std::get<1>(implStructIndex))
+            + L"#"
             + std::to_wstring(std::get<2>(implStructIndex)) + L" {\n";
-            
+
         for (auto &i : virtualMethods) {
             r += yoi::wstr(indent + 4, L' ') + L"virtual " + i->to_string() + L"\n";
         }
@@ -833,7 +833,21 @@ namespace yoi {
             IRStructTemplate{templateDefinition, templateMethods, templateArguments});
     }
 
-    IRTemplateBuilder &
+    IRInterfaceInstanceTemplate::IRInterfaceInstanceTemplate(const std::shared_ptr<IRInterfaceInstanceDefinition> &templateDefinition,
+                                             const yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArguments)
+        : templateDefinition(templateDefinition), templateArguments(templateArguments) {}
+
+    IRInterfaceInstanceTemplate::Builder &
+    IRInterfaceInstanceTemplate::Builder::setTemplateDefinition(const std::shared_ptr<IRInterfaceInstanceDefinition> &templateDefinition) {
+        this->templateDefinition = templateDefinition;
+        return *this;
+    }
+
+    std::shared_ptr<IRInterfaceInstanceTemplate> IRInterfaceInstanceTemplate::Builder::yield() {
+        return std::make_shared<IRInterfaceInstanceTemplate>(templateDefinition, templateArguments);
+    }
+
+    IRTemplateBuilder & 
     IRTemplateBuilder::addTemplateArgument(const yoi::wstr &templateName,
                                            const std::shared_ptr<IRValueType> &templateType,
                                            const std::pair<yoi::indexT, yoi::indexT> &interfaceType) {
diff --git a/compiler/ir/IR.h b/compiler/ir/IR.h
index 9eb89f3..7601fc9 100644
--- a/compiler/ir/IR.h
+++ b/compiler/ir/IR.h
@@ -551,6 +551,55 @@ namespace yoi {
         };
     };
 
+    class IRInterfaceInstanceTemplate {
+      public:
+        std::shared_ptr<IRInterfaceInstanceDefinition> templateDefinition;
+        yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> templateArguments;
+
+        struct Builder {
+            std::shared_ptr<IRInterfaceInstanceDefinition> templateDefinition;
+            yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> templateArguments;
+
+            Builder() = default;
+
+            Builder &setTemplateDefinition(const std::shared_ptr<IRInterfaceInstanceDefinition> &templateDefinition);
+
+            Builder &addTemplateArgument(const yoi::wstr &templateName,
+                                         const std::shared_ptr<IRValueType> &templateType,
+                                         const std::pair<yoi::indexT, yoi::indexT> &interfaceType = {0, 0});
+
+            std::shared_ptr<IRInterfaceInstanceTemplate> yield();
+        };
+
+        IRInterfaceInstanceTemplate(const std::shared_ptr<IRInterfaceInstanceDefinition> &templateDefinition,
+                            const yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArguments);
+    };
+
+    class IRInterfaceImplementationTemplate {
+      public:
+        std::shared_ptr<IRInterfaceImplementationDefinition> templateDefinition;
+        yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> templateArguments;
+
+        struct Builder {
+            std::shared_ptr<IRInterfaceImplementationDefinition> templateDefinition;
+            yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> templateArguments;
+
+            Builder() = default;
+
+            Builder &setTemplateDefinition(const std::shared_ptr<IRInterfaceImplementationDefinition> &templateDefinition);
+
+            Builder &addTemplateArgument(const yoi::wstr &templateName,
+                                         const std::shared_ptr<IRValueType> &templateType,
+                                         const std::pair<yoi::indexT, yoi::indexT> &interfaceType = {0, 0});
+
+            std::shared_ptr<IRInterfaceImplementationTemplate> yield();
+        };
+
+        IRInterfaceImplementationTemplate(
+            const std::shared_ptr<IRInterfaceImplementationDefinition> &templateDefinition,
+            const yoi::indexTable<yoi::wstr, IRTemplateBuilder::Argument> &templateArguments);
+    };
+
     class IRStringLiteralPool {
       public:
         yoi::indexPool<yoi::wstr> pool;
@@ -596,10 +645,14 @@ namespace yoi {
         yoi::indexTable<yoi::wstr, std::shared_ptr<IRInterfaceImplementationDefinition>> interfaceImplementationTable;
         yoi::indexTable<yoi::wstr, std::shared_ptr<IRFunctionTemplate>> functionTemplateTable;
         yoi::indexTable<yoi::wstr, std::shared_ptr<IRStructTemplate>> structTemplateTable;
+        yoi::indexTable<yoi::wstr, std::shared_ptr<IRInterfaceInstanceTemplate>> interfaceInstanceTemplateTable;
+        yoi::indexTable<yoi::wstr, std::shared_ptr<IRInterfaceImplementationTemplate>> interfaceImplTemplateTable;
 
         std::map<yoi::wstr, yoi::funcDefStmt *> funcTemplateAsts;
         std::map<yoi::wstr, yoi::structDefStmt *> structTemplateAsts;
+        std::map<yoi::wstr, yoi::interfaceDefStmt *> templateInterfaceAsts;
         std::map<yoi::wstr, yoi::implStmt *> templateImplAsts; // Maps struct template name to its impl block
+        std::map<yoi::wstr, yoi::vec<yoi::implStmt *>> templateInterfaceImplAsts;
 
         IRStringLiteralPool stringLiteralPool;
 
