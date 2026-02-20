/*
    Copyright (C) 2009  Arno Rehn <arno@arnorehn.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#pragma once

#include <QStringList>
#include <QStack>

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/StmtVisitor.h>

#include "type.h"

class NameCompiler;
class TypeCompiler;

struct ClangQProperty
{
    QString type;
    bool isPtr;
    QString name;
    QString read;
    QString write;
};

class QTRangedAnnotation {
public:
    clang::SourceRange Range;
    std::string Type;

    QTRangedAnnotation(clang::SourceRange Range,std::string const & Type)
    : Range(Range), Type(Type) {}
};

class ClangGeneratorVisitor : public clang::RecursiveASTVisitor<ClangGeneratorVisitor>
{
public:
    typedef clang::RecursiveASTVisitor<ClangGeneratorVisitor> SuperClass;
    ClangGeneratorVisitor(clang::ASTContext* context,
                          std::shared_ptr< std::vector< QTRangedAnnotation > > AccessSpecAnnotations,
                          std::shared_ptr< std::vector< QTRangedAnnotation > > PropertyAnnotations,
                          const QString& header = QString());
    virtual ~ClangGeneratorVisitor();

    Type makeTypeFromQualType(clang::QualType const &t) const;
    Type const * makeTypePtrFromQualType(clang::QualType const &t) const;
    Class *findOrCreateClass(clang::NamedDecl const *nd) const;
    Typedef *findOrCreateTypedef(clang::NamedDecl const *nd, Type const *type) const;
    Enum *findOrCreateEnum(clang::NamedDecl const *nd) const;
    QString getFullyQualifiedName(clang::NamedDecl const *nd) const;
    QString getClassName(clang::NamedDecl const *nd) const;

    void setupCXXClass(Class *c, clang::CXXRecordDecl *decl) const;
    void collectCXXMethods(Class *c, clang::CXXRecordDecl *decl,
                           QList<ClangQProperty> const &properties, bool is_base) const;
    void setupCXXMethod(Class *c, clang::CXXMethodDecl const *decl,
                                           QList<ClangQProperty> const &properties) const;
    void setupCXXCtor(Class *c, clang::CXXConstructorDecl const *decl) const;
    void setupCXXDtor(Class *c, clang::CXXDestructorDecl const *decl) const;
    void setupCXXConvFunc(Class *c, clang::CXXConversionDecl const *decl) const;

public:
    bool VisitAccessSpecDecl(clang::AccessSpecDecl *Declaration);
    bool TraverseCXXRecordDecl(clang::CXXRecordDecl *Declaration);
    bool TraverseNamespaceDecl(clang::NamespaceDecl *Declaration);
    bool TraverseClassTemplateDecl(clang::ClassTemplateDecl *Declaration);
    bool TraverseFunctionTemplateDecl(clang::FunctionTemplateDecl *Declaration);
    bool VisitFunctionDecl(clang::FunctionDecl *Declaration);
    bool VisitCXXMethodDecl(clang::CXXMethodDecl *Declaration);
    bool VisitCXXConstructorDecl(clang::CXXConstructorDecl *Declaration);
    bool VisitCXXDestructorDecl(clang::CXXDestructorDecl *Declaration);
    bool VisitCXXConversionDecl(clang::CXXConversionDecl *Declaration);
    bool VisitEnumDecl(clang::EnumDecl *Declaration);
    bool TraverseFriendDecl(clang::FriendDecl *Declaration);
    bool TraverseStmt(clang::Stmt *Declaration);


protected:
    /*
    virtual void visitDeclarator(DeclaratorAST* node);
    virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* node);
    virtual void visitEnumSpecifier(EnumSpecifierAST *);
    virtual void visitEnumerator(EnumeratorAST *);
    virtual void visitFunctionDefinition(FunctionDefinitionAST* );
    virtual void visitInitializerClause(InitializerClauseAST *);
    virtual void visitNamespace(NamespaceAST* node);
    virtual void visitParameterDeclaration(ParameterDeclarationAST* node);
    virtual void visitSimpleDeclaration(SimpleDeclarationAST* node);
    virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node);
    virtual void visitTemplateDeclaration(TemplateDeclarationAST* node);
    virtual void visitTemplateArgument(TemplateArgumentAST* node);
    virtual void visitTypeAlias(TypeAliasAST* node);
    virtual void visitTypedef(TypedefAST* node);
    virtual void visitUsing(UsingAST* node);
    virtual void visitUsingDirective(UsingDirectiveAST* node);
    */

private:
    clang::ASTContext* context;
    std::shared_ptr< std::vector< QTRangedAnnotation > > m_AccessSpecAnnotations;
    std::shared_ptr< std::vector< QTRangedAnnotation > > m_PropertyAnnotations;

    QString m_header;
    
    short inClass;
    bool inTemplate;
    bool inMethod;
    
    struct ClassScope {
        Class *klass;
        bool inSignals;
        bool inSlots;
        QList<ClangQProperty> q_properties;

        ClassScope(Class *klass, bool inSignals, bool inSlots, QList<ClangQProperty> const &q_properties)
        : klass(klass), inSignals(inSignals), inSlots(inSlots), q_properties(q_properties)
        {}
        ClassScope(const ClassScope &oth) =default;
        ClassScope() =default;
        ClassScope &operator=(const ClassScope &oth) =default;
    };

    QStack<ClassScope> scopes;
    
    QStringList nspace;
};

/* rolling our own here instead of using StmtPrinter since that one does not use fully qualified names for enum constants... */
class ClangDefaultExpressionVisitor : public clang::StmtVisitor<ClangDefaultExpressionVisitor> {
public:
    ClangDefaultExpressionVisitor(clang::PrintingPolicy const &Policy, clang::ASTContext* context);
    void VisitDeclRefExpr(clang::DeclRefExpr *Node);
    void VisitCXXConstructExpr(clang::CXXConstructExpr *E);
    void VisitImplicitCastExpr(clang::ImplicitCastExpr *Node);
    void VisitIntegerLiteral(clang::IntegerLiteral *Node);
    void VisitCXXFunctionalCastExpr(clang::CXXFunctionalCastExpr *Node);
    void VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr *Node);
    void VisitMaterializeTemporaryExpr(clang::MaterializeTemporaryExpr *Node);
    void VisitCXXBindTemporaryExpr(clang::CXXBindTemporaryExpr *Node);
    void VisitCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr *Node);
    void VisitCXXNullPtrLiteralExpr(clang::CXXNullPtrLiteralExpr *Node);
    void VisitUnaryOperator(clang::UnaryOperator *Node);
    void VisitCXXNamedCastExpr(clang::CXXNamedCastExpr *Node);
    void VisitCXXStaticCastExpr(clang::CXXStaticCastExpr *Node);
    void VisitCXXDynamicCastExpr(clang::CXXDynamicCastExpr *Node);
    void VisitCXXReinterpretCastExpr(clang::CXXReinterpretCastExpr *Node);
    void VisitCXXConstCastExpr(clang::CXXConstCastExpr *Node);
    void VisitCXXBoolLiteralExpr(clang::CXXBoolLiteralExpr *Node);
    void VisitCharacterLiteral(clang::CharacterLiteral *Node);
    void VisitCallExpr(clang::CallExpr *Call);
    void VisitFloatingLiteral(clang::FloatingLiteral *Node);
    void VisitCXXScalarValueInitExpr(clang::CXXScalarValueInitExpr *Node);
    void VisitParenExpr(clang::ParenExpr *Node);
    void VisitBinaryOperator(clang::BinaryOperator *Node);
    void VisitOpaqueValueExpr(clang::OpaqueValueExpr *Node);
    void VisitInitListExpr(clang::InitListExpr* Node);
    QString str() const { return result; }
private:
    clang::ASTContext* context;
    clang::PrintingPolicy Policy;
    QString result;
};
