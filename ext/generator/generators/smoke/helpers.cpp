/*
    Generator for the SMOKE sources
    Copyright (C) 2009 Arno Rehn <arno@arnorehn.de>

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

#include <QHash>
#include <QList>
#include <QStack>

#include <type.h>

#include "globals.h"
#include "../../options.h"

QHash<QString, QString> Util::typeMap;
QHash<const Method*, const Function*> Util::globalFunctionMap;
QHash<const Method*, const Field*> Util::fieldAccessors;

// looks up the inheritance path from desc to super and sets 'virt' to true if it encounters a virtual base
static bool isVirtualInheritancePathPrivate(const Class* desc, const Class* super, bool *virt)
{
    Q_FOREACH (const Class::BaseClassSpecifier bspec, desc->baseClasses()) {
        if (bspec.baseClass == super || isVirtualInheritancePathPrivate(bspec.baseClass, super, virt)) {
            if (bspec.isVirtual)
                *virt = true;
            return true;
        }
    }
    return false;
}

bool Util::isVirtualInheritancePath(const Class* desc, const Class* super)
{
    bool isVirtual = false;
    isVirtualInheritancePathPrivate(desc, super, &isVirtual);
    return isVirtual;
}

QList<const Class*> Util::superClassList(const Class* klass)
{
    static QHash<const Class*, QList<const Class*> > superClassCache;

    QList<const Class*> ret;
    if (superClassCache.contains(klass))
        return superClassCache[klass];
    Q_FOREACH (const Class::BaseClassSpecifier& base, klass->baseClasses()) {
        ret << base.baseClass;
        ret += superClassList(base.baseClass);
    }
    // cache
    superClassCache[klass] = ret;
    return ret;
}

QList<const Class*> Util::descendantsList(const Class* klass)
{
    static QHash<const Class*, QList<const Class*> > descendantsClassCache;

    QList<const Class*> ret;
    if (descendantsClassCache.contains(klass))
        return descendantsClassCache[klass];
    for (auto iter = classes.constBegin(); iter != classes.constEnd(); iter++) {
        if (superClassList(iter.value()).contains(klass))
            ret << iter.value();
    }
    // cache
    descendantsClassCache[klass] = ret;
    return ret;
}

bool operator==(const Field& lhs, const Field& rhs)
{
    return (lhs.name() == rhs.name() && lhs.declaringType() == rhs.declaringType() && lhs.type() == rhs.type());
}

bool operator==(const EnumMember& lhs, const EnumMember& rhs)
{
    return (lhs.name() == rhs.name() && lhs.declaringType() == rhs.declaringType() && lhs.type() == rhs.type());
}

void Util::preparse(QSet<Type const*> *usedTypes, QSet<const Class*> *superClasses, const QList<QString>& keys)
{
    if(!classes.contains("QGlobalSpace")) {
        classes.insert("QGlobalSpace", new Class());
    }
    Class* globalSpace = classes["QGlobalSpace"];
    globalSpace->setName("QGlobalSpace");
    globalSpace->setKind(Class::Kind_Class);
    globalSpace->setIsNameSpace(true);
    
    // add all functions as methods to a class called 'QGlobalSpace' or a class that represents a namespace
    for (QHash<QString, Function>::const_iterator it = functions.constBegin(); it != functions.constEnd(); it++) {
        const Function& fn = it.value();
        
        QString fnString = fn.toString();
        
        // gcc doesn't like this function... for whatever reason
        if (fn.name() == "_IO_ftrylockfile"
            // functions in named namespaces are covered by the class list - only check for top-level functions here
            || (fn.nameSpace().isEmpty() && !Options::functionNameIncluded(fn.qualifiedName()) && !Options::functionSignatureIncluded(fnString))
            || Options::typeExcluded(fnString))
        {
            // we don't want that function...
            continue;
        }
        
        Class* parent = globalSpace;
        if (!fn.nameSpace().isEmpty()) {
            parent = classes[fn.nameSpace()];
            if (parent->name().isEmpty()) {
                parent->setName(fn.nameSpace());
                parent->setKind(Class::Kind_Class);
                parent->setIsNameSpace(true);
            }
        }
        
        Method meth = Method(parent, fn.name(), fn.type(), Access_public, fn.parameters());
        meth.setFlag(Method::Static);
        if(!parent->methods().contains(meth)) {
            parent->appendMethod(meth);
        }
        // map this method to the function, so we can later retrieve the header it was defined in
        globalFunctionMap[&parent->methods().last()] = &fn;
        
        int methIndex = parent->methods().size() - 1;
        addOverloads(meth);
        // handle the methods appended by addOverloads()
        for (int i = parent->methods().size() - 1; i > methIndex; --i)
            globalFunctionMap[&parent->methods()[i]] = &fn;

        (*usedTypes) << meth.type();
        (*usedTypes) << meth.type()->resolveTypedefs();
        Q_FOREACH (const Parameter& param, meth.parameters()) {
            (*usedTypes) << param.type();
            (*usedTypes) << param.type()->resolveTypedefs();
        }
    }
    
    // all enums that don't have a parent are put under QGlobalSpace, too
    for (auto it = enums.begin(); it != enums.end(); it++) {
        Enum* e = it.value();
        if (!e->parent()) {
            Class* parent = globalSpace;
            if (!e->nameSpace().isEmpty()) {
                parent = classes[e->nameSpace()];
                if (parent->name().isEmpty()) {
                    parent->setName(e->nameSpace());
                    parent->setKind(Class::Kind_Class);
                    parent->setIsNameSpace(true);
                }
            }
            if (Options::typeExcluded(e->toString())) {
                continue;
            }

            Type const *t = 0;
            if (e->name().isEmpty()) {
                // unnamed enum
                Type longType = Type("long");
                longType.setIsIntegral(true);
                t = Type::registerType(longType);
            } else {
                t = Type::registerType(Type(e));
            }
            (*usedTypes) << t;
            Q_FOREACH (const EnumMember& member, e->members()) {
                if (Options::typeExcluded(member.toString())) {
                    e->membersRef().removeOne(member);
                }
            }
            if(!parent->enums().contains(e)) {
                parent->appendEnum(e);
            }
        }
    }
    
    Q_FOREACH (const QString& key, keys) {
        Class* klass = classes[key];
        Q_FOREACH (const Class::BaseClassSpecifier base, klass->baseClasses()) {
            superClasses->insert(base.baseClass);
        }
        if (!klass->isNameSpace()) {
            addDefaultConstructor(klass);
            addCopyConstructor(klass);
            addDestructor(klass);
            checkForAbstractClass(klass);
            //qDebug() << "Collecting types in class" << klass->toString() << Qt::endl;
            Q_FOREACH (const Method& m, klass->methods()) {
                //qDebug() << "Collecting types in method(checking)" << m.toString() << Qt::endl;
                if (m.access() == Access_private)
                    continue;
                if (m.isDeleted())
                    continue;
                bool param_types_usable = true;
                Q_FOREACH (const Parameter& param, m.parameters()) {
                    if (typeAccess(param.type()) == Access_private) {
                        param_types_usable = false;
                        break;
                    }
                }
                if ((typeAccess(m.type()) == Access_private)
                        || Options::typeExcluded(m.toString(false, true))
                        || !param_types_usable)
                {
                    klass->methodsRef().removeOne(m);
                    continue;
                }
                //qDebug() << "Collecting types in method" << m.toString() << Qt::endl;
                addOverloads(m);
                (*usedTypes) << m.type();
                (*usedTypes) << m.type()->resolveTypedefs();
                Q_FOREACH (const Parameter& param, m.parameters()) {
                    //qDebug() << "Collecting type in parameter" << param.type()->toString() << param.type() << types[param.type()->toString()] << Qt::endl;
                    if(param.type() != types[param.type()->toString()]) {
                        qFatal("parameter has type that does not match type registry");
                    }
                    (*usedTypes) << param.type();
                    (*usedTypes) << param.type()->resolveTypedefs();

                    if (m.isSlot() || m.isSignal() || m.isQPropertyAccessor()) {
                        (*usedTypes) << Util::normalizeType(param.type());
                    }
                }
            }
            Q_FOREACH (const Field& f, klass->fields()) {
                if (f.access() == Access_private)
                    continue;
                if (Options::typeExcluded(f.toString(false, true))) {
                    klass->fieldsRef().removeOne(f);
                    continue;
                }
                //exclude array types, we cannot generate accessors for those at the moment. TODO
                if(f.type()->arrayDimensions() > 0) {
                    klass->fieldsRef().removeOne(f);
                    continue;
                }
            }
            Q_FOREACH (const Field& f, klass->fields()) {
                if (f.access() == Access_private)
                    continue;
                addAccessorMethods(f, usedTypes);
            }
        }
        Q_FOREACH (BasicTypeDeclaration* decl, klass->enums()) {
            Enum* e = 0;
            if ((e = dynamic_cast<Enum*>(decl))) {
                if (Options::typeExcluded(e->toString())) {
                    e->setAccess(Access_private);
                    continue;
                }
                Type const *t = 0;
                if (e->name().isEmpty()) {
                    // unnamed enum
                    Type longType = Type("long");
                    longType.setIsIntegral(true);
                    t = Type::registerType(longType);
                } else {
                    t = Type::registerType(Type(e));
                }
                (*usedTypes) << t;
                Q_FOREACH (const EnumMember& member, e->members()) {
                    if (Options::typeExcluded(member.toString())) {
                        e->membersRef().removeOne(member);
                    }
                }
            }
            
        }
        
    }
}

bool Util::canClassBeInstanciated(const Class* klass)
{
    static QHash<const Class*, bool> cache;
    if (cache.contains(klass))
        return cache[klass];
    
    bool ctorFound = false, publicCtorFound = false, privatePureVirtualsFound = false;
    Q_FOREACH (const Method& meth, klass->methods()) {
        if (meth.isConstructor()) {
            ctorFound = true;
            if (meth.access() != Access_private && !meth.isDeleted()) {
                // this class can be instanstiated
                publicCtorFound = true;
            }
        } else if ((meth.flags() & Method::PureVirtual) && meth.access() == Access_private) {
            privatePureVirtualsFound = true;
        }
    }
    
    // The class can be instanstiated if it has a public constructor or no constructor at all
    // because then it has a default one generated by the compiler.
    // If it has private pure virtuals, then it can't be instanstiated either.
    bool ret = ((publicCtorFound || !ctorFound) && !privatePureVirtualsFound);
    cache[klass] = ret;
    return ret;
}

bool Util::canClassBeCopied(const Class* klass)
{
    static QHash<const Class*, bool> cache;
    if (cache.contains(klass))
        return cache[klass];

    bool privateCopyCtorFound = false;
    Q_FOREACH (const Method& meth, klass->methods()) {
        if (meth.access() != Access_private && !meth.isDeleted())
            continue;
        if (meth.isConstructor() && meth.parameters().count() == 1) {
            const Type *type = meth.parameters()[0].type();
            // c'tor should be Foo(const Foo& copy)
            if (type->isConst() && type->isRef() && type->getClass() == klass) {
                privateCopyCtorFound = true;
                break;
            }
        }
    }
    
    bool parentCanBeCopied = true;
    Q_FOREACH (const Class::BaseClassSpecifier& base, klass->baseClasses()) {
        if (!canClassBeCopied(base.baseClass)) {
            parentCanBeCopied = false;
            break;
        }
    }

    //qDebug() << "class" << klass->toString() << "can be copied?" << parentCanBeCopied << privateCopyCtorFound << Qt::endl;
    
    // if the parent can be copied and we didn't find a private copy c'tor, the class is copiable
    bool ret = (parentCanBeCopied && !privateCopyCtorFound);
    cache[klass] = ret;
    return ret;
}

bool Util::hasClassVirtualDestructor(const Class* klass)
{
    static QHash<const Class*, bool> cache;
    if (cache.contains(klass))
        return cache[klass];

    bool virtualDtorFound = false;
    Q_FOREACH (const Method& meth, klass->methods()) {
        if (meth.isDestructor() && meth.flags() & Method::Virtual) {
            virtualDtorFound = true;
            break;
        }
    }
    
    bool superClassHasVirtualDtor = false;
    Q_FOREACH (const Class::BaseClassSpecifier& bspec, klass->baseClasses()) {
        if (hasClassVirtualDestructor(bspec.baseClass)) {
            superClassHasVirtualDtor = true;
            break;
        }
    }
    
    // if the superclass has a virtual d'tor, then the descendants have one automatically, too
    bool ret = (virtualDtorFound || superClassHasVirtualDtor);
    cache[klass] = ret;
    return ret;
}

bool Util::hasClassPublicDestructor(const Class* klass)
{
    static QHash<const Class*, bool> cache;
    if (cache.contains(klass))
        return cache[klass];

    if (klass->isNameSpace()) {
        cache[klass] = false;
        return false;
    }

    bool publicDtorFound = true;
    Q_FOREACH (const Method& meth, klass->methods()) {
        if (meth.isDestructor()) {
            if (meth.access() != Access_public || meth.isDeleted())
                publicDtorFound = false;
            // a class has only one destructor, so break here
            break;
        }
    }
    
    cache[klass] = publicDtorFound;
    return publicDtorFound;
}

const Method* Util::findDestructor(const Class* klass)
{
    Q_FOREACH (const Method& meth, klass->methods()) {
        if (meth.isDestructor()) {
            return &meth;
        }
    }
    const Method* dtor = 0;
    Q_FOREACH (const Class::BaseClassSpecifier& bspec, klass->baseClasses()) {
        if ((dtor = findDestructor(bspec.baseClass))) {
            return dtor;
        }
    }
    return 0;
}

void Util::checkForAbstractClass(Class* klass)
{
    QList<const Method*> list;
    
    bool hasPrivatePureVirtuals = false;
    Q_FOREACH (const Method& meth, klass->methods()) {
        if ((meth.flags() & Method::PureVirtual) && meth.access() == Access_private)
            hasPrivatePureVirtuals = true;
        if (meth.isConstructor())
            list << &meth;
    }
    
    // abstract classes can't be instanstiated - remove the constructors
    if (hasPrivatePureVirtuals) {
        Q_FOREACH (const Method* ctor, list) {
            klass->methodsRef().removeOne(*ctor);
        }
    }
}

void Util::addDefaultConstructor(Class* klass)
{
    Q_FOREACH (const Method& meth, klass->methods()) {
        // if the class already has a constructor or if it has pure virtuals, there's nothing to do for us
        if (meth.isConstructor())
            return;
        else if (meth.isDestructor() && (meth.access() == Access_private || meth.isDeleted()))
            return;
    }
    
    Type t = Type(klass);
    t.setPointerDepth(1);
    Method meth = Method(klass, klass->name(), Type::registerType(t));
    meth.setIsConstructor(true);
    if (!klass->methods().contains(meth)) {
        klass->appendMethod(meth);
    }
}

void Util::addCopyConstructor(Class* klass)
{
    Q_FOREACH (const Method& meth, klass->methods()) {
        if (meth.isConstructor() && meth.parameters().count() == 1) {
            const Type* type = meth.parameters()[0].type();
            // found a copy c'tor? then there's nothing to do
            if (type->isRef() && type->getClass() == klass)
                return;
        } else if (meth.isDestructor() && (meth.access() == Access_private || meth.isDeleted())) {
            // private destructor, so we can't create instances of that class
            return;
        }
    }
    
    // if the parent can't be copied, a copy c'tor is of no use
    Q_FOREACH (const Class::BaseClassSpecifier& base, klass->baseClasses()) {
        if (!canClassBeCopied(base.baseClass))
            return;
    }
    
    Type t = Type(klass);
    t.setPointerDepth(1);
    Method meth = Method(klass, klass->name(), Type::registerType(t));
    meth.setIsConstructor(true);
    // parameter is a constant reference to another object of the same types
    Type paramType = Type(klass, true);
    paramType.setIsRef(true);
    meth.appendParameter(Parameter("copy", Type::registerType(paramType)));
    if (!klass->methods().contains(meth)) {
        klass->appendMethod(meth);
    }
}

void Util::addDestructor(Class* klass)
{
    Q_FOREACH (const Method& meth, klass->methods()) {
        // we already have a destructor
        if (meth.isDestructor())
            return;
    }

    Method meth = Method(klass, "~" + klass->name(), const_cast<Type*>(Type::Void));
    meth.setIsDestructor(true);
    
    const Method* dtor = findDestructor(klass);
    if (dtor && dtor->hasExceptionSpec()) {
        meth.setHasExceptionSpec(true);
        Q_FOREACH (Type const* t, dtor->exceptionTypes()) {
            meth.appendExceptionType(t);
        }
    }
    
    if (!klass->methods().contains(meth)) {
        klass->appendMethod(meth);
    }
}

/*
 * types are munged to indicate their kind
 *
 * '#': c++ object (also simple pointer/reference), except for templated classes(at least that used to be true, with clang we actually have specialized templates as proper classes), also except for every type that is in Options::scalarTypes or Options::voidpTypes
 * '$': Numbers and Enums (also simple pointer/reference), also everything in Options::scalarTypes that is not in Options::voidpTypes
 * '?': Multi-pointers, templated classes, everything in Options::voidpTypes, and everything that does not fit one of the above.
 *
 */
QChar Util::munge(const Type *type) {
    if (type->getTypedef()) {
        //only try to resolve once; resolveTypedefs may elect to not resolve some typedefs to their class/struct.
        type = type->resolveTypedefs();
    }

    if (type->pointerDepth() > 1 ||
        (type->getClass() && type->getClass()->isTemplate()) ||
        (Options::voidpTypes.contains(type->name()) && !Options::scalarTypes.contains(type->name())) )
    {
        // QString and QStringList are both mapped to Smoke::t_voidp, but QString is a scalar as well
        // TODO: fix this - neither QStringList nor QString should be mapped to Smoke::t_voidp or munged as ? or $

        if (type->pointerDepth() > 1) {
            //qDebug() << "type is multi pointer(munged to \"?\"):" << type->toString() << Qt::endl;
        }
        if (type->getClass() && type->getClass()->isTemplate()) {
            //qDebug() << "type is class and template(munged to \"?\"):" << type->toString() << Qt::endl;
        }
        if (Options::voidpTypes.contains(type->name()) && !Options::scalarTypes.contains(type->name())) {
            //qDebug() << "type is forced to voidp(munged to \"?\"):" << type->toString() << Qt::endl;
        }

        // reference to array or hash or unknown
        return '?';
    } else if (type->isIntegral() ||
               type->getEnum() ||
               Options::scalarTypes.contains(type->name()) ||
               (!type->getClass() && !type->getEnum() && type->name() == "QIntegerForSizeof< void* >::Unsigned"))
    {
        if (type->isIntegral()) {
            //qDebug() << "type is integral(munged to \"$\"):" << type->toString() << Qt::endl;
        }
        if (type->getEnum()) {
            //qDebug() << "type is enum(munged to \"$\"):" << type->toString() << Qt::endl;
        }
        if(Options::scalarTypes.contains(type->name())) {
            //qDebug() << "type is forced to scalar(munged to \"$\"):" << type->toString() << Qt::endl;
        }
        if (Options::qtMode && !type->isRef() && type->pointerDepth() == 0 &&
                (type->getClass() && type->getClass()->isTemplate())) {
            //qDebug() << "type is class and template in qtmode(munged to \"$\"):" << type->toString() << Qt::endl;
        }
        if (!type->getClass() && !type->getEnum() && type->name() == "QIntegerForSizeof< void* >::Unsigned") {
            //qDebug() << "type is QIntegerForSizeof< void* >::Unsigned(munged to \"$\"):" << type->toString() << Qt::endl;
        }
        // plain scalar
        return '$';
    } else if (type->getClass()) {
        //qDebug() << "type is class(munged to \"#\"):" << type->toString() << Qt::endl;
        // object
        return '#';
    } else {
        //qDebug() << "type is unknown(munged to \"?\"):" << type->toString() << Qt::endl;
        // unknown
        return '?';
    }
}

QString Util::mungedName(const Method& meth) {
    QString ret = meth.name();
    Q_FOREACH (const Parameter& param, meth.parameters()) {
        const Type* type = param.type();
        ret += munge(type);
   }
    return ret;
}

Type const* Util::normalizeType(const Type* type) {
    Type normalizedType = *type;
    if (normalizedType.isConst() && normalizedType.isRef()) {
        normalizedType.setIsConst(false);
        normalizedType.setIsRef(false);
    }

    if (normalizedType.pointerDepth() == 0) {
        normalizedType.setIsConst(false);
    }

    return Type::registerType(normalizedType);
}

QString Util::stackItemField(const Type* type)
{
    if (type->getTypedef()) {
        //only try to resolve once; resolveTypedefs may elect to not resolve some typedefs to their class/struct.
        type = type->resolveTypedefs();
    }

    if (Options::qtMode && !type->isRef() && type->pointerDepth() == 0 &&
        type->getClass() && type->getClass()->isTemplate())
    {
        return "s_uint";
    }

    if (type->pointerDepth() > 0 || type->isRef() || type->isFunctionPointer() || type->isArray() || Options::voidpTypes.contains(type->name())
        || (!type->isIntegral() && !type->getEnum()))
    {
        return "s_class";
    }
    
    if (type->getEnum())
        return "s_enum";
    
    QString typeName = type->name();
    // replace the unsigned stuff, look the type up in Util::typeMap and if
    // necessary, add a 'u' for unsigned types at the beginning again
    bool _unsigned = false;
    if (typeName.contains("unsigned ")) {
        typeName.replace("unsigned ", "");
        _unsigned = true;
    }
    typeName.replace("signed ", "");
    typeName = Util::typeMap.value(typeName, typeName);
    if (_unsigned)
        typeName.prepend('u');
    return "s_" + typeName;
}

QString Util::assignmentString(const Type* type, const QString& var)
{
    if (type->getTypedef()) {
        //only try to resolve once; resolveTypedefs may elect to not resolve some typedefs to their class/struct.
        type = type->resolveTypedefs();
    }

    if (type->pointerDepth() > 0 || type->isFunctionPointer()) {
        return "(void*)" + var;
    } else if (type->isRef()) {
        return "(void*)&" + var;
    } else if (type->isIntegral() && !Options::voidpTypes.contains(type->name())) {
        return var;
    } else if (type->getEnum()) {
        return "(long)" + var;
    } else if (Options::qtMode && type->getClass() && type->getClass()->isTemplate())
    {
        return "(uint)" + var;
    } else {
        QString ret = "(void*)new " + type->toString(QString(), true);
        ret += '(' + var + ')';
        return ret;
    }
    return QString();
}

QList<const Method*> Util::collectVirtualMethods(const Class* klass)
{
    QList<const Method*> methods;
    Q_FOREACH (const Method& meth, klass->methods()) {
        if ((meth.flags() & Method::Virtual || meth.flags() & Method::PureVirtual)
            && !meth.isDestructor() && meth.access() != Access_private && !meth.isDeleted()
            && !Options::typeExcluded(meth.toString(false, true))
	)
        {
            methods << &meth;
        }
    }
    Q_FOREACH (const Class::BaseClassSpecifier& baseClass, klass->baseClasses()) {
        methods += collectVirtualMethods(baseClass.baseClass);
    }
    return methods;
}

// don't make this public - it's just a utility function for the next method and probably not what you would expect it to be
static bool operator==(const Method& rhs, const Method& lhs)
{
    // These have to be equal for methods to be the same. Return types don't have an effect, ignore them.
    bool ok = (rhs.name() == lhs.name() && rhs.isConst() == lhs.isConst() && rhs.parameters().count() == lhs.parameters().count());
    if (!ok)
        return false;
    
    // now check the parameter types for equality
    for (int i = 0; i < rhs.parameters().count(); i++) {
        if (rhs.parameters()[i].type() != lhs.parameters()[i].type())
            return false;
    }
    
    return true;
}

void Util::addAccessorMethods(const Field& field, QSet<Type const*> *usedTypes)
{
    Class* klass = field.getClass();
    Type const* type = field.type();
    if (type->getClass() && type->pointerDepth() == 0 && !(ParserOptions::qtMode)) {
        Type newType = *type;
        newType.setIsRef(true);
        type = Type::registerType(newType);
    }
    (*usedTypes) << type;
    (*usedTypes) << type->resolveTypedefs();
    Method getter = Method(klass, field.name(), type, field.access());
    getter.setIsConst(true);
    if (field.flags() & Field::Static)
        getter.setFlag(Method::Static);
    if (!klass->methods().contains(getter)) {
        klass->appendMethod(getter);
    }
    fieldAccessors[&klass->methods().last()] = &field;
    
    // constant field? (i.e. no setter method)
    if (field.type()->isConst() && field.type()->pointerDepth() == 0)
        return;
    
    // foo => setFoo
    QString newName = field.name();
    newName[0] = newName[0].toUpper();
    Method setter = Method(klass, "set" + newName, const_cast<Type*>(Type::Void), field.access());
    if (field.flags() & Field::Static)
        setter.setFlag(Method::Static);
    
    // reset
    type = field.type();
    // to avoid copying around more stuff than necessary, convert setFoo(Bar) to setFoo(const Bar&)
    if (type->pointerDepth() == 0 && type->getClass() && !(ParserOptions::qtMode)) {
        Type newType = *type;
        newType.setIsRef(true);
        newType.setIsConst(true);
        type = Type::registerType(newType);
    }

    (*usedTypes) << type;
    (*usedTypes) << type->resolveTypedefs();
    setter.appendParameter(Parameter(QString(), type));
    if (klass->methods().contains(setter))
        return;
    klass->appendMethod(setter);
    fieldAccessors[&klass->methods().last()] = &field;
}

void Util::addOverloads(const Method& meth)
{
    ParameterList params;
    Class* klass = meth.getClass();
    
    for (int i = 0; i < meth.parameters().count(); i++) {
        const Parameter& param = meth.parameters()[i];
        if (!param.isDefault()) {
            params << param;
            continue;
        }
        Method overload = meth;
        if (meth.flags() & Method::PureVirtual) {
            overload.setFlag(Method::DynamicDispatch);
        }
        overload.removeFlag(Method::Virtual);
        overload.removeFlag(Method::PureVirtual);
        overload.setParameterList(params);
        if (klass->methods().contains(overload)) {
            // we already have that, skip it
            params << param;
            continue;
        }
        
        QStringList remainingDefaultValues;
        for (int j = i; j < meth.parameters().count(); j++) {
            const Parameter defParam = meth.parameters()[j];
            QString cast = "(";
            cast += defParam.type()->toString() + ')';
            cast += defParam.defaultValue();
            remainingDefaultValues << cast;
        }
        overload.setRemainingDefaultValues(remainingDefaultValues);
        klass->appendMethod(overload);
        
        params << param;
    }
}

// checks if method meth is overriden in class klass or any of its superclasses
const Method* Util::isVirtualOverriden(const Method& meth, const Class* klass)
{
    // is the method virtual at all?
    if (!(meth.flags() & Method::Virtual) && !(meth.flags() & Method::PureVirtual))
        return 0;
    
    // if the method is defined in klass, it can't be overriden there or in any parent class
    if (meth.getClass() == klass)
        return 0;
    
    Q_FOREACH (const Method& m, klass->methods()) {
        if (!(m.flags() & Method::Static) && m == meth)
            // the method m overrides meth
            return &m;
    }
    
    Q_FOREACH (const Class::BaseClassSpecifier& base, klass->baseClasses()) {
        // we reached the class in which meth was defined and we still didn't find any overrides => return
        if (base.baseClass == meth.getClass())
            return 0;
        
        // recurse into the base classes
        const Method* m = 0;
        if ((m = isVirtualOverriden(meth, base.baseClass)))
            return m;
    }
    
    return 0;
}

static bool qListContainsMethodPointer(const QList<const Method*> list, const Method* ptr) {
    Q_FOREACH (const Method* meth, list) {
        if (*meth == *ptr)
            return true;
    }
    return false;
}

QList<const Method*> Util::virtualMethodsForClass(const Class* klass)
{
    static QHash<const Class*, QList<const Method*> > cache;
    
    // virtual method callbacks for classes that can't be instanstiated aren't useful
    if (!Util::canClassBeInstanciated(klass))
        return QList<const Method*>();
    
    if (cache.contains(klass))
        return cache[klass];
    
    QList<const Method*> ret;

    Q_FOREACH (const Method* meth, Util::collectVirtualMethods(klass)) {
        // this is a synthesized overload, skip it.
        if (!meth->remainingDefaultValues().isEmpty())
            continue;
        if (meth->getClass() == klass) {
            // this method can't be overriden, because it's defined in the class for which this method was called
            ret << meth;
            continue;
        }
        // Check if the method is overriden, so the callback will always point to the latest definition of the virtual method.
        const Method* override = 0;
        if ((override = Util::isVirtualOverriden(*meth, klass))) {
            // If the method was overriden and put under private access, skip it. If we already have the method, skip it as well.
            if (override->access() == Access_private || qListContainsMethodPointer(ret, override))
                continue;
            ret << override;
        } else if (!qListContainsMethodPointer(ret, meth)) {
            ret << meth;
        }
    }

    cache[klass] = ret;
    return ret;
}

Access Util::typeAccess(const Type *type) {
    if(type->getClass()) {
        return type->getClass()->access();
    }
    if(type->getTypedef()) {
        return type->getTypedef()->access();
    }
    if(type->getEnum()) {
        return type->getEnum()->access();
    }
    return Access_public;
}

bool Options::typeExcluded(const QString& typeName)
{
    Q_FOREACH (const QRegExp& exp, Options::excludeExpressions) {
        if (exp.exactMatch(typeName))
            return true;
    }
    return false;
}

bool Options::functionNameIncluded(const QString& fnName) {
    Q_FOREACH (const QRegExp& exp, Options::includeFunctionNames) {
        if (exp.exactMatch(fnName))
            return true;
    }
    return false;
}

bool Options::functionSignatureIncluded(const QString& sig) {
    Q_FOREACH (const QRegExp& exp, Options::includeFunctionNames) {
        if (exp.exactMatch(sig))
            return true;
    }
    return false;
}

