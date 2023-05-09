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
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include <tuple>

#include "umba/debug_helpers.h"
#include "umba/string_plus.h"
#include "umba/program_location.h"
#include "umba/scope_exec.h"
#include "umba/macro_helpers.h"
#include "umba/macros.h"

#include "marty_cpp/marty_cpp.h"
#include "marty_cpp/sort_includes.h"
#include "marty_cpp/enums.h"
#include "marty_cpp/src_normalization.h"
#include "marty_cpp/marty_ns.h"

#include "encoding/encoding.h"
#include "umba/cli_tool_helpers.h"
#include "umba/time_service.h"


#if defined(WIN32) || defined(_WIN32)

    #define HAS_CLIPBOARD_SUPPORT 1
    #include "umba/clipboard_win32.h"

#endif


// #include "enums.h"

// #include "umba/time_service.h"
// #include "umba/text_utils.h"

struct EnumGenerationArgs
{
    std::string  valsText           ;
    std::string  enumName           ;
    std::string  enumComment        ;
    std::string  underlayingType    ;
    std::string  enumNameStyle      ;
    std::string  valuesNameStyle    ;
    std::string  serializedNameStyle;
    std::string  enumElementPrefix  ;
    unsigned     generatorOptions   ;
    unsigned     hexNumberWidth     ;
    unsigned     octNumberWidth     ;
};




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
std::string namespacesStr;
std::string targetLang;
std::string templateName;

std::string outputFilename;

std::unordered_set<std::string>   unsignedTypes;

using marty_cpp::ELinefeedType;
ELinefeedType                     outputLinefeed         = ELinefeedType::detect;

std::string                       enumName;
std::string                       enumComment;
std::string                       underlayingType;
marty_cpp::NameStyle              namespaceNameStyle  = marty_cpp::NameStyle::pascalStyle;
marty_cpp::NameStyle              enumNameStyle       = marty_cpp::NameStyle::pascalStyle;
marty_cpp::NameStyle              valuesNameStyle     = marty_cpp::NameStyle::pascalStyle;
marty_cpp::NameStyle              serializedNameStyle = marty_cpp::NameStyle::pascalStyle;
std::string                       enumElementPrefix;
unsigned                          enumGeneratorOptions   = 0;

unsigned                          hexNumberWidth      = 8;
unsigned                          octNumberWidth      = 3;

std::vector<std::string>          overrideTemplateParameters;


std::vector<EnumGenerationArgs>   enumGenerationArgsList;


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

    // Force set CLI arguments while running under debugger
    if (umba::isDebuggerPresent())
    {
        argsParser.args.clear();
        argsParser.args.push_back("--options=0");
        argsParser.args.push_back("--hex-width=4");
        argsParser.args.push_back("--override-template-parameter=EnumFlagsNameFormat:F$(ENAMNAME)");
        argsParser.args.push_back("--options=type-decl,enum-class,flags");
        argsParser.args.push_back("--enum-name-style=PascalStyle");
        argsParser.args.push_back("--enum-values-style=CamelStyle");
        argsParser.args.push_back("--enum-serialize-style=HyphenStyle");
        argsParser.args.push_back("--indent-increment=4");
        argsParser.args.push_back("--namespace=test/ns");
        argsParser.args.push_back("--enum-name=MyCoolEnum");
        argsParser.args.push_back("--enum-definition=//Some kind of test enum;invalid:-1; begin-some=0x0; next=1; nextOne; hex=0x11; final");
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


    // Прочитать файл с настройками шаблона
    // --language --output-language --output-lang -L
    // --template


    // StringType getConfFilename( StringType fileName // name relative to conf root
    //                           , bool useUserConf
    //                           ) const
    //  

    unsigned allGenerationOptions = 0;

    for(auto &genArgs : enumGenerationArgsList)
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


    for(auto otp : overrideTemplateParameters)
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
        marty_cpp::enum_generate_includes(oss, allGenerationOptions, genTpl);
        oss << "\n";
    }


    {
        auto ns = marty_cpp::makeNamespaceOutputWriteGuard( oss
                                                          , namespacesStr
                                                          , namespaceNameStyle // marty_cpp::NameStyle::defaultStyle //TODO: !!!
                                                          , genTpl.nsBegin
                                                          , genTpl.nsEnd
                                                          , genTpl.nsNameFormat
                                                          );


        for(auto &genArgs : enumGenerationArgsList)
        {
            std::vector<std::string> dups;
       
            marty_cpp::NameStyle enumNameStyle = marty_cpp::fromString(genArgs.enumNameStyle, marty_cpp::NameStyle::invalid);
            std::string enumName = marty_cpp::formatName( genArgs.enumName, enumNameStyle, true /* fixStartDigit */ );
       
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


            std::size_t startsLen = marty_cpp::sort_includes_utils::startsWith(genArgs.valsText.begin(), genArgs.valsText.end(), "//");
            if (startsLen==2)
            {
                genArgs.valsText.erase(0, startsLen);
                std::size_t endPos = genArgs.valsText.find_first_of(";\n");
                if (endPos!=genArgs.valsText.npos)
                {
                    std::string comment = std::string(genArgs.valsText, 0, endPos);
                    genArgs.valsText.erase(0, endPos+1);
                    if (genArgs.enumComment.empty())
                    {
                        genArgs.enumComment = comment;
                    }
                }
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

            marty_cpp::enum_generate_serialize( oss
                                              , genArgs.valsText
                                              , indentStr
                                              , indentIncStr
                                              , enumName
                                              , genArgs.underlayingType
                                              , genArgs.valuesNameStyle
                                              , genArgs.serializedNameStyle
                                              , genArgs.enumElementPrefix
                                              , genArgs.generatorOptions
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

    if (outputLinefeed==ELinefeedType::unknown || outputLinefeed==ELinefeedType::detect)
    {
        #if defined(WIN32) || defined(_WIN32)
            outputLinefeed = ELinefeedType::crlf;
        #else
            outputLinefeed = ELinefeedType::lf;
        #endif
    }


    std::string resultText = marty_cpp::converLfToOutputFormat(oss.str(), outputLinefeed);

    try
    {
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

