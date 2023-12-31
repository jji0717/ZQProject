// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef GEN_H
#define GEN_H

#include <Slice/Parser.h>
#include <IceUtil/OutputUtil.h>

namespace Slice
{

class Gen : private ::IceUtil::noncopyable
{
public:

    Gen(const std::string&,
        const std::string&,
        const std::string&,
        const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        const std::vector<std::string>&,
        const std::string&,
        const std::string&,
        bool,
        bool,
        bool,
        bool);
    ~Gen();

    bool operator!() const; // Returns true if there was a constructor error

    void generate(const UnitPtr&);
    
    static bool setUseWstring(ContainedPtr, std::list<bool>&, bool);
    static bool resetUseWstring(std::list<bool>&);

private:

    void writeExtraHeaders(::IceUtil::Output&);

    ::IceUtil::Output H;
    ::IceUtil::Output C;

    ::IceUtil::Output implH;
    ::IceUtil::Output implC;

    std::string _base;
    std::string _headerExtension;
    std::string _sourceExtension;
    std::vector<std::string> _extraHeaders;
    std::string _include;
    std::vector<std::string> _includePaths;
    std::string _dllExport;
    bool _impl;
    bool _checksum;
    bool _stream;
    bool _ice;

    class GlobalIncludeVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        GlobalIncludeVisitor(::IceUtil::Output&);

        virtual bool visitModuleStart(const ModulePtr&);

    private:

        ::IceUtil::Output& H;

        bool _finished;
    };

    class TypesVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        TypesVisitor(::IceUtil::Output&, ::IceUtil::Output&, const std::string&, bool);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitExceptionEnd(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitStructEnd(const StructPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);
        virtual void visitDataMember(const DataMemberPtr&);

    private:

        void emitUpcall(const ExceptionPtr&, const std::string&, bool = false);

        ::IceUtil::Output& H;
        ::IceUtil::Output& C;

        std::string _dllExport;
        bool _stream;
        bool _doneStaticSymbol;
        bool _useWstring;
        std::list<bool> _useWstringHist;
    };

    class ProxyDeclVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        ProxyDeclVisitor(::IceUtil::Output&, ::IceUtil::Output&, const std::string&);

        virtual bool visitUnitStart(const UnitPtr&);
        virtual void visitUnitEnd(const UnitPtr&);
        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);

    private:

        ::IceUtil::Output& H;
        ::IceUtil::Output& C;

        std::string _dllExport;
    };

    class ProxyVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        ProxyVisitor(::IceUtil::Output&, ::IceUtil::Output&, const std::string&);

        virtual bool visitUnitStart(const UnitPtr&);
        virtual void visitUnitEnd(const UnitPtr&);
        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        ::IceUtil::Output& H;
        ::IceUtil::Output& C;

        std::string _dllExport;
        bool _useWstring;
        std::list<bool> _useWstringHist;
    };

    class DelegateVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        DelegateVisitor(::IceUtil::Output&, ::IceUtil::Output&, const std::string&);

        virtual bool visitUnitStart(const UnitPtr&);
        virtual void visitUnitEnd(const UnitPtr&);
        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        ::IceUtil::Output& H;
        ::IceUtil::Output& C;

        std::string _dllExport;
        bool _useWstring;
        std::list<bool> _useWstringHist;
    };

    class DelegateMVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        DelegateMVisitor(::IceUtil::Output&, ::IceUtil::Output&, const std::string&);

        virtual bool visitUnitStart(const UnitPtr&);
        virtual void visitUnitEnd(const UnitPtr&);
        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        ::IceUtil::Output& H;
        ::IceUtil::Output& C;

        std::string _dllExport;
        bool _useWstring;
        std::list<bool> _useWstringHist;
    };

    class DelegateDVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        DelegateDVisitor(::IceUtil::Output&, ::IceUtil::Output&, const std::string&);

        virtual bool visitUnitStart(const UnitPtr&);
        virtual void visitUnitEnd(const UnitPtr&);
        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        ::IceUtil::Output& H;
        ::IceUtil::Output& C;

        std::string _dllExport;
        bool _useWstring;
        std::list<bool> _useWstringHist;
    };

    class ObjectDeclVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        ObjectDeclVisitor(::IceUtil::Output&, ::IceUtil::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        ::IceUtil::Output& H;
        ::IceUtil::Output& C;

        std::string _dllExport;
    };

    class ObjectVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        ObjectVisitor(::IceUtil::Output&, ::IceUtil::Output&, const std::string&, bool);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        void emitGCFunctions(const ClassDefPtr&);
        void emitGCInsertCode(const TypePtr&, const std::string&, const std::string&, int);
        void emitGCClearCode(const TypePtr&, const std::string&, const std::string&, int);
        bool emitVirtualBaseInitializers(const ClassDefPtr&);
        void emitOneShotConstructor(const ClassDefPtr&);
        void emitUpcall(const ClassDefPtr&, const std::string&);

        ::IceUtil::Output& H;
        ::IceUtil::Output& C;

        std::string _dllExport;
        bool _stream;
        bool _doneStaticSymbol;
        bool _useWstring;
        std::list<bool> _useWstringHist;
    };

    class IceInternalVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        IceInternalVisitor(::IceUtil::Output&, ::IceUtil::Output&, const std::string&);

        virtual bool visitUnitStart(const UnitPtr&);
        virtual void visitUnitEnd(const UnitPtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);

    private:

        ::IceUtil::Output& H;
        ::IceUtil::Output& C;

        std::string _dllExport;
    };

    class HandleVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        HandleVisitor(::IceUtil::Output&, ::IceUtil::Output&, const std::string&, bool);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);

    private:

        ::IceUtil::Output& H;
        ::IceUtil::Output& C;

        std::string _dllExport;
        bool _stream;
    };

    class ImplVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        ImplVisitor(::IceUtil::Output&, ::IceUtil::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);

    private:

        ::IceUtil::Output& H;
        ::IceUtil::Output& C;

        std::string _dllExport;
        bool _useWstring;
        std::list<bool> _useWstringHist;

        //
        // Generate code to emit a local variable declaration and initialize it
        // if necessary.
        //
        void writeDecl(::IceUtil::Output&, const std::string&, const TypePtr&, const StringList&);

        //
        // Generate code to return a dummy value
        //
        void writeReturn(::IceUtil::Output&, const TypePtr&, const StringList&);
    };

    class AsyncVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        AsyncVisitor(::IceUtil::Output&, ::IceUtil::Output&, const std::string&);

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        ::IceUtil::Output& H;
        ::IceUtil::Output& C;

        std::string _dllExport;
        bool _useWstring;
        std::list<bool> _useWstringHist;
    };

    class AsyncImplVisitor : private ::IceUtil::noncopyable, public ParserVisitor
    {
    public:

        AsyncImplVisitor(::IceUtil::Output&, ::IceUtil::Output&, const std::string&);

        virtual bool visitUnitStart(const UnitPtr&);
        virtual void visitUnitEnd(const UnitPtr&);
        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual void visitOperation(const OperationPtr&);

    private:

        ::IceUtil::Output& H;
        ::IceUtil::Output& C;

        std::string _dllExport;
        bool _useWstring;
        std::list<bool> _useWstringHist;
    };

private:

    class MetaDataVisitor : public ParserVisitor
    {
    public:

        virtual bool visitModuleStart(const ModulePtr&);
        virtual void visitModuleEnd(const ModulePtr&);
        virtual void visitClassDecl(const ClassDeclPtr&);
        virtual bool visitClassDefStart(const ClassDefPtr&);
        virtual void visitClassDefEnd(const ClassDefPtr&);
        virtual bool visitExceptionStart(const ExceptionPtr&);
        virtual void visitExceptionEnd(const ExceptionPtr&);
        virtual bool visitStructStart(const StructPtr&);
        virtual void visitStructEnd(const StructPtr&);
        virtual void visitOperation(const OperationPtr&);
        virtual void visitParamDecl(const ParamDeclPtr&);
        virtual void visitDataMember(const DataMemberPtr&);
        virtual void visitSequence(const SequencePtr&);
        virtual void visitDictionary(const DictionaryPtr&);
        virtual void visitEnum(const EnumPtr&);
        virtual void visitConst(const ConstPtr&);

    private:

        void validate(const SyntaxTreeBasePtr&, const StringList&, const std::string&, const std::string&,
                      bool = false);

        StringSet _history;
    };

    static void validateMetaData(const UnitPtr&);
};

}

#endif
