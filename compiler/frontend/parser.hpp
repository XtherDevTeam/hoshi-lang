//
// Created by XIaokang00010 on 2023/2/11.
//

#ifndef HOSHI_LANG_PARSER_HPP
#define HOSHI_LANG_PARSER_HPP

#include "lexer.hpp"
#include "ast.hpp"

namespace yoi {
    void parse(basicLiterals *&o, lexer &lex);

    void parse(identifier *&o, lexer &lex);

    void parse(identifierWithTypeSpec *&o, lexer &lex);

    void parse(defTemplateArgSpec *&o, lexer &lex);

    void parse(defTemplateArg *&o, lexer &lex);

    void parse(templateArgSpec *&o, lexer &lex);

    void parse(templateArg *&o, lexer &lex);

    void parse(invocationArguments *&o, lexer &lex);

    void parse(definitionArguments *&o, lexer &lex);

    void parse(funcTypeSpec *&o, lexer &lex);

    void parse(typeSpec *&o, lexer &lex);

    void parse(subscript *&o, lexer &lex);

    void parse(identifierWithTemplateArg *&o, lexer &lex);

    void parse(identifierWithDefTemplateArg *&o, lexer &lex);

    void parse(externModuleAccessExpression *&o, lexer &lex);

    void parse(subscriptExpr *&o, lexer &lex);

    void parse(memberExpr *&o, lexer &lex);

    void parse(primary *&o, lexer &lex);

    void parse(uniqueExpr *&o, lexer &lex);

    void parse(leftExpr *&o, lexer &lex);

    void parse(mulExpr *&o, lexer &lex);

    void parse(addExpr *&o, lexer &lex);

    void parse(shiftExpr *&o, lexer &lex);

    void parse(relationalExpr *&o, lexer &lex);

    void parse(equalityExpr *&o, lexer &lex);

    void parse(andExpr *&o, lexer &lex);

    void parse(exclusiveExpr *&o, lexer &lex);

    void parse(inclusiveExpr *&o, lexer &lex);

    void parse(logicalAndExpr *&o, lexer &lex);

    void parse(logicalOrExpr *&o, lexer &lex);

    void parse(rExpr *&o, lexer &lex);

    void parse(codeBlock *&o, lexer &lex);

    void parse(useStmt *&o, lexer &lex);

    void parse(funcDefStmt *&o, lexer &lex);

    void parse(interfaceDefInnerPair *&o, lexer &lex);

    void parse(interfaceDefInner *&o, lexer &lex);

    void parse(interfaceDefStmt *&o, lexer &lex);

    void parse(structDefInnerPair *&o, lexer &lex);

    void parse(structDefInner *&o, lexer &lex);

    void parse(structDefStmt *&o, lexer &lex);

    void parse(implInnerPair *&o, lexer &lex);

    void parse(implInner *&o, lexer &lex);

    void parse(implStmt *&o, lexer &lex);

    void parse(letAssignmentPair *&o, lexer &lex);

    void parse(letStmt *&o, lexer &lex);

    void parse(globalStmt *&o, lexer &lex);

    void parse(ifStmt::ifBlock &o, lexer &lex);

    void parse(ifStmt *&o, lexer &lex);

    void parse(whileStmt *&o, lexer &lex);

    void parse(forStmt *&o, lexer &lex);

    void parse(forEachStmt *&o, lexer &lex);

    void parse(returnStmt *&o, lexer &lex);

    void parse(continueStmt *&o, lexer &lex);

    void parse(breakStmt *&o, lexer &lex);

    void parse(inCodeBlockStmt *&o, lexer &lex);

    void parse(innerMethodDecl *&o, lexer &lex);

    void parse(innerMethodDef *&o, lexer &lex);

    void parse(constructorDecl *&o, lexer &lex);

    void parse(constructorDef *&o, lexer &lex);

    void parse(hoshiModule *&o, lexer &lex);

    void parse(importInner *&o, lexer &lex);

    void parse(importDecl *&o, lexer &lex);

    void parse(exportDecl *&o, lexer &lex);
}

#endif //HOSHI_LANG_PARSER_HPP