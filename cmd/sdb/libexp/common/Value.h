/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:libexp/common/Value.h	1.3"

#ifndef VALUE_H
#define VALUE_H

// Value -- The "result" of an expression evaluation; contains an
// Obj_info and an Rvalue.

#include <assert.h>
#include "Place.h"
#include "Rvalue.h"

class Frame;
class Process;

struct Obj_info {
    Process *process;
    Frame   *frame;
    TYPE     type;
    Symbol   entry;
    Place    loc;

    Obj_info() : process(0), frame(0) {}
    Obj_info(Process *p, Frame *f, TYPE& t, Symbol& s, Place& l) :
		process(p), frame(f), type(t), entry(s), loc(l) {}

    Obj_info(Obj_info& obj) :
	process(obj.process), frame(obj.frame),
	type(obj.type), entry(obj.entry), loc(obj.loc) {}

    Obj_info& operator=(Obj_info& rhs) {
	process = rhs.process;
	frame   = rhs.frame;
	type    = rhs.type;
	entry   = rhs.entry;
	loc     = rhs.loc;
	return *this;
    }
    ~Obj_info() {}

    void null();
    int  isnull();
    int  get_rvalue(Rvalue&);
    int  init(Process *, Frame *, Symbol&, Iaddr = 0);
    int  init(Process *, Frame *, Iaddr, TYPE&);
    int  deref(Obj_info&);
};

class Value {
    Obj_info _obj;
    Rvalue   _val;
public:
    Value() {}
    Value(Obj_info& obj)	   : _obj(obj)   {}
    Value(Rvalue& r)               : _val(r)     {}
    Value(void *p, int n, TYPE& t) : _val(p,n,t) {}
    Value(Value& v) : _obj(v._obj), _val(v._val) {}

    Value& operator=(Value& v) {
	_obj = v._obj; _val = v._val; return *this;
    }
    Value& operator=(Obj_info& obj) {
	_obj = obj; _val.null(); return *this;
    }
    Value& operator=(Rvalue& r) {
	_obj.null(); _val = r; return *this;
    }
    ~Value() {}

    Obj_info& object() { return _obj; }

    int rvalue(Rvalue& rval) {
	if (_val.isnull()) get_rvalue();
	rval = _val;
	return ! rval.isnull();
    }
    int get_rvalue();
    int deref(Process *, Frame *, const TYPE * = 0);
    int assign(Obj_info&);
};

#endif /*VALUE_H*/
