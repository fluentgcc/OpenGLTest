#include <string.h>
#include "utf8-utils.h"
#include <Windows.h>
#include <string>
// ----------------------------------------------------- utf8_surrogate_len ---
size_t ftgl::utf8_surrogate_len( const char* character )
{
    size_t result = 0;
    char test_char;

    if (!character)
        return 0;

    test_char = character[0];

    if ((test_char & 0x80) == 0)
        return 1;

    while (test_char & 0x80)
    {
        test_char <<= 1;
        result++;
    }

    return result;
}

// ------------------------------------------------------------ utf8_strlen ---
size_t ftgl::utf8_strlen( const char* string )
{
    const char* ptr = string;
    size_t result = 0;

    while (*ptr)
    {
        ptr += utf8_surrogate_len(ptr);
        result++;
    }

    return result;
}

uint32_t ftgl::utf8_to_utf32( const char * character )
{
    uint32_t result = -1;

    if( !character )
    {
        return result;
    }

    if( ( character[0] & 0x80 ) == 0x0 )
    {
        result = character[0];
    }

    if( ( character[0] & 0xC0 ) == 0xC0 )
    {
        result = ( ( character[0] & 0x3F ) << 6 ) | ( character[1] & 0x3F );
    }

    if( ( character[0] & 0xE0 ) == 0xE0 )
    {
        result = ( ( character[0] & 0x1F ) << ( 6 + 6 ) ) | ( ( character[1] & 0x3F ) << 6 ) | ( character[2] & 0x3F );
    }

    if( ( character[0] & 0xF0 ) == 0xF0 )
    {
        result = ( ( character[0] & 0x0F ) << ( 6 + 6 + 6 ) ) | ( ( character[1] & 0x3F ) << ( 6 + 6 ) ) | ( ( character[2] & 0x3F ) << 6 ) | ( character[3] & 0x3F );
    }

    if( ( character[0] & 0xF8 ) == 0xF8 )
    {
        result = ( ( character[0] & 0x07 ) << ( 6 + 6 + 6 + 6 ) ) | ( ( character[1] & 0x3F ) << ( 6 + 6 + 6 ) ) | ( ( character[2] & 0x3F ) << ( 6 + 6 ) ) | ( ( character[3] & 0x3F ) << 6 ) | ( character[4] & 0x3F );
    }

    return result;
}

const char* ftgl::GBK_to_utf8( const char* string )
{
	//todo£º

	WCHAR * str1;  
	int n = MultiByteToWideChar( CP_ACP, 0,string, -1, NULL, 0 );
	str1 = new WCHAR[n];
	MultiByteToWideChar( CP_ACP, 0, string, -1, str1, n );

	n = WideCharToMultiByte( CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);  
	char * str2 = new char[n];
	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);  
	
//	std::string strOutUTF8 = str2;  
	delete[]str1;  
	str1 = NULL;  
// 	delete[]str2;  
// 	str2 = NULL;  
//	return strOutUTF8.c_str();

	return str2;
}


