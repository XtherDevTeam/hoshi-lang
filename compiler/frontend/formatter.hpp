//
// Created by XIaokang00010 on 2025/2/16.
//
#ifndef HOSHI_LANG_FORMATTER_HPP
#define HOSHI_LANG_FORMATTER_HPP

#include "compiler/compilerContext.h"
#include "lexer.hpp"
#include <share/def.hpp>

namespace yoi {
    struct FormatOption {
        enum class IndentType {
            Space,
            Tab
        } indentType{FormatOption::IndentType::Space};
        size_t indentSize{4};
        enum class BraceType {
            Attached,
            NewLine
        } braceType{FormatOption::BraceType::Attached};
        size_t maxWidth{80};

        FormatOption() = default;

        FormatOption(IndentType indentType, size_t indentSize, BraceType braceType, size_t maxWidth = 80);
    };

    void formatToken(std::wostream &os, FormatOption option, const lexer::token &token);

    class Formatter {
      public:
        FormatOption option;
        std::wostream &os;
        vec<lexer::Comment> comments;

        Formatter(std::wostream &os, FormatOption option);

        Formatter(std::wostream &os, FormatOption option, vec<lexer::Comment> comments);

    private:
        size_t indentLevel{0};
        size_t currentColumn{0};
        size_t lastCommentIdx{0};
        uint64_t lastLine{0};
        void indent();
        void newLine();
        void write(const yoi::wstr &s);
        bool printComments(AST *node);
        bool printComments(uint64_t line, uint64_t col);
        bool willFit(invocationArguments *node);

    public:
        void format(const lexer::token &token);
        void format(basicLiterals *node);

        void format(identifier *node);

        void format(identifierWithTypeSpec *node);

        void format(defTemplateArgSpec *node);

        void format(defTemplateArg *node);

        void format(templateArgSpec *node);

        void format(templateArg *node);

        void format(invocationArguments *node);

        void format(definitionArguments *node);

        void format(funcTypeSpec *node);

        void format(typeSpec *node);

        void format(decltypeExpr *node);

        void format(subscript *node);

        void format(identifierWithTemplateArg *node);

        void format(identifierWithDefTemplateArg *node);

        void format(subscriptExpr *node);

        void format(memberExpr *node);

        void format(primary *node);

        void format(uniqueExpr *node);

        void format(mulExpr *node);

        void format(addExpr *node);

        void format(shiftExpr *node);

        void format(relationalExpr *node);

        void format(equalityExpr *node);

        void format(andExpr *node);

        void format(exclusiveExpr *node);

        void format(inclusiveExpr *node);

        void format(logicalAndExpr *node);

        void format(logicalOrExpr *node);

        void format(rExpr *node);

        void format(codeBlock *node);

        void format(useStmt *node);

        void format(funcDefStmt *node);

        void format(interfaceDefInnerPair *node);

        void format(interfaceDefInner *node);

        void format(interfaceDefStmt *node);

        void format(structDefInnerPair *node);

        void format(structDefInner *node);

        void format(structDefStmt *node);

        void format(dataStructDefStmt *node);

        void format(implInnerPair *node);

        void format(implInner *node);

        void format(implStmt *node);

        void format(letAssignmentPair *node);

        void format(letStmt *node);

        void format(globalStmt *node);

        void format(ifStmt *node);

        void format(whileStmt *node);

        void format(forStmt *node);

        void format(forEachStmt *node);

        void format(returnStmt *node);

        void format(continueStmt *node);

        void format(breakStmt *node);

        void format(inCodeBlockStmt *node);

        void format(leftExpr *node);

        void format(externModuleAccessExpression *node);

        void format(exportDecl *node);

        void format(importDecl *node);

        void format(importInner *node);

        void format(throwStmt *node);

        void format(catchParam *node);

        void format(tryCatchStmt *node);

        void format(dynCastExpression *node);

        void format(typeIdExpression *node);

        void format(newExpression *node);

        void format(abstractExpr *node);

        void format(lambdaExpr *node);

        void format(unnamedDefinitionArguments *node);

        void format(marcoPair *node);

        void format(marcoDescriptor *node);

        void format(typeAliasStmt *node);

        void format(finalizerDef *node);

        void format(finalizerDecl *node);

        void format(funcExpr *node);

        void format(letAssignmentPairLHS *node);

        void format(enumerationDefinition *node);

        void format(enumerationPair *node);

        void format(bracedInitalizerList *node);

        void format(hoshiModule *node);

        void format(innerMethodDecl *node);

        void format(innerMethodDef *node);

        void format(constructorDecl *node);

        void format(constructorDef *node);

        void format(yieldStmt *node);
    };
}

#endif