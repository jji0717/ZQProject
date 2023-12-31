// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CURRENT_ICE
#define ICE_CURRENT_ICE

#include <Ice/ObjectAdapterF.ice>
#include <Ice/ConnectionF.ice>
#include <Ice/Identity.ice>

module Ice
{

/**
 *
 * A request context. [Context] is used to transmit metadata about a
 * request from the server to the client, such as Quality-of-Service
 * (QoS) parameters. Each operation on the client has a [Context] as
 * its implicit final parameter.
 *
 **/
dictionary<string, string> Context;

/**
 *
 * The [OperationMode] determines the retry behavior an
 * invocation in case of a (potentially) recoverable error.
 *
 **/
//
// Note: The order of definitions here *must* match the order of
// definitions for ::Slice::Operation::Mode in include/Slice/Parser.h!
//
enum OperationMode
{
    /**
     * Ordinary operations have [Normal] mode.  These operations
     * modify object state; invoking such an operation twice in a row
     * has different semantics than invoking it once. The Ice run time
     * guarantees that it will not violate at-most-once semantics for
     * [Normal] operations.
     */
    Normal,

    /**
     * Operations that use the Slice [nonmutating] keyword must not
     * modify object state. For C++, nonmutating operations generate
     * [const] member functions in the skeleton. In addition, the Ice
     * run time will attempt to transparently recover from certain
     * run-time errors by re-issuing a failed request and propagate
     * the failure to the application only if the second attempt
     * fails.
     *
     * <p class="Deprecated"><tt>Nonmutating</tt> is deprecated; Use the
     * <tt>idempotent</tt> keyword instead. For C++, to retain the mapping
     * of <tt>nonmutating</tt> operations to C++ <tt>const</tt>
     * member functions, use the <tt>\["cpp:const"]</tt> metadata
     * directive.
     */
    \Nonmutating,

    /**
     * Operations that use the Slice [idempotent] keyword can modify
     * object state, but invoking an operation twice in a row must
     * result in the same object state as invoking it once.  For
     * example, <tt>x = 1</tt> is an idempotent statement,
     * whereas <tt>x += 1</tt> is not. For idempotent
     * operations, the Ice run-time uses the same retry behavior
     * as for nonmutating operations in case of a potentially
     * recoverable error.
     */
    \Idempotent
};

/**
 *
 * Information about the current method invocation for servers. Each
 * operation on the server has a [Current] as its implicit final
 * parameter. [Current] is mostly used for Ice services. Most
 * applications ignore this parameter.
 *
 **/
local struct Current
{
    /**
     *
     * The object adapter.
     *
     **/
    ObjectAdapter adapter;
    
    /**
     *
     * Information about the connection over which the current method
     * invocation was received. If the invocation is direct due to
     * collocation optimization, this value is set to null.
     *
     **/
    Connection con;

    /**
     *
     * The Ice object identity.
     *
     **/
    Identity id;

    /**
     *
     * The facet.
     *
     ***/
    string facet;

    /**
     *
     * The operation name.
     *
     **/
    string operation;

    /**
     *
     * The mode of the operation.
     *
     **/
    OperationMode mode;

    /**
     *
     * The request context, as received from the client.
     *
     **/
    Context ctx;

    /**
     *
     * The request id unless oneway (0) or collocated (-1).
     *
     **/
    int requestId;
};

};

#endif
