// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Acceptor.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Acceptor* p) { p->__incRef(); }
void IceInternal::decRef(Acceptor* p) { p->__decRef(); }
