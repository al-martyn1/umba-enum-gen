#pragma once

//
#include "marty_tr/marty_tr.h"
#include "encoding/encoding.h"

//
#include <string>

inline
std::string autoConvertToUtf8(std::string str)
{
    encoding::EncodingsApi* pEncApi = encoding::getEncodingsApi();
    
    std::size_t bomSize = 0;
    std::string detectRes = pEncApi->detect( str, bomSize );
    
    if (bomSize)
    {
       str.erase(0,bomSize);
    }
    
    auto cpId = pEncApi->getCodePageByName(detectRes);
    
    return pEncApi->convert( str, cpId, encoding::EncodingsApi::cpid_UTF8 );
}
