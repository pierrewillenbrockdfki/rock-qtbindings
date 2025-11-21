#include <QtTest/QtTest>

#define private public
#include "ast.h"
#undef private

#include "parser.h"
#include "rpp/preprocessor.h"
#include "control.h"
#include "dumptree.h"
#include "tokens.h"
#include "parsesession.h"
#include "commentformatter.h"

#include "testconfig.h"

#include <QByteArray>
#include <QDataStream>
#include <QFile>

#include <iostream>
#include <optional>
#include <rpp/chartools.h>
#include <rpp/pp-engine.h>


#include <clang/AST/ASTDumper.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/CommentCommandTraits.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclGroup.h>
#include <clang/AST/DeclObjC.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/AST/DeclarationName.h>
#include <clang/AST/ExternalASTSource.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Type.h>
#include <clang/AST/TypeOrdering.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/IdentifierTable.h>
#include <clang/Basic/LLVM.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Basic/LangStandard.h>
#include <clang/Basic/Module.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Basic/TargetOptions.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Frontend/FrontendDiagnostic.h>
#include <clang/Frontend/FrontendOptions.h>
#include <clang/Frontend/MultiplexConsumer.h>
#include <clang/Frontend/PrecompiledPreamble.h>
#include <clang/Frontend/Utils.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/HeaderSearchOptions.h>
#include <clang/Lex/Lexer.h>
#include <clang/Lex/PPCallbacks.h>
#include <clang/Lex/PreprocessingRecord.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <clang/Lex/Token.h>
#include <clang/Sema/CodeCompleteConsumer.h>
#include <clang/Sema/CodeCompleteOptions.h>
#include <clang/Serialization/ASTBitCodes.h>
#include <clang/Serialization/ASTReader.h>
#include <clang/Serialization/ASTWriter.h>
#include <clang/Serialization/ContinuousRangeMap.h>
#include <clang/Serialization/InMemoryModuleCache.h>
#include <clang/Serialization/ModuleFile.h>
#include <clang/Serialization/PCHContainerOperations.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/ScopeExit.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/ADT/Twine.h>
#include <llvm/ADT/iterator_range.h>
#include <llvm/Support/ErrorHandling.h>

bool hasKind(AST*, AST::NODE_KIND);
bool hasCXXRecordDecl(clang::Decl* DC);
bool hasFunctionDecl(clang::Decl* DC);
bool hasForStmt(clang::Decl* DC);
bool hasIfStmt(clang::Decl* DC);
bool hasDeclStmt(clang::Decl* DC);
bool hasBinaryOperator(clang::Decl* DC, std::string const &opcode);
bool hasUnaryOperator(clang::Decl* DC, std::string const &opcode);
AST* getAST(AST*, AST::NODE_KIND, int num = 0);

std::string getClangResourcesPath();

class TestParser : public QObject
{
  Q_OBJECT

  Control control;
  DumpTree dumper;

public:
  TestParser()
  {
  }

private Q_SLOTS:

  void initTestCase()
  {
  }

  void testSymbolTable()
  {
    NameTable table;
    table.findOrInsert("Ideal::MainWindow", sizeof("Ideal::MainWindow"));
    table.findOrInsert("QMainWindow", sizeof("QMainWindow"));
    table.findOrInsert("KXmlGuiWindow", sizeof("KXmlGuiWindow"));
    QCOMPARE(table.count(), size_t(3));
    const NameSymbol *s = table.findOrInsert("QMainWindow", sizeof("QMainWindow"));
    QCOMPARE(QString(s->data), QStringLiteral("QMainWindow"));
    QCOMPARE(table.count(), size_t(3));
  }

  ///@todo reenable
//   void testControlContexts()
//   {
//     Control control;
//     const NameSymbol *n1 = control.findOrInsertName("a", 1);
//     int *type1 = new int(1); // don't care much about types
//     control.declare(n1, (Type*)type1);
//
//     control.pushContext();
//     int *type2 = new int(2);
//     const NameSymbol *n2 = control.findOrInsertName("b", 1);
//     control.declare(n2, (Type*)type2);
//
//     QCOMPARE(control.lookupType(n1), (Type*)type1);
//     QCOMPARE(control.lookupType(n2), (Type*)type2);
//
//     control.popContext();
//     QCOMPARE(control.lookupType(n1), (Type*)type1);
//     QCOMPARE(control.lookupType(n2), (Type*)0);
//   }

  void testTokenTable()
  {
    QCOMPARE(token_name(Token_EOF), "eof");
    QCOMPARE(token_name('a'), "a");
    QCOMPARE(token_name(Token_delete), "delete");
  }

///@todo reenable
//   void testLexer()
//   {
//     QByteArray code("#include <foo.h>");
//     TokenStream token_stream;
//     LocationTable location_table;
//     LocationTable line_table;
//     Control control;
//
//     Lexer lexer(token_stream, location_table, line_table, &control);
//     lexer.tokenize(code, code.size()+1);
//     QCOMPARE(control.problem(0).message(), QStringLiteral("expected end of line"));
//
//     QByteArray code2("class Foo { int foo() {} }; ");
//     lexer.tokenize(code2, code2.size()+1);
//     QCOMPARE(control.problemCount(), 1);    //we still have the old problem in the list
//   }

  void testParser()
  {
    QByteArray clazz("struct A { int i; A() : i(5) { } virtual void test() = 0; };");
    pool mem_pool;
    TranslationUnitAST* ast = parse(clazz, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(clazz);
    QVERIFY(ast != 0);
    QVERIFY(ast->declarations != 0);
    QVERIFY(clangast);
    QVERIFY(!clangast->top_level_empty());
  }
  
  void testManyComparisons()
  {
    //Should not crash
    {
      QByteArray clazz("void test() { if(val < f && val < val1 && val < val2 && val < val3 ){ } }");
      pool mem_pool;
      TranslationUnitAST* ast = parse(clazz, &mem_pool);
      std::unique_ptr<clang::ASTUnit> clangast = parseClang(clazz);
      QVERIFY(ast != 0);
      QVERIFY(ast->declarations != 0);
      QVERIFY(clangast);
      QVERIFY(!clangast->top_level_empty());
      dumper.dump(ast, lastSession->token_stream);
      std::string dumped;
      llvm::raw_string_ostream dumpStream(dumped);
      clang::ASTDumper clangdumper(dumpStream, clangast->getASTContext(), true);
      clangdumper.dumpDeclContext(clangast->getASTContext().getTranslationUnitDecl());
      qDebug().noquote() << QString::fromStdString(dumped) << Qt::endl;
    }
    {
      QByteArray clazz("void test() { if(val < f && val < val1 && val < val2 && val < val3 && val < val4 && val < val5 && val < val6 && val < val7 && val < val8 && val < val9 && val < val10 && val < val11 && val < val12 && val < val13 && val < val14 && val < val15 && val < val16 && val < val17 && val < val18 && val < val19 && val < val20 && val < val21 && val < val22 && val < val23 && val < val24 && val < val25 && val < val26){ } }");
      pool mem_pool;
      TranslationUnitAST* ast = parse(clazz, &mem_pool);
      std::unique_ptr<clang::ASTUnit> clangast = parseClang(clazz);
      QVERIFY(ast != 0);
      QVERIFY(ast->declarations != 0);
      QVERIFY(clangast);
      QVERIFY(!clangast->top_level_empty());
    }
  }
  
  void testParserFail()
  {
    QByteArray stuff("foo bar !!! nothing that really looks like valid c++ code");
    pool mem_pool;
    TranslationUnitAST *ast = parse(stuff, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(stuff);
    QVERIFY(ast->declarations == 0);
    QVERIFY(control.problems().count() > 3);
    std::string dumped;
    llvm::raw_string_ostream dumpStream(dumped);
    clang::ASTDumper clangdumper(dumpStream, clangast->getASTContext(), true);
    clangdumper.dumpDeclContext(clangast->getASTContext().getTranslationUnitDecl());
    qDebug().noquote() << QString::fromStdString(dumped) << Qt::endl;
    //clang sees a VarDecl here.
    //QVERIFY(clangast->top_level_empty());
    //TODO check for errors; we are currently dropping them, i think, not sure.
  }

  void testPartialParseFail() {
    {
    QByteArray method("struct C { Something invalid is here };");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(method);
    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_ClassSpecifier));
    QVERIFY(clangast);
    QVERIFY(hasCXXRecordDecl(clangast->getASTContext().getTranslationUnitDecl()));
    }
    {
    QByteArray method("void test() { Something invalid is here };");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(method);
    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
    QVERIFY(clangast);
    QVERIFY(hasFunctionDecl(clangast->getASTContext().getTranslationUnitDecl()));
    }
    {
    QByteArray method("void test() { {Something invalid is here };");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(method);
    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
    QVERIFY(ast->hadMissingCompoundTokens);
    QVERIFY(clangast);
    QVERIFY(hasFunctionDecl(clangast->getASTContext().getTranslationUnitDecl()));
    //TODO ast->hadMissingCompoundTokens
    }
    {
    QByteArray method("void test() { case:{};");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(method);
    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
    QVERIFY(ast->hadMissingCompoundTokens);
    QVERIFY(clangast);
    QVERIFY(hasFunctionDecl(clangast->getASTContext().getTranslationUnitDecl()));
    //TODO ast->hadMissingCompoundTokens
    }
  }

  void testParseMethod()
  {
    /* clang requires the class definition to accept the function */
    QByteArray method("class A { void test(); }; void A::test() {  }");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(method);
    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_FunctionDefinition));
    QVERIFY(clangast);
    QVERIFY(hasFunctionDecl(clangast->getASTContext().getTranslationUnitDecl()));
  }

///@todo reenable
//   void testMethodArgs()
//   {
//     QByteArray method("int A::test(int primitive, B* pointer) { return primitive; }");
//     pool mem_pool;
//     Parser parser(&control);
//     TranslationUnitAST* ast = parser.parse(method.constData(),
// 					   method.size() + 1, &mem_pool);
//     // return type
//     SimpleTypeSpecifierAST* retType = static_cast<SimpleTypeSpecifierAST*>
//       (getAST(ast, AST::Kind_SimpleTypeSpecifier));
//     QCOMPARE((TOKEN_KIND)parser.token_stream.kind(retType->start_token),
// 	    Token_int);
//
//     // first param
//     ParameterDeclarationAST* param = static_cast<ParameterDeclarationAST*>
//       (getAST(ast, AST::Kind_ParameterDeclaration));
//     SimpleTypeSpecifierAST* paramType = static_cast<SimpleTypeSpecifierAST*>
//       (getAST(param, AST::Kind_SimpleTypeSpecifier));
//     QCOMPARE((TOKEN_KIND)parser.token_stream.kind(paramType->start_token),
// 	    Token_int);
//     UnqualifiedNameAST* argName  = static_cast<UnqualifiedNameAST*>
//       (getAST(param, AST::Kind_UnqualifiedName));
//     QCOMPARE(parser.token_stream.symbol(argName->id)->as_string(),
// 	    QStringLiteral("primitive"));
//
//     // second param
//     param = static_cast<ParameterDeclarationAST*>
//       (getAST(ast, AST::Kind_ParameterDeclaration, 1));
//     UnqualifiedNameAST* argType = static_cast<UnqualifiedNameAST*>
//       (getAST(param, AST::Kind_UnqualifiedName));
//     QCOMPARE(parser.token_stream.symbol(argType->id)->as_string(),
// 	    QStringLiteral("B"));
//
//     // pointer operator
//     QVERIFY(hasKind(param, AST::Kind_PtrOperator));
//
//     argName = static_cast<UnqualifiedNameAST*>
//       (getAST(param, AST::Kind_UnqualifiedName, 1));
//     QCOMPARE(parser.token_stream.symbol(argName->id)->as_string(),
// 	    QStringLiteral("pointer"));
//
//   }

  void testForStatements()
  {
    QByteArray method("void t() { for (int i = 0; i < 10; i++) { ; }}");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(method);

    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_ForStatement));
    QVERIFY(hasKind(ast, AST::Kind_Condition));
    QVERIFY(hasKind(ast, AST::Kind_IncrDecrExpression));
    QVERIFY(hasKind(ast, AST::Kind_SimpleDeclaration));
    QVERIFY(clangast);
    QVERIFY(hasForStmt(clangast->getASTContext().getTranslationUnitDecl()));
    QVERIFY(hasBinaryOperator(clangast->getASTContext().getTranslationUnitDecl(), "<"));
    QVERIFY(hasUnaryOperator(clangast->getASTContext().getTranslationUnitDecl(), "++"));
    QVERIFY(hasDeclStmt(clangast->getASTContext().getTranslationUnitDecl()));

    QByteArray emptyFor("void t() { for (;;) { } }");
    ast = parse(emptyFor, &mem_pool);
    clangast = parseClang(emptyFor);
    QVERIFY(ast != 0);
    QVERIFY(hasKind(ast, AST::Kind_ForStatement));
    QVERIFY(!hasKind(ast, AST::Kind_Condition));
    QVERIFY(!hasKind(ast, AST::Kind_SimpleDeclaration));
    QVERIFY(clangast);
    QVERIFY(hasForStmt(clangast->getASTContext().getTranslationUnitDecl()));
    QVERIFY(!hasBinaryOperator(clangast->getASTContext().getTranslationUnitDecl(), "<"));
    QVERIFY(!hasDeclStmt(clangast->getASTContext().getTranslationUnitDecl()));
  }

  void testIfStatements()
  {
    QByteArray method("void t() { if (1 < 2) { } }");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(method);
    QVERIFY(hasKind(ast, AST::Kind_Condition));
    QVERIFY(hasKind(ast, AST::Kind_BinaryExpression));
    QVERIFY(hasIfStmt(clangast->getASTContext().getTranslationUnitDecl()));
    QVERIFY(hasBinaryOperator(clangast->getASTContext().getTranslationUnitDecl(), "<"));
  }

  void testComments()
  {
    QByteArray method("//TranslationUnitComment\n//Hello\nint A; //behind\n /*between*/\n /*Hello2*/\n class B{}; //behind\n//Hello3\n //beforeTest\nvoid test(); //testBehind");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(method);

    //TODO

    QCOMPARE(CommentFormatter::formatComment(ast->comments, lastSession), QByteArray("TranslationUnitComment")); //The comments were merged

    const ListNode<DeclarationAST*>* it = ast->declarations;
    QVERIFY(it);
    it = it->next;
    QVERIFY(it);
    QCOMPARE(CommentFormatter::formatComment(it->element->comments, lastSession), QByteArray("Hello\n(behind)"));

    it = it->next;
    QVERIFY(it);
    QCOMPARE(CommentFormatter::formatComment(it->element->comments, lastSession), QByteArray("between\nHello2\n(behind)"));

    it = it->next;
    QVERIFY(it);
    QCOMPARE(CommentFormatter::formatComment(it->element->comments, lastSession), QByteArray("Hello3\nbeforeTest\n(testBehind)"));
  }

  void testComments2()
  {
    QByteArray method("enum Enum\n {//enumerator1Comment\nenumerator1, //enumerator1BehindComment\n /*enumerator2Comment*/ enumerator2 /*enumerator2BehindComment*/};");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(method);

    //TODO

    const ListNode<DeclarationAST*>* it = ast->declarations;
    QVERIFY(it);
    it = it->next;
    QVERIFY(it);
    const SimpleDeclarationAST* simpleDecl = static_cast<const SimpleDeclarationAST*>(it->element);
    QVERIFY(simpleDecl);

    const EnumSpecifierAST* enumSpec = (const EnumSpecifierAST*)simpleDecl->type_specifier;
    QVERIFY(enumSpec);

    const ListNode<EnumeratorAST*> *enumerator = enumSpec->enumerators;
    QVERIFY(enumerator);
    enumerator = enumerator->next;
    QVERIFY(enumerator);

    QCOMPARE(CommentFormatter::formatComment(enumerator->element->comments, lastSession), QByteArray("enumerator1Comment\n(enumerator1BehindComment)"));

    enumerator = enumerator->next;
    QVERIFY(enumerator);

    QCOMPARE(CommentFormatter::formatComment(enumerator->element->comments, lastSession), QByteArray("enumerator2Comment\n(enumerator2BehindComment)"));
  }

  void testComments3()
  {
    QByteArray method("class Class{\n//Comment\n int val;};");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(method);

    //TODO

    const ListNode<DeclarationAST*>* it = ast->declarations;
    QVERIFY(it);
    it = it->next;
    QVERIFY(it);
    const SimpleDeclarationAST* simpleDecl = static_cast<const SimpleDeclarationAST*>(it->element);
    QVERIFY(simpleDecl);

    const ClassSpecifierAST* classSpec = (const ClassSpecifierAST*)simpleDecl->type_specifier;
    QVERIFY(classSpec);

    const ListNode<DeclarationAST*> *members = classSpec->member_specs;
    QVERIFY(members);
    members = members->next;
    QVERIFY(members);

    QCOMPARE(CommentFormatter::formatComment(members->element->comments, lastSession), QByteArray("Comment"));
  }

  void testComments4()
  {
    QByteArray method("//TranslationUnitComment\n//Comment\ntemplate<class C> class Class{};");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(method);

    //TODO

    const ListNode<DeclarationAST*>* it = ast->declarations;
    QVERIFY(it);
    it = it->next;
    QVERIFY(it);
    const TemplateDeclarationAST* templDecl = static_cast<const TemplateDeclarationAST*>(it->element);
    QVERIFY(templDecl);
    QVERIFY(templDecl->declaration);

    //QCOMPARE(CommentFormatter::formatComment(templDecl->declaration->comments, lastSession), QStringLiteral("Comment"));
  }

  QString preprocess(const QString& contents) {
    rpp::Preprocessor preprocessor;
    rpp::pp pp(&preprocessor);
    return QString::fromUtf8(stringFromContents(pp.processFile("anonymous", contents.toUtf8())));
  }

  void testPreprocessor() {
    rpp::Preprocessor preprocessor;
    //QCOMPARE(preprocess("#define TEST (1L<<10)\nTEST").trimmed(), QStringLiteral("(1L<<10)"));
    QCOMPARE(preprocess("#define TEST //Comment\nTEST 1").trimmed(), QStringLiteral("1")); //Comments are not included in macros
    QCOMPARE(preprocess("#define TEST /*Comment\n*/\nTEST 1").trimmed(), QStringLiteral("1")); //Comments are not included in macros

  }

  void testStringConcatenation()
  {
    rpp::Preprocessor preprocessor;
    QCOMPARE(preprocess("Hello##You"), QStringLiteral("HelloYou"));
    QCOMPARE(preprocess("#define CONCAT(Var1, Var2) Var1##Var2 Var2##Var1\nCONCAT(      Hello      ,      You     )").simplified(), QStringLiteral("\nHelloYou YouHello").simplified());
  }

  void testCondition()
  {
    QByteArray method("bool i = (small < big || big > small);");
    pool mem_pool;
    TranslationUnitAST* ast = parse(method, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(method);
    dumper.dump(ast, lastSession->token_stream);
    std::string dumped;
    llvm::raw_string_ostream dumpStream(dumped);
    clang::ASTDumper clangdumper(dumpStream, clangast->getASTContext(), true);
    clangdumper.dumpDeclContext(clangast->getASTContext().getTranslationUnitDecl());
    qDebug().noquote() << QString::fromStdString(dumped) << Qt::endl;
    ///@todo make this work, it should yield something like TranslationUnit -> SimpleDeclaration -> InitDeclarator -> BinaryExpression
  }

  void testNonTemplateDeclaration()
  {
    /*{
      QByteArray templateMethod("template <int> class a {}; int main() { const int b = 1; const int c = 2; a<b|c> d; }");
      pool mem_pool;
      TranslationUnitAST* ast = parse(templateMethod, &mem_pool);
      parseClange(templateMethod);
      dumper.dump(ast, lastSession->token_stream);
      std::string dumped;
      llvm::raw_string_ostream dumpStream(dumped);
      clang::ASTDumper clangdumper(dumpStream, clangast->getASTContext(), true);
      clangdumper.dumpDeclContext(clangast->getASTContext().getTranslationUnitDecl());
      qDebug().noquote() << QString::fromStdString(dumped) << Qt::endl;
    }*/

    //int a, b, c, d; bool e;
    QByteArray declaration("void expression() { if (a < b || c > d) {} }");
    pool mem_pool;
    TranslationUnitAST* ast = parse(declaration, &mem_pool);
    std::unique_ptr<clang::ASTUnit> clangast = parseClang(declaration);
    dumper.dump(ast, lastSession->token_stream);
    std::string dumped;
    llvm::raw_string_ostream dumpStream(dumped);
    clang::ASTDumper clangdumper(dumpStream, clangast->getASTContext(), true);
    clangdumper.dumpDeclContext(clangast->getASTContext().getTranslationUnitDecl());
    qDebug().noquote() << QString::fromStdString(dumped) << Qt::endl;
  }

  /*void testParseFile()
  {
     QFile file(TEST_FILE);
     QVERIFY(file.open(QFile::ReadOnly));
     QByteArray contents = file.readAll();
     file.close();
     pool mem_pool;
     Parser parser(&control);
     ParseSession session;
     session.setContents(contents);
     TranslationUnitAST* ast = parser.parse(&session);
     QVERIFY(ast != 0);
     QVERIFY(ast->declarations != 0);
   }*/

private:
  ParseSession* lastSession;

  TranslationUnitAST* parse(const QByteArray& unit, pool* mem_pool)
  {
    Parser parser(&control);
    lastSession = new ParseSession();
    lastSession->setContentsAndGenerateLocationTable(tokenizeFromByteArray(unit));
    return  parser.parse(lastSession);
  }

    std::unique_ptr<clang::ASTUnit> parseClang(const QByteArray &source)
    {
        // Configure the diagnostics.
        clang::IntrusiveRefCntPtr<clang::DiagnosticsEngine>
        Diags(clang::CompilerInstance::createDiagnostics(new clang::DiagnosticOptions));

        std::unique_ptr<std::vector<const char *> >
        Args(new std::vector<const char *>());

        Args->push_back("-fno-spell-checking");
        Args->push_back("-fparse-all-comments");
        Args->push_back("source.cpp");

        clang::IntrusiveRefCntPtr<llvm::vfs::FileSystem> VFS = llvm::vfs::createPhysicalFileSystem();

        clang::SmallVector<clang::StoredDiagnostic, 4> StoredDiagnostics;

        std::shared_ptr<clang::CompilerInvocation> CI;

        CI = std::make_shared<clang::CompilerInvocation>();
        clang::CompilerInvocation::CreateFromArgs(*CI, *Args,
                *Diags);

        clang::StringRef Data(source.data(), source.size());
        llvm::MemoryBuffer *Buffer
            = llvm::MemoryBuffer::getMemBufferCopy(Data, "source.cpp").release();
        CI->getPreprocessorOpts().addRemappedFile("source.cpp", Buffer);


        clang::PreprocessorOptions &PPOpts = CI->getPreprocessorOpts();
        PPOpts.RemappedFilesKeepOriginalName = true;
        PPOpts.AllowPCHWithCompilerErrors = true;
        PPOpts.SingleFileParseMode = false;
        PPOpts.RetainExcludedConditionalBlocks = false;

        // Override the resources path.
        CI->getHeaderSearchOpts().ResourceDir = getClangResourcesPath();

        CI->getFrontendOpts().SkipFunctionBodies = false;

        // Create the AST unit.
        std::unique_ptr<clang::ASTUnit> AST = clang::ASTUnit::LoadFromCompilerInvocation(
                CI,
                std::make_shared< clang::PCHContainerOperations >(),
                Diags,
                new clang::FileManager(CI->getFileSystemOpts(), VFS),
                /*OnlyLocalDecls=*/false,
                clang::CaptureDiagsKind::All,
                /*PrecompilePreambleAfterNParses=*/0,
                clang::TU_Complete,
                /*CacheCodeCompletionResults=*/false,
                /*IncludeBriefCommentsInCodeCompletion=*/false,
                /*UserFilesAreVolatile=*/true);
        // Zero out now to ease cleanup during crash recovery.
        CI = nullptr;
        Diags = nullptr;

        return AST;

    }


};

std::string getClangResourcesPath()
{
    static std::string foundPath;
    if(foundPath.empty()) {
        FILE *f = popen("clang -print-resource-dir", "r");
        char buffer[256];
        int res = fread(buffer, 1, 255, f);
        pclose(f);
        if (res <= 0) {
            return std::string();
        }
        buffer[res] = 0;
        if (buffer[res - 1] == '\n') {
            buffer[res - 1] = 0;
            res--;
        }
        foundPath = buffer;
    }
    return foundPath;
}


struct HasKindVisitor : protected DefaultVisitor
{

  AST::NODE_KIND kind;
  AST* ast;
  int num;

  HasKindVisitor(AST::NODE_KIND kind, int num = 0)
    : kind(kind), ast(0), num(num)
  {
  }

  bool hasKind() const
  {
    return ast != 0;
  }

  void visit(AST* node)
  {
    if (!ast && node) {
      if (node->kind == kind && --num < 0) {
	ast = node;
      }
      else {
	DefaultVisitor::visit(node);
      }
    }
  }
};

class ClangHasCXXRecordDeclVisitor
  : public clang::RecursiveASTVisitor<ClangHasCXXRecordDeclVisitor> {
public:
  clang::CXXRecordDecl *decl;
  int num;
  ClangHasCXXRecordDeclVisitor(int num = 0) : decl(nullptr), num(num) {}
  bool VisitCXXRecordDecl(clang::CXXRecordDecl *Declaration) {
    if (--num < 0) {
        decl = Declaration;
        return false;
    }

    // The return value indicates whether we want the visitation to proceed.
    // Return false to stop the traversal of the AST.
    return true;
  }
};

class ClangHasFunctionDeclVisitor
  : public clang::RecursiveASTVisitor<ClangHasFunctionDeclVisitor> {
public:
  clang::FunctionDecl *decl;
  int num;
  ClangHasFunctionDeclVisitor(int num = 0) : decl(nullptr), num(num) {}
  bool VisitFunctionDecl(clang::FunctionDecl *Declaration)  {
    if (--num < 0) {
        decl = Declaration;
        return false;
    }

    // The return value indicates whether we want the visitation to proceed.
    // Return false to stop the traversal of the AST.
    return true;
  }
};

class ClangHasForStmtVisitor
  : public clang::RecursiveASTVisitor<ClangHasForStmtVisitor> {
public:
  clang::ForStmt *decl;
  int num;
  ClangHasForStmtVisitor(int num = 0) : decl(nullptr), num(num) {}
  bool VisitForStmt(clang::ForStmt *Declaration)  {
    if (--num < 0) {
        decl = Declaration;
        return false;
    }
    return true;
  }
};

class ClangHasIfStmtVisitor
  : public clang::RecursiveASTVisitor<ClangHasIfStmtVisitor> {
public:
  clang::IfStmt *decl;
  int num;
  ClangHasIfStmtVisitor(int num = 0) : decl(nullptr), num(num) {}
  bool VisitIfStmt(clang::IfStmt *Declaration)  {
    if (--num < 0) {
        decl = Declaration;
        return false;
    }
    return true;
  }
};

class ClangHasDeclStmtVisitor
  : public clang::RecursiveASTVisitor<ClangHasDeclStmtVisitor> {
public:
  clang::DeclStmt *decl;
  int num;
  ClangHasDeclStmtVisitor(int num = 0) : decl(nullptr), num(num) {}
  bool VisitDeclStmt(clang::DeclStmt *Declaration)  {
    if (--num < 0) {
        decl = Declaration;
        return false;
    }

    // The return value indicates whether we want the visitation to proceed.
    // Return false to stop the traversal of the AST.
    return true;
  }
};

class ClangHasBinaryOperatorVisitor
  : public clang::RecursiveASTVisitor<ClangHasBinaryOperatorVisitor> {
public:
  clang::BinaryOperator *decl;
  int num;
  std::string opcode;
  ClangHasBinaryOperatorVisitor(std::string const &opcode, int num = 0) : decl(nullptr), num(num), opcode(opcode) {}
  bool VisitBinaryOperator(clang::BinaryOperator *Declaration)  {
    if (Declaration->getOpcodeStr() == opcode && --num < 0) {
        decl = Declaration;
        return false;
    }

    // The return value indicates whether we want the visitation to proceed.
    // Return false to stop the traversal of the AST.
    return true;
  }
};

class ClangHasUnaryOperatorVisitor
  : public clang::RecursiveASTVisitor<ClangHasUnaryOperatorVisitor> {
public:
  clang::UnaryOperator *decl;
  int num;
  std::string opcode;
  ClangHasUnaryOperatorVisitor(std::string const &opcode, int num = 0) : decl(nullptr), num(num), opcode(opcode) {}
  bool VisitUnaryOperator(clang::UnaryOperator *Declaration)  {
    if (clang::UnaryOperator::getOpcodeStr(Declaration->getOpcode()) == opcode && --num < 0) {
        decl = Declaration;
        return false;
    }

    // The return value indicates whether we want the visitation to proceed.
    // Return false to stop the traversal of the AST.
    return true;
  }
};


bool hasKind(AST* ast, AST::NODE_KIND kind)
{
  HasKindVisitor visitor(kind);
  visitor.visit(ast);
  return visitor.hasKind();
}

bool hasCXXRecordDecl(clang::Decl* DC)
{
    ClangHasCXXRecordDeclVisitor visitor;
    visitor.TraverseDecl(DC);
    return visitor.decl != nullptr;
}

bool hasFunctionDecl(clang::Decl* DC) {
    ClangHasFunctionDeclVisitor visitor;
    visitor.TraverseDecl(DC);
    return visitor.decl != nullptr;
}

bool hasForStmt(clang::Decl* DC) {
    ClangHasForStmtVisitor visitor;
    visitor.TraverseDecl(DC);
    return visitor.decl != nullptr;
}

bool hasIfStmt(clang::Decl* DC) {
    ClangHasIfStmtVisitor visitor;
    visitor.TraverseDecl(DC);
    return visitor.decl != nullptr;
}

bool hasDeclStmt(clang::Decl* DC) {
    ClangHasDeclStmtVisitor visitor;
    visitor.TraverseDecl(DC);
    return visitor.decl != nullptr;
}

bool hasBinaryOperator(clang::Decl* DC, std::string const &opcode) {
    ClangHasBinaryOperatorVisitor visitor(opcode);
    visitor.TraverseDecl(DC);
    return visitor.decl != nullptr;
}

bool hasUnaryOperator(clang::Decl* DC, std::string const &opcode) {
    ClangHasUnaryOperatorVisitor visitor(opcode);
    visitor.TraverseDecl(DC);
    return visitor.decl != nullptr;
}

AST* getAST(AST* ast, AST::NODE_KIND kind, int num)
{
  HasKindVisitor visitor(kind, num);
  visitor.visit(ast);
  return visitor.ast;
}


#include "test_parser.moc"

QTEST_MAIN(TestParser)
