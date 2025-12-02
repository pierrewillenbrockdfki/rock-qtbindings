/*
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

#include <QCoreApplication>
#include <QList>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLibrary>

#include <QtXml>

#include <QtDebug>

#include <iostream>

#include <clang/AST/ASTDumper.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/HeaderSearchOptions.h>
#include <clang/Lex/MacroArgs.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <clang/Sema/Sema.h>
#include <clang/Serialization/ASTReader.h>
#include <llvm/ADT/IntrusiveRefCntPtr.h>

#include "clanggeneratorvisitor.h"
#include "options.h"
#include "config.h"

#include "compiletime_settings.inc"

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
namespace Qt
{
    using ::endl;
}
#endif

class TopLevelDeclTrackerConsumer : public clang::ASTConsumer {
  unsigned Hash;

public:
  TopLevelDeclTrackerConsumer() {
    Hash = 0;
  }

  void handleTopLevelDecl(clang::Decl *D) {
    if (!D)
      return;

    // FIXME: Currently ObjC method declarations are incorrectly being
    // reported as top-level declarations, even though their DeclContext
    // is the containing ObjC @interface/@implementation.  This is a
    // fundamental problem in the parser right now.
    if (clang::isa<clang::ObjCMethodDecl>(D))
      return;

    //AddTopLevelDeclarationToHash(D, Hash);
    //Unit.addTopLevelDecl(D);

    handleFileLevelDecl(D);
  }

  void handleFileLevelDecl(clang::Decl *D) {
    //Unit.addFileLevelDecl(D);
    if (auto *NSD = clang::dyn_cast<clang::NamespaceDecl>(D)) {
      for (auto *I : NSD->decls())
        handleFileLevelDecl(I);
    }
  }

  bool HandleTopLevelDecl(clang::DeclGroupRef D) override {
    for (auto *TopLevelDecl : D)
      handleTopLevelDecl(TopLevelDecl);
    return true;
  }

  // We're not interested in "interesting" decls.
  void HandleInterestingDecl(clang::DeclGroupRef) override {}

  void HandleTopLevelDeclInObjCContainer(clang::DeclGroupRef D) override {
    for (auto *TopLevelDecl : D)
      handleTopLevelDecl(TopLevelDecl);
  }

  clang::ASTMutationListener *GetASTMutationListener() override {
    return nullptr;//Unit.getASTMutationListener();
  }

  clang::ASTDeserializationListener *GetASTDeserializationListener() override {
    return nullptr;//Unit.getDeserializationListener();
  }
};

class TopLevelDeclTrackerAction : public clang::ASTFrontendAction {
public:
  std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance &CI,
                                                 clang::StringRef InFile) override {
    /*CI.getPreprocessor().addPPCallbacks(
        std::make_unique<clang::MacroDefinitionTrackerPPCallbacks>(
                                           Unit.getCurrentTopLevelHashValue()));*/
    return std::make_unique<TopLevelDeclTrackerConsumer>();
  }

public:
  TopLevelDeclTrackerAction() {}

  bool hasCodeCompletionSupport() const override { return false; }

  clang::TranslationUnitKind getTranslationUnitKind() override {
    return clang::TU_Complete;
  }
};

class MacroOverrideCallbacks : public clang::PPCallbacks {
private:
    std::shared_ptr< std::vector<QTRangedAnnotation> > AccessSpecAnnotations;
    std::shared_ptr< std::vector<QTRangedAnnotation> > PropertyAnnotations;
public:
    MacroOverrideCallbacks(std::shared_ptr< std::vector<QTRangedAnnotation> > AccessSpecAnnotations,
                           std::shared_ptr< std::vector<QTRangedAnnotation> > PropertyAnnotations);
    void MacroDefined(const clang::Token &MacroNameTok, const clang::MacroDirective *MD) override;
    void MacroExpands(const clang::Token &MacroNameTok, const clang::MacroDefinition &MD, clang::SourceRange Range, const clang::MacroArgs *Args) override;
};

MacroOverrideCallbacks::MacroOverrideCallbacks(std::shared_ptr< std::vector<QTRangedAnnotation> > AccessSpecAnnotations,
                           std::shared_ptr< std::vector<QTRangedAnnotation> > PropertyAnnotations)
    : AccessSpecAnnotations(AccessSpecAnnotations), PropertyAnnotations(PropertyAnnotations) {}

void MacroOverrideCallbacks::MacroDefined(const clang::Token &MacroNameTok, const clang::MacroDirective *MD) {
#if 0
    auto II = MacroNameTok.getIdentifierInfo();
    auto Undef = new clang::UndefMacroDirective(MacroNameTok.getLocation());
    if((II->getName() == "signals" || II->getName() == "slots" ||
        II->getName() == "Q_SIGNALS" || II->getName() == "Q_SLOTS")
        && MD->isDefined()) {
        qDebug() << "MacroDefined " << QString::fromStdString(std::string(II->getName())) << Qt::endl;
        pp->appendMacroDirective(II, Undef);
    }
#endif
}

void MacroOverrideCallbacks::MacroExpands(const clang::Token &MacroNameTok, const clang::MacroDefinition &MD, clang::SourceRange Range, const clang::MacroArgs *Args) {
    auto II = MacroNameTok.getIdentifierInfo();
    if(II->getName() == "QT_ANNOTATE_ACCESS_SPECIFIER") {
        if(Args->getNumMacroArguments() >= 1) {
            auto AII = Args->getUnexpArgument(0)->getIdentifierInfo();
            AccessSpecAnnotations->emplace_back(Range, std::string(AII->getName()));
        }
    }
    if(II->getName() == "QT_ANNOTATE_CLASS") {
        if(Args->getNumMacroArguments() >= 1) {
            auto AII0 = Args->getUnexpArgument(0)->getIdentifierInfo();
            auto AII1 = Args->getUnexpArgument(1)->getIdentifierInfo();
            if (AII0->getName() == "qt_property") {
                PropertyAnnotations->emplace_back(Range, std::string(AII1->getName()));
            }
        }
    }
}

typedef int (*GenerateFn)();


/// A diagnostic client that ignores all diagnostics.
class DiagConsumer : public clang::DiagnosticConsumer
{
    void HandleDiagnostic(clang::DiagnosticsEngine::Level DiagLevel,
                          const clang::Diagnostic &Info) override
    {
        QString loc = QString::fromStdString(Info.getLocation().printToString(Info.getSourceManager()));
        clang::SmallString<80> str;
        Info.FormatDiagnostic(str);
        switch (DiagLevel) {
        case clang::DiagnosticsEngine::Level::Fatal:
            qCritical().noquote() << loc << " Fatal: " << QString::fromStdString(std::string(str.str()));
            qFatal("Aborting due to Fatal error");
            break;
        case clang::DiagnosticsEngine::Level::Error:
            qCritical().noquote() << loc << " Error: " << QString::fromStdString(std::string(str.str()));
            break;
        case clang::DiagnosticsEngine::Level::Warning:
            qWarning().noquote() << loc << " Warning: " << QString::fromStdString(std::string(str.str()));
            break;
        case clang::DiagnosticsEngine::Level::Remark:
            qInfo().noquote() << loc << " Remark: " << QString::fromStdString(std::string(str.str()));
            break;
        case clang::DiagnosticsEngine::Level::Note:
            qInfo().noquote() << loc << " Note: " << QString::fromStdString(std::string(str.str()));
            break;
        case clang::DiagnosticsEngine::Level::Ignored:
            qDebug().noquote() << loc << " Ignored: " << QString::fromStdString(std::string(str.str()));
            break;
        }
    }
};

static void showUsage()
{
    std::cout << 
    "Usage: smokegen [options] -- <header files>" << std::endl <<
    "Possible command line options are:" << std::endl <<
    "    -I <include dir>" << std::endl <<
    "    -d <path to file containing #defines>" << std::endl <<
    "    -dm <list of macros that should be ignored>" << std::endl <<
    "    -g <generator to use>" << std::endl <<
    "    -qt enables Qt-mode (special treatment of QFlags)" << std::endl <<
    "    -t resolve typedefs" << std::endl <<
    "    -o <output dir>" << std::endl <<
    "    -config <config file>" << std::endl <<
    "    -h shows this message" << std::endl;
}

int main(int argc, char **argv)
{
    if (argc == 1) {
        showUsage();
        return EXIT_SUCCESS;
    }

    QCoreApplication app(argc, argv);
    const QStringList& args = app.arguments();

    QFileInfo configFile;
    QString generator;
    bool addHeaders = false;
    bool hasCommandLineGenerator = false;
    QStringList classes;

    ParserOptions::notToBeResolved << "FILE";
    ParserOptions::notToBeResolved << "va_list";

    for (int i = 1; i < args.count(); i++) {
        if ((args[i] == "-I" || args[i] == "-d" || args[i] == "-dm" ||
             args[i] == "-g" || args[i] == "-config") && i + 1 >= args.count())
        {
            qCritical() << "not enough parameters for option" << args[i];
            return EXIT_FAILURE;
        }
        if (args[i] == "-I") {
            ParserOptions::includeDirs << QDir(args[++i]);
        } else if (args[i] == "-config") {
            configFile = QFileInfo(args[++i]);
        } else if (args[i] == "-d") {
            ParserOptions::definesList = QFileInfo(args[++i]);
        } else if (args[i] == "-dm") {
            ParserOptions::dropMacros += args[++i].split(',');
        } else if (args[i] == "-g") {
            generator = args[++i];
            hasCommandLineGenerator = true;
        } else if ((args[i] == "-h" || args[i] == "--help") && argc == 2) {
            showUsage();
            return EXIT_SUCCESS;
        } else if (args[i] == "-t") {
            ParserOptions::resolveTypedefs = true;
        } else if (args[i] == "-qt") {
            ParserOptions::qtMode = true;
        } else if (args[i] == "--") {
            addHeaders = true;
        } else if (addHeaders) {
            ParserOptions::headerList << QFileInfo(args[i]);
        }
    }
        
    if (configFile.exists()) {
        QFile file(configFile.filePath());
        file.open(QIODevice::ReadOnly);
        QDomDocument doc;
        doc.setContent(file.readAll());
        file.close();
        QDomElement root = doc.documentElement();
        QDomNode node = root.firstChild();
        while (!node.isNull()) {
            QDomElement elem = node.toElement();
            if (elem.isNull()) {
                node = node.nextSibling();
                continue;
            }
            if (elem.tagName() == "resolveTypedefs") {
                ParserOptions::resolveTypedefs = (elem.text() == "true");
            } else if (elem.tagName() == "qtMode") {
                ParserOptions::qtMode = (elem.text() == "true");
            } else if (!hasCommandLineGenerator && elem.tagName() == "generator") {
                generator = elem.text();
            } else if (elem.tagName() == "includeDirs") {
                QDomNode dir = elem.firstChild();
                while (!dir.isNull()) {
                    QDomElement elem = dir.toElement();
                    if (elem.isNull()) {
                        dir = dir.nextSibling();
                        continue;
                    }
                    if (elem.tagName() == "dir") {
                        ParserOptions::includeDirs << QDir(elem.text());
                    }
                    dir = dir.nextSibling();
                }
            } else if (elem.tagName() == "definesList") {
                // reference to an external file, so it can be auto-generated
                ParserOptions::definesList = QFileInfo(elem.text());
            } else if (elem.tagName() == "dropMacros") {
                QDomNode macro = elem.firstChild();
                while (!macro.isNull()) {
                    QDomElement elem = macro.toElement();
                    if (elem.isNull()) {
                        macro = macro.nextSibling();
                        continue;
                    }
                    if (elem.tagName() == "name") {
                        ParserOptions::dropMacros << elem.text();
                    }
                    macro = macro.nextSibling();
                }
            }
            node = node.nextSibling();
        }
    } else {
        qWarning() << "Couldn't find config file" << configFile.filePath();
    }

    // first try to load plugins from the executable's directory
    QLibrary lib(app.applicationDirPath() + "/generator_" + generator);
    lib.load();
    if (!lib.isLoaded()) {
        lib.unload();
        lib.setFileName(app.applicationDirPath() + "/../lib" + LIB_SUFFIX + "/smokegen/generator_" + generator);
        lib.load();
    }
    if (!lib.isLoaded()) {
        lib.unload();
        lib.setFileName("generator_" + generator);
        lib.load();
    }
    if (!lib.isLoaded()) {
        qCritical() << lib.errorString();
        return EXIT_FAILURE;
    }
    qDebug() << "using generator" << lib.fileName();
    GenerateFn generate = (GenerateFn) lib.resolve("generate");
    if (!generate) {
        qCritical() << "couldn't resolve symbol 'generate', aborting";
        return EXIT_FAILURE;
    }
    
    Q_FOREACH (QDir dir, ParserOptions::includeDirs) {
        if (!dir.exists()) {
            qWarning() << "include directory" << dir.path() << "doesn't exist";
            ParserOptions::includeDirs.removeAll(dir);
        }
    }
    
    QStringList defines;
    if (ParserOptions::definesList.exists()) {
        QFile file(ParserOptions::definesList.filePath());
        file.open(QIODevice::ReadOnly);
        while (!file.atEnd()) {
            QByteArray array = file.readLine();
            if (!array.isEmpty())
                defines << array.trimmed();
        }
        file.close();
    } else if (!ParserOptions::definesList.filePath().isEmpty()) {
        qWarning() << "didn't find file" << ParserOptions::definesList.filePath();
    }
    
    QFile log("generator.log");
    bool logErrors = log.open(QFile::WriteOnly | QFile::Truncate);
    QTextStream logOut(&log);

    std::vector<std::string> clangMacroDef;//this is storage for the strings referenced by the StringRefs
    for(auto &d : defines) {
        clangMacroDef.emplace_back(d.toStdString());
    }
    std::vector<std::string> clangIncludeOpts;
    std::vector<std::string> clangIncludeDirs;
    for (auto &id : ParserOptions::includeDirs) {
        clangIncludeOpts.emplace_back(std::string("-I")+id.absolutePath().toStdString());
        clangIncludeDirs.emplace_back(id.absolutePath().toStdString());
    }
    std::string clangResourceDirInclude = std::string(COMPILETIME_CLANG_RESOURCEPATH) + "/include";

    Q_FOREACH (QFileInfo file, ParserOptions::headerList) {
        qDebug() << "parsing" << file.absoluteFilePath();
        // this has already been parsed because it was included by some header
        //TODO this is currently not tracked but clang should be able to tell us.
        //global: QList<QString> parsedHeaders;
        //if (parsedHeaders.contains(file.absoluteFilePath()))
        //    continue;


        //Preprocessor pp(ParserOptions::includeDirs, defines, file);
        //Control c;
        //Parser parser(&c);
        //ParseSession session;
        //session.setContentsAndGenerateLocationTable(pp.preprocess());
        //TranslationUnitAST* ast = parser.parse(&session);



        {
            //issues:
            //macro definitions in ParserOptions::dropMacros are to be ignored and the macros are not to be expanded
            //
            //"signals", "slots", "Q_SIGNALS", "Q_SLOTS" are special and the non-clang parser knows about them.
            //we look at for them while doing macro substitution.
            //any definition of "Q_PROPERTY(text)" macro is replaced by "void __q_property(const char* foo = #text);"
            //TODO this does not happen

            // we can get notified about the macro from the preprocessor:
            // virtual void clang::PPCallbacks::MacroDefined(const Token &MacroNameTok, const MacroDirective *MD);
            // and when they are expanded:
            // virtual void clang::PPCallbacks::MacroExpands(const Token &MacroNameTok, const MacroDefinition &MD, SourceRange Range, const MacroArgs *Args )
            // can be added with clang::Preprocessor::addPPCallbacks


            // Configure the diagnostics.
            llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine>
            Diags(clang::CompilerInstance::createDiagnostics(new clang::DiagnosticOptions, new DiagConsumer()));

            std::unique_ptr<std::vector<const char *> >
            ClangArgs(new std::vector<const char *>());

            ClangArgs->push_back("-fno-spell-checking");
            ClangArgs->push_back("-fparse-all-comments");
            for(char const **elem = COMPILETIME_CLANG_ARGS; *elem; elem++) {
                ClangArgs->push_back(*elem);
            }
            ClangArgs->push_back("-x");
            ClangArgs->push_back("c++");
            QByteArray absfile_path = file.absoluteFilePath().toLocal8Bit();
            ClangArgs->push_back(absfile_path.data());

            clang::IntrusiveRefCntPtr<llvm::vfs::FileSystem> VFS = llvm::vfs::createPhysicalFileSystem();

            clang::SmallVector<clang::StoredDiagnostic, 4> StoredDiagnostics;

            std::shared_ptr<clang::CompilerInvocation> CI;

            CI = std::make_shared<clang::CompilerInvocation>();
            clang::CompilerInvocation::CreateFromArgs(*CI, *ClangArgs,
                    *Diags);

            clang::PreprocessorOptions &PPOpts = CI->getPreprocessorOpts();
            PPOpts.RemappedFilesKeepOriginalName = true;
            PPOpts.AllowPCHWithCompilerErrors = true;
            PPOpts.SingleFileParseMode = false;
            PPOpts.RetainExcludedConditionalBlocks = false;
            for(auto &d : clangMacroDef) {
                PPOpts.addMacroDef(d);
            }

            // we should probably just invoke clang to tell us about the #defines...
            // clang++-14 -E -dI -dM -std=c++17 -x c++ - < /dev/null

            PPOpts.addMacroDef("__SMOKEGEN_RUN__=1");

            PPOpts.addMacroDef("__DEPRECATED=1");
            PPOpts.addMacroDef("__EXCEPTIONS=1");
            PPOpts.addMacroDef("__FXSR__=1");
            PPOpts.addMacroDef("__GCC_ATOMIC_BOOL_LOCK_FREE=2");
            PPOpts.addMacroDef("__GCC_ATOMIC_CHAR16_T_LOCK_FREE=2");
            PPOpts.addMacroDef("__GCC_ATOMIC_CHAR32_T_LOCK_FREE=2");
            PPOpts.addMacroDef("__GCC_ATOMIC_CHAR_LOCK_FREE=2");
            PPOpts.addMacroDef("__GCC_ATOMIC_INT_LOCK_FREE=2");
            PPOpts.addMacroDef("__GCC_ATOMIC_LLONG_LOCK_FREE=2");
            PPOpts.addMacroDef("__GCC_ATOMIC_LONG_LOCK_FREE=2");
            PPOpts.addMacroDef("__GCC_ATOMIC_POINTER_LOCK_FREE=2");
            PPOpts.addMacroDef("__GCC_ATOMIC_SHORT_LOCK_FREE=2");
            PPOpts.addMacroDef("__GCC_ATOMIC_TEST_AND_SET_TRUEVAL=1");
            PPOpts.addMacroDef("__GCC_ATOMIC_WCHAR_T_LOCK_FREE=2");
            PPOpts.addMacroDef("__GCC_HAVE_DWARF2_CFI_ASM=1");
            PPOpts.addMacroDef("__GNUC_GNU_INLINE__=1");
            PPOpts.addMacroDef("__GNUC_MINOR__=2");
            PPOpts.addMacroDef("__GNUC_PATCHLEVEL__=1");
            PPOpts.addMacroDef("__GNUC__=4");
            PPOpts.addMacroDef("__GNUG__=4");
            PPOpts.addMacroDef("__GXX_ABI_VERSION=1002");
            PPOpts.addMacroDef("__GXX_EXPERIMENTAL_CXX0X__=1");
            PPOpts.addMacroDef("__GXX_RTTI=1");
            PPOpts.addMacroDef("__GXX_WEAK__=1");
            PPOpts.addMacroDef("__PIC__=2");
            PPOpts.addMacroDef("__PIE__=2");
            PPOpts.addMacroDef("__pic__=2");
            PPOpts.addMacroDef("__pie__=2");

            PPOpts.UsePredefines = true;


            auto &hsi = CI->getHeaderSearchOpts();
            // Override the resources path.
            hsi.ResourceDir = COMPILETIME_CLANG_RESOURCEPATH;

            for (auto &id : clangIncludeDirs) {
                hsi.AddPath(id, clang::frontend::IncludeDirGroup::After, false, false);
            }

            for(char const **elem = COMPILETIME_CLANG_INCLUDES; *elem; elem++) {
                hsi.AddPath(*elem, clang::frontend::IncludeDirGroup::System, false, true);
            }
            CI->getFrontendOpts().SkipFunctionBodies = false;

            auto FileMgr = new clang::FileManager(CI->getFileSystemOpts(), VFS);

            auto Clang = std::make_unique<clang::CompilerInstance>();
            Clang->setInvocation(CI);

            Clang->setFileManager(FileMgr);

            std::string OriginalSourceFile =
                std::string(Clang->getFrontendOpts().Inputs[0].getFile());

            // Set up diagnostics, capturing any diagnostics that would
            // otherwise be dropped.
            Clang->setDiagnostics(Diags.get());

            // Create the target instance.
            if (!Clang->createTarget()) {
                qDebug() << "Could not create Clang target" << Qt::endl;
                return 1;
            }

            auto SourceMgr = llvm::makeIntrusiveRefCnt<clang::SourceManager>(
                                 *Diags, *FileMgr, true);

            // Create the source manager.
            Clang->setSourceManager(SourceMgr.get());

            std::unique_ptr<TopLevelDeclTrackerAction> Act(
                new TopLevelDeclTrackerAction());

            if (!Act->BeginSourceFile(*Clang, Clang->getFrontendOpts().Inputs[0])) {
                qDebug() << "Act->BeginSourceFile" << Qt::endl;
                return 1;
            }
            auto AccessSpecAnnotations = std::make_shared< std::vector<QTRangedAnnotation> >();
            auto PropertyAnnotations = std::make_shared< std::vector<QTRangedAnnotation> >();
            Clang->getPreprocessor().addPPCallbacks(std::make_unique<MacroOverrideCallbacks>(AccessSpecAnnotations, PropertyAnnotations));

            if (llvm::Error Err = Act->Execute()) {
                consumeError(std::move(Err));
                qDebug() << "Act->Execute" << Qt::endl;
                return 1;
            }

            clang::ASTContext *context = nullptr;

            if (Clang->hasASTContext()) {
                context = &Clang->getASTContext();
            }

            // Zero out now to ease cleanup during crash recovery.
            CI = nullptr;
            Diags = nullptr;

            if(!context) {
                qDebug() << "No context generated" << Qt::endl;
            } else {
/*
                std::string dumped;
                llvm::raw_string_ostream dumpStream(dumped);
                clang::ASTDumper clangdumper(dumpStream, *context, true);
                clangdumper.dumpDeclContext(context->getTranslationUnitDecl());
                qDebug().noquote() << QString::fromStdString(dumped) << Qt::endl;
*/
                ClangGeneratorVisitor visitor(context, AccessSpecAnnotations, PropertyAnnotations, file.fileName());
                visitor.TraverseDecl(context->getTranslationUnitDecl());
            }

            //this releases the context
            Clang->setSourceManager(nullptr);
            Clang->setFileManager(nullptr);

            Act->EndSourceFile();
        }



        // TODO: improve 'header => class' association
        //GeneratorVisitor visitor(&session, file.fileName());
        //visitor.visit(ast);
        
        if (!logErrors)
            continue;
        /*
        Q_FOREACH (const Problem* p, c.problems()) {
            logOut << file.fileName() << ": " << p->file << "(" << p->position.line << ", " << p->position.column << "): "
                   << p->description << "\n";
        }
        */
    }
    
    log.close();
    
    return generate();
}
