#pragma once

#include <stack>
#include <string>

//#include "app_config.h"
#include "umba/cmd_line.h"


#if defined(WIN32) || defined(_WIN32)
    #include <shellapi.h>
#endif

// AppConfig    appConfig;


struct ArgParser
{

std::stack<std::string> optFiles;


std::string makeAbsPath( std::string p )
{
    std::string basePath;

    if (optFiles.empty())
        basePath = umba::filesys::getCurrentDirectory();
    else
        basePath = umba::filename::getPath(optFiles.top());


    return umba::filename::makeAbsPath( p, basePath );

}



// 0 - ok, 1 normal stop, -1 - error
template<typename ArgsParser>
int operator()( const std::string                               &a           //!< строка - текущий аргумент
              , umba::command_line::CommandLineOption           &opt         //!< Объект-опция, содержит разобранный аргумент и умеет отвечать на некоторые вопросы
              , ArgsParser                                      &argsParser  //!< Класс, который нас вызывает, содержит некоторый контекст
              , umba::command_line::ICommandLineOptionCollector *pCol        //!< Коллектор опций - собирает инфу по всем опциям и готов вывести справку
              , bool fBuiltin
              , bool ignoreInfos
              )
{
    //using namespace marty::clang::helpers;

    std::string dppof = "Don't parse predefined options from ";

    if (opt.isOption())
    {
        std::string errMsg;
        std::string strVal;
        int intVal;
        //unsigned uintVal;
        std::size_t szVal;
        bool boolVal;

        if (opt.name.empty())
        {
            LOG_ERR_OPT<<"invalid (empty) option name\n";
            return -1;
        }

       if (opt.isOption("quet") || opt.isOption('q') || opt.setDescription("Operate quetly"))  // . Short alias for '--verbose=quet'
        {
            argsParser.quet = true;
            //appConfig.setOptQuet(true);
        }

        #if defined(WIN32) || defined(_WIN32)
        else if (opt.isOption("home") || opt.setDescription("Open homepage"))
        {
            if (argsParser.hasHelpOption) return 0;
            ShellExecuteA( 0, "open", appHomeUrl, 0, 0, SW_SHOW );
            return 1;
        }
        #endif

        // else if (opt.setParam("LEVEL", 1, "0/quet/no/q|" 
        //                                   "1/normal/n|" 
        //                                   "2/config/c|" 
        //                                   "3/detailed/detail/d|" 
        //                                   "4/extra/high/e" 
        //                      )
        //       || opt.setInitial(1) || opt.isOption("verbose") || opt.isOption('V')
        //       || opt.setDescription("Set verbosity level. LEVEL parameter can be one of the next values:\n"
        //                             "quet - maximum quet mode (same as --quet).\n"
        //                             "normal - print common details.\n"
        //                             "config - print common details and app config.\n"
        //                             "detailed - print common details, app config and all declarations, which are found in user files.\n"
        //                             "extra - print common details, app config and all found declarations (from all files)." // "\n"
        //                            )
        //       )
        // {
        //     if (argsParser.hasHelpOption) return 0;
        //  
        //     auto mapper = [](int i) -> VerbosityLevel
        //                   {
        //                       //return AppConfig::VerbosityLevel_fromStdString((VerbosityLevel)i);
        //                       switch(i)
        //                       {
        //                           case  0: case  1: case  2: case  3: case  4: return (VerbosityLevel)i;
        //                           default: return VerbosityLevel::begin;
        //                       }
        //                   };
        //  
        //     VerbosityLevel lvl;
        //     if (!opt.getParamValue( lvl, errMsg, mapper ) )
        //     {
        //         LOG_ERR_OPT<<errMsg<<"\n";
        //         return -1;
        //     }
        //  
        //     //appConfig.setVerbosityLevel(lvl);
        //     if (lvl==VerbosityLevel::quet)
        //         argsParser.quet = true;
        // }

        else if ( opt.isBuiltinsDisableOptionMain  () 
               || opt.setDescription( dppof + "main distribution options file '" + argsParser.getBuiltinsOptFileName(umba::program_location::BuiltinOptionsLocationFlag::appGlobal   ) + "'"))
        { } // simple skip - обработка уже сделана

        else if ( opt.isBuiltinsDisableOptionCustom() 
               || opt.setDescription( dppof + "custom global options file '"     + argsParser.getBuiltinsOptFileName(umba::program_location::BuiltinOptionsLocationFlag::customGlobal) + "'"))
        { } // simple skip - обработка уже сделана

        else if ( opt.isBuiltinsDisableOptionUser  () 
               || opt.setDescription( dppof + "user local options file '"        + argsParser.getBuiltinsOptFileName(umba::program_location::BuiltinOptionsLocationFlag::userLocal   ) + "'"))
        { } // simple skip - обработка уже сделана

        else if (opt.isOption("version") || opt.isOption('v') || opt.setDescription("Show version info"))
        {
            if (argsParser.hasHelpOption) return 0;

            if (!ignoreInfos)
            {
                printOnlyVersion();
                return 1;
            }
        }

        else if (opt.isOption("where") || opt.setDescription("Show where the executable file is"))
        {
            if (argsParser.hasHelpOption) return 0;

            //LOG_MSG_OPT << programLocationInfo.exeFullName << "\n";
            LOG_MSG_OPT << argsParser.programLocationInfo.exeFullName << "\n";
            return 0;
        }

        else if (opt.setParam("CLR", 0, "no/none/file|" 
                                        "ansi/term|" 
                                        #if defined(WIN32) || defined(_WIN32)
                                        "win32/win/windows/cmd/console"
                                        #endif
                             )
              || opt.setInitial(-1) || opt.isOption("color") 
              || opt.setDescription("Force set console output coloring")
              /* ", can be:\nno, none, file - disable coloring\nansi, term - set ansi terminal coloring\nwin32, win, windows, cmd, console - windows console specific coloring method" */
              )
        {
            if (argsParser.hasHelpOption) return 0;

            umba::term::ConsoleType res;
            auto mapper = [](int i) -> umba::term::ConsoleType
                          {
                              switch(i)
                              {
                                  case 0 : return umba::term::ConsoleType::file;
                                  case 1 : return umba::term::ConsoleType::ansi_terminal;
                                  case 2 : return umba::term::ConsoleType::windows_console;
                                  default: return umba::term::ConsoleType::file;
                              };
                          };
            if (!opt.getParamValue( res, errMsg, mapper ) )
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            coutWriter.forceSetConsoleType(res);
            cerrWriter.forceSetConsoleType(res);
        }

        else if ( opt.setParam("?MODE",true)
               || opt.isOption("overwrite") || opt.isOption('Y') 
               // || opt.setParam("VAL",true)
               || opt.setDescription("Allow overwrite existing file."))
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(boolVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }
            
            bOverwrite = boolVal;
            return 0;
        }

        else if ( opt.setParam("?MODE",true)
               || opt.isOption("prolog") || opt.isOption('P') 
               // || opt.setParam("VAL",true)
               || opt.setDescription("Enabling/disabling generation of prolog/epilog. Prolog also contain includes. Prolog/epilog used for generate files with enums, not a standalone enums"))
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(boolVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }
            
            bPrologEpilog = boolVal;
            return 0;
        }

        else if ( opt.setParam("?MODE",true)
               || opt.isOption("includes") || opt.isOption('i') 
               // || opt.setParam("VAL",true)
               || opt.setDescription("Enabling/disabling generation of includes even if prolog is enabled"))
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(boolVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }
            
            bIncludes = boolVal;
            return 0;
        }



        else if ( opt.setParam("N", 0, 0, 128)
               || opt.isOption("base-indent") || opt.isOption("indent") || opt.isOption('B') 
               || opt.setDescription("Set base indent size"))
        {
            if (argsParser.hasHelpOption) return 0;
         
            if (!opt.getParamValue( szVal, errMsg ) )
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }
         
            indentSize = szVal;
         
            return 0;
        }

        else if ( opt.setParam("N", 0, 0, 32)
               || opt.isOption("hex-number-width") || opt.isOption("hex-width") || opt.isOption('H') 
               || opt.setDescription("Set number width for hex format"))
        {
            if (argsParser.hasHelpOption) return 0;
         
            if (!opt.getParamValue( szVal, errMsg ) )
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }
         
            hexNumberWidth = (unsigned)szVal;
         
            return 0;
        }

        else if ( opt.setParam("N", 0, 0, 32)
               || opt.isOption("oct-number-width") || opt.isOption("oct-width") || opt.isOption('8') 
               || opt.setDescription("Set number width for oct format"))
        {
            if (argsParser.hasHelpOption) return 0;
         
            if (!opt.getParamValue( szVal, errMsg ) )
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }
         
            octNumberWidth = (unsigned)szVal;
         
            return 0;
        }

        else if ( opt.setParam("N", 0, 0, 16)
               || opt.isOption("indent-increment") || opt.isOption("indent-inc") || opt.isOption('C') 
               || opt.setDescription("Set indent increment size - cur indent is base-indent+LEVEL*indent-inc"))
        {
            if (argsParser.hasHelpOption) return 0;
         
            if (!opt.getParamValue( szVal, errMsg ) )
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }
         
            indentInc = szVal;
         
            return 0;
        }

        else if ( opt.setParam("LINEFEED",umba::command_line::OptionType::optString)
               || opt.isOption("linefeed") || opt.isOption("LF") || opt.isOption('L')
               // || opt.setParam("VAL",true)
               || opt.setDescription("Output linefeed. LINEFEED is one of: CR/LF/CRLF/LFCR/DETECT."))
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }
            
            ELinefeedType tmp = marty_cpp::enum_deserialize( strVal, ELinefeedType::invalid );
            if (tmp==ELinefeedType::invalid)
            {
                LOG_ERR_OPT<<"Invalid linefeed option value: "<<strVal<<"\n";
                return -1;
            }

            outputLinefeed = tmp;

            return 0;
        }

        else if ( opt.setParam("OPTFLAGS",umba::command_line::OptionType::optString)
               || opt.isOption("enum-generation-flags") || opt.isOption("enum-flags") || opt.isOption("flags") || opt.isOption("options") || opt.isOption('f')
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Enum generation flags/options. Flags. Can be combined with ',', '+' or '|'. Can be taken multiple times and will be ORed with previous value\n"
                                      "Can be a combination of next values:\n"
                                      "0 - reset previously defined flags\n"
                                      "type-decl - generate enum type declaration\n"
                                      "enum-class - generate enum class instead of unscoped enum\n"
                                      "flags - generate flags with bitwise operators defined\n"
                                      "serialize - generate serialization\n"
                                      "deserialize - generate deserialization\n"
                                      "serialize-both - serialize|deserialize\n"
                                      "serialize-set - serialization from set or from flag bits\n"
                                      "deserialize-set - deserialization to set or to flag bits from ORed combination of vals\n"
                                      "extra - generate extra (de)serialization - to/from enum set, or combination of flags - serializeSet|deserializeSet\n"
                                      "all - typeDecl|serialize|deserialize|serializeSet|deserializeSet\n"
                                      "integers - allow to deserialize from integer values (decimal)\n"
                                      "unsigned-vals - underlaying type is unsigned\n"
                                      "umap - use unordered_map/set to hold strings instead of map/set\n"
                                      "lowercase - deserialize trough lowercase (case independed deserialization)\n"
                                      "single-name - use only one (first) name for deserialization, don't allow multiple aliases\n"
                                      "no-comments - disable comments for enum entries\n"
                                      "no-extra-linefeed - don't add extra linefeeds between sections (in addition to template)\n"
                                      "fmt-auto - enum values auto format (depending on it's definition)\n"
                                      "fmt-oct - force format enum value numbers as octal\n"
                                      "fmt-dec - force format enum value numbers as decimal\n"
                                      "fmt-hex - force format enum value numbers as hexadecimal\n"
                                      "sys - system includes before user\n"
                                      // "no-pe, no-prolog-epilog - don't generate file prolog/epilog - to generate only part of file. Includes don't generated also\n"
                                      // "\n"
                                      // Хотел ещё какой-то флаг добавить
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            if (strVal=="0")
            {
                enumGeneratorOptions = 0;
                return 0;
            }
            
            marty_cpp::EnumGeneratorOptionFlagsSerializable tmp;
            try
            {
                marty_cpp::enum_deserialize_flags(tmp, strVal /*, seps = ",|+"*/);
                if (tmp==marty_cpp::EnumGeneratorOptionFlagsSerializable::invalid)
                {
                    //LOG_ERR_OPT << "Invalid options value: '" << strVal << "'\n"; // , --options\n"
                    throw std::runtime_error("Invalid options value");
                }
            }
            catch(const std::exception &e)
            {
                LOG_ERR_OPT << e.what() << "\n";
                return -1;
            }

            unsigned utmp = (unsigned)tmp;

            enumGeneratorOptions |= utmp;

            return 0;
        }

        else if ( opt.setParam("STYLE",umba::command_line::OptionType::optString)
               || opt.isOption("namespace-name-style") || opt.isOption("ns-name-style") || opt.isOption("namespace-style") || opt.isOption("ns-style")
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Namespace name style. Can be one of (default is PascalStyle):\n"
                                      // "All - generate all style names for deserialization\n"
                                      // "HyphenStyle - lowercase name parts separated by hyphen\n"
                                      // "HyphenCamelMixedStyle - name parts separated by hyphen, first is in lowercase, next ones - PascalStyle\n"
                                      // "HyphenPascalMixedStyle - name parts separated by hyphen, all part in PascalStyle\n"
                                      "DefaultStyle - use name as is\n"
                                      "CppStyle - lowercase name parts separated by underscore\n"
                                      "CamelStyle - first name part is in lowercase, next ones - PascalStyle\n"
                                      "PascalStyle - all name parts starts with uppercas letter\n"
                                      "CppCamelMixedStyle - name parts separated by underscore, first is in lowercase, next ones - PascalStyle"
                                      "CppPascalMixedStyle - all PascalStyle name parts separated by underscore\n"
                                      "DefineStyle - uppercase name parts separated by underscore\n"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            marty_cpp::NameStyle nameStyle = marty_cpp::fromString(strVal, marty_cpp::NameStyle::invalid);
            switch(nameStyle)
            {
                // case marty_cpp::NameStyle::all:
                // case marty_cpp::NameStyle::hyphenStyle:
                // case marty_cpp::NameStyle::hyphenCamelMixedStyle:
                // case marty_cpp::NameStyle::hyphenPascalMixedStyle:
                case marty_cpp::NameStyle::cppStyle:
                case marty_cpp::NameStyle::camelStyle:
                case marty_cpp::NameStyle::pascalStyle:
                case marty_cpp::NameStyle::cppCamelMixedStyle:
                case marty_cpp::NameStyle::cppPascalMixedStyle:
                case marty_cpp::NameStyle::defineStyle:
                // case marty_cpp::NameStyle:::
                    break;

                case marty_cpp::NameStyle::defaultStyle:
                    if (strVal=="DefaultStyle")
                        break;

                default:
                    LOG_ERR_OPT << "Invalid enum name style value: '" << strVal << "' (--namespace-name-style)\n"; // , --options\n"
                    return -1;
            };

            namespaceNameStyle = nameStyle;
            return 0;
        }

        else if ( opt.setParam("STYLE",umba::command_line::OptionType::optString)
               || opt.isOption("enum-name-style") || opt.isOption("enum-style") || opt.isOption('M')
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Enum name style. Can be one of (default is PascalStyle):\n"
                                      // "All - generate all style names for deserialization\n"
                                      // "HyphenStyle - lowercase name parts separated by hyphen\n"
                                      // "HyphenCamelMixedStyle - name parts separated by hyphen, first is in lowercase, next ones - PascalStyle\n"
                                      // "HyphenPascalMixedStyle - name parts separated by hyphen, all part in PascalStyle\n"
                                      "DefaultStyle - use name as is\n"
                                      "CppStyle - lowercase name parts separated by underscore\n"
                                      "CamelStyle - first name part is in lowercase, next ones - PascalStyle\n"
                                      "PascalStyle - all name parts starts with uppercas letter\n"
                                      "CppCamelMixedStyle - name parts separated by underscore, first is in lowercase, next ones - PascalStyle"
                                      "CppPascalMixedStyle - all PascalStyle name parts separated by underscore\n"
                                      "DefineStyle - uppercase name parts separated by underscore\n"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            marty_cpp::NameStyle nameStyle = marty_cpp::fromString(strVal, marty_cpp::NameStyle::invalid);
            switch(nameStyle)
            {
                // case marty_cpp::NameStyle::all:
                // case marty_cpp::NameStyle::hyphenStyle:
                // case marty_cpp::NameStyle::hyphenCamelMixedStyle:
                // case marty_cpp::NameStyle::hyphenPascalMixedStyle:
                case marty_cpp::NameStyle::cppStyle:
                case marty_cpp::NameStyle::camelStyle:
                case marty_cpp::NameStyle::pascalStyle:
                case marty_cpp::NameStyle::cppCamelMixedStyle:
                case marty_cpp::NameStyle::cppPascalMixedStyle:
                case marty_cpp::NameStyle::defineStyle:
                // case marty_cpp::NameStyle:::
                    break;

                case marty_cpp::NameStyle::defaultStyle:
                    if (strVal=="DefaultStyle")
                        break;

                default:
                    LOG_ERR_OPT << "Invalid enum name style value: '" << strVal << "' (--enum-name-style)\n"; // , --options\n"
                    return -1;
            };

            enumNameStyle = nameStyle;
            return 0;
        }

        else if ( opt.setParam("STYLE",umba::command_line::OptionType::optString)
               || opt.isOption("enum-values-style") || opt.isOption("values-style") || opt.isOption('V')
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Enum values style. Can be one of (default is PascalStyle):\n"
                                      // "All - generate all style names for deserialization\n"
                                      // "HyphenStyle - lowercase name parts separated by hyphen\n"
                                      // "HyphenCamelMixedStyle - name parts separated by hyphen, first is in lowercase, next ones - PascalStyle\n"
                                      // "HyphenPascalMixedStyle - name parts separated by hyphen, all part in PascalStyle\n"
                                      "DefaultStyle - use name as is\n"
                                      "CppStyle - lowercase name parts separated by underscore\n"
                                      "CamelStyle - first name part is in lowercase, next ones - PascalStyle\n"
                                      "PascalStyle - all name parts starts with uppercas letter\n"
                                      "CppCamelMixedStyle - name parts separated by underscore, first is in lowercase, next ones - PascalStyle"
                                      "CppPascalMixedStyle - all PascalStyle name parts separated by underscore\n"
                                      "DefineStyle - uppercase name parts separated by underscore\n"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            marty_cpp::NameStyle nameStyle = marty_cpp::fromString(strVal, marty_cpp::NameStyle::invalid);
            switch(nameStyle)
            {
                // case marty_cpp::NameStyle::all:
                // case marty_cpp::NameStyle::hyphenStyle:
                // case marty_cpp::NameStyle::hyphenCamelMixedStyle:
                // case marty_cpp::NameStyle::hyphenPascalMixedStyle:
                case marty_cpp::NameStyle::cppStyle:
                case marty_cpp::NameStyle::camelStyle:
                case marty_cpp::NameStyle::pascalStyle:
                case marty_cpp::NameStyle::cppCamelMixedStyle:
                case marty_cpp::NameStyle::cppPascalMixedStyle:
                case marty_cpp::NameStyle::defineStyle:
                // case marty_cpp::NameStyle:::
                    break;

                case marty_cpp::NameStyle::defaultStyle:
                    if (strVal=="DefaultStyle")
                        break;

                default:
                    LOG_ERR_OPT << "Invalid enum values style value: '" << strVal << "' (--enum-values-style)\n"; // , --options\n"
                    return -1;
            };

            valuesNameStyle = nameStyle;
            return 0;
        }

        else if ( opt.setParam("STYLE",umba::command_line::OptionType::optString)
               || opt.isOption("enum-serialize-style") || opt.isOption("serialize-style") || opt.isOption('S')
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Enum serialize/deserialize names style.\n"
                                      //""
                                      "Can be one of (default is PascalStyle):\n"
                                      "All - generate all style names for deserialization. If this serialize value name style taken, PascalStyle names used to serialize value\n"
                                      "DefaultStyle - use name as is\n"
                                      "HyphenStyle - lowercase name parts separated by hyphen\n"
                                      "HyphenCamelMixedStyle - name parts separated by hyphen, first is in lowercase, next ones - PascalStyle\n"
                                      "HyphenPascalMixedStyle - name parts separated by hyphen, all part in PascalStyle\n"
                                      "CppStyle - lowercase name parts separated by underscore\n"
                                      "CamelStyle - first name part is in lowercase, next ones - PascalStyle\n"
                                      "PascalStyle - all name parts starts with uppercas letter\n"
                                      "CppCamelMixedStyle - name parts separated by underscore, first is in lowercase, next ones - PascalStyle"
                                      "CppPascalMixedStyle - all PascalStyle name parts separated by underscore\n"
                                      "DefineStyle - uppercase name parts separated by underscore\n"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            marty_cpp::NameStyle nameStyle = marty_cpp::fromString(strVal, marty_cpp::NameStyle::invalid);
            switch(nameStyle)
            {
                case marty_cpp::NameStyle::all:
                case marty_cpp::NameStyle::hyphenStyle:
                case marty_cpp::NameStyle::hyphenCamelMixedStyle:
                case marty_cpp::NameStyle::hyphenPascalMixedStyle:
                case marty_cpp::NameStyle::cppStyle:
                case marty_cpp::NameStyle::camelStyle:
                case marty_cpp::NameStyle::pascalStyle:
                case marty_cpp::NameStyle::cppCamelMixedStyle:
                case marty_cpp::NameStyle::cppPascalMixedStyle:
                case marty_cpp::NameStyle::defineStyle:
                // case marty_cpp::NameStyle:::
                    break;

                case marty_cpp::NameStyle::defaultStyle:
                    if (strVal=="DefaultStyle")
                    break;

                default:
                    LOG_ERR_OPT << "Invalid enum serialize/deserialize names style value: '" << strVal << "' (--enum-serialize-style)\n"; // , --options\n"
                    return -1;
            };

            serializedNameStyle = nameStyle;
            return 0;
        }

        else if ( opt.setParam("TYPENAME",umba::command_line::OptionType::optString)
               || opt.isOption("unsigned-type") || opt.isOption('U')
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Add type signature for unsigned type for unsigned types autodetection\n"
                                      "Example: --unsigned-type=std::uint32_t - after that std::uint32_t detected as unsigned if that used as underlaying type"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            unsignedTypes.insert(strVal);

            return 0;
        }

        else if ( opt.setParam("TYPENAME",umba::command_line::OptionType::optString)
               || opt.isOption("underlaying-type") || opt.isOption("underlaying") || opt.isOption('D')
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Set underlaying type for enum"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            underlayingType = strVal;

            if (unsignedTypes.find(underlayingType)!=unsignedTypes.end())
            {
                enumGeneratorOptions |= marty_cpp::EnumGeneratorOptionFlags::unsignedVals;
            }
            else
            {
                enumGeneratorOptions &= ~(marty_cpp::EnumGeneratorOptionFlags::unsignedVals);
            }

            return 0;
        }

        else if ( opt.setParam("NS",umba::command_line::OptionType::optString)
               || opt.isOption("namespace") || opt.isOption('N')
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Set namespace(s) name(s) in form outer_ns::inner1::inner2 or outer_ns/inner1/inner2"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            namespacesStr = strVal;

            return 0;
        }

        else if ( opt.setParam("PREFIX",umba::command_line::OptionType::optString)
               || opt.isOption("element-prefix") || opt.isOption("prefix") || opt.isOption('X')
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Set prefix for enum element names. Not used in style formatting, added as is"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            enumElementPrefix = strVal;

            return 0;
        }

        else if ( opt.setParam("LANG",umba::command_line::OptionType::optString)
               || opt.isOption("target-language") || opt.isOption("target-lang") || opt.isOption("language") || opt.isOption("lang") || opt.isOption('G')
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Set target language for enum generation"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            targetLang = strVal;

            return 0;
        }

        else if ( opt.setParam("NAME",umba::command_line::OptionType::optString)
               || opt.isOption("template") || opt.isOption('T')
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Specify template name for enum generation. If not taken, 'default' template name will be used"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            templateName = strVal;

            return 0;
        }

        else if ( opt.setParam("NAME:VALUE",umba::command_line::OptionType::optString)
               || opt.isOption("override-template-parameter") || opt.isOption("OTP")
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Override template parameter"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            overrideTemplateParameters.emplace_back(strVal);

            return 0;
        }

        else if ( opt.setParam("NAME",umba::command_line::OptionType::optString)
               || opt.isOption("enum-name") || opt.isOption("enum") || opt.isOption('E')
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Specify enum name for generation"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            enumName = strVal;

            return 0;
        }

        else if ( opt.setParam("NAME",umba::command_line::OptionType::optString)
               || opt.isOption("enum-comment") || opt.isOption("comment") || opt.isOption('m')
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Specify enum name for generation.\n"
                                      "You can also use string starting with the '//', like '// Some enum description' as first line/item of the enum definition, but this option overrides enum description comment"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                //LOG_ERR_OPT<<errMsg<<"\n";
                //return -1;
            }

            enumComment = strVal;

            return 0;
        }

        else if ( opt.setParam("ENUMDEF",umba::command_line::OptionType::optString)
               || opt.isOption("enum-definition") || opt.isOption("definition") || opt.isOption('F')
               // || opt.setParam("VAL",true)
               || opt.setDescription( "Specify enum definition. Use '@' at first position to specify definition file instead of immediate definition (-F=@EnumDefFile.txt)\n"
                                      "When this option encountered, enum parameters are stored in queue, and --enum-name, --enum-comment, --element-prefix values are cleared\n"
                                      "Enum generation flags (--enum-generation-flags) will be inherited for the next enum definition. To reset enum generation flags use '--enum-generation-flags=0'\n"
                                      "All other parameters are inherited by the next enum defs and must be overridden explicitly\n"
                                      "Format of enum definition:\n"
                                      "[// Enum description;] NAME[,ALIASES...] [(=|:)VALUE] [// Enum item comment]; [MORE_ITEM_DEFINITIONS...]\n"
                                      "When enum definition file is used (with leading '@') the good idea place each item definition on a separate line, line break and ';' symbol means the same\n"
                                      "Restrictions is in that you can't use ';', ':', '=' symbols in comment part\n"
                                      "VALUE, if taken, can references to previously defined names. That names will be formatted properly. Mixed expressions of a names and constants are also allowed"
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            if (!opt.getParamValue(strVal,errMsg))
            {
                LOG_ERR_OPT<<errMsg<<"\n";
                return -1;
            }

            if (enumName.empty())
            {
                LOG_ERR_OPT<<"enum name not taken. Use '--enum-name' option"<<"\n";
                return -1;
            }

            //!!! std::string makeAbsPath( std::string p )

            EnumGenerationArgs args;

            if (!strVal.empty() && strVal[0]=='@')
            {
                // read from file
                std::string enumDefFileName = std::string(strVal, 1, strVal.size()-1);
                strVal = std::string("@") + makeAbsPath(enumDefFileName);
                args.fromFile = true;
            }

            args.valsText            = strVal;
            args.enumName            = enumName;
            args.enumComment         = enumComment;
            args.underlayingType     = underlayingType;
            args.enumNameStyle       = marty_cpp::toString<std::string>(enumNameStyle);
            args.valuesNameStyle     = marty_cpp::toString<std::string>(valuesNameStyle);
            args.serializedNameStyle = marty_cpp::toString<std::string>(serializedNameStyle);
            args.enumElementPrefix   = enumElementPrefix;
            args.generatorOptions    = enumGeneratorOptions;
            args.hexNumberWidth      = hexNumberWidth;
            args.octNumberWidth      = octNumberWidth;

            enumGenerationArgsList.push_back(args);

            enumName.clear();
            enumComment.clear();
            enumElementPrefix.clear();

            return 0;
        }


        // indent - base indent (num spaces)
        // indent-inc - indent increment for next levels (num spaces)
        // enumName - имя enum'а
        // enum values name style
        // enum type name style
        // enum serialization name style
        // enum values prefix (actual for non-class enums)
        // lang - target lang
        // template - generator template name


        else if ( opt.isOption("autocomplete-install") 
               || opt.setDescription("Install autocompletion to bash"
                                     #if defined(WIN32) || defined(_WIN32)
                                         "/clink(cmd)"
                                     #endif
                                    )
               )
        {
            if (argsParser.hasHelpOption) return 0;

            //return autocomplete(opt, true);
            return umba::command_line::autocompletionInstaller( pCol, opt, pCol->getPrintHelpStyle(), true, [&]( bool bErr ) -> decltype(auto) { return bErr ? LOG_ERR_OPT : LOG_MSG_OPT; } );
        }
        else if ( opt.isOption("autocomplete-uninstall") 
               || opt.setDescription("Remove autocompletion from bash"
                                     #if defined(WIN32) || defined(_WIN32)
                                         "/clink(cmd)"
                                     #endif
                                    )
                )
        {
            if (argsParser.hasHelpOption) return 0;

            //return autocomplete(opt, false);
            return umba::command_line::autocompletionInstaller( pCol, opt, pCol->getPrintHelpStyle(), false, [&]( bool bErr ) -> decltype(auto) { return bErr ? LOG_ERR_OPT : LOG_MSG_OPT; } );
        }

        else if (opt.isHelpStyleOption())
        {
            // Job is done in isHelpStyleOption
        }
        else if (opt.isHelpOption()) // if (opt.infoIgnore() || opt.isOption("help") || opt.isOption('h') || opt.isOption('?') || opt.setDescription(""))
        {
            if (!ignoreInfos)
            {
                if (pCol && !pCol->isNormalPrintHelpStyle())
                    argsParser.quet = true;
                //printNameVersion();
                if (!argsParser.quet)
                {
                    printNameVersion();
                    printBuildDateTime();
                    printCommitHash();
                    std::cout<<"\n";
                //printHelp();
                }

                if (pCol && pCol->isNormalPrintHelpStyle() && argsParser.argsNeedHelp.empty())
                {
                    auto helpText = opt.getHelpOptionsString();
                    //std::cout << "Usage: " << programLocationInfo.exeName
                    std::cout << "Usage: " << argsParser.programLocationInfo.exeName
                              << " [OPTIONS] [output_file]\n"
                              << "  If output_file not taken, STDOUT used (can be tken explicitly)\n"
                              << "  Use 'CLPB'/'CLIPBRD' to use clipboard as output file\n"
                              << "  Performs lookup for 'umba-enum-gen-options.txt' or 'umba-enum-gen-flags.txt' up from current directory - for project-related global umba-enum-gen options\n"
                              << "  'umba-enum-gen-flags.txt', if found, processed after parsing builtins, but before processing currently taken options\n"
                              << "\nOptions:\n\n"
                              << helpText;
                              //<< " [OPTIONS] input_file [output_file]\n\nOptions:\n\n"<<helpText;
                }
                
                if (pCol) // argsNeedHelp
                    std::cout<<pCol->makeText( 78, &argsParser.argsNeedHelp );

                return 1;

            }

            return 0; // simple skip then parse builtins
        }
        else
        {
            LOG_ERR_OPT<<"unknown option: "<<opt.argOrg<<"\n";
            return -1;
        }

        return 0;

    } // if (opt.isOption())
    else if (opt.isResponseFile())
    {
        std::string optFileName = makeAbsPath(opt.name);

        optFiles.push(optFileName);

        auto parseRes = argsParser.parseOptionsFile( optFileName );

        optFiles.pop();

        if (!parseRes)
            return -1;

        if (argsParser.mustExit)
            return 1;

        return 0;
    
    }

    //appConfig.clangCompileFlagsTxtFilename.push_back(makeAbsPath(a));

    //appConfig.outputName = makeAbsPath(a);

    outputFilename = argsParser.makeAbsPath(a);

    return 0;

}

}; // struct ArgParser



class CommandLineOptionCollector : public umba::command_line::CommandLineOptionCollectorImplBase
{
protected:
    virtual void onOptionDup( const std::string &opt ) override
    {
        LOG_ERR_OPT<<"Duplicated option key - '"<<opt<<"'\n";
        throw std::runtime_error("Duplicated option key");
    }

};



