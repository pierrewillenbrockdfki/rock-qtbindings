/***************************************************************************
    marshall_types.h - Derived from the PerlQt sources, see AUTHORS 
                       for details
                             -------------------
    begin                : Fri Jul 4 2003
    copyright            : (C) 2003-2006 by Richard Dale
    email                : Richard_Dale@tipitina.demon.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef MARSHALL_TYPES_H
#define MARSHALL_TYPES_H

#include <QtCore/qstring.h>
#include <QtCore/qobject.h>
#include <QtCore/qmetaobject.h>

#include <smoke/smoke.h>

#include "marshall.h"
#include "qtruby.h"
#include "smokeruby.h"

Marshall::HandlerFn getMarshallFn(const SmokeType &type);

extern void smokeStackToQtStack(Smoke::Stack stack, void ** o, int start, int end, QList<MocArgument*> args);
extern void smokeStackFromQtStack(Smoke::Stack stack, void ** _o, int start, int end, QList<MocArgument*> args);

namespace QtRuby {

class Q_DECL_EXPORT MethodReturnValueBase : public Marshall 
{
public:
	MethodReturnValueBase(Smoke::ModuleIndex mi, Smoke::Stack stack);
	const Smoke::Method &method();
	Smoke::StackItem &item() override;
	Smoke *smoke() override;
	SmokeType type() override;
	void next() override;
	bool cleanup() override;
	void unsupported() override;
    VALUE * var() override;
protected:
	Smoke *_smoke;
	Smoke::Index _method;
	Smoke::Stack _stack;
	SmokeType _st;
	VALUE *_retval;
	virtual const char *classname();
};


class Q_DECL_EXPORT VirtualMethodReturnValue : public MethodReturnValueBase {
public:
	VirtualMethodReturnValue(Smoke::ModuleIndex mi, Smoke::Stack stack, VALUE retval);
	Marshall::Action action() override;

private:
	VALUE _retval2;
};


class Q_DECL_EXPORT MethodReturnValue : public MethodReturnValueBase {
public:
	MethodReturnValue(Smoke::ModuleIndex mi, Smoke::Stack stack, VALUE * retval);
    Marshall::Action action() override;

private:
	const char *classname() override;
};

class Q_DECL_EXPORT MethodCallBase : public Marshall
{
public:
	MethodCallBase(Smoke::ModuleIndex mi, QHash<unsigned int, Smoke::ModuleIndex> const &conversions);
	MethodCallBase(Smoke::ModuleIndex mi, QHash<unsigned int, Smoke::ModuleIndex> const &conversions,
				   Smoke::Stack stack);
	Smoke *smoke() override;
	SmokeType type() override;
	Smoke::StackItem &item() override;
	const Smoke::Method &method();
	virtual int items() = 0;
	virtual void callMethod() = 0;	
	void next() override;
	void unsupported() override;

protected:
	Smoke *_smoke;
	Smoke::Index _method;
	QHash<unsigned int, Smoke::ModuleIndex> _conversionConstructors;
	Smoke::Stack _stack;
	int _cur;
	Smoke::Index *_args;
	bool _called;
	VALUE *_sp;
	virtual const char* classname();
};


class Q_DECL_EXPORT VirtualMethodCall : public MethodCallBase {
public:
	VirtualMethodCall(Smoke::ModuleIndex mi, QHash<unsigned int, Smoke::ModuleIndex> const &conversions,
					  Smoke::Stack stack, VALUE obj, VALUE *sp);
	~VirtualMethodCall();
	Marshall::Action action() override;
	VALUE * var() override;
	int items() override;
	void callMethod() override;
	bool cleanup() override;
 
private:
	VALUE _obj;
};


class Q_DECL_EXPORT MethodCall : public MethodCallBase {
public:
	MethodCall(Smoke::ModuleIndex mi, QHash<unsigned int, Smoke::ModuleIndex> const &conversionConstructors,
			   VALUE target, VALUE *sp, int items);
	~MethodCall();
	Marshall::Action action() override;
	VALUE * var() override;

	inline void callMethod() override {
		if(_called) return;
		_called = true;

		if (_target == Qnil && !(method().flags & (Smoke::mf_static | Smoke::mf_ctor))) {
			rb_raise(rb_eArgError, "%s is not a class method\n", _smoke->methodNames[method().name]);
		}
	
		Smoke::ClassFn fn = _smoke->classes[method().classId].classFn;
		void * ptr = 0;

		if (_o != 0) {
			const Smoke::Class &cl = _smoke->classes[method().classId];

			ptr = _o->smoke->cast(	_o->ptr,
									_o->classId,
									_o->smoke->idClass(cl.className, true).index );
		}

		_items = -1;
		(*fn)(method().method, ptr, _stack);
		if (method().flags & Smoke::mf_ctor) {
			Smoke::StackItem s[2];
			s[1].s_voidp = qtruby_modules[_smoke].binding;
			(*fn)(0, _stack[0].s_voidp, s);
		}
		MethodReturnValue r(Smoke::ModuleIndex(_smoke, _method), _stack, &_retval);
	}

	int items() override;
	bool cleanup() override;
private:
	VALUE _target;
	smokeruby_object * _o;
	VALUE *_sp;
	int _items;
	VALUE _retval;
	const char *classname() override;
};


class Q_DECL_EXPORT SigSlotBase : public Marshall {
public:
	SigSlotBase(QList<MocArgument*> args);
	~SigSlotBase();
	const MocArgument &arg();
	SmokeType type() override;
	Smoke::StackItem &item() override;
	VALUE * var() override;
	Smoke *smoke() override;
	virtual const char *mytype() = 0;
	virtual void mainfunction() = 0;
	void unsupported() override;
	void next() override;
	void prepareReturnValue(void** o);

protected:
	QList<MocArgument*> _args;
	int _cur;
	bool _called;
	Smoke::Stack _stack;
	int _items;
	VALUE *_sp;
};


class Q_DECL_EXPORT EmitSignal : public SigSlotBase {
    QObject *_obj;
    int _id;
	VALUE * _result;
 public:
    EmitSignal(QObject *obj, int id, int items, QList<MocArgument*> args, VALUE * sp, VALUE * result);
    Marshall::Action action() override;
    Smoke::StackItem &item() override;
	const char *mytype() override;
	void emitSignal();
	void mainfunction() override;
	bool cleanup() override;

};

class Q_DECL_EXPORT InvokeNativeSlot : public SigSlotBase {
    QObject *_obj;
    int _id;
	VALUE * _result;
 public:
    InvokeNativeSlot(QObject *obj, int id, int items, QList<MocArgument*> args, VALUE * sp, VALUE * result);
    Marshall::Action action() override;
    Smoke::StackItem &item() override;
	const char *mytype() override;
	void invokeSlot();
	void mainfunction() override;
	bool cleanup() override;
};

/*
	Converts a ruby value returned by a slot invocation to a Qt slot
	reply type
*/
class SlotReturnValue : public Marshall {
    QList<MocArgument*>	_replyType;
    Smoke::Stack _stack;
	VALUE * _result;
public:
    SlotReturnValue(void ** o, VALUE * result, QList<MocArgument*> replyType);

    SmokeType type() override;
    Marshall::Action action() override;
    Smoke::StackItem &item() override;
    VALUE * var() override;
    void unsupported() override;
    Smoke *smoke() override;
    void next() override;
    bool cleanup() override;
    ~SlotReturnValue() override;
};

class Q_DECL_EXPORT InvokeSlot : public SigSlotBase {
    VALUE _obj;
    ID _slotname;
    void **_o;
public:
    InvokeSlot(VALUE obj, ID slotname, QList<MocArgument*> args, void ** o);
	~InvokeSlot();
    Marshall::Action action() override;
	const char *mytype() override;
    bool cleanup() override;
	void copyArguments();
	void invokeSlot(); 
	void mainfunction() override;
};

}

#endif
