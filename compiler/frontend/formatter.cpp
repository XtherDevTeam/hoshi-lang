//
// Created by XIaokang00010 on 2025/2/16.
//

#include "formatter.hpp"
#include "compiler/compilerContext.h"
#include "compiler/frontend/lexer.hpp"
#include "share/def.hpp"
#include <ostream>

yoi::FormatOption::FormatOption(IndentType indentType, size_t indentSize, BraceType braceType, size_t maxWidth)
        : indentType(indentType), indentSize(indentSize), braceType(braceType), maxWidth(maxWidth) {}

void yoi::formatToken(std::wostream &os, FormatOption option, const lexer::token &token) {
    switch (token.kind) {
        case lexer::token::tokenKind::unknown:
            break;
        case lexer::token::tokenKind::identifier:
            os << token.strVal;
            break;
        case lexer::token::tokenKind::character:
            os << '\'' << yoi::escapeString(token.strVal) << '\'';
            break;
        case lexer::token::tokenKind::string:
            os << '\"' << yoi::escapeString(token.strVal) << '\"';
            break;
        case lexer::token::tokenKind::integer:
            os << token.basicVal.vInt;
            break;
        case lexer::token::tokenKind::unsignedInt:
            os << token.basicVal.vUint << "u";
            break;
        case lexer::token::tokenKind::shortInt:
            os << token.basicVal.vShort << "s";
            break;
        case lexer::token::tokenKind::decimal:
            os << token.basicVal.vDeci;
            break;
        case lexer::token::tokenKind::boolean:
            os << (token.basicVal.vBool ? L"true" : L"false");
            break;
        case lexer::token::tokenKind::toSign:
            os << L"->";
            break;
        case lexer::token::tokenKind::plus:
            os << L"+";
            break;
        case lexer::token::tokenKind::minus:
            os << L"-";
            break;
        case lexer::token::tokenKind::asterisk:
            os << L"*";
            break;
        case lexer::token::tokenKind::slash:
            os << L"/";
            break;
        case lexer::token::tokenKind::percentSign:
            os << L"%";
            break;
        case lexer::token::tokenKind::binaryXor:
            os << L"^";
            break;
        case lexer::token::tokenKind::binaryOr:
            os << L"|";
            break;
        case lexer::token::tokenKind::binaryAnd:
            os << L"&";
            break;
        case lexer::token::tokenKind::binaryNot:
            os << L"~";
            break;
        case lexer::token::tokenKind::logicNot:
            os << L"!";
            break;
        case lexer::token::tokenKind::incrementSign:
            os << L"++";
            break;
        case lexer::token::tokenKind::decrementSign:
            os << L"--";
            break;
        case lexer::token::tokenKind::binaryShiftLeft:
            os << L"<<";
            break;
        case lexer::token::tokenKind::binaryShiftRight:
            os << L">>";
            break;
        case lexer::token::tokenKind::additionAssignment:
            os << L"+=";
            break;
        case lexer::token::tokenKind::subtractionAssignment:
            os << L"-=";
            break;
        case lexer::token::tokenKind::multiplicationAssignment:
            os << L"*=";
            break;
        case lexer::token::tokenKind::divisionAssignment:
            os << L"/=";
            break;
        case lexer::token::tokenKind::reminderAssignment:
            os << L"%=";
            break;
        case lexer::token::tokenKind::greaterThan:
            os << L">";
            break;
        case lexer::token::tokenKind::lessThan:
            os << L"<";
            break;
        case lexer::token::tokenKind::greaterEqual:
            os << L">=";
            break;
        case lexer::token::tokenKind::lessEqual:
            os << L"<=";
            break;
        case lexer::token::tokenKind::equal:
            os << L"==";
            break;
        case lexer::token::tokenKind::notEqual:
            os << L"!=";
            break;
        case lexer::token::tokenKind::logicAnd:
            os << L"&&";
            break;
        case lexer::token::tokenKind::logicOr:
            os << L"||";
            break;
        case lexer::token::tokenKind::assignSign:
            os << L"=";
            break;
        case lexer::token::tokenKind::directAssignSign:
            os << L":=";
            break;
        case lexer::token::tokenKind::leftParentheses:
            os << L"(";
            break;
        case lexer::token::tokenKind::rightParentheses:
            os << L")";
            break;
        case lexer::token::tokenKind::leftBracket:
            os << L"[";
            break;
        case lexer::token::tokenKind::rightBracket:
            os << L"]";
            break;
        case lexer::token::tokenKind::leftBraces:
            os << L"{";
            break;
        case lexer::token::tokenKind::rightBraces:
            os << L"}";
            break;
        case lexer::token::tokenKind::semicolon:
            os << L";";
            break;
        case lexer::token::tokenKind::colon:
            os << L":";
            break;
        case lexer::token::tokenKind::comma:
            os << L",";
            break;
        case lexer::token::tokenKind::dot:
            os << L".";
            break;
        case lexer::token::tokenKind::sharp:
            os << L"#";
            break;
        case lexer::token::tokenKind::kYield:
            os << L"yield";
            break;
        case lexer::token::tokenKind::kUse:
            os << L"use";
            break;
        case lexer::token::tokenKind::kFunc:
            os << L"func";
            break;
        case lexer::token::tokenKind::kInterface:
            os << L"interface";
            break;
        case lexer::token::tokenKind::kConstructor:
            os << L"constructor";
            break;
        case lexer::token::tokenKind::kFinalizer:
            os << L"finalizer";
            break;
        case lexer::token::tokenKind::kStruct:
            os << L"struct";
            break;
        case lexer::token::tokenKind::kImpl:
            os << L"impl";
            break;
        case lexer::token::tokenKind::kLet:
            os << L"let";
            break;
        case lexer::token::tokenKind::kIn:
            os << L"in";
            break;
        case lexer::token::tokenKind::kFor:
            os << L"for";
            break;
        case lexer::token::tokenKind::kForEach:
            os << L"forEach";
            break;
        case lexer::token::tokenKind::kWhile:
            os << L"while";
            break;
        case lexer::token::tokenKind::kIf:
            os << L"if";
            break;
        case lexer::token::tokenKind::kElif:
            os << L"elif";
            break;
        case lexer::token::tokenKind::kElse:
            os << L"else";
            break;
        case lexer::token::tokenKind::kReturn:
            os << L"return";
            break;
        case lexer::token::tokenKind::kContinue:
            os << L"continue";
            break;
        case lexer::token::tokenKind::kBreak:
            os << L"break";
            break;
        case lexer::token::tokenKind::kCast:
            os << L"cast";
            break;
        case lexer::token::tokenKind::kNull:
            os << L"null";
            break;
        case lexer::token::tokenKind::kImport:
            os << L"import";
            break;
        case lexer::token::tokenKind::kExport:
            os << L"export";
            break;
        case lexer::token::tokenKind::kAs:
            os << L"as";
            break;
        case lexer::token::tokenKind::kFrom:
            os << L"from";
            break;
        case lexer::token::tokenKind::kTry:
            os << L"try";
            break;
        case lexer::token::tokenKind::kCatch:
            os << L"catch";
            break;
        case lexer::token::tokenKind::kFinally:
            os << L"finally";
            break;
        case lexer::token::tokenKind::kThrow:
            os << L"throw";
            break;
        case lexer::token::tokenKind::kTypeId:
            os << L"type_id";
            break;
        case lexer::token::tokenKind::kDynCast:
            os << L"dyn_cast";
            break;
        case lexer::token::tokenKind::kNoFFI:
            os << L"noffi";
            break;
        case lexer::token::tokenKind::kStatic:
            os << L"static";
            break;
        case lexer::token::tokenKind::kIntrinsic:
            os << L"intrinsic";
            break;
        case lexer::token::tokenKind::kAlwaysInline:
            os << L"always_inline";
            break;
        case lexer::token::tokenKind::kNew:
            os << L"new";
            break;
        case lexer::token::tokenKind::kCallable:
            os << L"callable";
            break;
        case lexer::token::tokenKind::kThreeDots:
            os << L"...";
            break;
        case lexer::token::tokenKind::kInterfaceOf:
            os << L"interfaceof";
            break;
        case lexer::token::tokenKind::kAlias:
            os << L"alias";
            break;
        case lexer::token::tokenKind::kEnum:
            os << L"enum";
            break;
        case lexer::token::tokenKind::kDataStruct:
            os << L"datastruct";
            break;
        case lexer::token::tokenKind::kDataField:
            os << L"datafield";
            break;
        case lexer::token::tokenKind::kGenerator:
            os << L"generator";
            break;
        case lexer::token::tokenKind::kDecltype:
            os << "decltype";
            break;
        case lexer::token::tokenKind::kConcept:
            os << "concept";
            break;
        case lexer::token::tokenKind::kSatisfy:
            os << "satisfy";
            break;
        case lexer::token::tokenKind::eof:
            break;
    }
}

yoi::Formatter::Formatter(std::wostream &os, FormatOption option) : os(os), option(option), lastLine(-1) {}

yoi::Formatter::Formatter(std::wostream &os, FormatOption option, vec<lexer::Comment> comments)
        : os(os), option(option), comments(std::move(comments)), lastLine(-1) {}

void yoi::Formatter::indent() {
    for (size_t i = 0; i < indentLevel * option.indentSize; ++i) {
        yoi::wstr s = (option.indentType == FormatOption::IndentType::Space ? L" " : L"\t");
        os << s;
        currentColumn += (option.indentType == FormatOption::IndentType::Space ? 1 : option.indentSize);
    }
}

void yoi::Formatter::newLine() {
    os << L"\n";
    currentColumn = 0;
    indent();
}

void yoi::Formatter::write(const yoi::wstr &s) {
    os << s;
    currentColumn += s.length();
}

bool yoi::Formatter::printComments(AST *node) {
    if (!node) return false;
    return printComments(node->getLine(), node->getColumn());
}

bool yoi::Formatter::printComments(uint64_t line, uint64_t col) {
    bool printedStandalone = false;
    while (lastCommentIdx < comments.size() &&
           (comments[lastCommentIdx].line < line ||
            (comments[lastCommentIdx].line == line && comments[lastCommentIdx].col < col))) {

        const auto &comment = comments[lastCommentIdx++];
        if (lastLine != (uint64_t)-1) {
            if (comment.line > lastLine) {
                newLine();
            } else {
                write(L" ");
            }
        }
        write(trim(comment.text));
        lastLine = comment.line;
        if (comment.line < line) {
            printedStandalone = true;
        }
    }
    if (printedStandalone) {
        newLine();
        return true;
    }
    return false;
}

void yoi::Formatter::format(const lexer::token &token) {
    std::wstringstream ss;
    yoi::formatToken(ss, option, token);
    write(ss.str());
}

bool yoi::Formatter::willFit(invocationArguments *node) {
    if (!node || option.maxWidth == (size_t)-1) return true;
    std::wstringstream ss;
    FormatOption tempOpt = option;
    tempOpt.maxWidth = (size_t)-1;
    Formatter temp(ss, tempOpt);
    temp.format(node);
    return currentColumn + ss.str().length() <= option.maxWidth;
}

void yoi::Formatter::format(basicLiterals *node) {
    if (!node) return;
    format(node->node);
}

void yoi::Formatter::format(identifier *node) {
    if (!node) return;
    format(node->node);
}

void yoi::Formatter::format(identifierWithTypeSpec *node) {
    if (!node) return;
    format(node->id);
    write(L": ");
    format(node->spec);
}

void yoi::Formatter::format(defTemplateArgSpec *node) {
    if (!node) return;
    format(node->id);
    if (node->satisfyCondition) {
        os << " ";
        format(node->satisfyCondition);
    }
}

void yoi::Formatter::format(defTemplateArg *node) {
    if (!node || node->spec.empty()) return;
    write(L"<");
    for (size_t i = 0; i < node->spec.size(); ++i) {
        format(node->spec[i]);
        if (i < node->spec.size() - 1) write(L", ");
    }
    write(L">");
}

void yoi::Formatter::format(templateArgSpec *node) {
    if (!node) return;
    format(node->spec);
}

void yoi::Formatter::format(templateArg *node) {
    if (!node || node->spec.empty()) return;
    write(L"<");
    for (size_t i = 0; i < node->spec.size(); ++i) {
        format(node->spec[i]);
        if (i < node->spec.size() - 1) write(L", ");
    }
    write(L">");
}

void yoi::Formatter::format(invocationArguments *node) {
    if (!node) return;
    if (willFit(node)) {
        write(L"(");
        for (size_t i = 0; i < node->arg.size(); ++i) {
            format(node->arg[i]);
            if (i < node->arg.size() - 1) write(L", ");
        }
        write(L")");
    } else {
        write(L"(");
        indentLevel++;
        for (size_t i = 0; i < node->arg.size(); ++i) {
            newLine();
            format(node->arg[i]);
            if (i < node->arg.size() - 1) write(L",");
        }
        indentLevel--;
        newLine();
        write(L")");
    }
}

void yoi::Formatter::format(definitionArguments *node) {
    if (!node) return;
    // For simplicity, we use the same willFit logic (invocationArguments is used for measurement)
    // but in a real implementation we'd have a more generic measure function.
    write(L"(");
    for (size_t i = 0; i < node->spec.size(); ++i) {
        format(node->spec[i]);
        if (i < node->spec.size() - 1) write(L", ");
    }
    write(L")");
}

void yoi::Formatter::format(funcTypeSpec *node) {
    if (!node) return;
    write(L"func");
    format(node->args);
    write(L" : ");
    format(node->resultType);
}

void yoi::Formatter::format(typeSpec *node) {
    if (!node) return;
    if (node->kind == typeSpec::typeSpecKind::Elipsis) {
        write(L"...");
        format(node->elipsis);
    } else if (node->isNull) {
        write(L"null");
    } else if (node->kind == typeSpec::typeSpecKind::Member) {
        format(node->member);
    } else if (node->kind == typeSpec::typeSpecKind::Func) {
        format(node->func);
    } else if (node->kind == typeSpec::typeSpecKind::DecltypeExpr) {
        format(node->decltypeExpression);
    }
    
    if (node->arraySubscript) {
        for (auto val : *node->arraySubscript) {
            write(L"[");
            if (val != (uint64_t)-1) write(yoi::string2wstring(std::to_string(val)));
            write(L"]");
        }
    }
}

void yoi::Formatter::format(subscript *node) {
    if (!node) return;
    if (node->isInvocation()) {
        format(node->args);
    } else {
        write(L"[");
        format(node->expr);
        write(L"]");
    }
}

void yoi::Formatter::format(identifierWithTemplateArg *node) {
    if (!node) return;
    format(node->id);
    if (node->hasTemplateArg()) {
        format(node->arg);
    }
}

void yoi::Formatter::format(identifierWithDefTemplateArg *node) {
    if (!node) return;
    format(node->id);
    if (node->hasDefTemplateArg()) {
        format(node->arg);
    }
}

void yoi::Formatter::format(subscriptExpr *node) {
    if (!node) return;
    format(node->id);
    for (auto s : node->subscriptVal) {
        format(s);
    }
}

void yoi::Formatter::format(memberExpr *node) {
    if (!node) return;
    for (size_t i = 0; i < node->terms.size(); ++i) {
        format(node->terms[i]);
        if (i < node->terms.size() - 1) write(L".");
    }
}

void yoi::Formatter::format(primary *node) {
    if (!node) return;
    switch (node->kind) {
        case primary::primaryKind::memberExpr: format(node->member); break;
        case primary::primaryKind::basicLiterals: format(node->literals); break;
        case primary::primaryKind::rExpr: 
            write(L"(");
            format(node->expr);
            write(L")");
            break;
        case primary::primaryKind::typeIdExpression: format(node->typeId); break;
        case primary::primaryKind::dynCastExpression: format(node->dynCast); break;
        case primary::primaryKind::newExpression: format(node->newExpr); break;
        case primary::primaryKind::lambdaExpr: format(node->lambda); break;
        case primary::primaryKind::funcExpr: format(node->func); break;
        case primary::primaryKind::bracedInitalizerList: format(node->bracedInitalizer); break;
    }
}

void yoi::Formatter::format(abstractExpr *node) {
    if (!node) return;
    format(node->lhs);
    if (node->op.kind != lexer::token::tokenKind::unknown) {
        write(L" ");
        format(node->op);
        write(L" ");
        format(node->rhs);
    }
}

void yoi::Formatter::format(uniqueExpr *node) {
    if (!node) return;
    if (node->op.kind != lexer::token::tokenKind::unknown) {
        format(node->op);
    }
    format(node->lhs);
}

void yoi::Formatter::format(leftExpr *node) {
    if (!node) return;
    format(node->lhs);
    if (node->hasRhs()) {
        write(L" ");
        format(node->op);
        write(L" ");
        format(node->rhs);
    }
}

#define FORMAT_BINARY_EXPR(NODE_TYPE) \
void yoi::Formatter::format(NODE_TYPE *node) { \
    if (!node) return; \
    for (size_t i = 0; i < node->terms.size(); ++i) { \
        format(node->terms[i]); \
        if (i < node->ops.size()) { \
            write(L" "); \
            format(node->ops[i]); \
            write(L" "); \
        } \
    } \
}

FORMAT_BINARY_EXPR(mulExpr)
FORMAT_BINARY_EXPR(addExpr)
FORMAT_BINARY_EXPR(shiftExpr)
FORMAT_BINARY_EXPR(relationalExpr)
FORMAT_BINARY_EXPR(equalityExpr)
FORMAT_BINARY_EXPR(andExpr)
FORMAT_BINARY_EXPR(exclusiveExpr)
FORMAT_BINARY_EXPR(inclusiveExpr)
FORMAT_BINARY_EXPR(logicalAndExpr)
FORMAT_BINARY_EXPR(logicalOrExpr)

void yoi::Formatter::format(rExpr *node) {
    if (!node) return;
    format(node->expr);
}

void yoi::Formatter::format(externModuleAccessExpression *node) {
    if (!node) return;
    for (size_t i = 0; i < node->terms.size(); ++i) {
        format(node->terms[i]);
        if (i < node->terms.size() - 1) write(L".");
    }
}

void yoi::Formatter::format(codeBlock *node) {
    if (!node) return;
    if (option.braceType == FormatOption::BraceType::NewLine) {
        newLine();
    } else {
        os << L" ";
    }
    os << L"{";
    indentLevel++;
    for (auto stmt : node->stmts) {
        if (!printComments(stmt)) {
            newLine();
        }
        format(stmt);
        lastLine = std::max(lastLine, stmt->getLine());
        printComments(stmt->getLine(), -1);
    }
    indentLevel--;
    newLine();
    os << L"}";
}

void yoi::Formatter::format(ifStmt *node) {
    if (!node) return;
    write(L"if (");
    format(node->ifB.cond);
    write(L")");
    format(node->ifB.block);
    for (auto &elif : node->elifB) {
        write(L" elif (");
        format(elif.cond);
        write(L")");
        format(elif.block);
    }
    if (node->hasElseBlock()) {
        write(L" else");
        format(node->elseB);
    }
}

void yoi::Formatter::format(whileStmt *node) {
    if (!node) return;
    os << L"while (";
    format(node->cond);
    os << L")";
    format(node->block);
}

void yoi::Formatter::format(forStmt *node) {
    if (!node) return;
    os << L"for (";
    format(node->initStmt);
    os << L"; ";
    format(node->cond);
    os << L"; ";
    format(node->afterStmt);
    os << L")";
    format(node->block);
}

void yoi::Formatter::format(forEachStmt *node) {
    if (!node) return;
    os << L"forEach (";
    format(node->var);
    os << L" : ";
    format(node->container);
    os << L")";
    format(node->block);
}

void yoi::Formatter::format(returnStmt *node) {
    if (!node) return;
    os << L"return";
    if (node->hasValue()) {
        os << L" ";
        format(node->value);
    }
}

void yoi::Formatter::format(continueStmt *node) {
    if (!node) return;
    os << L"continue";
}

void yoi::Formatter::format(breakStmt *node) {
    if (!node) return;
    os << L"break";
}

void yoi::Formatter::format(inCodeBlockStmt *node) {
    if (!node) return;
    if (node->marco) {
        format(node->marco);
    }
    switch (node->kind) {
        case inCodeBlockStmt::vKind::ifStmt: format(node->value.ifStmtVal); break;
        case inCodeBlockStmt::vKind::whileStmt: format(node->value.whileStmtVal); break;
        case inCodeBlockStmt::vKind::forStmt: format(node->value.forStmtVal); break;
        case inCodeBlockStmt::vKind::forEachStmt: format(node->value.forEachStmtVal); break;
        case inCodeBlockStmt::vKind::returnStmt: format(node->value.returnStmtVal); break;
        case inCodeBlockStmt::vKind::continueStmt: format(node->value.continueStmtVal); break;
        case inCodeBlockStmt::vKind::breakStmt: format(node->value.breakStmtVal); break;
        case inCodeBlockStmt::vKind::letStmt: format(node->value.letStmtVal); break;
        case inCodeBlockStmt::vKind::codeBlock: format(node->value.codeBlockVal); break;
        case inCodeBlockStmt::vKind::tryCatchStmt: format((tryCatchStmt *)node->value.ptr); break;
        case inCodeBlockStmt::vKind::throwStmt: format((throwStmt *)node->value.ptr); break;
        case inCodeBlockStmt::vKind::rExpr: format(node->value.rExprVal); break;
        case inCodeBlockStmt::vKind::yieldStmt: format(node->value.yieldStmtVal); break;
    }
}

void yoi::Formatter::format(useStmt *node) {
    if (!node) return;
    os << L"use ";
    format(node->name);
    os << L" \"";
    os << yoi::escapeString(node->path.strVal);
    os << L"\"";
}

void yoi::Formatter::format(funcDefStmt *node) {
    if (!node) return;
    os << L"func ";
    for (auto &attr : node->attrs) {
        formatToken(os, option, attr);
        os << L" ";
    }
    format(node->id);
    format(node->args);
    os << L" : ";
    format(node->resultType);
    format(node->block);
}

void yoi::Formatter::format(interfaceDefInnerPair *node) {
    if (!node) return;
    if (node->isMethod()) {
        format(node->method);
    } else {
        format(node->var);
    }
}

void yoi::Formatter::format(interfaceDefInner *node) {
    if (!node) return;
    if (option.braceType == FormatOption::BraceType::NewLine) {
        newLine();
    } else {
        os << L" ";
    }
    os << L"{";
    indentLevel++;
    for (auto pair : node->inner) {
        if (!printComments(pair)) {
            newLine();
        }
        format(pair);
        lastLine = std::max(lastLine, pair->getLine());
        printComments(pair->getLine(), -1);
    }
    indentLevel--;
    newLine();
    os << L"}";
}

void yoi::Formatter::format(interfaceDefStmt *node) {
    if (!node) return;
    write(L"interface ");
    format(node->id);
    format(node->inner);
}

void yoi::Formatter::format(structDefInnerPair *node) {
    if (!node) return;
    switch (node->kind) {
        case 0: 
            if (node->modifier == structDefInnerPair::Modifier::DataField) write(L"datafield ");
            format(node->var); 
            break;
        case 1: format(node->con); break;
        case 2: format(node->method); break;
        case 3: format(node->finalizer); break;
    }
}

void yoi::Formatter::format(structDefInner *node) {
    if (!node) return;
    if (option.braceType == FormatOption::BraceType::NewLine) {
        newLine();
    } else {
        os << L" ";
    }
    os << L"{";
    indentLevel++;
    for (auto it = node->inner.begin(); it != node->inner.end(); it++) {
        auto pair = *it;
        if (!printComments(pair)) {
            newLine();
        }
        format(pair);
        if (it + 1 != node->inner.end()) {
            write(L",");
        }
        lastLine = std::max(lastLine, pair->getLine());
        printComments(pair->getLine(), -1);
    }
    indentLevel--;
    newLine();
    os << L"}";
}

void yoi::Formatter::format(structDefStmt *node) {
    if (!node) return;
    write(L"struct ");
    format(node->id);
    format(node->inner);
}

void yoi::Formatter::format(dataStructDefStmt *node) {
    if (!node) return;
    write(L"datastruct ");
    format(node->id);
    format(node->inner);
}

void yoi::Formatter::format(implInnerPair *node) {
    if (!node) return;
    if (node->isConstructor()) format(node->con);
    else if (node->isMethod()) format(node->met);
    else if (node->isFinalizer()) format(node->finalizer);
}

void yoi::Formatter::format(implInner *node) {
    if (!node) return;
    if (option.braceType == FormatOption::BraceType::NewLine) {
        newLine();
    } else {
        os << L" ";
    }
    os << L"{";
    indentLevel++;
    for (auto pair : node->inner) {
        if (!printComments(pair)) {
            newLine();
        }
        format(pair);
        lastLine = std::max(lastLine, pair->getLine());
        printComments(pair->getLine(), -1);
    }
    indentLevel--;
    newLine();
    os << L"}";
}

void yoi::Formatter::format(implStmt *node) {
    if (!node) return;
    write(L"impl ");
    if (node->interfaceName) {
        format(node->structName);
        write(L" : ");
        format(node->interfaceName);
    } else {
        format(node->structName);
    }
    format(node->inner);
}

void yoi::Formatter::format(letAssignmentPair *node) {
    if (!node) return;
    format(node->lhs);
    if (node->type) {
        os << L" : ";
        format(node->type);
    }
    if (node->rhs) {
        os << L" = ";
        format(node->rhs);
    }
}

void yoi::Formatter::format(letStmt *node) {
    if (!node) return;
    os << L"let ";
    for (size_t i = 0; i < node->terms.size(); ++i) {
        format(node->terms[i]);
        if (i < node->terms.size() - 1) os << L", ";
    }
}

void yoi::Formatter::format(globalStmt *node) {
    if (!node) return;
    if (node->marco) format(node->marco);
    switch (node->kind) {
        case globalStmt::vKind::useStmt: format(node->value.useStmtVal); break;
        case globalStmt::vKind::funcDefStmt: format(node->value.funcDefStmtVal); break;
        case globalStmt::vKind::interfaceDefStmt: format(node->value.interfaceDefStmtVal); break;
        case globalStmt::vKind::structDefStmt: format(node->value.structDefStmtVal); break;
        case globalStmt::vKind::dataStructDefStmt: format(node->value.dataStructDefStmtVal); break;
        case globalStmt::vKind::implStmt: format(node->value.implStmtVal); break;
        case globalStmt::vKind::letStmt: format(node->value.letStmtVal); break;
        case globalStmt::vKind::importDecl: format(node->value.importDeclVal); break;
        case globalStmt::vKind::exportDecl: format(node->value.exportDeclVal); break;
        case globalStmt::vKind::typeAliasStmt: format(node->value.typeAliasStmtVal); break;
        case globalStmt::vKind::enumerationDef: format(node->value.enumerationDefVal); break;
        case globalStmt::vKind::conceptDef: format(node->value.conceptDefVal); break;
    }
}

void yoi::Formatter::format(exportDecl *node) {
    if (!node) return;
    for (auto &attr : node->attrs) {
        formatToken(os, option, attr);
        os << L" ";
    }
    os << L"export ";
    format(node->from);
    if (node->as) {
        os << L" as ";
        format(node->as);
    }
}

void yoi::Formatter::format(importDecl *node) {
    if (!node) return;
    os << L"import ";
    format(node->inner);
    os << L" from \"";
    os << node->from_path.strVal;
    os << L"\"";
}

void yoi::Formatter::format(importInner *node) {
}

void yoi::Formatter::format(throwStmt *node) {
    if (!node) return;
    os << L"throw ";
    format(node->expr);
}

void yoi::Formatter::format(catchParam *node) {
    if (!node) return;
    os << L"catch (";
    format(node->name);
    os << L": ";
    format(node->type);
    os << L")";
    format(node->block);
}

void yoi::Formatter::format(tryCatchStmt *node) {
    if (!node) return;
    os << L"try";
    format(node->tryBlock);
    for (auto cp : node->catchParams) {
        os << L" ";
        format(cp);
    }
    if (node->finallyBlock) {
        os << L" finally";
        format(node->finallyBlock);
    }
}

void yoi::Formatter::format(dynCastExpression *node) {
    if (!node) return;
    os << L"dyn_cast<";
    format(node->type);
    os << L">(";
    format(node->expr);
    os << L")";
}

void yoi::Formatter::format(typeIdExpression *node) {
    if (!node) return;
    os << L"type_id";
    if (node->type) {
        os << L"<";
        format(node->type);
        os << L">";
    }
    if (node->expr) {
        os << L"(";
        format(node->expr);
        os << L")";
    }
}

void yoi::Formatter::format(newExpression *node) {
    if (!node) return;
    os << L"new ";
    format(node->type);
    if (node->length) format(node->length);
    if (node->args) format(node->args);
}

void yoi::Formatter::format(lambdaExpr *node) {
    if (!node) return;
    os << L"func[";
    for (size_t i = 0; i < node->captures.size(); ++i) {
        format(node->captures[i]);
        if (i < node->captures.size() - 1) os << L", ";
    }
    os << L"] ";
    format(node->args);
    os << L" : ";
    format(node->resultType);
    format(node->block);
}

void yoi::Formatter::format(unnamedDefinitionArguments *node) {
    if (!node) return;
    write(L" (");
    for (size_t i = 0; i < node->types.size(); ++i) {
        format(node->types[i]);
        if (i < node->types.size() - 1) write(L", ");
    }
    write(L")");
}

void yoi::Formatter::format(marcoPair *node) {
    if (!node) return;
    os << node->identifier.strVal;
    if (node->constraint.kind != lexer::token::tokenKind::unknown) {
        os << L": " << node->constraint.strVal;
    }
    os << L" = " << node->rhs.strVal;
}

void yoi::Formatter::format(marcoDescriptor *node) {
    if (!node || node->pairs.empty()) return;
    os << L"#(";
    for (size_t i = 0; i < node->pairs.size(); ++i) {
        format(node->pairs[i]);
        if (i < node->pairs.size() - 1) os << L", ";
    }
    os << L") ";
}

void yoi::Formatter::format(typeAliasStmt *node) {
    if (!node) return;
    os << L"alias ";
    format(node->lhs);
    os << L" = ";
    format(node->rhs);
}

void yoi::Formatter::format(finalizerDef *node) {
    if (!node) return;
    os << L"finalizer";
    format(node->block);
}

void yoi::Formatter::format(finalizerDecl *node) {
    if (!node) return;
    os << L"finalizer";
}

void yoi::Formatter::format(funcExpr *node) {
    if (!node) return;
    os << L"func ";
    format(node->name);
    format(node->args);
}

void yoi::Formatter::format(letAssignmentPairLHS *node) {
    if (!node) return;
    if (node->kind == letAssignmentPairLHS::vKind::identifier) {
        format(node->id);
    } else {
        os << L"[";
        for (size_t i = 0; i < node->list.size(); ++i) {
            formatToken(os, option, node->list[i]);
            if (i < node->list.size() - 1) os << L", ";
        }
        os << L"]";
    }
}

void yoi::Formatter::format(enumerationDefinition *node) {
    if (!node) return;
    os << L"enum ";
    format(node->name);
    os << L" {";
    indentLevel++;
    for (size_t i = 0; i < node->values.size(); ++i) {
        if (!printComments(node->values[i])) {
            newLine();
        }
        format(node->values[i]);
        if (i < node->values.size() - 1) os << L",";
        lastLine = std::max(lastLine, node->values[i]->getLine());
        printComments(node->values[i]->getLine(), -1);
    }
    indentLevel--;
    newLine();
    os << L"}";
}

void yoi::Formatter::format(enumerationPair *node) {
    if (!node) return;
    format(node->name);
    if (node->value.kind != lexer::token::tokenKind::unknown) {
        os << L" = ";
        formatToken(os, option, node->value);
    }
}

void yoi::Formatter::format(bracedInitalizerList *node) {
    if (!node) return;
    os << L"{";
    for (size_t i = 0; i < node->exprs.size(); ++i) {
        format(node->exprs[i]);
        if (i < node->exprs.size() - 1) os << L", ";
    }
    os << L"}";
}

void yoi::Formatter::format(hoshiModule *node) {
    if (!node) return;
    for (auto it = node->stmts.begin(); it != node->stmts.end(); ++it) {
        if (it != node->stmts.begin()) {
            auto prev = std::prev(it);
            if (((*prev)->kind == globalStmt::vKind::useStmt || (*prev)->kind == globalStmt::vKind::importDecl) &&
                ((*it)->kind == globalStmt::vKind::useStmt || (*it)->kind == globalStmt::vKind::importDecl)) {
                os << L"\n";
                // indent();
            } else {
                os << L"\n\n";
                // indent();
            }
        }

        bool printedStandalone = printComments(*it);
        format(*it);
        lastLine = std::max(lastLine, (*it)->getLine());
        printComments((*it)->getLine(), -1);
    }
    printComments(-1, -1);
}

void yoi::Formatter::format(innerMethodDecl *node) {
    if (!node) return;
    for (auto &attr : node->attrs) {
        formatToken(os, option, attr);
        os << L" ";
    }
    format(node->name);
    format(node->args);
    if (node->resultType) {
        os << L" : ";
        format(node->resultType);
    }
}

void yoi::Formatter::format(innerMethodDef *node) {
    if (!node) return;
    for (auto &attr : node->attrs) {
        formatToken(os, option, attr);
        os << L" ";
    }
    format(node->name);
    format(node->args);
    if (node->resultType) {
        os << L" : ";
        format(node->resultType);
    }
    format(node->block);
}

void yoi::Formatter::format(constructorDecl *node) {
    if (!node) return;
    os << L"constructor";
    if (node->tempArgs) format(node->tempArgs);
    format(node->args);
}

void yoi::Formatter::format(constructorDef *node) {
    if (!node) return;
    os << L"constructor";
    if (node->tempArgs) format(node->tempArgs);
    format(node->args);
    format(node->block);
}

void yoi::Formatter::format(yieldStmt *node) {
    if (!node) return;
    os << L"yield ";
    format(node->expr);
}

void yoi::Formatter::format(decltypeExpr *node) {
    os << "decltype(";
    format(node->expr);
    os << ")";
}

void yoi::Formatter::format(conceptDefinition *node) {
    if (!node) return;
    os << L"concept " << node->name.strVal << L"<";
    for (auto it = node->typeParams.begin(); it != node->typeParams.end(); it++) {
        if (it != node->typeParams.begin()) {
            os << ", ";
        }
        os << it->strVal;
    }
    os << ">(";
    for (auto it = node->algebraParams.begin(); it != node->algebraParams.end(); it++) {
        if (it != node->algebraParams.begin()) {
            os << ", ";
        }
        format(*it);
    }
    os << ") {";
    indentLevel++;
    for (auto it = node->conceptBlock.begin(); it != node->conceptBlock.end(); it++) {
        newLine();
        format(*it);
    }
    indentLevel--;
    newLine();
    os << "}";
}

void yoi::Formatter::format(conceptStmt *node) {
    if (!node) return;
    switch (node->kind) {
        case conceptStmt::Kind::Expression: {
            format(node->value.expression);
            break;
        }
        case conceptStmt::Kind::SatisfyStmt: {
            format(node->value.satisfyStmt);
            break;
        }
        default:
            break;
    }
}

void yoi::Formatter::format(satisfyStmt *node) {
    os << L"satisfy ";
    format(node->emae);
}

void yoi::Formatter::format(satisfyClause *node) {
    os << L"satisfy(";
    for (auto it = node->emaes.begin(); it != node->emaes.end(); it++) {
        if (it != node->emaes.begin()) {
            os << ", ";
        }
        format(*it);
    }
    os << ")";
}
