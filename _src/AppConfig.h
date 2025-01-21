/*! \file
    \brief Класс конфигурации. Настраивается опциями командной строки.
 */
#pragma once


struct EnumGenerationArgs
{
    bool         fromFile           = false;
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




struct AppConfig
{

    std::unordered_set<std::string>   unsignedTypes;
    
    marty_cpp::ELinefeedType          outputLinefeed           = marty_cpp::ELinefeedType::detect;
    
    std::string                       enumName;
    std::string                       enumComment;
    std::string                       underlayingType;
    marty_cpp::NameStyle              namespaceNameStyle       = marty_cpp::NameStyle::pascalStyle;
    marty_cpp::NameStyle              enumNameStyle            = marty_cpp::NameStyle::pascalStyle;
    marty_cpp::NameStyle              valuesNameStyle          = marty_cpp::NameStyle::pascalStyle;
    marty_cpp::NameStyle              serializedNameStyle      = marty_cpp::NameStyle::pascalStyle;
    std::string                       enumElementPrefix;
    unsigned                          enumGeneratorOptions     = 0;

    unsigned                          hexNumberWidth           = 8;
    unsigned                          octNumberWidth           = 3;
    
    std::vector<std::string>          overrideTemplateParameters;
    
    std::vector<EnumGenerationArgs>   enumGenerationArgsList;

};