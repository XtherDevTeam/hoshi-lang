//
// Symbol Extractor implementation — walks AST and collects symbols
//

#include "symbolExtractor.h"
#include <share/def.hpp>

namespace lsp {

static yoi::wstr tokenStrVal(yoi::lexer::token &tok) {
    return tok.strVal;
}

yoi::vec<Symbol> SymbolExtractor::extract(yoi::hoshiModule *module) {
    symbols.clear();
    if (!module) return symbols;

    for (auto *stmt : module->stmts) {
        visitGlobal(stmt);
    }

    return symbols;
}

void SymbolExtractor::visitGlobal(yoi::globalStmt *stmt) {
    if (!stmt) return;

    switch (stmt->kind) {
        case yoi::globalStmt::vKind::funcDefStmt:
            visitFuncDef(static_cast<yoi::funcDefStmt *>(stmt->value.ptr));
            break;
        case yoi::globalStmt::vKind::structDefStmt:
            visitStructDef(static_cast<yoi::structDefStmt *>(stmt->value.ptr));
            break;
        case yoi::globalStmt::vKind::interfaceDefStmt:
            visitInterfaceDef(static_cast<yoi::interfaceDefStmt *>(stmt->value.ptr));
            break;
        case yoi::globalStmt::vKind::letStmt:
            visitLetStmt(static_cast<yoi::letStmt *>(stmt->value.ptr));
            break;
        case yoi::globalStmt::vKind::typeAliasStmt:
            visitTypeAlias(static_cast<yoi::typeAliasStmt *>(stmt->value.ptr));
            break;
        case yoi::globalStmt::vKind::useStmt:
            visitUse(static_cast<yoi::useStmt *>(stmt->value.ptr));
            break;
        case yoi::globalStmt::vKind::importDecl:
            visitImport(static_cast<yoi::importDecl *>(stmt->value.ptr));
            break;
        case yoi::globalStmt::vKind::exportDecl:
            visitExport(static_cast<yoi::exportDecl *>(stmt->value.ptr));
            break;
        case yoi::globalStmt::vKind::implStmt:
            visitImpl(static_cast<yoi::implStmt *>(stmt->value.ptr));
            break;
        case yoi::globalStmt::vKind::dataStructDefStmt:
            visitDataStruct(static_cast<yoi::dataStructDefStmt *>(stmt->value.ptr));
            break;
        case yoi::globalStmt::vKind::enumerationDef:
            visitEnum(static_cast<yoi::enumerationDefinition *>(stmt->value.ptr));
            break;
        case yoi::globalStmt::vKind::conceptDef:
            visitConceptDef(static_cast<yoi::conceptDefinition *>(stmt->value.ptr));
            break;
        default:
            break;
    }
}

void SymbolExtractor::visitFuncDef(yoi::funcDefStmt *stmt) {
    if (!stmt || !stmt->id || !stmt->id->id) return;

    Symbol sym;
    sym.kind = HoshiSymbolKind::Function;
    sym.line = stmt->getLine();
    sym.column = stmt->getColumn();
    sym.name = stmt->id->id->node.strVal;

    // Build detail: "func name(args): returnType"
    yoi::wstr detail = L"func ";
    detail += formatIdentifierWithDefTemplateArg(stmt->id);
    if (stmt->args) {
        detail += formatDefinitionArgs(stmt->args);
    }
    if (stmt->resultType) {
        detail += L": ";
        detail += formatTypeSpec(stmt->resultType);
    }
    sym.detail = detail;

    if (stmt->resultType) {
        sym.typeInfo = formatTypeSpec(stmt->resultType);
    }

    symbols.push_back(sym);

    // Visit function body for local symbols
    if (stmt->block) {
        for (auto *cbStmt : stmt->block->stmts) {
            visitInCodeBlock(cbStmt, sym.name);
        }
    }
}

void SymbolExtractor::visitStructDef(yoi::structDefStmt *stmt) {
    if (!stmt || !stmt->id || !stmt->id->id) return;

    Symbol sym;
    sym.kind = HoshiSymbolKind::Struct;
    sym.line = stmt->getLine();
    sym.column = stmt->getColumn();
    sym.name = stmt->id->id->node.strVal;
    sym.detail = L"struct " + formatIdentifierWithDefTemplateArg(stmt->id);

    // Extract fields and methods as children
    if (stmt->inner) {
        for (auto *pair : stmt->inner->inner) {
            if (!pair) continue;

            Symbol child;
            child.parentName = sym.name;

            if (pair->kind == 0 && pair->var) {
                // Field
                child.kind = HoshiSymbolKind::Field;
                child.line = pair->getLine();
                child.column = pair->getColumn();
                child.name = tokenStrVal(pair->var->id->node);
                child.typeInfo = formatTypeSpec(pair->var->spec);
                child.detail = child.name + L": " + child.typeInfo;
            } else if (pair->kind == 1 && pair->con) {
                // Constructor
                child.kind = HoshiSymbolKind::Constructor;
                child.line = pair->getLine();
                child.column = pair->getColumn();
                child.name = L"constructor";
                child.detail = L"constructor" + formatDefinitionArgs(pair->con->args);
            } else if (pair->kind == 2 && pair->method) {
                // Method
                child.kind = HoshiSymbolKind::Method;
                child.line = pair->getLine();
                child.column = pair->getColumn();
                child.name = tokenStrVal(pair->method->name->id->node);
                child.detail = L"func " + child.name + formatDefinitionArgs(pair->method->args);
                if (pair->method->resultType) {
                    child.detail += L": ";
                    child.detail += formatTypeSpec(pair->method->resultType);
                    child.typeInfo = formatTypeSpec(pair->method->resultType);
                }
            } else if (pair->kind == 3 && pair->finalizer) {
                // Finalizer
                child.kind = HoshiSymbolKind::Finalizer;
                child.line = pair->getLine();
                child.column = pair->getColumn();
                child.name = L"finalizer";
                child.detail = L"finalizer()";
            }

            if (!child.name.empty())
                sym.children.push_back(child);
        }
    }

    symbols.push_back(sym);
}

void SymbolExtractor::visitInterfaceDef(yoi::interfaceDefStmt *stmt) {
    if (!stmt || !stmt->id || !stmt->id->id) return;

    Symbol sym;
    sym.kind = HoshiSymbolKind::Interface;
    sym.line = stmt->getLine();
    sym.column = stmt->getColumn();
    sym.name = stmt->id->id->node.strVal;
    sym.detail = L"interface " + formatIdentifierWithDefTemplateArg(stmt->id);

    // Extract methods
    if (stmt->inner) {
        for (auto *pair : stmt->inner->inner) {
            if (!pair) continue;

            Symbol child;
            child.parentName = sym.name;

            if (pair->method) {
                child.kind = HoshiSymbolKind::Method;
                child.line = pair->getLine();
                child.column = pair->getColumn();
                child.name = tokenStrVal(pair->method->name->id->node);
                child.detail = L"func " + child.name + formatDefinitionArgs(pair->method->args);
                if (pair->method->resultType) {
                    child.detail += L": ";
                    child.detail += formatTypeSpec(pair->method->resultType);
                    child.typeInfo = formatTypeSpec(pair->method->resultType);
                }
            } else if (pair->var) {
                child.kind = HoshiSymbolKind::Field;
                child.line = pair->getLine();
                child.column = pair->getColumn();
                child.name = tokenStrVal(pair->var->id->node);
                child.typeInfo = formatTypeSpec(pair->var->spec);
                child.detail = child.name + L": " + child.typeInfo;
            }

            if (!child.name.empty())
                sym.children.push_back(child);
        }
    }

    symbols.push_back(sym);
}

// Try to extract a type name from an initializer expression.
// For "str.Str(...)" → returns "Str"
// For other expressions → returns empty
static yoi::wstr tryExtractTypeFromExpr(yoi::rExpr *expr) {
    if (!expr || !expr->expr) return L"";

    // Walk down the single-term expression chain to reach the primary
    yoi::logicalOrExpr *lor = expr->expr;
    if (!lor || lor->terms.empty()) return L"";
    yoi::logicalAndExpr *land = lor->terms.back();
    if (!land || land->terms.empty()) return L"";
    yoi::inclusiveExpr *inc = land->terms.back();
    if (!inc || inc->terms.empty()) return L"";
    yoi::exclusiveExpr *exc = inc->terms.back();
    if (!exc || exc->terms.empty()) return L"";
    yoi::andExpr *andE = exc->terms.back();
    if (!andE || andE->terms.empty()) return L"";
    yoi::equalityExpr *eq = andE->terms.back();
    if (!eq || eq->terms.empty()) return L"";
    yoi::relationalExpr *rel = eq->terms.back();
    if (!rel || rel->terms.empty()) return L"";
    yoi::shiftExpr *sh = rel->terms.back();
    if (!sh || sh->terms.empty()) return L"";
    yoi::addExpr *add = sh->terms.back();
    if (!add || add->terms.empty()) return L"";
    yoi::mulExpr *mul = add->terms.back();
    if (!mul || mul->terms.empty()) return L"";
    yoi::leftExpr *left = mul->terms.back();
    if (!left || !left->lhs) return L"";
    yoi::uniqueExpr *unq = left->lhs;
    if (!unq || !unq->lhs) return L"";
    yoi::abstractExpr *abs = unq->lhs;
    if (!abs || !abs->lhs) return L"";
    yoi::primary *prim = abs->lhs;
    if (!prim) return L"";

    // Check for memberExpr (e.g., str.Str(...))
    if (prim->kind == yoi::primary::primaryKind::memberExpr && prim->member) {
        auto &terms = prim->member->terms;
        if (terms.empty()) return L"";
        auto *lastTerm = terms.back();
        if (!lastTerm || !lastTerm->id || !lastTerm->id->id) return L"";
        yoi::wstr name = lastTerm->id->id->node.strVal;
        // Constructor name starts with uppercase
        if (!name.empty() && std::iswupper(name[0])) {
            return name;
        }
    }

    return L"";
}

void SymbolExtractor::visitLetStmt(yoi::letStmt *stmt, const yoi::wstr &inParent) {
    if (!stmt) return;

    for (auto *pair : stmt->terms) {
        if (!pair || !pair->lhs) continue;

        Symbol sym;
        sym.kind = HoshiSymbolKind::Variable;
        sym.line = pair->getLine();
        sym.column = pair->getColumn();
        sym.parentName = inParent;

        if (pair->lhs->kind == yoi::letAssignmentPairLHS::vKind::identifier && pair->lhs->id) {
            sym.name = tokenStrVal(pair->lhs->id->node);
        } else if (!pair->lhs->list.empty()) {
            sym.name = pair->lhs->list[0].strVal;
        }

        if (pair->type) {
            sym.typeInfo = formatTypeSpec(pair->type);
            sym.detail = L"let " + sym.name + L": " + sym.typeInfo;
        } else {
            // Try to infer type from the initializer expression
            sym.typeInfo = tryExtractTypeFromExpr(pair->rhs);
            if (!sym.typeInfo.empty()) {
                sym.detail = L"let " + sym.name + L": " + sym.typeInfo;
            } else {
                sym.detail = L"let " + sym.name;
            }
        }

        if (!sym.name.empty())
            symbols.push_back(sym);
    }
}

void SymbolExtractor::visitTypeAlias(yoi::typeAliasStmt *stmt) {
    if (!stmt || !stmt->lhs || !stmt->lhs->id) return;

    Symbol sym;
    sym.kind = HoshiSymbolKind::TypeAlias;
    sym.line = stmt->getLine();
    sym.column = stmt->getColumn();
    sym.name = stmt->lhs->id->node.strVal;
    sym.detail = L"alias " + formatIdentifierWithDefTemplateArg(stmt->lhs);
    if (stmt->rhs) {
        sym.typeInfo = formatTypeSpec(stmt->rhs);
        sym.detail += L" = " + sym.typeInfo;
    }

    symbols.push_back(sym);
}

void SymbolExtractor::visitEnum(yoi::enumerationDefinition *stmt) {
    if (!stmt || !stmt->name) return;

    Symbol sym;
    sym.kind = HoshiSymbolKind::Enum;
    sym.line = stmt->getLine();
    sym.column = stmt->getColumn();
    sym.name = tokenStrVal(stmt->name->node);
    sym.detail = L"enum " + sym.name;

    for (auto *ep : stmt->values) {
        if (!ep || !ep->name) continue;
        Symbol child;
        child.kind = HoshiSymbolKind::EnumMember;
        child.parentName = sym.name;
        child.line = ep->getLine();
        child.column = ep->getColumn();
        child.name = tokenStrVal(ep->name->node);
        child.detail = sym.name + L"." + child.name;
        sym.children.push_back(child);
    }

    symbols.push_back(sym);
}

void SymbolExtractor::visitImport(yoi::importDecl *stmt) {
    if (!stmt || !stmt->inner || !stmt->inner->name || !stmt->inner->name->id) return;

    Symbol sym;
    sym.kind = HoshiSymbolKind::Import;
    sym.line = stmt->getLine();
    sym.column = stmt->getColumn();
    sym.name = tokenStrVal(stmt->inner->name->id->node);
    sym.detail = L"import " + sym.name + formatDefinitionArgs(stmt->inner->args)
               + L": " + formatTypeSpec(stmt->inner->resultType)
               + L" from " + stmt->from_path.strVal;
    sym.typeInfo = formatTypeSpec(stmt->inner->resultType);
    sym.importPath = stmt->from_path.strVal;  // Store path for cross-module resolution

    symbols.push_back(sym);
}

void SymbolExtractor::visitUse(yoi::useStmt *stmt) {
    if (!stmt || !stmt->name) return;

    Symbol sym;
    sym.kind = HoshiSymbolKind::ModuleAlias;
    sym.line = stmt->getLine();
    sym.column = stmt->getColumn();
    sym.name = tokenStrVal(stmt->name->node);
    sym.importPath = stmt->path.strVal;  // The module path string
    sym.detail = L"use " + sym.name + L" \"" + sym.importPath + L"\"";

    symbols.push_back(sym);
}

void SymbolExtractor::visitExport(yoi::exportDecl *stmt) {
    if (!stmt || !stmt->as) return;

    Symbol sym;
    sym.kind = HoshiSymbolKind::Export;
    sym.line = stmt->getLine();
    sym.column = stmt->getColumn();
    sym.name = tokenStrVal(stmt->as->node);
    sym.detail = L"export ";
    if (stmt->from) {
        sym.typeInfo = formatTypeSpec(stmt->from);
        sym.detail += sym.typeInfo;
    }
    sym.detail += L" as " + sym.name;

    symbols.push_back(sym);
}

void SymbolExtractor::visitImpl(yoi::implStmt *stmt) {
    if (!stmt) return;

    // For impl blocks, extract method implementations
    if (stmt->inner) {
        yoi::wstr structName = formatExternModuleAccessExpression(stmt->structName);

        for (auto *pair : stmt->inner->inner) {
            if (!pair) continue;

            Symbol sym;
            sym.parentName = structName;

            if (pair->met) {
                sym.kind = HoshiSymbolKind::Method;
                sym.line = pair->getLine();
                sym.column = pair->getColumn();
                sym.name = tokenStrVal(pair->met->name->id->node);
                sym.detail = L"func " + sym.name + formatDefinitionArgs(pair->met->args);
                if (pair->met->resultType) {
                    sym.detail += L": " + formatTypeSpec(pair->met->resultType);
                    sym.typeInfo = formatTypeSpec(pair->met->resultType);
                }
            } else if (pair->con) {
                sym.kind = HoshiSymbolKind::Constructor;
                sym.line = pair->getLine();
                sym.column = pair->getColumn();
                sym.name = L"constructor";
                sym.detail = L"constructor" + formatDefinitionArgs(pair->con->args);
            } else if (pair->finalizer) {
                sym.kind = HoshiSymbolKind::Finalizer;
                sym.line = pair->getLine();
                sym.column = pair->getColumn();
                sym.name = L"finalizer";
                sym.detail = L"finalizer()";
            }

            if (!sym.name.empty())
                symbols.push_back(sym);
        }
    }
}

void SymbolExtractor::visitDataStruct(yoi::dataStructDefStmt *stmt) {
    if (!stmt || !stmt->id) return;

    Symbol sym;
    sym.kind = HoshiSymbolKind::Struct;
    sym.line = stmt->getLine();
    sym.column = stmt->getColumn();
    sym.name = tokenStrVal(stmt->id->node);
    sym.detail = L"datastruct " + sym.name;

    if (stmt->inner) {
        for (auto *pair : stmt->inner->inner) {
            if (!pair || !pair->var) continue;

            Symbol child;
            child.kind = HoshiSymbolKind::Field;
            child.parentName = sym.name;
            child.line = pair->getLine();
            child.column = pair->getColumn();
            child.name = tokenStrVal(pair->var->id->node);
            child.typeInfo = formatTypeSpec(pair->var->spec);
            child.detail = child.name + L": " + child.typeInfo;
            sym.children.push_back(child);
        }
    }

    symbols.push_back(sym);
}

void SymbolExtractor::visitConceptDef(yoi::conceptDefinition *stmt) {
    if (!stmt) return;

    Symbol sym;
    sym.kind = HoshiSymbolKind::Concept_;
    sym.line = stmt->getLine();
    sym.column = stmt->getColumn();
    sym.name = stmt->name.strVal;
    sym.detail = L"concept " + sym.name;

    symbols.push_back(sym);
}

void SymbolExtractor::visitInCodeBlock(yoi::inCodeBlockStmt *stmt, const yoi::wstr &inParent) {
    if (!stmt) return;

    switch (stmt->kind) {
        case yoi::inCodeBlockStmt::vKind::letStmt:
            visitLetStmt(static_cast<yoi::letStmt *>(stmt->value.ptr), inParent);
            break;
        case yoi::inCodeBlockStmt::vKind::codeBlock: {
            auto *cb = static_cast<yoi::codeBlock *>(stmt->value.ptr);
            if (cb) {
                for (auto *s : cb->stmts)
                    visitInCodeBlock(s, inParent);
            }
            break;
        }
        case yoi::inCodeBlockStmt::vKind::ifStmt: {
            auto *ifs = static_cast<yoi::ifStmt *>(stmt->value.ptr);
            if (ifs) {
                if (ifs->ifB.block)
                    for (auto *s : ifs->ifB.block->stmts)
                        visitInCodeBlock(s, inParent);
                for (auto &elif : ifs->elifB)
                    if (elif.block)
                        for (auto *s : elif.block->stmts)
                            visitInCodeBlock(s, inParent);
                if (ifs->elseB)
                    for (auto *s : ifs->elseB->stmts)
                        visitInCodeBlock(s, inParent);
            }
            break;
        }
        case yoi::inCodeBlockStmt::vKind::whileStmt: {
            auto *ws = static_cast<yoi::whileStmt *>(stmt->value.ptr);
            if (ws && ws->block)
                for (auto *s : ws->block->stmts)
                    visitInCodeBlock(s, inParent);
            break;
        }
        case yoi::inCodeBlockStmt::vKind::forStmt: {
            auto *fs = static_cast<yoi::forStmt *>(stmt->value.ptr);
            if (fs) {
                if (fs->initStmt) visitInCodeBlock(fs->initStmt, inParent);
                if (fs->afterStmt) visitInCodeBlock(fs->afterStmt, inParent);
                if (fs->block)
                    for (auto *s : fs->block->stmts)
                        visitInCodeBlock(s, inParent);
            }
            break;
        }
        case yoi::inCodeBlockStmt::vKind::forEachStmt: {
            auto *fes = static_cast<yoi::forEachStmt *>(stmt->value.ptr);
            if (fes && fes->var) {
                Symbol sym;
                sym.kind = HoshiSymbolKind::Variable;
                sym.line = fes->var->getLine();
                sym.column = fes->var->getColumn();
                sym.name = tokenStrVal(fes->var->node);
                sym.detail = L"foreach " + sym.name;
                sym.parentName = inParent;
                symbols.push_back(sym);
            }
            if (fes && fes->block)
                for (auto *s : fes->block->stmts)
                    visitInCodeBlock(s, inParent);
            break;
        }
        case yoi::inCodeBlockStmt::vKind::tryCatchStmt: {
            auto *tcs = static_cast<yoi::tryCatchStmt *>(stmt->value.ptr);
            if (tcs) {
                if (tcs->tryBlock)
                    for (auto *s : tcs->tryBlock->stmts)
                        visitInCodeBlock(s, inParent);
                for (auto *cp : tcs->catchParams) {
                    if (cp && cp->name) {
                        Symbol sym;
                        sym.kind = HoshiSymbolKind::Variable;
                        sym.line = cp->name->getLine();
                        sym.column = cp->name->getColumn();
                        sym.name = tokenStrVal(cp->name->node);
                        sym.detail = L"catch " + sym.name;
                        sym.parentName = inParent;
                        symbols.push_back(sym);
                    }
                    if (cp && cp->block)
                        for (auto *s : cp->block->stmts)
                            visitInCodeBlock(s, inParent);
                }
                if (tcs->finallyBlock)
                    for (auto *s : tcs->finallyBlock->stmts)
                        visitInCodeBlock(s, inParent);
            }
            break;
        }
        default:
            break;
    }
}

// ---- Formatting helpers ----

yoi::wstr SymbolExtractor::formatTypeSpec(yoi::typeSpec *spec) {
    if (!spec) return L"void";

    switch (spec->kind) {
        case yoi::typeSpec::typeSpecKind::Null:
            return L"null";
        case yoi::typeSpec::typeSpecKind::Func:
            if (spec->func) {
                yoi::wstr result = L"func";
                if (spec->func->args) {
                    result += L"(";
                    for (size_t i = 0; i < spec->func->args->types.size(); i++) {
                        if (i > 0) result += L", ";
                        result += formatTypeSpec(spec->func->args->types[i]);
                    }
                    result += L")";
                }
                if (spec->func->resultType)
                    result += L": " + formatTypeSpec(spec->func->resultType);
                return result;
            }
            return L"func";
        case yoi::typeSpec::typeSpecKind::Elipsis:
            if (spec->elipsis)
                return L"..." + formatTypeSpec(spec->elipsis);
            return L"...";
        case yoi::typeSpec::typeSpecKind::DecltypeExpr:
            return L"decltype(...)";
        case yoi::typeSpec::typeSpecKind::Member:
        default:
            if (spec->member)
                return formatExternModuleAccessExpression(spec->member);
            return L"unknown";
    }
}

yoi::wstr SymbolExtractor::formatDefinitionArgs(yoi::definitionArguments *args) {
    if (!args || args->spec.empty()) return L"()";

    yoi::wstr result = L"(";
    for (size_t i = 0; i < args->spec.size(); i++) {
        if (i > 0) result += L", ";
        auto *spec = args->spec[i];
        if (spec && spec->id) {
            result += spec->id->node.strVal;
            if (spec->spec)
                result += L": " + formatTypeSpec(spec->spec);
        }
    }
    result += L")";
    return result;
}

yoi::wstr SymbolExtractor::formatIdentifierWithDefTemplateArg(yoi::identifierWithDefTemplateArg *id) {
    if (!id || !id->id) return L"";
    return tokenStrVal(id->id->node);
}

yoi::wstr SymbolExtractor::formatIdentifierWithTemplateArg(yoi::identifierWithTemplateArg *id) {
    if (!id || !id->id) return L"";
    return tokenStrVal(id->id->node);
}

yoi::wstr SymbolExtractor::formatExternModuleAccessExpression(yoi::externModuleAccessExpression *expr) {
    if (!expr || expr->terms.empty()) return L"";

    yoi::wstr result;
    for (size_t i = 0; i < expr->terms.size(); i++) {
        if (i > 0) result += L".";
        result += formatIdentifierWithTemplateArg(expr->terms[i]);
    }
    return result;
}

} // namespace lsp
