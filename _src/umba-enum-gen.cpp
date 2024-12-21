/*! \file
    \brief Утилита umba-tabtool - нормализация табов
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

        argsParser.args.push_back("@" + rootPath + "_libs/marty_cpp/_generators/enums.rsp");
        argsParser.args.push_back("--enum-flags=enum-class,type-decl,serialize,deserialize,lowercase");
        argsParser.args.push_back("-N=marty_cpp");
        argsParser.args.push_back("-E=LinefeedType");
        //argsParser.args.push_back("-F=invalid,unknown=-1;detect=0;lf;cr;lfcr;crlf;");
        argsParser.args.push_back("--enum-flags=extra,flags");
        argsParser.args.push_back("--underlaying-type=std::uint32_t");
        argsParser.args.push_back("--override-template-parameter=EnumFlagsNameFormat:$(ENAMNAME)");
        argsParser.args.push_back("-E=EnumGeneratorOptionFlagsSerializable");
        argsParser.args.push_back("-F=@" + rootPath + "_libs/marty_cpp/_generators/TestNumParsing.txt");
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


    if (!argsParser.quet)
    {
        umba::cli_tool_helpers::printNameVersion(umbaLogStreamMsg);

        // Отложенный вывод инфы о найденом flags файле
        if (!projectFlagsFileName.empty())
        {
            umbaLogStreamMsg << "Found project umba-enum-gen flags file: " << projectFlagsFileName << "\n";
        }
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

            genArgs.valsText = marty_cpp::simple_trim( genArgs.valsText
                                                     , [](char ch)
                                                       {
                                                           if (ch==' ' || ch=='\t')
                                                               return true;
                                                           return false;
                                                       }
                                                     );

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
                genArgs.valsText = marty_cpp::simple_string_replace<std::string>(genArgs.valsText, marty_cpp::make_string<std::string>(";"), marty_cpp::make_string<std::string>("\n") );
            }

            marty_cpp::enum_generate_serialize( oss
                                              , genArgs.valsText
                                              , indentStr
                                              , indentIncStr
                                              , appConfig.enumName
                                              , genArgs.underlayingType
                                              , genArgs.valuesNameStyle
                                              , genArgs.serializedNameStyle
                                              , genArgs.enumElementPrefix
                                              , generatorOptions
                                              , genTpl
                                              , &dups
                                              );
       
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

