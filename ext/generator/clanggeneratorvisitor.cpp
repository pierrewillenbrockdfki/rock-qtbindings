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

#include <clang/Lex/Lexer.h>

#include "clanggeneratorvisitor.h"
#include "options.h"

#include <QtDebug>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
namespace Qt
{
    using ::endl;
}
#endif

#if LLVM_VERSION_MAJOR < 10
static inline bool operator<(const clang::SourceLocation &LHS, const clang::SourceLocation &RHS) {
	return LHS.getRawEncoding() < RHS.getRawEncoding();
}
static inline bool operator>(const clang::SourceLocation &LHS, const clang::SourceLocation &RHS) {
	return LHS.getRawEncoding() > RHS.getRawEncoding();
}
static inline bool operator<=(const clang::SourceLocation &LHS, const clang::SourceLocation &RHS) {
	return LHS.getRawEncoding() <= RHS.getRawEncoding();
}
static inline bool operator>=(const clang::SourceLocation &LHS, const clang::SourceLocation &RHS) {
	return LHS.getRawEncoding() >= RHS.getRawEncoding();
}

static bool fullyContains(const clang::SourceRange &outer, const clang::SourceRange &inner) {
	return outer.getBegin() <= inner.getBegin() && outer.getEnd() >= inner.getEnd();
}
#endif

ClangGeneratorVisitor::ClangGeneratorVisitor(
    clang::ASTContext *context,
    std::shared_ptr< std::vector< QTRangedAnnotation > > AccessSpecAnnotations,
    std::shared_ptr< std::vector< QTRangedAnnotation > > PropertyAnnotations,
    const QString &header)
    : context(context), m_AccessSpecAnnotations(AccessSpecAnnotations), m_PropertyAnnotations(PropertyAnnotations), m_header(header),
      inClass(0), inTemplate(false), inMethod(false)
{
}

ClangGeneratorVisitor::~ClangGeneratorVisitor()
{
}

static QString getNestedNameSpecifier(clang::NamedDecl const *nd) {
    std::string nspace;
    llvm::raw_string_ostream nspaceStream(nspace);
    nd->printNestedNameSpecifier(nspaceStream);
    QString result = QString::fromStdString(nspaceStream.str());
    if(result.endsWith("::")) {
        result.resize(result.size()-2);
    }
    return result;
}

QString ClangGeneratorVisitor::getFullyQualifiedName(clang::NamedDecl const *nd) const {
    QString fqn = QString::fromStdString(nd->getQualifiedNameAsString());
    if (clang::isa<clang::ClassTemplateSpecializationDecl>(nd)) {
        auto classtempldecl = clang::cast_or_null<clang::ClassTemplateSpecializationDecl>(nd);
        auto &tal = classtempldecl->getTemplateArgs();
        fqn += "<";
        bool first = true;
        for (auto &ta : tal.asArray()) {
            std::string dumped;
            llvm::raw_string_ostream dumpStream(dumped);
            ta.print(context->getPrintingPolicy(), dumpStream, false);
            if (!first) {
                fqn += ",";
            }
            first = false;
            fqn += QString::fromStdString(dumpStream.str());
        }
        fqn += ">";
        fqn.replace(" *","*");
        fqn.replace(" &","&");
    }
    return fqn;
}

QString ClangGeneratorVisitor::getClassName(clang::NamedDecl const *nd) const {
    QString name = QString::fromStdString(nd->getNameAsString());
    if (clang::isa<clang::ClassTemplateSpecializationDecl>(nd)) {
        auto classtempldecl = clang::cast_or_null<clang::ClassTemplateSpecializationDecl>(nd);
        auto &tal = classtempldecl->getTemplateArgs();
        name += "<";
        bool first = true;
        for (auto &ta : tal.asArray()) {
            std::string dumped;
            llvm::raw_string_ostream dumpStream(dumped);
            ta.print(context->getPrintingPolicy(), dumpStream, false);
            if (!first) {
                name += ",";
            }
            first = false;
            name += QString::fromStdString(dumpStream.str());
        }
        name += ">";
        name.replace(" *","*");
        name.replace(" &","&");
    }
    return name;
}

Class *ClangGeneratorVisitor::findOrCreateClass(clang::NamedDecl const *nd) const {
    //qDebug() << "begin findOrCreateClass" << Qt::endl;
    auto fqn = getFullyQualifiedName(nd);
    if(nd->getName() == "") {
        //we don't want these is classes[].
        //qDebug() << "end findOrCreateClass" << Qt::endl;
        return nullptr;
    }
    if (!classes.contains(fqn)) {
        QString nspace = getNestedNameSpecifier(nd);
        QString name = getClassName(nd);
        //qDebug() << "Creating class" << fqn << ":" << name << "in" << nspace << Qt::endl;

        Class *parent = nullptr;
        auto Ctx = nd->getDeclContext();
        if (Ctx && clang::isa<clang::NamedDecl>(Ctx)) {
            parent = findOrCreateClass(clang::cast<clang::NamedDecl>(Ctx));
            //qDebug() << " parent of class " << fqn << " is " << parent->toString() << Qt::endl;
        } else {
            //qDebug() << " no parent of class " << fqn << Qt::endl;
        }
        if (!classes.contains(fqn)) {
            Class *c = new Class(name, QString(), parent);
            classes.insert(fqn, c);
            c->setFileName(m_header);
            if (clang::isa<clang::CXXRecordDecl>(nd)) {
                if (clang::cast<clang::CXXRecordDecl>(nd)->getDefinition()) {
                    setupCXXClass(c, clang::cast<clang::CXXRecordDecl>(nd)->getDefinition());
                }
            } else if (clang::isa<clang::NamespaceDecl>(nd)) {
                c->setIsNameSpace(true);
                c->setIsForwardDecl(false);
            } else {
                {
                    //std::string dumped;
                    //llvm::raw_string_ostream dumpStream(dumped);
                    //nd->dump(dumpStream);
                    //qDebug().noquote() << QString::fromStdString(dumpStream.str()) << Qt::endl;
                }
                qFatal("This is not a class");
            }
            if (nd->getAccess() == clang::AccessSpecifier::AS_private) {
                c->setAccess(Access_private);
            } else if (nd->getAccess() == clang::AccessSpecifier::AS_protected) {
                c->setAccess(Access_protected);
            } else  {
                c->setAccess(Access_public);
            }
        }
    }
    //qDebug() << "end findOrCreateClass" << Qt::endl;
    return classes[fqn];
}

Typedef *ClangGeneratorVisitor::findOrCreateTypedef(clang::NamedDecl const *nd, Type const *type) const {
    auto fqn = getFullyQualifiedName(nd);
    if (!typedefs.contains(fqn)) {
        QString nspace = getNestedNameSpecifier(nd);
        QString name = QString::fromStdString(nd->getNameAsString());
        //qDebug() << "Creating typedef" << fqn << ":" << name << " in" << nspace << Qt::endl;
        Class *parent = nullptr;
        auto Ctx = nd->getDeclContext();
        if (Ctx && clang::isa<clang::ClassTemplateSpecializationDecl>(Ctx)) {
            //qDebug() << " parent is template specialization, fqn: " << getFullyQualifiedName(clang::cast<clang::NamedDecl>(Ctx)) << Qt::endl;
            parent = findOrCreateClass(clang::cast<clang::NamedDecl>(Ctx));
            //qDebug() << " parent of typedef " << fqn << " is " << parent->toString() << Qt::endl;
        } else if (Ctx && clang::isa<clang::NamedDecl>(Ctx)) {
            parent = findOrCreateClass(clang::cast<clang::NamedDecl>(Ctx));
            //qDebug() << " parent of typedef " << fqn << " is " << parent->toString() << Qt::endl;
        } else {
            //qDebug() << " no parent of typedef " << fqn << Qt::endl;
        }
        if (!typedefs.contains(fqn)) {
            Typedef *td = new Typedef(type, name, QString(), parent);
            typedefs.insert(fqn, td);
            td->setFileName(m_header);
            if (nd->getAccess() == clang::AccessSpecifier::AS_private) {
                td->setAccess(Access_private);
                //qDebug() << "is private typedef:" << fqn;
            } else if (nd->getAccess() == clang::AccessSpecifier::AS_protected) {
                td->setAccess(Access_protected);
                //qDebug() << "is protected typedef:" << fqn;
            } else  {
                td->setAccess(Access_public);
                //qDebug() << "is public typedef:" << fqn;
            }
        }
    }
    return typedefs[fqn];
}

Enum *ClangGeneratorVisitor::findOrCreateEnum(clang::NamedDecl const *nd) const {
    auto fqn = getFullyQualifiedName(nd);
    if (!enums.contains(fqn)) {
        QString nspace = getNestedNameSpecifier(nd);
        QString name = QString::fromStdString(nd->getNameAsString());
        //qDebug() << "Creating enum" << fqn << ":" << name << "in" << nspace << Qt::endl;
        Class *parent = nullptr;
        auto Ctx = nd->getDeclContext();
        if (Ctx && clang::isa<clang::NamedDecl>(Ctx)) {
            parent = findOrCreateClass(clang::cast<clang::NamedDecl>(Ctx));
        }
        if (!enums.contains(fqn)) {
            Enum *e = new Enum(name, QString(), false, parent);
            enums.insert(fqn, e);
            e->setFileName(m_header);
            if (parent) {
                if (!parent->enums().contains(e)) {
                    parent->appendEnum(e);
                }
            }
            if (nd->getAccess() == clang::AccessSpecifier::AS_private) {
                e->setAccess(Access_private);
            } else if (nd->getAccess() == clang::AccessSpecifier::AS_protected) {
                e->setAccess(Access_protected);
            } else  {
                e->setAccess(Access_public);
            }
        }
    }
    return enums[fqn];
}

Type ClangGeneratorVisitor::makeTypeFromQualType(clang::QualType const &q) const {
    Type result;
    bool isConst = q.isConstQualified();
    bool isVolatile = q.isVolatileQualified();
    clang::Type const *t = q.getTypePtr();

    {
        //std::string dumped;
        //llvm::raw_string_ostream dumpStream(dumped);
        //t->dump(dumpStream, *context);
        //qDebug().noquote() << "Converting type" << (isVolatile ? "volatile" : "") << (isConst ? "const" : "") <<  "" << Qt::endl << QString::fromStdString(dumpStream.str()) << Qt::endl;
    }

    if(clang::isa<clang::DecltypeType>(t)) {
        result = makeTypeFromQualType(t->getAs<clang::DecltypeType>()->getUnderlyingType());
        result.setIsConst(isConst);
        result.setIsVolatile(isVolatile);
    } else if(clang::isa<clang::BuiltinType>(t)) {
        auto builtint = t->getAs<clang::BuiltinType>();
        result.setName(QString::fromStdString(std::string(
                builtint->getName(clang::PrintingPolicy(context->getPrintingPolicy()))
                                              )));
        result.setIsIntegral(builtint->isInteger() ||
                             builtint->isFloatingPoint());
        result.setIsConst(isConst);
        result.setIsVolatile(isVolatile);
    } else if(clang::isa<clang::ReferenceType>(t)) {
        result = makeTypeFromQualType(t->getAs<clang::ReferenceType>()->getPointeeType());
        result.setIsRef(true);
    } else if (clang::isa<clang::PointerType>(t)) {
        auto pointeetype = t->getAs<clang::PointerType>()->getPointeeType();
        if (clang::isa<clang::ParenType>(pointeetype.getTypePtr()) &&
            clang::isa<clang::FunctionProtoType>(pointeetype->getAs<clang::ParenType>()->getInnerType())) {
            auto pt = pointeetype->getAs<clang::ParenType>();
            auto fp = pt->getInnerType()->getAs<clang::FunctionProtoType>();
            result = makeTypeFromQualType(fp->getReturnType());
            bool missing_types = result.name() == "";
            for (auto const &p : fp->param_types()) {
                Type const *t = makeTypePtrFromQualType(p);
                if(!t) {
                    result = Type();
                    missing_types = true;
                }
            }
            if(!missing_types) {
                result.setIsFunctionPointer(true);
                for (auto const &p : fp->param_types()) {
                    Type const *t = makeTypePtrFromQualType(p);
                    if (t != types[t->toString()]) {
                        qFatal("types do not match");
                    }
                    result.appendParameter(Parameter(QString(), t));
                }
                result.setIsConst(isConst);
                result.setIsVolatile(isVolatile);
            }
        } else if (clang::isa<clang::FunctionProtoType>(pointeetype.getTypePtr())) {
            auto fp = pointeetype->getAs<clang::FunctionProtoType>();
            result = makeTypeFromQualType(fp->getReturnType());
            bool missing_types = result.name() == "";
            for (auto const &p : fp->param_types()) {
                Type const *t = makeTypePtrFromQualType(p);
                if(!t) {
                    result = Type();
                    missing_types = true;
                }
            }
            if(!missing_types) {
                result.setIsFunctionPointer(true);
                for (auto const &p : fp->param_types()) {
                    Type const *t = makeTypePtrFromQualType(p);
                    if (t != types[t->toString()]) {
                        qFatal("types do not match");
                    }
                    result.appendParameter(Parameter(QString(), t));
                }
                result.setIsConst(isConst);
                result.setIsVolatile(isVolatile);
            }
        } else {
            result = makeTypeFromQualType(pointeetype);
            result.setPointerDepth(result.pointerDepth() + 1);
            result.setIsConstPointer(result.pointerDepth() - 1, isConst);
        }
    } else if(clang::isa<clang::MemberPointerType>(t)) {
        auto mpt = t->getAs<clang::MemberPointerType>();
        auto pointeetype = mpt->getPointeeType();
        if (clang::isa<clang::ParenType>(pointeetype.getTypePtr()) &&
            clang::isa<clang::FunctionProtoType>(pointeetype->getAs<clang::ParenType>()->getInnerType())) {
            auto pt = pointeetype->getAs<clang::ParenType>();
            auto fp = pt->getInnerType()->getAs<clang::FunctionProtoType>();
            result = makeTypeFromQualType(fp->getReturnType());
            bool missing_types = result.name() == "";
            for (auto const &p : fp->param_types()) {
                Type const *t = makeTypePtrFromQualType(p);
                if(!t) {
                    result = Type();
                    missing_types = true;
                }
            }
            if(!missing_types) {
                result.setIsFunctionPointer(true);
                for (auto const &p : fp->param_types()) {
                    Type const *t = makeTypePtrFromQualType(p);
                    if (t != types[t->toString()]) {
                        qFatal("types do not match");
                    }
                    result.appendParameter(Parameter(QString(), t));
                }
                result.setIsConst(isConst);
                result.setIsVolatile(isVolatile);
            }
        } else if(clang::isa<clang::FunctionProtoType>(pointeetype.getTypePtr())) {
            auto fp = pointeetype->getAs<clang::FunctionProtoType>();
            result = makeTypeFromQualType(fp->getReturnType());
            bool missing_types = result.name() == "";
            for (auto const &p : fp->param_types()) {
                Type const *t = makeTypePtrFromQualType(p);
                if(!t) {
                    result = Type();
                    missing_types = true;
                }
            }
            if(!missing_types) {
                result.setIsFunctionPointer(true);
                for (auto const &p : fp->param_types()) {
                    Type const *t = makeTypePtrFromQualType(p);
                    if (t != types[t->toString()]) {
                        qFatal("types do not match");
                    }
                    result.appendParameter(Parameter(QString(), t));
                }
                result.setIsConst(isConst);
                result.setIsVolatile(isVolatile);
            }
        } else {
            result = makeTypeFromQualType(pointeetype);
            result.setPointerDepth(result.pointerDepth() + 1);
            result.setIsConstPointer(result.pointerDepth() - 1, isConst);
        }
        auto cls = mpt->getClass();
        auto cxxrd = cls->getAsCXXRecordDecl();
        if (cxxrd) {
            result.setMemberPointerOf(result.pointerDepth() - 1, findOrCreateClass(cxxrd->getMostRecentNonInjectedDecl()));
        }
        //mpt->isMemberFunctionPointer();
    /*} else if(clang::isa<clang::TemplateTypeParmType>(t)) {
        result = makeTypeFromQualType(t->getAs<clang::TemplateTypeParmType>()->getDecl());*/
    } else if(clang::isa<clang::RecordType>(t)) {
        auto decl = t->getAs<clang::RecordType>()->getDecl();
        if(clang::isa<clang::CXXRecordDecl>(decl)) {
            auto fqn = getFullyQualifiedName(decl);
            //qDebug() << "RecordType with CXXRecordDecl asString: " << fqn;
            //and here we can create the class.
            Class *c = findOrCreateClass(decl);
            if(c) {
                result.setClass(c);
                result.setIsConst(isConst);
                result.setIsVolatile(isVolatile);
            } else {
                result.setName("");
            }
        } else {
            //qDebug() << "RecordType asString: " << QString::fromStdString(decl->getQualifiedNameAsString());
        }
    } else if(clang::isa<clang::TypedefType>(t)) {
        auto decl = t->getAs<clang::TypedefType>()->getDecl();
        QString name = QString::fromStdString(decl->getQualifiedNameAsString());

        Typedef *td = findOrCreateTypedef(decl, makeTypePtrFromQualType(decl->getUnderlyingType()));

        result.setTypedef(td);
        result.setIsConst(isConst);
        result.setIsVolatile(isVolatile);
        if (types.contains(result.toString())) {
            //qDebug() << "Type" << result.toString() << "maps to" << types[result.toString()] << types[result.toString()]->toString() << Qt::endl;
            if(result.toString() != types[result.toString()]->toString()) {
                qFatal("Type retrieved from registry show incorrect toString");
            }
        }
    } else if(clang::isa<clang::UsingType>(t)) {
        auto decl = t->getAs<clang::UsingType>()->getFoundDecl();
        QString name = QString::fromStdString(decl->getQualifiedNameAsString());

        Type const *type = nullptr;
        QString actual_type_name = QString::fromStdString(decl->getTargetDecl()->getQualifiedNameAsString());
        if (!types.contains(actual_type_name)) {
            //qDebug() << "Cannot find type for " << actual_type_name << Qt::endl;
        } else {
            type = types[actual_type_name];
        }
        Typedef *td = findOrCreateTypedef(decl, type);
        result.setTypedef(td);
        result.setIsConst(isConst);
        result.setIsVolatile(isVolatile);
        if (types.contains(result.toString())) {
            //qDebug() << "Type" << result.toString() << "maps to" << types[result.toString()] << types[result.toString()]->toString() << Qt::endl;
        }
    } else if(clang::isa<clang::ElaboratedType>(t)) {
        result = makeTypeFromQualType(t->getAs<clang::ElaboratedType>()->getNamedType());
        result.setIsConst(isConst);
        result.setIsVolatile(isVolatile);
    } else if(clang::isa<clang::SubstTemplateTypeParmType>(t)) {
        result = makeTypeFromQualType(t->getAs<clang::SubstTemplateTypeParmType>()->getReplacementType());
    } else if(clang::isa<clang::EnumType>(t)) {
        auto decl = t->getAs<clang::EnumType>()->getDecl();

        Enum *e = findOrCreateEnum(decl);
        result.setEnum(e);
        result.setIsConst(isConst);
        result.setIsVolatile(isVolatile);

        //qDebug() << "enums toString says:" << e->toString() << Qt::endl << "results toString says:" << result.toString() << Qt::endl;

    } else if(clang::isa<clang::TemplateSpecializationType>(t)) {
        auto tst = t->getAs<clang::TemplateSpecializationType>();
        auto nosugar = tst->desugar();
        clang::Type const *nosugart = nosugar.getTypePtr();
        if(t == nosugart) {
            //qDebug() << "Don't know what to do with TemplateSpecializationType desugaring to itself";
        } else {
            result = makeTypeFromQualType(nosugar);
        }
        result.setIsConst(isConst);
        result.setIsVolatile(isVolatile);
    } else if (clang::isa<clang::DecayedType>(t)) {
        auto dt = t->getAs<clang::DecayedType>();
        result = makeTypeFromQualType(dt->getOriginalType());
    } else if(clang::isa<clang::ConstantArrayType>(t)) {
        auto cat = clang::cast<clang::ConstantArrayType>(t->getAsArrayTypeUnsafe());
        result = makeTypeFromQualType(cat->getElementType());
        result.setArrayDimensions(result.arrayDimensions() + 1);
        result.setArrayLength(result.arrayDimensions()-1, cat->getSize().getLimitedValue());
    } else if(clang::isa<clang::IncompleteArrayType>(t)) {
        auto iat = clang::cast<clang::IncompleteArrayType>(t->getAsArrayTypeUnsafe());
        result = makeTypeFromQualType(iat->getElementType());
        result.setPointerDepth(result.pointerDepth() + 1);
        result.setIsConstPointer(result.pointerDepth() - 1, isConst);
  /*} else if(clang::isa<clang::ArrayType>(t)) {
    } else if(clang::isa<clang::FunctionType>(t)) {
    } else if(clang::isa<clang::BlockPointerType>(t)) {//function pointers
        */
    } else {
        //qDebug() << "Don't know what to do with type";
    }
    //Type result(name, isConst, isVolatile, pointerDepth, isRef);
    //Type result(e, isConst, isVolatile, pointerDepth, isRef);
    //Type result(tdef, isConst, isVolatile, pointerDepth, isRef);
    //Type result(klass, isConst, isVolatile, pointerDepth, isRef);

    //std::string dumped;
    //llvm::raw_string_ostream dumpStream(dumped);
    //t->dump(dumpStream, *context);
    //qDebug().noquote() << "Converted type" << (isVolatile?"volatile":"") << (isConst?"const":"") <<  "" << Qt::endl << QString::fromStdString(dumpStream.str())
    //<< "to" << Qt::endl << result.toString() << Qt::endl;

    return result;
}

Type const *ClangGeneratorVisitor::makeTypePtrFromQualType(clang::QualType const &t) const {
    Type res = makeTypeFromQualType(t);
    if(res.name() == "") {
        return nullptr;
    }
    Type const * ptr = Type::registerType(res);
    if(ptr->toString() != res.toString()) {
        qFatal("Type %s maps to type %s from registry", qPrintable(res.toString()), qPrintable(ptr->toString()));
    }
    if(ptr->toString() != types[ptr->toString()]->toString()) {
        qFatal("Type %s maps to type %s from registry", qPrintable(ptr->toString()), qPrintable(types[ptr->toString()]->toString()));
    }
    if(types.contains("size_type")) {
        if(types["size_type"]->toString() != "size_type") {
            qFatal("Bad size_type from registry, maps to %s", qPrintable(types["size_type"]->toString()));
        }
    }
    return ptr;
}

// don't make this public - it's just a utility function for the next method and probably not what you would expect it to be
static bool operator==(const Method& rhs, const Method& lhs)
{
    // these have to be equal for methods to be the same
    bool ok = (rhs.name() == lhs.name() && rhs.isConst() == lhs.isConst() &&
               rhs.parameters().count() == lhs.parameters().count() && rhs.type() == lhs.type());
    if (!ok)
        return false;

    // now check the parameter types for equality
    for (int i = 0; i < rhs.parameters().count(); i++) {
        if (rhs.parameters()[i].type() != lhs.parameters()[i].type())
            return false;
    }

    return true;
}

bool ClangGeneratorVisitor::VisitAccessSpecDecl(clang::AccessSpecDecl *Declaration) {
    if (!inClass) {
        return true;
    }

    scopes.top().inSignals = false;
    scopes.top().inSlots = false;

    //we check if any of our annotations are between here.
    auto range = Declaration->getSourceRange();//this is from getAccessSpecifierLoc to getColonLoc
    for(auto &anno : *m_AccessSpecAnnotations) {
        if(
#if LLVM_VERSION_MAJOR < 10
          fullyContains(range, anno.Range)
#else
          range.fullyContains(anno.Range)
#endif
        ) {
            if(anno.Type == "qt_signal") {
                scopes.top().inSignals = true;
                ParserOptions::resolveTypedefs = false;
            }
            if(anno.Type == "qt_slot") {
                scopes.top().inSlots = true;
                ParserOptions::resolveTypedefs = false;
            }
        }
    }

    return true;
}

bool ClangGeneratorVisitor::VisitEnumDecl(clang::EnumDecl *Declaration)
{
    {
        //std::string dumped;
        //llvm::raw_string_ostream dumpStream(dumped);
        //Declaration->dump(dumpStream);
        //qDebug().noquote() << "enum:" << Qt::endl << QString::fromStdString(dumpStream.str()) << Qt::endl;
    }
    Enum *e;
    if(Declaration->getIdentifier()) {
        //Make sure there is a type registered.
        Type const *t = makeTypePtrFromQualType(context->getEnumType(Declaration));
        e = t->getEnum();
        if (e->parent()) {
            //qDebug("parsing enum type: %s in %s", qPrintable(t->toString()), qPrintable(e->parent()->toString()));
        } else {
            //qDebug("parsing enum type: %s in global scope", qPrintable(t->toString()));
        }
    } else {
        e = findOrCreateEnum(Declaration);
        if (e->parent()) {
            //qDebug("parsing enum type: anonymous in %s", qPrintable(e->parent()->toString()));
        } else {
            //qDebug("parsing enum type: anonymous in global scope");
        }
    }
    e->setIsClass(Declaration->isScoped());

    for (auto enumerator : Declaration->enumerators()) {
        e->appendMember(EnumMember(e,
                                   QString::fromStdString(enumerator->getName().str()),
                                   QString::number(enumerator->getInitVal().getLimitedValue())));
    }


    return true;
}

void ClangGeneratorVisitor::setupCXXMethod(Class *c, clang::CXXMethodDecl const *Declaration,
                                           QList<ClangQProperty> const &properties) const {
    //this is called before VisitCXXDestructorDecl or VisitCXXConstructorDecl, if they apply
    if(clang::isa<clang::CXXConstructorDecl>(Declaration)) {
        return;
    }
    if(clang::isa<clang::CXXDestructorDecl>(Declaration)) {
        return;
    }
    if(clang::isa<clang::CXXConversionDecl>(Declaration)) {
        return;
    }

    bool isVirtual = Declaration->isVirtual();
    //methods cannot be explicit. constructors and conversion operators can.
    bool isExplicit = false;
    bool isStatic = Declaration->isStatic();
    bool isPure = Declaration->isPure();

    const QString declName = QString::fromStdString(Declaration->getNameAsString());

    // we don't care about methods with ellipsis paramaters (i.e. 'foo(const char*, ...)') for now..
    if (Declaration->isVariadic())
        return;
    if (Declaration->getRefQualifier() != clang::RQ_None)
        return;

    //qDebug() << "Method" << declName << Qt::endl;
    //qDebug() << "inMethod:" << inMethod << "inClass:" << inClass << Qt::endl;

    //qDebug() << "Template Kind is " << (int)Declaration->getTemplatedKind() << Qt::endl;
    //qDebug() << "in class" << c->name() << Qt::endl;

    Type const* returnType = makeTypePtrFromQualType(Declaration->getReturnType()->getCanonicalTypeUnqualified());
    if(!returnType) {
        return;
    }
    for (auto const &p : Declaration->parameters()) {
        Type const *t = makeTypePtrFromQualType(p->getType());
        if(!t) {
            return;
        }
    }

    //qDebug() << "method " << declName << returnType->toString() << Qt::endl;
    //std::string dumped;
    //llvm::raw_string_ostream dumpStream(dumped);
    //Declaration->dump(dumpStream);
    //qDebug().noquote() << QString::fromStdString(dumpStream.str());

    //we cannot get back to the AccessSpecDecl from the CXXMethodDecl alone to check if the specifier is a qt signal or slot.
    //so, for the time being, we will need to traverse the CXXRecordDecl and track the access state, and finally modify
    //the methods where applicable.

    Access clangaccess = Access_public;
    if (Declaration->getAccess() == clang::AccessSpecifier::AS_private) {
        clangaccess = Access_private;
    } else if (Declaration->getAccess() == clang::AccessSpecifier::AS_protected) {
        clangaccess = Access_protected;
    } else  {
        clangaccess = Access_public;
    }

    Method currentMethod = Method(c, declName, returnType, clangaccess);
    currentMethod.setIsConstructor(false);
    currentMethod.setIsDestructor(false);
    currentMethod.setIsDeleted(Declaration->isDeleted());
    // build parameter list

    for (auto const &p : Declaration->parameters()) {
        QString name = QString::fromStdString(p->getNameAsString());

        QString defaultValue;
        auto defaultarg = p->getDefaultArg();
        if (defaultarg) {
            // this parameter has a default value

            {
                //std::string dumped;
                //llvm::raw_string_ostream dumpStream(dumped);
                //defaultarg->dump(dumpStream, *context);
                //qDebug().noquote() << "default:" << Qt::endl << QString::fromStdString(dumpStream.str()) << Qt::endl;
            }

            ClangDefaultExpressionVisitor ev(clang::PrintingPolicy(context->getLangOpts()), context);
            ev.Visit(defaultarg);
            defaultValue = ev.str();
            //qDebug() << "converts to: " << defaultValue << Qt::endl;
            if(defaultValue.isEmpty()) {
                qWarning("Default value resolved to empty string");
            }
        }

        Type const *t = makeTypePtrFromQualType(p->getType());
        if(t != types[t->toString()]) {
            qFatal("types do not match");
        }
        currentMethod.appendParameter(Parameter(name, t, defaultValue));

        //qDebug() << "parameter " << name << t->toString() << Qt::endl;
    }

    // const & volatile modifiers
    currentMethod.setIsConst(Declaration->isConst());

    if (isVirtual) currentMethod.setFlag(Method::Virtual);
    if (isPure) currentMethod.setFlag(Method::PureVirtual);
    if (isStatic) currentMethod.setFlag(Method::Static);
    if (isExplicit) currentMethod.setFlag(Method::Explicit);

    // the class already contains the method (probably imported by a 'using' statement)
    if (c->methods().contains(currentMethod)) {
        return;
    }

    // Q_PROPERTY accessor?
    if (ParserOptions::qtMode) {
        Q_FOREACH (const ClangQProperty& prop, properties) {
            if (   (currentMethod.parameters().count() == 0 && prop.read == currentMethod.name() && currentMethod.type()->toString().endsWith(prop.type)
                    && (currentMethod.type()->pointerDepth() == 1) == prop.isPtr)    // READ accessor?
                || (currentMethod.parameters().count() == 1 && prop.write == currentMethod.name()
                    && currentMethod.parameters()[0].type()->toString().remove(QRegExp("^const ")).remove(QRegExp("\\&$")).endsWith(prop.type)
                    && (currentMethod.parameters()[0].type()->pointerDepth() == 1) == prop.isPtr))   // or WRITE accessor?
            {
                currentMethod.setIsQPropertyAccessor(true);
            }
        }
    }

    const auto *FPT = Declaration->getType()->getAs<clang::FunctionProtoType>();

    if (FPT->hasExceptionSpec()) {
        currentMethod.setHasExceptionSpec(true);
        for(auto const &e : FPT->exceptions()) {
            currentMethod.appendExceptionType(makeTypePtrFromQualType(e));
        }
    }

    c->appendMethod(currentMethod);
}

void ClangGeneratorVisitor::setupCXXCtor(Class *c, clang::CXXConstructorDecl const *Declaration) const {
    bool isExplicit = Declaration->isExplicit();

    const QString declName = QString::fromStdString(Declaration->getNameAsString());

    // we don't care about methods with ellipsis paramaters (i.e. 'foo(const char*, ...)') for now..
    if (Declaration->isVariadic())
        return;

    //qDebug() << "Constructor" << declName << Qt::endl;
    //qDebug() << "inMethod:" << inMethod << "inClass:" << inClass << Qt::endl;

    // constructors return a pointer to the class they create
    Type t(c);
    t.setPointerDepth(1);
    Type const *returnType = Type::registerType(t);
    for (auto const &p : Declaration->parameters()) {
        Type const *t = makeTypePtrFromQualType(p->getType());
        if(!t) {
            return;
        }
    }


    Access clangaccess = Access_public;
    if (Declaration->getAccess() == clang::AccessSpecifier::AS_private) {
        clangaccess = Access_private;
    } else if (Declaration->getAccess() == clang::AccessSpecifier::AS_protected) {
        clangaccess = Access_protected;
    } else  {
        clangaccess = Access_public;
    }

    Method currentMethod = Method(c, declName, returnType, clangaccess);
    currentMethod.setIsConstructor(true);
    currentMethod.setIsDestructor(false);
    currentMethod.setIsSignal(false);
    currentMethod.setIsSlot(false);
    currentMethod.setIsDeleted(Declaration->isDeleted());
    if (currentMethod.isDeleted()) {
        //qDebug() << "is deleted" << Qt::endl;
    }

    // build parameter list

    for (auto const &p : Declaration->parameters()) {
        QString name = QString::fromStdString(p->getNameAsString());

        QString defaultValue;
        auto defaultarg = p->getDefaultArg();
        if (defaultarg) {
            // this parameter has a default value

            {
                //std::string dumped;
                //llvm::raw_string_ostream dumpStream(dumped);
                //defaultarg->dump(dumpStream, *context);
                //qDebug().noquote() << "default:" << Qt::endl << QString::fromStdString(dumpStream.str()) << Qt::endl;
            }

            ClangDefaultExpressionVisitor ev(clang::PrintingPolicy(context->getLangOpts()), context);
            ev.Visit(defaultarg);
            defaultValue = ev.str();
            //qDebug() << "converts to: " << defaultValue << Qt::endl;
            if (defaultValue.isEmpty()) {
                qWarning("Default value resolved to empty string");
            }
        }

        Type const *t = makeTypePtrFromQualType(p->getType());
        if (t != types[t->toString()]) {
            qFatal("types do not match");
        }
        currentMethod.appendParameter(Parameter(name, t, defaultValue));

        //qDebug() << "parameter " << name << t->toString() << Qt::endl;
    }

    // const & volatile modifiers
    currentMethod.setIsConst(Declaration->isConst());

    if (isExplicit) currentMethod.setFlag(Method::Explicit);

    // the class already contains the method (probably imported by a 'using' statement)
    if (c->methods().contains(currentMethod)) {
        return;
    }

    const auto *FPT = Declaration->getType()->getAs<clang::FunctionProtoType>();

    if (FPT->hasExceptionSpec()) {
        currentMethod.setHasExceptionSpec(true);
        for (auto const &e : FPT->exceptions()) {
            currentMethod.appendExceptionType(makeTypePtrFromQualType(e));
        }
    }

    c->appendMethod(currentMethod);
}

void ClangGeneratorVisitor::setupCXXDtor(Class *c, clang::CXXDestructorDecl const *Declaration) const {
    bool isVirtual = Declaration->isVirtual();
    bool isPure = Declaration->isPure();

    const QString declName = QString::fromStdString(Declaration->getNameAsString());

    // we don't care about methods with ellipsis paramaters (i.e. 'foo(const char*, ...)') for now..

    //qDebug() << "Destructor" << declName << Qt::endl;
    //qDebug() << "inMethod:" << inMethod << "inClass:" << inClass << Qt::endl;

    // destructors don't have a return type.. so return void
    Type *returnType = const_cast<Type *>(Type::Void);

    Access clangaccess = Access_public;
    if (Declaration->getAccess() == clang::AccessSpecifier::AS_private) {
        clangaccess = Access_private;
    } else if (Declaration->getAccess() == clang::AccessSpecifier::AS_protected) {
        clangaccess = Access_protected;
    } else  {
        clangaccess = Access_public;
    }

    Method currentMethod = Method(c, declName, returnType, clangaccess);
    currentMethod.setIsConstructor(false);
    currentMethod.setIsDestructor(true);
    currentMethod.setIsSignal(false);
    currentMethod.setIsSlot(false);
    // build parameter list

    for (auto const &p : Declaration->parameters()) {
        p->getType();
        QString name = QString::fromStdString(p->getNameAsString());

        QString defaultValue;
        auto defaultarg = p->getDefaultArg();
        if (defaultarg) {
            // this parameter has a default value

            {
                //std::string dumped;
                //llvm::raw_string_ostream dumpStream(dumped);
                //defaultarg->dump(dumpStream, *context);
                //qDebug().noquote() << "default:" << Qt::endl << QString::fromStdString(dumpStream.str()) << Qt::endl;
            }

            ClangDefaultExpressionVisitor ev(clang::PrintingPolicy(context->getLangOpts()), context);
            ev.Visit(defaultarg);
            defaultValue = ev.str();
            //qDebug() << "converts to: " << defaultValue << Qt::endl;
            if (defaultValue.isEmpty()) {
                qWarning("Default value resolved to empty string");
            }
        }

        Type const *t = makeTypePtrFromQualType(p->getType());
        if (t != types[t->toString()]) {
            qFatal("types do not match");
        }
        currentMethod.appendParameter(Parameter(name, t, defaultValue));
    }

    // const & volatile modifiers
    currentMethod.setIsConst(Declaration->isConst());

    if (isVirtual) currentMethod.setFlag(Method::Virtual);
    if (isPure) currentMethod.setFlag(Method::PureVirtual);

    // the class already contains the method (probably imported by a 'using' statement)
    if (c->methods().contains(currentMethod)) {
        return;
    }

    const auto *FPT = Declaration->getType()->getAs<clang::FunctionProtoType>();

    if (FPT->hasExceptionSpec()) {
        currentMethod.setHasExceptionSpec(true);
        for (auto const &e : FPT->exceptions()) {
            currentMethod.appendExceptionType(makeTypePtrFromQualType(e));
        }
    }

    c->appendMethod(currentMethod);
}

void ClangGeneratorVisitor::setupCXXConvFunc(Class *c, clang::CXXConversionDecl const *Declaration) const {
    bool isVirtual = Declaration->isVirtual();
    bool isExplicit = Declaration->isExplicit();
    bool isStatic = Declaration->isStatic();
    bool isPure = Declaration->isPure();

    const QString declName = QString::fromStdString(Declaration->getNameAsString());

    // we don't care about methods with ellipsis paramaters (i.e. 'foo(const char*, ...)') for now..
    if (Declaration->isVariadic())
        return;
    if (Declaration->getRefQualifier() != clang::RQ_None)
        return;

    //qDebug() << "Conversion operator" << declName << Qt::endl;
    //qDebug() << "inMethod:" << inMethod << "inClass:" << inClass << Qt::endl;

    //qDebug() << "in class" << c->name() << Qt::endl;

    Type const *returnType = makeTypePtrFromQualType(Declaration->getReturnType()->getCanonicalTypeUnqualified());
    if(!returnType) {
        return;
    }
    for (auto const &p : Declaration->parameters()) {
        Type const *t = makeTypePtrFromQualType(p->getType());
        if(!t) {
            return;
        }
    }

    //qDebug() << "method " << declName << returnType->toString() << Qt::endl;
    //std::string dumped;
    //llvm::raw_string_ostream dumpStream(dumped);
    //Declaration->dump(dumpStream);
    //qDebug().noquote() << QString::fromStdString(dumpStream.str());

    Access clangaccess = Access_public;
    if (Declaration->getAccess() == clang::AccessSpecifier::AS_private) {
        clangaccess = Access_private;
    } else if (Declaration->getAccess() == clang::AccessSpecifier::AS_protected) {
        clangaccess = Access_protected;
    } else  {
        clangaccess = Access_public;
    }

    Method currentMethod = Method(c, declName, returnType, clangaccess);
    currentMethod.setIsConstructor(false);
    currentMethod.setIsDestructor(false);
    currentMethod.setIsDeleted(Declaration->isDeleted());
    // build parameter list

    for (auto const &p : Declaration->parameters()) {
        QString name = QString::fromStdString(p->getNameAsString());

        QString defaultValue;
        auto defaultarg = p->getDefaultArg();
        if (defaultarg) {
            // this parameter has a default value

            {
                //std::string dumped;
                //llvm::raw_string_ostream dumpStream(dumped);
                //defaultarg->dump(dumpStream, *context);
                //qDebug().noquote() << "default:" << Qt::endl << QString::fromStdString(dumpStream.str()) << Qt::endl;
            }

            ClangDefaultExpressionVisitor ev(clang::PrintingPolicy(context->getLangOpts()), context);
            ev.Visit(defaultarg);
            defaultValue = ev.str();
            //qDebug() << "converts to: " << defaultValue << Qt::endl;
            if (defaultValue.isEmpty()) {
                qWarning("Default value resolved to empty string");
            }
        }

        Type const *t = makeTypePtrFromQualType(p->getType());
        if (t != types[t->toString()]) {
            qFatal("types do not match");
        }
        currentMethod.appendParameter(Parameter(name, t, defaultValue));

        //qDebug() << "parameter " << name << t->toString() << Qt::endl;
    }

    // const & volatile modifiers
    currentMethod.setIsConst(Declaration->isConst());

    if (isVirtual) currentMethod.setFlag(Method::Virtual);
    if (isPure) currentMethod.setFlag(Method::PureVirtual);
    if (isStatic) currentMethod.setFlag(Method::Static);
    if (isExplicit) currentMethod.setFlag(Method::Explicit);

    // the class already contains the method (probably imported by a 'using' statement)
    if (c->methods().contains(currentMethod)) {
        return;
    }

    const auto *FPT = Declaration->getType()->getAs<clang::FunctionProtoType>();

    if (FPT->hasExceptionSpec()) {
        currentMethod.setHasExceptionSpec(true);
        for (auto const &e : FPT->exceptions()) {
            currentMethod.appendExceptionType(makeTypePtrFromQualType(e));
        }
    }

    c->appendMethod(currentMethod);
}

void ClangGeneratorVisitor::setupCXXClass(Class *c, clang::CXXRecordDecl *decl) const {
    if(!decl->isCompleteDefinition()) {
        return;
    }

    clang::TagDecl::TagKind _kind = decl->getTagKind();
    Class::Kind kind = Class::Kind_Struct;
    if (_kind == clang::TTK_Class) {
        kind = Class::Kind_Class;
    } else if (_kind == clang::TTK_Struct) {
        kind = Class::Kind_Struct;
    } else if (_kind == clang::TTK_Union) {
        kind = Class::Kind_Union;
    }

    //Declaration->getNameAsString(): just the name. std::thread::_State results in _State
    //Declaration->getQualifiedNameAsString(): the qualified name. std::thread::_State results in std::thread::_State
    //clang::cast<clang::NamedDecl>(Ctx)->getQualifiedNameAsString() is the qualified name of the parent space. for std::thread::_State, it is std::thread

    c->setKind(kind);

    QString name = c->toString();
    // This class has already been parsed.
    if (classes.contains(name) && !classes[name]->isForwardDecl()) {
        return;
    }

    c->setFileName(m_header);
    c->setIsForwardDecl(false);

    for(auto const &b : decl->bases()) {
        Class::BaseClassSpecifier baseClass;
        switch(b.getAccessSpecifier()) {
        default:
        case clang::AS_private:
            baseClass.access = Access_private;
            break;
        case clang::AS_protected:
            baseClass.access = Access_protected;
            break;
        case clang::AS_public:
            baseClass.access = Access_public;
            break;
        }
        baseClass.isVirtual = b.isVirtual();

        auto t = b.getType().getTypePtr();
        if(!clang::isa<clang::RecordType>(t)) {
            continue;
        }
        auto decl = t->getAs<clang::RecordType>()->getDecl();
        BasicTypeDeclaration *base = findOrCreateClass(decl);
        Class *bptr = dynamic_cast<Class *>(base);
        if (!bptr) {
            Typedef *tdef = dynamic_cast<Typedef *>(base);
            if (!tdef)
                continue;
            bptr = tdef->resolve()->getClass();
            if (!bptr)
                continue;
        }
        baseClass.baseClass = bptr;
        c->appendBaseClass(baseClass);
    }
    for(auto const &vb : decl->vbases()) {
        Class::BaseClassSpecifier baseClass;
        switch(vb.getAccessSpecifier()) {
        default:
        case clang::AS_private:
            baseClass.access = Access_private;
            break;
        case clang::AS_protected:
            baseClass.access = Access_protected;
            break;
        case clang::AS_public:
            baseClass.access = Access_public;
            break;
        }
        baseClass.isVirtual = vb.isVirtual();

        auto t = vb.getType().getTypePtr();
        if(!clang::isa<clang::RecordType>(t)) {
            continue;
        }
        auto decl = t->getAs<clang::RecordType>()->getDecl();
        BasicTypeDeclaration *base = findOrCreateClass(decl);
        if (!base)
            continue;
        Class *bptr = dynamic_cast<Class *>(base);
        if (!bptr) {
            Typedef *tdef = dynamic_cast<Typedef *>(base);
            if (!tdef)
                continue;
            bptr = tdef->resolve()->getClass();
            if (!bptr)
                continue;
        }
        baseClass.baseClass = bptr;
        c->appendBaseClass(baseClass);
    }

    QList<ClangQProperty> properties;
    auto range = decl->getSourceRange();
    for(auto &prop : *m_PropertyAnnotations) {
        if(
#if LLVM_VERSION_MAJOR < 10
          fullyContains(range, prop.Range)
#else
          range.fullyContains(prop.Range)
#endif
	) {
            //its one of our properties.
            // this monster only matches "type name READ getMethod WRITE setMethod"
            static QRegExp regexp("^([\\w:<>\\*]+)\\s+(\\w+)\\s+READ\\s+(\\w+)(\\s+WRITE\\s+\\w+)?");
            static QRegExp typePtr(".*\\*$");
            if (regexp.indexIn(QString::fromStdString(prop.Type)) != -1) {
                ClangQProperty prop = { QMetaObject::normalizedType(regexp.cap(1).toLocal8Bit()), (typePtr.indexIn(regexp.cap(1)) !=  -1), regexp.cap(2),
                                   regexp.cap(3), regexp.cap(4).replace(QRegExp("\\s+WRITE\\s+"), QString()) };
                properties.append(prop);
            }
        }
    }

    for(auto m : decl->methods()) {
        setupCXXMethod(c, m, properties);
    }
    for(auto m : decl->ctors()) {
        setupCXXCtor(c, m);
    }
    if(decl->getDestructor()) {
        setupCXXDtor(c, decl->getDestructor());
    }
    for(auto m : decl->getVisibleConversionFunctions()) {
        if(clang::isa<clang::CXXConversionDecl>(m)) {
            setupCXXConvFunc(c, clang::cast<clang::CXXConversionDecl>(m));
        } else if (clang::isa<clang::FunctionTemplateDecl>(m)) {
            //we don't want these in any case.
        } else if (clang::isa<clang::UsingShadowDecl>(m)) {
            //UsingShadowDecl 0x55e0a0641a40 </usr/include/c++/11/atomic:835:26> col:26 implicit CXXConversion 0x55e0a063af40 'operator int' 'std::__atomic_base<int>::__int_type () const noexcept'
        } else {
            //std::string dumped;
            //llvm::raw_string_ostream dumpStream(dumped);
            //m->dump(dumpStream);
            //qDebug().noquote() << QString::fromStdString(dumpStream.str()) << Qt::endl;
            qFatal("Conversion func is not a CXXConversionDecl");
        }
    }

    for(auto f : decl->fields()) {
        Access clangaccess = Access_public;
        if (f->getAccess() == clang::AccessSpecifier::AS_private) {
            clangaccess = Access_private;
        } else if (f->getAccess() == clang::AccessSpecifier::AS_protected) {
            clangaccess = Access_protected;
        } else  {
            clangaccess = Access_public;
        }
        //qDebug() << "Field" << c->name() << "::" << QString::fromStdString(f->getNameAsString()) << Qt::endl;
        Type const *t = makeTypePtrFromQualType(f->getType());
        if(t) {
            c->appendField(Field(c, QString::fromStdString(f->getName().str()), t, clangaccess));
        }
    }
    //catch static member fields; they specifically are excluded from fields(),
    //probably because they do not become part of any of the objects layout.
    for(auto d : decl->decls()) {
        if(clang::isa<clang::VarDecl>(d)) {
            auto vd = clang::cast<clang::VarDecl>(d);
            if (vd->isStaticDataMember()) {

                Access clangaccess = Access_public;
                if (vd->getAccess() == clang::AccessSpecifier::AS_private) {
                    clangaccess = Access_private;
                } else if (vd->getAccess() == clang::AccessSpecifier::AS_protected) {
                    clangaccess = Access_protected;
                } else  {
                    clangaccess = Access_public;
                }
                //qDebug() << "Static Field" << c->name() << "::" << QString::fromStdString(vd->getNameAsString()) << Qt::endl;
                Type const *t = makeTypePtrFromQualType(vd->getType());
                if (t) {
                    Field nf(c, QString::fromStdString(vd->getName().str()), t, clangaccess);
                    nf.setFlag(Member::Static);
                    c->appendField(nf);
                }
            }
        }
    }
}

bool ClangGeneratorVisitor::TraverseCXXRecordDecl(clang::CXXRecordDecl *Declaration)
{
    if(Declaration->getName() == "") {
        //qDebug() << "Skipping anonymous class" << QString::fromStdString(Declaration->getQualifiedNameAsString()) << Qt::endl;
        return true;
    }
    Class *_class = findOrCreateClass(Declaration);

    //setupCXXClass(_class, Declaration);

    if(!Declaration->isCompleteDefinition()) {
        //qDebug() << "Skipping class without definition" << QString::fromStdString(Declaration->getName().str()) << Qt::endl;
        //std::string dumped;
        //llvm::raw_string_ostream dumpStream(dumped);
        //Declaration->dump(dumpStream);
        //qDebug().noquote() << QString::fromStdString(dumpStream.str()) << Qt::endl;
        return true;
    }
    if(Declaration->isTemplateDecl()) {
        //qDebug() << "Skipping class that is template" << QString::fromStdString(Declaration->getName().str()) << Qt::endl;
        return true;
    }

    //Declaration->getNameAsString(): just the name. std::thread::_State results in _State
    //Declaration->getQualifiedNameAsString(): the qualified name. std::thread::_State results in std::thread::_State
    //clang::cast<clang::NamedDecl>(Ctx)->getQualifiedNameAsString() is the qualified name of the parent space. for std::thread::_State, it is std::thread

    QString name = _class->toString();
    // This class has already been parsed.
    if (classes.contains(name) && !classes[name]->isForwardDecl()) {
        scopes.push_back(ClassScope(_class, false, false, QList<ClangQProperty>()));
        inClass++;
        //qDebug() << "Skipping class with definition but already parsed begin" << QString::fromStdString(Declaration->getName().str()) << Qt::endl;
        bool res = SuperClass::TraverseCXXRecordDecl(Declaration);
        //qDebug() << "Skipping class with definition but already parsed end" << QString::fromStdString(Declaration->getName().str()) << Qt::endl;
        scopes.pop();
        inClass--;
        return res;
    }
    if (inTemplate) {
        _class->setIsTemplate(true);
        //qDebug() << "Skipping template class begin" << QString::fromStdString(Declaration->getName().str()) << Qt::endl;
        bool res = SuperClass::TraverseCXXRecordDecl(Declaration);
        //qDebug() << "Skipping template class end" << QString::fromStdString(Declaration->getName().str()) << Qt::endl;
        return res;
    }

    //_class->setFileName(m_header);
    _class->setIsForwardDecl(false);

    scopes.push_back(ClassScope(_class, false, false, QList<ClangQProperty>()));
    inClass++;

    auto range = Declaration->getSourceRange();
    for(auto &prop : *m_PropertyAnnotations) {
        if(
#if LLVM_VERSION_MAJOR < 10
          fullyContains(range, prop.Range)
#else
          range.fullyContains(prop.Range)
#endif
	) {
            //its one of our properties.
            // this monster only matches "type name READ getMethod WRITE setMethod"
            static QRegExp regexp("^([\\w:<>\\*]+)\\s+(\\w+)\\s+READ\\s+(\\w+)(\\s+WRITE\\s+\\w+)?");
            static QRegExp typePtr(".*\\*$");
            if (regexp.indexIn(QString::fromStdString(prop.Type)) != -1) {
                ClangQProperty prop = { QMetaObject::normalizedType(regexp.cap(1).toLocal8Bit()), (typePtr.indexIn(regexp.cap(1)) !=  -1), regexp.cap(2),
                                   regexp.cap(3), regexp.cap(4).replace(QRegExp("\\s+WRITE\\s+"), QString()) };
                scopes.top().q_properties.append(prop);
            }
        }
    }

    //qDebug() << "Parsing class begin" << QString::fromStdString(Declaration->getName().str()) << Qt::endl;

    // This will visit the members before visiting the CXXRecordDecl
    bool res = SuperClass::TraverseCXXRecordDecl(Declaration);

    //qDebug() << "Parsing class end" << QString::fromStdString(Declaration->getName().str()) << Qt::endl;

    scopes.pop();
    inClass--;


    return res;
}

bool ClangGeneratorVisitor::TraverseNamespaceDecl(clang::NamespaceDecl *Declaration) {
    QString name = QString::fromStdString(Declaration->getNameAsString());
    nspace.push_back(name);
    Class *nsclass = findOrCreateClass(Declaration);
    nsclass->setIsNameSpace(true);

    bool res = SuperClass::TraverseNamespaceDecl(Declaration);

    nspace.pop_back();

    return res;
}

bool ClangGeneratorVisitor::TraverseClassTemplateDecl(clang::ClassTemplateDecl *Declaration)
{
    bool oldInTemplate = inTemplate;
    inTemplate = true;
    bool res = SuperClass::TraverseClassTemplateDecl(Declaration);
    inTemplate = oldInTemplate;
    return res;
}

bool ClangGeneratorVisitor::TraverseFunctionTemplateDecl(clang::FunctionTemplateDecl *Declaration)
{
    bool oldInTemplate = inTemplate;
    inTemplate = true;
    bool res = SuperClass::TraverseFunctionTemplateDecl(Declaration);
    inTemplate = oldInTemplate;
    return res;
}


bool ClangGeneratorVisitor::VisitCXXConstructorDecl(clang::CXXConstructorDecl *Declaration) {
    bool isVirtual = false;
    bool isExplicit = Declaration->isExplicit();
    bool isStatic = false;
    bool isPure = false;

    const QString declName = QString::fromStdString(Declaration->getNameAsString());

    // we don't care about methods with ellipsis paramaters (i.e. 'foo(const char*, ...)') for now..
    if (Declaration->isVariadic())
        return true;
    if(inTemplate)
        return true;

    //qDebug() << "Constructor" << declName << Qt::endl;
    //qDebug() << "inMethod:" << inMethod << "inClass:" << inClass << Qt::endl;

    if (!inMethod && inClass) {

        // constructors return a pointer to the class they create
        Type t(scopes.top().klass);
        t.setPointerDepth(1);
        Type const* returnType = Type::registerType(t);
        for (auto const &p : Declaration->parameters()) {
            Type const *t = makeTypePtrFromQualType(p->getType());
            if (!t) {
                return true;
            }
        }


        Access clangaccess = Access_public;
        if (Declaration->getAccess() == clang::AccessSpecifier::AS_private) {
            clangaccess = Access_private;
        } else if (Declaration->getAccess() == clang::AccessSpecifier::AS_protected) {
            clangaccess = Access_protected;
        } else  {
            clangaccess = Access_public;
        }

        Method currentMethod = Method(scopes.top().klass, declName, returnType, clangaccess);
        currentMethod.setIsConstructor(true);
        currentMethod.setIsDestructor(false);
        currentMethod.setIsSignal(false);
        currentMethod.setIsSlot(false);
        currentMethod.setIsDeleted(Declaration->isDeleted());
        if(currentMethod.isDeleted()) {
            //qDebug() << "is deleted" << Qt::endl;
        }

        // build parameter list
        inMethod = true;

        for (auto const &p : Declaration->parameters()) {
            QString name = QString::fromStdString(p->getNameAsString());

            QString defaultValue;
            auto defaultarg = p->getDefaultArg();
            if (defaultarg) {
                // this parameter has a default value

                {
                    //std::string dumped;
                    //llvm::raw_string_ostream dumpStream(dumped);
                    //defaultarg->dump(dumpStream, *context);
                    //qDebug().noquote() << "default:" << Qt::endl << QString::fromStdString(dumpStream.str()) << Qt::endl;
                }

                ClangDefaultExpressionVisitor ev(clang::PrintingPolicy(context->getLangOpts()), context);
                ev.Visit(defaultarg);
                defaultValue = ev.str();
                //qDebug() << "converts to: " << defaultValue << Qt::endl;
                if(defaultValue.isEmpty()) {
                    qWarning("Default value resolved to empty string");
                }
            }

            Type const *t = makeTypePtrFromQualType(p->getType());
            if (t != types[t->toString()]) {
                qFatal("types do not match");
            }
            currentMethod.appendParameter(Parameter(name, t, defaultValue));

            //qDebug() << "parameter " << name << t->toString() << Qt::endl;
        }

        inMethod = false;

        // const & volatile modifiers
        currentMethod.setIsConst(Declaration->isConst());

        if (isVirtual) currentMethod.setFlag(Method::Virtual);
        if (isPure) currentMethod.setFlag(Method::PureVirtual);
        if (isStatic) currentMethod.setFlag(Method::Static);
        if (isExplicit) currentMethod.setFlag(Method::Explicit);

        // the class already contains the method (probably imported by a 'using' statement)
        if (scopes.top().klass->methods().contains(currentMethod)) {
            return true;
        }
        qFatal("setupCXXClass failed to setup constructor");
    }

    return true;
}

bool ClangGeneratorVisitor::VisitCXXDestructorDecl(clang::CXXDestructorDecl *Declaration) {
    bool isVirtual = Declaration->isVirtual();
    bool isExplicit = false;
    bool isStatic = false;
    bool isPure = Declaration->isPure();

    const QString declName = QString::fromStdString(Declaration->getNameAsString());

    // we don't care about methods with ellipsis paramaters (i.e. 'foo(const char*, ...)') for now..
    if (Declaration->isVariadic())
        return true;
    if(inTemplate)
        return true;

    //qDebug() << "Destructor" << declName << Qt::endl;
    //qDebug() << "inMethod:" << inMethod << "inClass:" << inClass << Qt::endl;

    if (!inMethod && inClass) {
        // detect Q_PROPERTIES

        // destructors don't have a return type.. so return void
        Type *returnType = const_cast<Type *>(Type::Void);

        Access clangaccess = Access_public;
        if (Declaration->getAccess() == clang::AccessSpecifier::AS_private) {
            clangaccess = Access_private;
        } else if (Declaration->getAccess() == clang::AccessSpecifier::AS_protected) {
            clangaccess = Access_protected;
        } else  {
            clangaccess = Access_public;
        }

        Method currentMethod = Method(scopes.top().klass, declName, returnType, clangaccess);
        currentMethod.setIsConstructor(false);
        currentMethod.setIsDestructor(true);
        currentMethod.setIsSignal(false);
        currentMethod.setIsSlot(false);
        // build parameter list
        inMethod = true;

        for (auto const &p : Declaration->parameters()) {
            p->getType();
            QString name = QString::fromStdString(p->getNameAsString());

            QString defaultValue;
            auto defaultarg = p->getDefaultArg();
            if (defaultarg) {
                // this parameter has a default value

                {
                    //std::string dumped;
                    //llvm::raw_string_ostream dumpStream(dumped);
                    //defaultarg->dump(dumpStream, *context);
                    //qDebug().noquote() << "default:" << Qt::endl << QString::fromStdString(dumpStream.str()) << Qt::endl;
                }

                ClangDefaultExpressionVisitor ev(clang::PrintingPolicy(context->getLangOpts()), context);
                ev.Visit(defaultarg);
                defaultValue = ev.str();
                //qDebug() << "converts to: " << defaultValue << Qt::endl;
                if(defaultValue.isEmpty()) {
                    qWarning("Default value resolved to empty string");
                }
            }

            Type const *t = makeTypePtrFromQualType(p->getType());
            if (t != types[t->toString()]) {
                qFatal("types do not match");
            }
            currentMethod.appendParameter(Parameter(name, t, defaultValue));
        }

        inMethod = false;

        // const & volatile modifiers
        currentMethod.setIsConst(Declaration->isConst());

        if (isVirtual) currentMethod.setFlag(Method::Virtual);
        if (isPure) currentMethod.setFlag(Method::PureVirtual);
        if (isStatic) currentMethod.setFlag(Method::Static);
        if (isExplicit) currentMethod.setFlag(Method::Explicit);

        // the class already contains the method (probably imported by a 'using' statement)
        if (scopes.top().klass->methods().contains(currentMethod)) {
            return true;
        }
        qFatal("setupCXXClass failed to setup destructor");

        const auto *FPT = Declaration->getType()->getAs<clang::FunctionProtoType>();

        if (FPT->hasExceptionSpec()) {
            currentMethod.setHasExceptionSpec(true);
            for(auto const &e : FPT->exceptions()) {
                currentMethod.appendExceptionType(makeTypePtrFromQualType(e));
            }
        }

        scopes.top().klass->appendMethod(currentMethod);
    }

    return true;
}

bool ClangGeneratorVisitor::VisitCXXConversionDecl(clang::CXXConversionDecl *Declaration) {
    bool isVirtual = Declaration->isVirtual();
    bool isExplicit = Declaration->isExplicit();
    bool isStatic = Declaration->isStatic();
    bool isPure = Declaration->isPure();

    const QString declName = QString::fromStdString(Declaration->getNameAsString());

    // we don't care about methods with ellipsis paramaters (i.e. 'foo(const char*, ...)') for now..
    if (Declaration->isVariadic())
        return true;
    if (Declaration->getRefQualifier() != clang::RQ_None)
        return true;
    if(inTemplate)
        return true;

    //qDebug() << "Conversion operator" << declName << Qt::endl;
    //qDebug() << "inMethod:" << inMethod << "inClass:" << inClass << Qt::endl;

    if (!inMethod && inClass) {
        //qDebug() << "in class" << scopes.top().klass->name() << Qt::endl;

        Type const* returnType = makeTypePtrFromQualType(Declaration->getReturnType()->getCanonicalTypeUnqualified());
        if (!returnType) {
            return true;
        }
        for (auto const &p : Declaration->parameters()) {
            Type const *t = makeTypePtrFromQualType(p->getType());
            if (!t) {
                return true;
            }
        }

        //qDebug() << "method " << declName << returnType->toString() << Qt::endl;
        //std::string dumped;
        //llvm::raw_string_ostream dumpStream(dumped);
        //Declaration->dump(dumpStream);
        //qDebug().noquote() << QString::fromStdString(dumpStream.str());

        Access clangaccess = Access_public;
        if (Declaration->getAccess() == clang::AccessSpecifier::AS_private) {
            clangaccess = Access_private;
        } else if (Declaration->getAccess() == clang::AccessSpecifier::AS_protected) {
            clangaccess = Access_protected;
        } else  {
            clangaccess = Access_public;
        }

        Method currentMethod = Method(scopes.top().klass, declName, returnType, clangaccess);
        currentMethod.setIsConstructor(false);
        currentMethod.setIsDestructor(false);
        currentMethod.setIsSignal(scopes.top().inSignals);
        currentMethod.setIsSlot(scopes.top().inSlots);
        currentMethod.setIsDeleted(Declaration->isDeleted());
        // build parameter list
        inMethod = true;

        for (auto const &p : Declaration->parameters()) {
            QString name = QString::fromStdString(p->getNameAsString());

            QString defaultValue;
            auto defaultarg = p->getDefaultArg();
            if (defaultarg) {
                // this parameter has a default value

                {
                    //std::string dumped;
                    //llvm::raw_string_ostream dumpStream(dumped);
                    //defaultarg->dump(dumpStream, *context);
                    //qDebug().noquote() << "default:" << Qt::endl << QString::fromStdString(dumpStream.str()) << Qt::endl;
                }

                ClangDefaultExpressionVisitor ev(clang::PrintingPolicy(context->getLangOpts()), context);
                ev.Visit(defaultarg);
                defaultValue = ev.str();
                //qDebug() << "converts to: " << defaultValue << Qt::endl;
                if(defaultValue.isEmpty()) {
                    qWarning("Default value resolved to empty string");
                }
            }

            Type const *t = makeTypePtrFromQualType(p->getType());
            if (t != types[t->toString()]) {
                qFatal("types do not match");
            }
            currentMethod.appendParameter(Parameter(name, t, defaultValue));

            //qDebug() << "parameter " << name << t->toString() << Qt::endl;
        }

        inMethod = false;
        // const & volatile modifiers
        currentMethod.setIsConst(Declaration->isConst());

        if (isVirtual) currentMethod.setFlag(Method::Virtual);
        if (isPure) currentMethod.setFlag(Method::PureVirtual);
        if (isStatic) currentMethod.setFlag(Method::Static);
        if (isExplicit) currentMethod.setFlag(Method::Explicit);

        // the class already contains the method (probably imported by a 'using' statement)
        if (scopes.top().klass->methods().contains(currentMethod)) {
            return true;
        }
        qFatal("setupCXXClass failed to setup conversion function");

        const auto *FPT = Declaration->getType()->getAs<clang::FunctionProtoType>();

        if (FPT->hasExceptionSpec()) {
            currentMethod.setHasExceptionSpec(true);
            for(auto const &e : FPT->exceptions()) {
                currentMethod.appendExceptionType(makeTypePtrFromQualType(e));
            }
        }

        scopes.top().klass->appendMethod(currentMethod);
    }

    return true;
}

//TODO handle variable declarations

bool ClangGeneratorVisitor::VisitCXXMethodDecl(clang::CXXMethodDecl *Declaration) {
    //this is called before VisitCXXDestructorDecl or VisitCXXConstructorDecl, if they apply
    if(clang::isa<clang::CXXConstructorDecl>(Declaration)) {
        return true;
    }
    if(clang::isa<clang::CXXDestructorDecl>(Declaration)) {
        return true;
    }
    if(clang::isa<clang::CXXConversionDecl>(Declaration)) {
        return true;
    }

    bool isVirtual = Declaration->isVirtual();
    //methods cannot be explicit. constructors and conversion operators can.
    bool isExplicit = false;
    bool isStatic = Declaration->isStatic();
    bool isPure = Declaration->isPure();

    const QString declName = QString::fromStdString(Declaration->getNameAsString());

    // we don't care about methods with ellipsis paramaters (i.e. 'foo(const char*, ...)') for now..
    if (Declaration->isVariadic())
        return true;
    if (Declaration->getRefQualifier() != clang::RQ_None)
        return true;
    if(inTemplate)
        return true;

    //qDebug() << "Method" << declName << Qt::endl;
    //qDebug() << "inMethod:" << inMethod << "inClass:" << inClass << Qt::endl;

    if (!inMethod && inClass) {
        //qDebug() << "in class" << scopes.top().klass->name() << Qt::endl;

        Type const* returnType = makeTypePtrFromQualType(Declaration->getReturnType()->getCanonicalTypeUnqualified());
        if (!returnType) {
            return true;
        }
        for (auto const &p : Declaration->parameters()) {
            Type const *t = makeTypePtrFromQualType(p->getType());
            if (!t) {
                return true;
            }
        }

        //qDebug() << "method " << declName << returnType->toString() << Qt::endl;
        //std::string dumped;
        //llvm::raw_string_ostream dumpStream(dumped);
        //Declaration->dump(dumpStream);
        //qDebug().noquote() << QString::fromStdString(dumpStream.str());

        //we cannot get back to the AccessSpecDecl from the CXXMethodDecl alone to check if the specifier is a qt signal or slot.
        //so, for the time being, we will need to traverse the CXXRecordDecl and track the access state, and finally modify
        //the methods where applicable.

        Access clangaccess = Access_public;
        if (Declaration->getAccess() == clang::AccessSpecifier::AS_private) {
            clangaccess = Access_private;
        } else if (Declaration->getAccess() == clang::AccessSpecifier::AS_protected) {
            clangaccess = Access_protected;
        } else  {
            clangaccess = Access_public;
        }

        Method currentMethod = Method(scopes.top().klass, declName, returnType, clangaccess);
        currentMethod.setIsConstructor(false);
        currentMethod.setIsDestructor(false);
        currentMethod.setIsSignal(scopes.top().inSignals);
        currentMethod.setIsSlot(scopes.top().inSlots);
        currentMethod.setIsDeleted(Declaration->isDeleted());
        // build parameter list
        inMethod = true;

        for (auto const &p : Declaration->parameters()) {
            QString name = QString::fromStdString(p->getNameAsString());

            QString defaultValue;
            auto defaultarg = p->getDefaultArg();
            if (defaultarg) {
                // this parameter has a default value

                {
                    //std::string dumped;
                    //llvm::raw_string_ostream dumpStream(dumped);
                    //defaultarg->dump(dumpStream, *context);
                    //qDebug().noquote() << "default:" << Qt::endl << QString::fromStdString(dumpStream.str()) << Qt::endl;
                }

                ClangDefaultExpressionVisitor ev(clang::PrintingPolicy(context->getLangOpts()), context);
                ev.Visit(defaultarg);
                defaultValue = ev.str();
                //qDebug() << "converts to: " << defaultValue << Qt::endl;
                if(defaultValue.isEmpty()) {
                    qWarning("Default value resolved to empty string");
                }
            }

            Type const *t = makeTypePtrFromQualType(p->getType());
            if (t != types[t->toString()]) {
                qFatal("types do not match");
            }
            currentMethod.appendParameter(Parameter(name, t, defaultValue));

            //qDebug() << "parameter " << name << t->toString() << Qt::endl;
        }

        inMethod = false;
        // const & volatile modifiers
        currentMethod.setIsConst(Declaration->isConst());

        if (isVirtual) currentMethod.setFlag(Method::Virtual);
        if (isPure) currentMethod.setFlag(Method::PureVirtual);
        if (isStatic) currentMethod.setFlag(Method::Static);
        if (isExplicit) currentMethod.setFlag(Method::Explicit);

        // the class already contains the method (probably imported by a 'using' statement)
        if (scopes.top().klass->methods().contains(currentMethod)) {
            int idx = scopes.top().klass->methods().indexOf(currentMethod);
            scopes.top().klass->methodsRef()[idx].setIsSignal(scopes.top().inSignals);
            scopes.top().klass->methodsRef()[idx].setIsSlot(scopes.top().inSlots);
            return true;
        }
        //qFatal("setupCXXClass failed to setup method"); //TODO

        // Q_PROPERTY accessor?
        if (ParserOptions::qtMode) {
            Q_FOREACH (const ClangQProperty& prop, scopes.top().q_properties) {
                if (   (currentMethod.parameters().count() == 0 && prop.read == currentMethod.name() && currentMethod.type()->toString().endsWith(prop.type)
                       && (currentMethod.type()->pointerDepth() == 1) == prop.isPtr)    // READ accessor?
                    || (currentMethod.parameters().count() == 1 && prop.write == currentMethod.name()
                       && currentMethod.parameters()[0].type()->toString().remove(QRegExp("^const ")).remove(QRegExp("\\&$")).endsWith(prop.type)
                       && (currentMethod.parameters()[0].type()->pointerDepth() == 1) == prop.isPtr))   // or WRITE accessor?
                {
                    currentMethod.setIsQPropertyAccessor(true);
                }
            }
        }

        const auto *FPT = Declaration->getType()->getAs<clang::FunctionProtoType>();

        if (FPT->hasExceptionSpec()) {
            currentMethod.setHasExceptionSpec(true);
            for(auto const &e : FPT->exceptions()) {
                currentMethod.appendExceptionType(makeTypePtrFromQualType(e));
            }
        }

        scopes.top().klass->appendMethod(currentMethod);
    }

    return true;
}

bool ClangGeneratorVisitor::VisitFunctionDecl(clang::FunctionDecl *Declaration) {
    //this is called before VisitCXXMethodDecl, if it applies
    if(clang::isa<clang::CXXMethodDecl>(Declaration)) {
        return true;
    }

    const QString declName = QString::fromStdString(Declaration->getQualifiedNameAsString());

    if (Declaration->isVariadic())
        return true;
    if(Declaration->isInvalidDecl())
        return true;
    /* when we try to forward calls to template specialisations, this function provides the actual args. otherwise, it returns NULL. */
    if (Declaration->getTemplateSpecializationArgs())
        return true;
    if (inTemplate)
        return true;

    //qDebug() << "Function" << declName << Qt::endl;
    //std::string dumped;
    //llvm::raw_string_ostream dumpStream(dumped);
    //Declaration->dump(dumpStream);
    //qDebug().noquote() << QString::fromStdString(dumpStream.str()) << Qt::endl;

    //qDebug() << "inMethod:" << inMethod << "inClass:" << inClass << Qt::endl;

    if (!inMethod && !inClass) {
        if (!declName.contains("::")) {
            Type const *returnType = makeTypePtrFromQualType(Declaration->getReturnType()->getCanonicalTypeUnqualified());
            if (!returnType) {
                return true;
            }
            for (auto const &p : Declaration->parameters()) {
                Type const *t = makeTypePtrFromQualType(p->getType());
                if (!t) {
                    return true;
                }
            }
            Function currentFunction = Function(declName, nspace.join("::"), returnType);
            currentFunction.setFileName(m_header);
            // build parameter list
            inMethod = true;

            for (auto const &p : Declaration->parameters()) {
                p->getType();
                QString name = QString::fromStdString(p->getNameAsString());

                QString defaultValue;
                auto defaultarg = p->getDefaultArg();
                if (defaultarg) {
                    // this parameter has a default value

                    {
                        //std::string dumped;
                        //llvm::raw_string_ostream dumpStream(dumped);
                        //defaultarg->dump(dumpStream, *context);
                        //qDebug().noquote() << "default:" << Qt::endl << QString::fromStdString(dumpStream.str()) << Qt::endl;
                    }

                    ClangDefaultExpressionVisitor ev(clang::PrintingPolicy(context->getLangOpts()), context);
                    ev.Visit(defaultarg);
                    defaultValue = ev.str();
                    //qDebug() << "converts to: " << defaultValue << Qt::endl;
                    if (defaultValue.isEmpty()) {
                        qWarning("Default value resolved to empty string");
                    }
                }

                Type const *t = makeTypePtrFromQualType(p->getType());
                if (t != types[t->toString()]) {
                    qFatal("types do not match");
                }
                currentFunction.appendParameter(Parameter(name, t, defaultValue));
            }

            inMethod = false;
            functions.insert(currentFunction.toString(), currentFunction);
        }
    }
    return true;
}

bool ClangGeneratorVisitor::TraverseFriendDecl(clang::FriendDecl *Declaration) {
    //we just don't go into friend declarations. these can have other CXXRecordDecl, CXXMethodDecl, FunctionDecl, making the
    //Visit functions of those think they belong into the class they are friend with.
    return true;
}

bool ClangGeneratorVisitor::TraverseStmt(clang::Stmt *Declaration) {
    //do not traverse into Statements
    return true;
}

ClangDefaultExpressionVisitor::ClangDefaultExpressionVisitor(clang::PrintingPolicy const &Policy, clang::ASTContext *context)
: context(context), Policy(Policy)
{
}

void ClangDefaultExpressionVisitor::VisitDeclRefExpr(clang::DeclRefExpr *Node) {
    result += QString::fromStdString(Node->getDecl()->getQualifiedNameAsString());
}

void ClangDefaultExpressionVisitor::VisitCXXConstructExpr(clang::CXXConstructExpr *E) {
    if (E->isListInitialization() && !E->isStdInitListInitialization())
        result += "{";

    for (unsigned i = 0, e = E->getNumArgs(); i != e; ++i) {
        if (clang::isa<clang::CXXDefaultArgExpr>(E->getArg(i))) {
            // Don't print any defaulted arguments
            break;
        }

        if (i) result += ", ";
        Visit(E->getArg(i));
    }

    if (E->isListInitialization() && !E->isStdInitListInitialization())
        result += "}";
}

void ClangDefaultExpressionVisitor::VisitImplicitCastExpr(clang::ImplicitCastExpr *Node) {
    // No need to print anything, simply forward to the subexpression.
    Visit(Node->getSubExpr());
}

void ClangDefaultExpressionVisitor::VisitIntegerLiteral(clang::IntegerLiteral *Node) {
    bool isSigned = Node->getType()->isSignedIntegerType();
    uint64_t v = Node->getValue().getLimitedValue();

    if(isSigned) {
        result += QString::number((int64_t)v, 10);
    } else {
        result += QString::number(v, 10);
    }

    // Emit suffixes.  Integer literals are always a builtin integer type.
    switch (Node->getType()->castAs<clang::BuiltinType>()->getKind()) {
    default: llvm_unreachable("Unexpected type for integer literal!");
    case clang::BuiltinType::Char_S:
    case clang::BuiltinType::Char_U:    result += "i8"; break;
    case clang::BuiltinType::UChar:     result += "Ui8"; break;
    case clang::BuiltinType::SChar:     result += "i8"; break;
    case clang::BuiltinType::Short:     result += "i16"; break;
    case clang::BuiltinType::UShort:    result += "Ui16"; break;
    case clang::BuiltinType::Int:       break; // no suffix.
    case clang::BuiltinType::UInt:      result += 'U'; break;
    case clang::BuiltinType::Long:      result += 'L'; break;
    case clang::BuiltinType::ULong:     result += "UL"; break;
    case clang::BuiltinType::LongLong:  result += "LL"; break;
    case clang::BuiltinType::ULongLong: result += "ULL"; break;
    case clang::BuiltinType::Int128:
        break; // no suffix.
    case clang::BuiltinType::UInt128:
        break; // no suffix.
    case clang::BuiltinType::WChar_S:
    case clang::BuiltinType::WChar_U:
        break; // no suffix
    }
}

void ClangDefaultExpressionVisitor::VisitCXXFunctionalCastExpr(clang::CXXFunctionalCastExpr *Node) {
    auto TargetType = Node->getType();
    auto *Auto = TargetType->getContainedDeducedType();
    bool Bare = Auto && Auto->isDeduced();

    // Parenthesize deduced casts.
    if (Bare)
        result += '(';

    std::string dumped;
    llvm::raw_string_ostream dumpStream(dumped);
    TargetType.print(dumpStream, Policy);
    result += QString::fromStdString(dumpStream.str());

    if (Bare)
        result += ')';

    // No extra braces surrounding the inner construct.
    if (!Node->isListInitialization())
        result += '(';
    Visit(Node->getSubExpr());
    if (!Node->isListInitialization())
        result += ')';
}

void ClangDefaultExpressionVisitor::VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr *Node) {
  clang::OverloadedOperatorKind Kind = Node->getOperator();
  if (Kind == clang::OO_PlusPlus || Kind == clang::OO_MinusMinus) {
    if (Node->getNumArgs() == 1) {
      result += QString(getOperatorSpelling(Kind)) + QString(" ");
      Visit(Node->getArg(0));
    } else {
      Visit(Node->getArg(0));
      result += QString(" ") + QString(getOperatorSpelling(Kind));
    }
  } else if (Kind == clang::OO_Arrow) {
    Visit(Node->getArg(0));
  } else if (Kind == clang::OO_Call || Kind == clang::OO_Subscript) {
    Visit(Node->getArg(0));
    result += (Kind == clang::OO_Call ? "(" : "[");
    for (unsigned ArgIdx = 1; ArgIdx < Node->getNumArgs(); ++ArgIdx) {
      if (ArgIdx > 1)
        result += ", ";
      if (!clang::isa<clang::CXXDefaultArgExpr>(Node->getArg(ArgIdx)))
        Visit(Node->getArg(ArgIdx));
    }
    result += (Kind == clang::OO_Call ? ")" : "]");
  } else if (Node->getNumArgs() == 1) {
    result += QString(getOperatorSpelling(Kind)) + QString(" ");
    Visit(Node->getArg(0));
  } else if (Node->getNumArgs() == 2) {
    Visit(Node->getArg(0));
    result += QString(" ") + QString(getOperatorSpelling(Kind)) + QString(" ");
    Visit(Node->getArg(1));
  } else {
    llvm_unreachable("unknown overloaded operator");
  }
}

void ClangDefaultExpressionVisitor::VisitMaterializeTemporaryExpr(clang::MaterializeTemporaryExpr *Node) {
    Visit(Node->getSubExpr());
}

void ClangDefaultExpressionVisitor::VisitCXXBindTemporaryExpr(clang::CXXBindTemporaryExpr *Node) {
    Visit(Node->getSubExpr());
}


void ClangDefaultExpressionVisitor::VisitCXXTemporaryObjectExpr(clang::CXXTemporaryObjectExpr *Node) {

    std::string dumped;
    llvm::raw_string_ostream dumpStream(dumped);
    Node->getType().print(dumpStream, Policy);
    result += QString::fromStdString(dumpStream.str());

    if (Node->isStdInitListInitialization())
        /* Nothing to do; braces are part of creating the std::initializer_list. */;
    else if (Node->isListInitialization())
        result += "{";
    else
        result += "(";
    for (clang::CXXTemporaryObjectExpr::arg_iterator Arg = Node->arg_begin(),
            ArgEnd = Node->arg_end();
            Arg != ArgEnd; ++Arg) {
        if ((*Arg)->isDefaultArgument())
            break;
        if (Arg != Node->arg_begin())
            result += ", ";
        Visit(*Arg);
    }
    if (Node->isStdInitListInitialization())
        /* See above. */;
    else if (Node->isListInitialization())
        result += "}";
    else
        result += ")";
}


void ClangDefaultExpressionVisitor::VisitCXXNullPtrLiteralExpr(clang::CXXNullPtrLiteralExpr *Node)
{
    result += "nullptr";
}

void ClangDefaultExpressionVisitor::VisitUnaryOperator(clang::UnaryOperator *Node) {
    if (!Node->isPostfix()) {
        result += QString::fromStdString(std::string(clang::UnaryOperator::getOpcodeStr(Node->getOpcode())));

        // Print a space if this is an "identifier operator" like __real, or if
        // it might be concatenated incorrectly like '+'.
        switch (Node->getOpcode()) {
        default: break;
        case clang::UO_Real:
        case clang::UO_Imag:
        case clang::UO_Extension:
            result += " ";
            break;
        case clang::UO_Plus:
        case clang::UO_Minus:
            if (clang::isa<clang::UnaryOperator>(Node->getSubExpr()))
                result += " ";
            break;
        }
    }
    Visit(Node->getSubExpr());

    if (Node->isPostfix())
        result += QString::fromStdString(std::string(clang::UnaryOperator::getOpcodeStr(Node->getOpcode())));
}

void ClangDefaultExpressionVisitor::VisitCXXNamedCastExpr(clang::CXXNamedCastExpr *Node) {
    result += QString(Node->getCastName()) + QString("<");

    std::string dumped;
    llvm::raw_string_ostream dumpStream(dumped);
    Node->getTypeAsWritten().print(dumpStream, Policy);
    result += QString::fromStdString(dumpStream.str());

    result += ">(";
    Visit(Node->getSubExpr());
    result += ")";
}

void ClangDefaultExpressionVisitor::VisitCXXStaticCastExpr(clang::CXXStaticCastExpr *Node) {
  VisitCXXNamedCastExpr(Node);
}

void ClangDefaultExpressionVisitor::VisitCXXDynamicCastExpr(clang::CXXDynamicCastExpr *Node) {
  VisitCXXNamedCastExpr(Node);
}

void ClangDefaultExpressionVisitor::VisitCXXReinterpretCastExpr(clang::CXXReinterpretCastExpr *Node) {
  VisitCXXNamedCastExpr(Node);
}

void ClangDefaultExpressionVisitor::VisitCXXConstCastExpr(clang::CXXConstCastExpr *Node) {
  VisitCXXNamedCastExpr(Node);
}

void ClangDefaultExpressionVisitor::VisitCXXBoolLiteralExpr(clang::CXXBoolLiteralExpr *Node) {
  result += (Node->getValue() ? "true" : "false");
}

void ClangDefaultExpressionVisitor::VisitCharacterLiteral(clang::CharacterLiteral *Node) {
    std::string dumped;
    llvm::raw_string_ostream dumpStream(dumped);
    clang::CharacterLiteral::print(Node->getValue(), Node->getKind(), dumpStream);
    result += QString::fromStdString(dumpStream.str());
}

void ClangDefaultExpressionVisitor::VisitCallExpr(clang::CallExpr *Call) {
  Visit(Call->getCallee());
  result += "(";
  for (unsigned i = 0, e = Call->getNumArgs(); i != e; ++i) {
    if (clang::isa<clang::CXXDefaultArgExpr>(Call->getArg(i))) {
      // Don't print any defaulted arguments
      break;
    }

    if (i) result += ", ";
    Visit(Call->getArg(i));
  }
  result += ")";
}

void ClangDefaultExpressionVisitor::VisitFloatingLiteral(clang::FloatingLiteral *Node) {
  clang::SmallString<16> Str;
  Node->getValue().toString(Str);
  result += QString::fromStdString(std::string(Str.str()));
  if (Str.find_first_not_of("-0123456789") == clang::StringRef::npos)
    result += "."; // Trailing dot in order to separate from ints.

  // Emit suffixes.  Float literals are always a builtin float type.
  switch (Node->getType()->castAs<clang::BuiltinType>()->getKind()) {
  default: llvm_unreachable("Unexpected type for float literal!");
  case clang::BuiltinType::Half:       break; // FIXME: suffix?
  case clang::BuiltinType::Ibm128:     break; // FIXME: No suffix for ibm128 literal
  case clang::BuiltinType::Double:     break; // no suffix.
  case clang::BuiltinType::Float16:    result += "F16"; break;
  case clang::BuiltinType::Float:      result += "F"; break;
  case clang::BuiltinType::LongDouble: result += "L"; break;
  case clang::BuiltinType::Float128:   result += "Q"; break;
  }
}


void ClangDefaultExpressionVisitor::VisitCXXScalarValueInitExpr(clang::CXXScalarValueInitExpr *Node) {
    std::string dumped;
    llvm::raw_string_ostream dumpStream(dumped);
    if (clang::TypeSourceInfo *TSInfo = Node->getTypeSourceInfo())
        TSInfo->getType().print(dumpStream, Policy);
    else
        Node->getType().print(dumpStream, Policy);
    result += QString::fromStdString(dumpStream.str());
    result += "()";
}

void ClangDefaultExpressionVisitor::VisitParenExpr(clang::ParenExpr *Node) {
  result += "(";
  Visit(Node->getSubExpr());
  result += ")";
}

void ClangDefaultExpressionVisitor::VisitBinaryOperator(clang::BinaryOperator *Node) {
  Visit(Node->getLHS());
  result += QString(" ") + QString::fromStdString(std::string(clang::BinaryOperator::getOpcodeStr(Node->getOpcode()))) + QString(" ");
  Visit(Node->getRHS());
}

void ClangDefaultExpressionVisitor::VisitOpaqueValueExpr(clang::OpaqueValueExpr *Node) {
    auto expr = Node->getSourceExpr();
    if(expr) {
        Visit(expr);
    } else {
        //this should never be seen and indicates an error during parsing leading to an invalid type.
        result += "< null expr >";
    }
}

void ClangDefaultExpressionVisitor::VisitInitListExpr(clang::InitListExpr* Node) {
  if (Node->getSyntacticForm()) {
    Visit(Node->getSyntacticForm());
    return;
  }

  result += "{";
  for (unsigned i = 0, e = Node->getNumInits(); i != e; ++i) {
    if (i) result += ", ";
    if (Node->getInit(i))
      Visit(Node->getInit(i));
    else
      result += "{}";
  }
  result += "}";
}
