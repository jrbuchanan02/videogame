/**
 * @file justification.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Functions for text justification in stringfunctions.
 * @version 1
 * @date 2022-01-29
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#include <io/console/manip/stringfunctions.h++>
#include <string>
#include <vector>

using namespace io::console::manip;

std::vector<std::string>
		io::console::manip::processString ( std::string string )
{
	std::vector<std::string> codePoints = splitByCodePoint ( string );
	std::vector<std::string> friendly;
	for ( auto &codePoint : codePoints )
	{
		bool friendlyCodePoint = true;
		for ( char c = 1; c < '\u001b'; c++ )
		{
			if ( codePoint.find ( c ) != std::string::npos )
			{
				friendlyCodePoint = false;
				break;
			}
		}
		if ( codePoint.find ( "\u001b[" ) != std::string::npos )
		{
			friendlyCodePoint &= codePoint.ends_with ( "m" );
		} else if ( codePoint.find ( "\u001b]" ) != std::string::npos )
		{
#ifdef WINDOWS
			friendlyCodePoint &= codePoint.at ( 2 ) == '4';
			friendlyCodePoint &= codePoint.at ( 3 ) == ';';
			friendlyCodePoint &=
					codePoint.find ( ";rgb:" ) != std::string::npos;
#else
			friendlyCodePoint &= codePoint.at ( 2 ) == 'P';
			// no other friendly check for linux :(
#endif
			friendlyCodePoint &= codePoint.ends_with ( "\u001b\\" );
		} else if ( codePoint.find ( '\u001b' ) != std::string::npos )
		{
			friendlyCodePoint = false;
		}
		for ( char c = '\u001c'; c < ' '; c++ )
		{
			if ( codePoint.find ( c ) != std::string::npos )
			{
				friendlyCodePoint = false;
				break;
			}
		}

		if ( codePoint.find ( '\x7f' ) != std::string::npos )
		{
			friendlyCodePoint = false;
		}

		if ( friendlyCodePoint ) friendly.push_back ( codePoint );
	}

	return friendly;
}

std::string io::console::manip::recombine ( std::vector<std::string> strings )
{
	std::string string = "";
	for ( auto &str : strings )
	{
		string += str;
	}
	return string;
}