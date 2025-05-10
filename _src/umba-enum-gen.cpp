/*! \file
    \brief Утилита umba-enum-gen - генератор перечислений
 */

// Должна быть первой
#include "umba/umba.h"
//---

//#-sort
#include "umba/simple_formatter.h"
#include "umba/char_writers.h"
//#+sort

#include "umba/debug_helpers.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <tuple>

#include "umba/debug_helpers.h"
#include "umba/string_plus.h"
#include "umba/program_location.h"
#include "umba/scope_exec.h"
#include "umba/macro_helpers.h"
#include "umba/macros.h"
#include "umba/scanners.h"

#include "marty_cpp/marty_cpp.h"
#include "marty_cpp/marty_enum.h"
#include "marty_cpp/marty_flags.h"
#include "marty_cpp/sort_includes.h"
#include "marty_cpp/enums.h"
#include "marty_cpp/src_normalization.h"
#include "marty_cpp/marty_ns.h"
#include "marty_cpp/marty_enum_impl_helpers.h"

#include "encoding/encoding.h"
#include "umba/cli_tool_helpers.h"
#include "umba/time_service.h"
#include "umba/shellapi.h"

//
#include "marty_tr/marty_tr.h"

//
#include "AppConfig.h"


#if defined(WIN32) || defined(_WIN32)

    #define HAS_CLIPBOARD_SUPPORT 1
    #include "umba/clipboard_win32.h"

#endif


// #include "enums.h"

// #include "umba/time_service.h"
// #include "umba/text_utils.h"




umba::StdStreamCharWriter coutWriter(std::cout);
umba::StdStreamCharWriter cerrWriter(std::cerr);
umba::NulCharWriter       nulWriter;

umba::SimpleFormatter umbaLogStreamErr(&cerrWriter);
umba::SimpleFormatter umbaLogStreamMsg(&cerrWriter);
umba::SimpleFormatter umbaLogStreamNul(&nulWriter);

bool umbaLogGccFormat   = false; // true;
bool umbaLogSourceInfo  = false;

bool bOverwrite         = false;
std::size_t indentSize  = 0;
std::size_t indentInc   = 0;
bool bPrologEpilog      = true;
bool bIncludes          = true;
std::string namespacesStr;
std::string targetLang;
std::string templateName;

std::string outputFilename;

AppConfig  appConfig;

#include "log.h"

//umba::program_location::ProgramLocation<std::string>   programLocationInfo;


#include "umba/cmd_line.h"

// Конфиг версии
#include "app_ver_config.h"
// Принтуем версию
#include "print_ver.h"
// Парсер параметров командной строки
#include "arg_parser.h"



int main(int argc, char* argv[])
{

    marty_tr::tr_set_lang_tag_format(marty_tr::ELangTagFormat::langTag); // langTagNeutral/langTagNeutralAuto/langId/langIdFull/langIdX/langIdFullX
    marty_tr::tr_set_def_lang(marty_tr::tr_fix_lang_tag_format("en-US"));
    marty_tr::tr_alter_set_def_lang(marty_tr::tr_fix_lang_tag_format("en-US"));


    using namespace umba::omanip;


    auto argsParser = umba::command_line::makeArgsParser( ArgParser()
                                                        , CommandLineOptionCollector()
                                                        , argc, argv
                                                        , umba::program_location::getProgramLocation
                                                            ( argc, argv
                                                            , false // useUserFolder = false
                                                            //, "" // overrideExeName
                                                            )
                                                        );

    std::string cwd;
    std::string rootPath = umba::shellapi::getDebugAppRootFolder(&cwd);


    // // std::vector<StringType> simpleProcessLineContinuations(const std::vector<StringType> &v)
    // if (umba::isDebuggerPresent())
    // {
    //     //simpleProcessLineContinuations
    //     auto vec = marty_cpp::simple_string_split(std::string("\\aa\\\nbbb\nccc\nddd\\\neee\n"), marty_cpp::make_string<std::string>("\n") /* , typename StringType::size_type nSplits = -1 */ );
    //     vec = marty_cpp::simple_process_line_continuations(vec);
    //  
    //     // 4817, 4819
    //     std::cout << "simple_process_line_continuations\n";
    // }

    // Force set CLI arguments while running under debugger
    if (umba::isDebuggerPresent())
    {
        argsParser.args.clear();
        argsParser.args.push_back("--overwrite");

        std::cout << "Working Dir: " << cwd << "\n";
        std::cout << "Root Path  : " << rootPath << "\n";

        // argsParser.args.push_back("--options=0");
        // argsParser.args.push_back("--hex-width=4");
        // argsParser.args.push_back("--override-template-parameter=EnumFlagsNameFormat:F$(ENAMNAME)");
        // argsParser.args.push_back("--options=type-decl,enum-class,flags");
        // argsParser.args.push_back("--enum-name-style=PascalStyle");
        // argsParser.args.push_back("--enum-values-style=CamelStyle");
        // argsParser.args.push_back("--enum-serialize-style=HyphenStyle");
        // argsParser.args.push_back("--indent-increment=4");
        // argsParser.args.push_back("--namespace=test/ns");
        // argsParser.args.push_back("--enum-name=MyCoolEnum");
        // argsParser.args.push_back("--enum-definition=//Some kind of test enum;invalid:-1; begin-some=0x0; next=1; nextOne; hex=0x11; final");

        argsParser.args.push_back("@" + rootPath + "_libs/marty_cpp/_enums/enums.rsp");
        argsParser.args.push_back("--tr-template-output=" + rootPath + "_libs/marty_cpp/_enums/tpl-tmp.tr.json");
        argsParser.args.push_back("--tr-lang=en-US,ru-RU");
        argsParser.args.push_back("--md-output-path=" + rootPath + "tests/generated_md");
        argsParser.args.push_back("--enum-flags=enum-class,type-decl,serialize,deserialize,lowercase");
        argsParser.args.push_back("-N=marty_cpp");
        argsParser.args.push_back("-E=LinefeedType");
        //argsParser.args.push_back("-F=invalid,unknown=-1;detect=0;lf;cr;lfcr;crlf;");
        argsParser.args.push_back("--enum-flags=extra,flags");
        argsParser.args.push_back("--underlaying-type=std::uint32_t");
        argsParser.args.push_back("--override-template-parameter=EnumFlagsNameFormat:$(ENAMNAME)");
        argsParser.args.push_back("-E=EnumGeneratorOptionFlagsSerializable");
        argsParser.args.push_back("-F=@" + rootPath + "_libs/marty_cpp/_enums/EnumGeneratorOptionFlagsSerializable.txt");
        //argsParser.args.push_back("-F=@" + rootPath + "_libs/marty_cpp/_enums/TestNumParsing.txt");
        argsParser.args.push_back(rootPath + "../enums2.h");

        // argsParser.args.push_back("");

        //argsParser.args.clear();
        //argsParser.args.push_back("@..\\tests\\data\\test01.rsp");

        //argsParser.args.push_back("@..\\make_sources_brief.rsp");
        // argsParser.args.push_back(umba::string_plus::make_string(""));
        // argsParser.args.push_back(umba::string_plus::make_string(""));
        // argsParser.args.push_back(umba::string_plus::make_string(""));
    }

    //programLocationInfo = argsParser.programLocationInfo;

    // try
    // {
        // Job completed - may be, --where option found
        if (argsParser.mustExit)
            return 0;
       
        // if (!argsParser.quet)
        // {
        //     printNameVersion();
        // }
       
        if (!argsParser.parseStdBuiltins())
            return 1;
        if (argsParser.mustExit)
            return 0;

        // Try to find project global flags
        //umba-enum-gen-options.txt
        std::string projectFlagsFileName = umba::scanners::scanForOptionsFile(std::vector<std::string>{"umba-enum-gen-options.txt", "umba-enum-gen-flags.txt"}, umba::filesys::getCurrentDirectory());

        // report later about found project global flags file

        // вставляем flags file как response file на первое место
        if (!projectFlagsFileName.empty())
        {
            argsParser.args.insert(argsParser.args.begin(), std::string("@")+projectFlagsFileName);
        }

       
        if (!argsParser.parse())
            return 1;
        if (argsParser.mustExit)
            return 0;
    // }
    // catch(const std::exception &e)
    // {
    //     LOG_ERR_OPT << e.what() << "\n";
    //     return -1;
    // }
    // catch(const std::exception &e)
    // {
    //     LOG_ERR_OPT << "command line arguments parsing error" << "\n";
    //     return -1;
    // }


        // std::string resultJson = marty_tr::tr_serialize_translations(marty_tr::tr_get_all_translations(), 2 /* indent */);
        // umba::filesys::createDirectoryEx( umba::filename::getPath(targetName), true /* forceCreatePath */  );
        // umba::filesys::writeFile(targetName, resultJson, appConfig.bOverwrite ); // overwrite

    // pAssetsManager->loadTranslations();


    if (!argsParser.quet)
    {
        umba::cli_tool_helpers::printNameVersion(umbaLogStreamMsg);

        // Отложенный вывод инфы о найденом flags файле
        if (!projectFlagsFileName.empty())
        {
            umbaLogStreamMsg << "Found project umba-enum-gen flags file: " << projectFlagsFileName << "\n";
        }
    }

    if (umba::isDebuggerPresent())
    {
        const auto &trMap     = marty_tr::tr_get_all_translations();
        const auto &trDefLang = marty_tr::tr_get_def_lang();
        // Inspect trMap here
        UMBA_ARG_USED(trMap);
        UMBA_ARG_USED(trDefLang);
    }


    unsigned allGenerationOptions = 0;

    for(auto &genArgs : appConfig.enumGenerationArgsList)
    {
        allGenerationOptions |= genArgs.generatorOptions;

        if (!genArgs.valsText.empty() && genArgs.valsText[0]=='@')
        {
            // read from file
            std::string enumDefFileName = std::string(genArgs.valsText, 1, genArgs.valsText.size()-1);
            std::vector<char> enumDefData;
            if (!umba::filesys::readFile(enumDefFileName, enumDefData))
            {
                LOG_ERR_OPT << "failed to read enum definition from file '" << enumDefFileName << "'\n";
                return 1;
            }

            std::string enumDefText = std::string(enumDefData.begin(), enumDefData.end());
            std::string enumDefTextNorm = marty_cpp::normalizeCrLfToLf(enumDefText, 0 /* pDetectedLinefeedType */);
            genArgs.valsText = enumDefTextNorm;
        }
        else
        {
            genArgs.valsText = marty_cpp::normalizeCrLfToLf(marty_cpp::cUnescapeString(genArgs.valsText), 0 /* pDetectedLinefeedType */);
        }
    }
    

    if (templateName.empty())
        templateName = "default";

    if (targetLang.empty())
    {
        LOG_WARN_OPT("target") << "target language not defined, 'cpp' will be used'\n";
        targetLang = "cpp";
    }


    std::string templateFromConfFullName = targetLang + "/" + templateName + ".txt";
    std::string userTplCandidate   = argsParser.programLocationInfo.getConfFilename(templateFromConfFullName, true );
    std::string customTplCandidate = argsParser.programLocationInfo.getConfFilename(argsParser.programLocationInfo.exeName + ".custom/" + templateFromConfFullName, false);
    std::string mainTplCandidate   = argsParser.programLocationInfo.getConfFilename(argsParser.programLocationInfo.exeName +        "/" + templateFromConfFullName, false);
    std::string templateFileName;

    marty_cpp::EnumGeneratorTemplate<std::string> genTpl; // = marty_cpp::EnumGeneratorTemplate<std::string>::defaultCpp();
    std::vector<char> templateData;

    auto readTemplateFile = [&]() -> bool
    {
        if (umba::filesys::readFile(userTplCandidate, templateData))
        {
            templateFileName = userTplCandidate;
            return true;
        }

        if (umba::filesys::readFile(customTplCandidate, templateData))
        {
            templateFileName = customTplCandidate;
            return true;
        }

        if (umba::filesys::readFile(mainTplCandidate, templateData))
        {
            templateFileName = mainTplCandidate;
            return true;
        }

        return false;
    };

    if (!readTemplateFile())
    {
        LOG_WARN_OPT("template") << "template not found in '" << userTplCandidate << "' nor in '" << customTplCandidate << "' nor in '" << mainTplCandidate << "'\n";
        LOG_WARN_OPT("template") << "default C++ template used\n";
        genTpl = marty_cpp::EnumGeneratorTemplate<std::string>::defaultCpp();
    }
    else
    {
        std::string templateText = std::string(templateData.begin(), templateData.end());
        std::string templateTextNorm = marty_cpp::normalizeCrLfToLf(templateText, 0 /* pDetectedLinefeedType */);

        auto parseTplRes = marty_cpp::EnumGeneratorTemplate<std::string>::parseTemplateOptionsText
            ( templateTextNorm
            , umbaLogStreamErr
            , false            // ignoreUnknownParams
            , umbaLogGccFormat
            , templateFileName
            );

        if (std::get<0>(parseTplRes)==false)
        {
            LOG_WARN_OPT("template") << "failed to parse template file '" << templateFileName << "'\n";
            LOG_WARN_OPT("template") << "default C++ template used\n";
            genTpl = marty_cpp::EnumGeneratorTemplate<std::string>::defaultCpp();
        }
        else
        {
            genTpl = std::get<1>(parseTplRes);
        }
    }


    for(auto otp : appConfig.overrideTemplateParameters)
    {
        if (!genTpl.checkAssignParam(otp))
        {
            LOG_ERR_OPT << "Invalid override template parameter value - '" << otp << "' (--override-template-parameter)" << "\n";
            return 1;
        }
    }

    if (outputFilename.empty())
        outputFilename = "STDOUT";

    umba::cli_tool_helpers::IoFileType outputFileType =
    umba::cli_tool_helpers::detectFilenameType(outputFilename, false /* bInput */);

    std::ostringstream oss;

    // prolog, if not disabled

    // Собрать по OR все флаги всех enum'ов
    // В зависимости от этого собираем требуемые иклюды - по шаблону с настройками - нужно, если bPrologEpilog разрешен

    if (bPrologEpilog)
    {
        // Генерим пролог и инклуды
        marty_cpp::enum_generate_prolog(oss, allGenerationOptions, genTpl);
        if (bIncludes)
        {
            marty_cpp::enum_generate_includes(oss, allGenerationOptions, genTpl);
        }
        oss << "\n";
    }


    {
        auto ns = marty_cpp::makeNamespaceOutputWriteGuard( oss
                                                          , namespacesStr
                                                          , appConfig.namespaceNameStyle // marty_cpp::NameStyle::defaultStyle //TODO: !!!
                                                          , genTpl.nsBegin
                                                          , genTpl.nsEnd
                                                          , genTpl.nsNameFormat
                                                          );


        for(auto &genArgs : appConfig.enumGenerationArgsList)
        {
            std::vector<std::string> dups;
       
            //marty_cpp::NameStyle 
            appConfig.enumNameStyle = marty_cpp::fromString(genArgs.enumNameStyle, marty_cpp::NameStyle::invalid);
            //std::string 
            appConfig.enumName = marty_cpp::formatName( genArgs.enumName, appConfig.enumNameStyle, true /* fixStartDigit */, true /* fixKeywords */ );

            genTpl.hexWidth = genArgs.hexNumberWidth;
            genTpl.octWidth = genArgs.octNumberWidth;

            std::string indentStr    = std::string(indentSize, ' '); // indent
            std::string indentIncStr = std::string(indentInc , ' ');  // indentInc

            // genArgs.valsText = marty_cpp::simple_trim( genArgs.valsText
            //                                          , [](char ch)
            //                                            {
            //                                                if (ch==' ' || ch=='\t')
            //                                                    return true;
            //                                                return false;
            //                                            }
            //                                          );

            // getStripEnumDefComment и так делаед трим

            std::string inlineEnumComment = marty_cpp::enum_impl_helpers::getStripEnumDefComment(genArgs.valsText);
            // std::size_t startsLen = marty_cpp::sort_includes_utils::startsWith(genArgs.valsText.begin(), genArgs.valsText.end(), "//");
            // if (startsLen==2)
            // {
            //     genArgs.valsText.erase(0, startsLen);
            //     std::size_t endPos = genArgs.valsText.find_first_of(";\n");
            //     if (endPos!=genArgs.valsText.npos)
            //     {
            //         std::string comment = std::string(genArgs.valsText, 0, endPos);
            //         genArgs.valsText.erase(0, endPos+1);
            //         if (genArgs.enumComment.empty())
            //         {
            //             genArgs.enumComment = comment;
            //         }
            //     }
            // }

            if (genArgs.enumComment.empty())
            {
                genArgs.enumComment = inlineEnumComment;
            }

            if (!genArgs.enumComment.empty())
            {
                if ((genArgs.generatorOptions&marty_cpp::EnumGeneratorOptionFlags::noExtraLinefeed)==0)
                    oss << "\n";

                oss << "\n" << genTpl.formatComment(indentStr, genArgs.enumComment); // << "\n";

                // Если запрещены ExtraLinefeed, то enum_generate_serialize не добавит перевод строки перед enum, и нам надо добавить его тут
                if ((genArgs.generatorOptions&marty_cpp::EnumGeneratorOptionFlags::noExtraLinefeed)!=0)
                    oss << "\n";
            }

            unsigned generatorOptions = genArgs.generatorOptions;
            std::unordered_set<std::string>::const_iterator uit = appConfig.unsignedTypes.find(genArgs.underlayingType);
            if (uit!= appConfig.unsignedTypes.end())
            {
                if (!argsParser.quet)
                {
                    // umbaLogStreamMsg << "Detected unsigned type - '" << genArgs.underlayingType << "'\n";
                }
                generatorOptions |= marty_cpp::EnumGeneratorOptionFlags::unsignedVals;
            }

            if (!genArgs.fromFile)
            {
                // Если не из файла, то надо заменить semicolon (';') на перевод строки
                genArgs.valsText = marty_cpp::simple_string_replace<std::string>(genArgs.valsText, marty_cpp::make_string<std::string>(";"), marty_cpp::make_string<std::string>("\n") );
            }
            /*
            // Ой. Уже есть внутрях marty_cpp

            else // Если из файла, то обрабатываем line continuations
            {
                bool prevSlash = false;
                std::string tmpText; tmpText.reserve(genArgs.valsText.size());
                for(auto ch: genArgs.valsText)
                {
                    if (prevSlash)
                    {
                        prevSlash = false;

                        if (ch=='\n')
                        {
                            tmpText.append(1, ' '); // Если предыдущий символ - слэш, то перевод строки заменяем на пробел
                        }
                        else
                        {
                            // Любой символ, кроме перевода строки
                            if (ch=='\\')
                            {
                                tmpText.append(1, '\\'); // Предыдущий слэш добавили, текущий - пока захолдили
                                prevSlash = true;
                            }
                            else
                            {
                                tmpText.append(1, '\\');
                                tmpText.append(1, ch);
                            }
                        }
                    }
                    else
                    {
                        if (ch=='\\')
                        {
                            prevSlash = true;
                        }
                        else
                        {
                            tmpText.append(1, ch);
                        }
                    }
                }

                swap(tmpText, genArgs.valsText)
            }
            */
                // , namespacesStr
                // , appConfig.namespaceNameStyle // marty_cpp::NameStyle::defaultStyle //TODO: !!!
                // , appConfig.enumName

            bool genDoc = !appConfig.mdOutputPath.empty();
            if (genDoc)
            {
                generatorOptions |= marty_cpp::EnumGeneratorOptionFlags::generateDoc;
            }

            //std::string nsPath;
            std::string nsFullName;
            {
                auto nsNames = marty_cpp::parseNsNameList<std::string>(namespacesStr);
                for(auto nsName : nsNames)
                {
                    if (appConfig.namespaceNameStyle!=marty_cpp::NameStyle::defaultStyle)
                        nsName = formatName(nsName, appConfig.namespaceNameStyle, true, true /* fix startDigit, keywords */);

                    if (!nsFullName.empty())
                        nsFullName.append(genTpl.namespaceSep);

                    nsFullName.append(nsName);
                }
            }

            std::ostringstream ossDocTmp;
            std::ostringstream ossDocSerializeTmp;

            //marty_tr::tr_set_def_lang(marty_tr::tr_fix_lang_tag_format(strVal));
            marty_cpp::enum_generate_serialize( oss
                                              , ossDocTmp
                                              , ossDocSerializeTmp
                                              , genArgs.valsText
                                              , indentStr
                                              , indentIncStr
                                              , nsFullName
                                              , appConfig.enumName           // Уже отформатированно как надо
                                              , inlineEnumComment
                                              , genArgs.underlayingType
                                              , genArgs.valuesNameStyle
                                              , genArgs.serializedNameStyle
                                              , genArgs.enumElementPrefix
                                              , generatorOptions
                                              , genTpl
                                              , &dups
                                              );

            if (genDoc)
            {
                if (appConfig.trLangs.empty())
                    appConfig.trLangs.emplace_back("en-US");

                std::string deflang;

                for(auto lang: appConfig.trLangs)
                {
                    lang = marty_tr::tr_fix_lang_tag_format(lang);
                    marty_tr::tr_set_def_lang(lang);

                    if (deflang.empty())
                        deflang = lang;

                    std::ostringstream ossTmp;
                    std::ostringstream ossDoc;
                    std::ostringstream ossDocSerialize;
                    std::vector<std::string> dupsTmp;

                    marty_cpp::enum_generate_serialize( ossTmp
                                                      , ossDoc
                                                      , ossDocSerialize
                                                      , genArgs.valsText
                                                      , indentStr
                                                      , indentIncStr
                                                      , nsFullName
                                                      , appConfig.enumName           // Уже отформатированно как надо
                                                      , inlineEnumComment
                                                      , genArgs.underlayingType
                                                      , genArgs.valuesNameStyle
                                                      , genArgs.serializedNameStyle
                                                      , genArgs.enumElementPrefix
                                                      , generatorOptions
                                                      , genTpl
                                                      , &dupsTmp
                                                      );

                    auto mdOutputPath = appConfig.mdOutputPath;
                    mdOutputPath = umba::filename::appendPath(mdOutputPath, lang);
                    {
                        auto nsNames = marty_cpp::parseNsNameList<std::string>(namespacesStr);
                        for(auto nsName : nsNames)
                        {
                            if (appConfig.namespaceNameStyle!=marty_cpp::NameStyle::defaultStyle)
                                nsName = formatName(nsName, appConfig.namespaceNameStyle, true, true /* fix startDigit, keywords */);
                            mdOutputPath = umba::filename::appendPath(mdOutputPath, nsName);
                        }
                    }
    
                    auto fileNameDoc  = umba::filename::appendExt(appConfig.enumName, std::string("md_"));
                    auto mdOutputName = umba::filename::appendPath(mdOutputPath, fileNameDoc);
    
                    auto fileNameDocStrNames  = umba::filename::appendExt(appConfig.enumName+"_strings", std::string("md_"));
                    auto mdOutputNameStrNames = umba::filename::appendPath(mdOutputPath, fileNameDocStrNames);
    
                    umba::filesys::createDirectoryEx( umba::filename::getPath(mdOutputName), true /* forceCreatePath */  );
    
                    std::string
                    mdText = marty_cpp::converLfToOutputFormat(ossDoc.str(), appConfig.outputLinefeed);
                    umba::cli_tool_helpers::writeOutput( mdOutputName, outputFileType
                                                       , encoding::ToUtf8(), encoding::FromUtf8()
                                                       , mdText, std::string() // bomData
                                                       , true /* fromFile */, true /* utfSource */ , bOverwrite
                                                       );
    
                    mdText = marty_cpp::converLfToOutputFormat(ossDocSerialize.str(), appConfig.outputLinefeed);
                    umba::cli_tool_helpers::writeOutput( mdOutputNameStrNames, outputFileType
                                                       , encoding::ToUtf8(), encoding::FromUtf8()
                                                       , mdText, std::string() // bomData
                                                       , true /* fromFile */, true /* utfSource */ , bOverwrite
                                                       );

                    if (deflang==lang)
                    {
                        auto mdOutputPath2 = appConfig.mdOutputPath;
                        {
                            auto nsNames = marty_cpp::parseNsNameList<std::string>(namespacesStr);
                            for(auto nsName : nsNames)
                            {
                                if (appConfig.namespaceNameStyle!=marty_cpp::NameStyle::defaultStyle)
                                    nsName = formatName(nsName, appConfig.namespaceNameStyle, true, true /* fix startDigit, keywords */);
                                mdOutputPath2 = umba::filename::appendPath(mdOutputPath2, nsName);
                            }
                        }

                        auto mdOutputName2 = umba::filename::appendPath(mdOutputPath2, fileNameDoc);
                        auto mdOutputNameStrNames2 = umba::filename::appendPath(mdOutputPath2, fileNameDocStrNames);
        
                        umba::filesys::createDirectoryEx( umba::filename::getPath(mdOutputName2), true /* forceCreatePath */  );
        
                        mdText = marty_cpp::converLfToOutputFormat(ossDoc.str(), appConfig.outputLinefeed);
                        umba::cli_tool_helpers::writeOutput( mdOutputName2, outputFileType
                                                           , encoding::ToUtf8(), encoding::FromUtf8()
                                                           , mdText, std::string() // bomData
                                                           , true /* fromFile */, true /* utfSource */ , bOverwrite
                                                           );
        
                        mdText = marty_cpp::converLfToOutputFormat(ossDocSerialize.str(), appConfig.outputLinefeed);
                        umba::cli_tool_helpers::writeOutput( mdOutputNameStrNames2, outputFileType
                                                           , encoding::ToUtf8(), encoding::FromUtf8()
                                                           , mdText, std::string() // bomData
                                                           , true /* fromFile */, true /* utfSource */ , bOverwrite
                                                           );
                    }

                }
            }

                                                          // , appConfig.namespaceNameStyle // marty_cpp::NameStyle::defaultStyle //TODO: !!!
                                                          // , genTpl.nsBegin
                                                          // , genTpl.nsEnd
                                                          // , genTpl.nsNameFormat

                                                          // , namespacesStr
                                                          // , appConfig.namespaceNameStyle // marty_cpp::NameStyle::defaultStyle //TODO: !!!
                                              
        }

    } // NS scope

    if (bPrologEpilog)
    {
        // Генерим эпилог
        marty_cpp::enum_generate_epilog(oss, allGenerationOptions, genTpl);

    }

    if (appConfig.outputLinefeed==marty_cpp::ELinefeedType::unknown || appConfig.outputLinefeed==marty_cpp::ELinefeedType::detect)
    {
        #if defined(WIN32) || defined(_WIN32)
            appConfig.outputLinefeed = marty_cpp::ELinefeedType::crlf;
        #else
            appConfig.outputLinefeed = marty_cpp::ELinefeedType::lf;
        #endif
    }


    std::string resultText = marty_cpp::converLfToOutputFormat(oss.str(), appConfig.outputLinefeed);

    try
    {
        if (!argsParser.quet)
        {
            
            umbaLogStreamMsg << "Current working dir: "<<cwd<<"\n";
            umbaLogStreamMsg << "Writting output to : "<<outputFilename<<"\n";
        }

        umba::cli_tool_helpers::writeOutput( outputFilename, outputFileType
                                           , encoding::ToUtf8(), encoding::FromUtf8()
                                           , resultText, std::string() // bomData
                                           , true /* fromFile */, true /* utfSource */ , bOverwrite
                                           );

        if (!appConfig.trTemplateFile.empty())
        {
            std::string resultJson = marty_tr::tr_serialize_translations(marty_tr::tr_alter_get_all_translations(), 2 /* indent */);
            umba::filesys::createDirectoryEx( umba::filename::getPath(appConfig.trTemplateFile), true /* forceCreatePath */  );
            umba::filesys::writeFile(appConfig.trTemplateFile, resultJson,  /* appConfig. */ bOverwrite ); // overwrite
        }


    } // try
    catch(const std::runtime_error &e)
    {
        LOG_ERR_OPT << e.what() << "\n";
        return 1;
    }
   
    if (!argsParser.quet)
    {
        umbaLogStreamMsg << "umba-enum-gen: Done\n";
    }

   
    #if 0
    try
    {
        umba::cli_tool_helpers::IoFileType inputFileType  = umba::cli_tool_helpers::IoFileType::nameEmpty;
        umba::cli_tool_helpers::IoFileType outputFileType = umba::cli_tool_helpers::IoFileType::nameEmpty;
        adjustInputOutputFilenames(inputFilename, inputFileType, outputFilename, outputFileType);

        if (outputFileType==umba::cli_tool_helpers::IoFileType::stdoutFile)
        {
            argsParser.quet = true;
        }

        if (!argsParser.quet)
        {
            std::string strEmpty;
            LOG_MSG_OPT << "Processing '" << inputFilename << "'"
                << ((inputFilename!=outputFilename) ? " -> '" : "")
                << ((inputFilename!=outputFilename) ? outputFilename : strEmpty)
                << ((inputFilename!=outputFilename) ? "'" : "")
                << "\n";
        }
       
        bool utfSource = false;
        bool checkBom  = true;
        bool fromFile  = true;

        std::string srcData = umba::cli_tool_helpers::readInput( inputFilename , inputFileType
                            , encoding::ToUtf8(), checkBom, fromFile, utfSource //, outputLinefeed
                            );

        //  
        // // Есть ли данные на входе, нет их - это не наша проблема - процессим пустой текст в нормальном режиме

        std::string bomData = umba::cli_tool_helpers::stripTheBom(srcData, checkBom, encoding::BomStripper());
       
        //------------------------------
       
        // Do job itself
        // auto startTick = umba::time_service::getCurTimeMs();

       
        ELinefeedType detectedSrcLinefeed = ELinefeedType::crlf;
       
        std::string lfNormalizedText = marty_cpp::normalizeCrLfToLf(srcData, &detectedSrcLinefeed);
       
        if (outputLinefeed==ELinefeedType::unknown || outputLinefeed==ELinefeedType::detect)
        {
            outputLinefeed = detectedSrcLinefeed;
        }

        #if defined(WIN32) || defined(_WIN32)
        if (outputFileType==umba::cli_tool_helpers::IoFileType::clipboard)
        {
            outputLinefeed = ELinefeedType::crlf;
        }
        #endif

        std::vector<std::string> textLines = marty_cpp::splitToLinesSimple( lfNormalizedText
                                                                          , true // addEmptyLineAfterLastLf
                                                                          , '\n' // lfChar
                                                                          );



        //----------------------------------------------------------------------------

        //std::vector<std::string> resLines = textLines;

        const std::string &curFile = inputFilename;
        size_t idx = 0;

        size_t errCount = 0;

        bool bUpdated = false;
        bool bTrStrip = false;

        std::string strEmpty;


        
        std::string resultText; //  = marty_cpp::mergeLines(textLines, outputLinefeed, false /* addTrailingNewLine */);

        //------------------------------

        umba::cli_tool_helpers::writeOutput( outputFilename, outputFileType
                                           , encoding::ToUtf8(), encoding::FromUtf8()
                                           , resultText, bomData
                                           , fromFile, utfSource, bOverwrite
                                           );

        // auto endTick = umba::time_service::getCurTimeMs();
        // if (!argsParser.quet)
        // {
        //     LOG_MSG_OPT << "    time: " << (endTick-startTick) << "ms\n";
        // }


    } // try
    catch(const std::runtime_error &e)
    {
        LOG_ERR_OPT << e.what() << "\n";
        return 1;
    }
    #endif

    return 0;
}    

