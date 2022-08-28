//
// Created by Jerry Chou on 2022/5/21.
//

#ifndef XSCRIPT2_AST_HPP
#define XSCRIPT2_AST_HPP

#include "Utils.hpp"
#include "Lexer.hpp"

namespace Hoshi {

    class AST {
    public:
        enum class TreeType : int {
/* AST output AST kinds */
/* PrimaryParser */
            NotMatch,
            Identifier,
            Literals,
            TemplateArguments,
            FunctionInvokeExpression,
            MemberExpression,
            StaticMemberAccessExpression,
            Operator,
            SingleExpression,
            MultiplicationExpression,
            BinaryMoveExpression,
            AdditionExpression,
            LogicComparingExpression,
            LogicEqualExpression,
            BinaryExpression,
            BooleanExpression,
            AsExpression,
            IdentifierWithTemplateArguments,
            AssignmentExpression,
            Arguments,
            IdentifierWithTypeDescriptor,
            CodeBlock,
            FunctionDefinition,
            VariableDeclaration,
            IfStatement,
            IfElseStatement,
            WhileStatement,
            ForStatement,
            ReturnStatement,
            BreakStatement,
            ContinueStatement,
            ExtendBlock,
            ImplBlock,
            MethodDefinition,
            InitializerList,
            ClassDefinition,
            VirtualMethodDeclaration,
            InterfaceDefinition,
            ImportStatement,
            File
        } Type;
        Lexer::Token Node;
        XArray<AST> Subtrees;

        AST();

        AST(TreeType Type, Lexer::Token Node, XArray<AST> Subtrees);

        /**
         * @biref Construct a leaf node.
         * @param Type AST Kind of this node
         * @param Node Subtrees of this node
         */
        AST(TreeType Type, Lexer::Token Node);

        /**
         * @biref Construct a branch node.
         * @param Type AST Kind of this node
         * @param Subtrees Subtrees of this node
         */
        AST(TreeType Type, XArray<AST> Subtrees);

        /**
         * @biref Return this node is or isn't leaf node
         * @return A boolean value
         */
        [[nodiscard]] XBoolean IsLeafNode() const;

        /**
         * @brief Return this node is or isn't a failed to match node
         * @return A boolean value
         */
        [[nodiscard]] XBoolean IsNotMatchNode() const;

        /**
         * @biref Get first non-null token
         * @return A token
         */
        Lexer::Token GetFirstNotNullToken();
    };

} // Hoshi

#endif //XSCRIPT2_AST_HPP
