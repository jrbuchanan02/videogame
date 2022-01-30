/**
 * @file codepoint.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Implementation of the CodePointManager constructor
 * @version 1
 * @date 2022-01-30
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */

#include <io/console/manip/codepoint.h++>
#include <list>
#include <memory>
#include <string>

io::console::manip::CodePointManager::CodePointManager ( )
{
	// all of our ranges.
	auto range = [ & ] ( CodePoint s, CodePoint e, CodePointAttributes c )
			-> std::shared_ptr<CodePointRange> {
		return ( CodePointRange ( s, e, c ) ).shared_from_this ( );
	};
	CodePointAttributes enus = {
			1, 1, 0 }; // en-US, characters that I normally see
	CodePointAttributes ctrl = { 0, 0, 1 }; // control characters
	CodePointAttributes mark = { 0, 1, 0 }; // combining marks
	// CodePointAttributes wide = { 2, 1, 0 }; // wide characters (eg, CJK
	// characters and emoji)

	// C0 Control Characters. By Default, these are
	// invisible, offensive, and 0 width.
	ranges.push_back (
			range ( CodePoint ( "\0" ), CodePoint ( "\u001f" ), ctrl ) );
	// ASCII (excl, C0 and DEL). By default, these
	// are visible, friendly, and 1 width.
	// Nightmare in words?
	// All non-whitespace, non-punctuation characters grouped below are
	// valid variable-names.
	ranges.push_back ( range ( CodePoint ( " " ), CodePoint ( "~" ), enus ) );
	// Latin Extended A. By Default, these have the
	// exact same attributes as ASCII
	ranges.push_back ( range ( CodePoint ( "Ā" ), CodePoint ( "ſ" ), enus ) );
	// Latin Extended B. Literally Latin Extended A but
	// more chracters.
	ranges.push_back ( range ( CodePoint ( "ƀ" ), CodePoint ( "ɏ" ), enus ) );
	// IPA Extensions. Believe it or not, but ʣ is only one
	// column wide.
	ranges.push_back ( range ( CodePoint ( "ɐ" ), CodePoint ( "ʯ" ), enus ) );
	// Spacing Modifier Letters. Despite the name, it seems that
	// these characters are all single width.
	ranges.push_back ( range ( CodePoint ( "ʰ" ), CodePoint ( "˿" ), enus ) );
	// for obvious reasons, we can't just place the combining marks
	// all willy nilly around our code. What if the compiler thinks we mean
	// to add the mark to the quotations!
	ranges.push_back (
			range ( CodePoint ( "\u0300" ), CodePoint ( "\u036F" ), mark ) );
	// Greek and Coptic have multiple sections where the Unicode standard
	// keeps no visible character assigned. Their code charts do not indicate
	// what these characters are, so they will go unassigned.
	{
		CodePoint starts [ 6 ] = {
				CodePoint ( "Ͱ" ),
				CodePoint ( "ͺ" ),
				CodePoint ( "΄" ),
				CodePoint ( "Ό" ),
				CodePoint ( "Ύ" ),
				CodePoint ( "Σ" ),
		};
		CodePoint ends [ 6 ] = {
				CodePoint ( "ͷ" ),
				CodePoint ( "Ϳ" ),
				CodePoint ( "Ί" ),
				CodePoint ( "Ό" ),
				CodePoint ( "Ρ" ),
				CodePoint ( "Ͽ" ),
		};

		for ( int i = 0; i < 6; i++ )
		{
			ranges.push_back ( range ( starts [ i ], ends [ i ], enus ) );
		}
	}
	// Cyrillic has a section where there are accents added.
	{
		// not accent -> 0, 2
		// accent -> 1
		CodePoint starts [ 3 ] = {
				CodePoint ( "Ѐ" ),
				CodePoint ( "\u0483" ),
				CodePoint ( "Ҋ" ),
		};
		CodePoint ends [ 3 ] = {
				CodePoint ( "҂" ),
				CodePoint ( "\u0489" ),
				CodePoint ( "ӿ" ),
		};
		ranges.push_back ( range ( starts [ 0 ], ends [ 0 ], enus ) );
		ranges.push_back ( range ( starts [ 1 ], ends [ 1 ], mark ) );
		ranges.push_back ( range ( starts [ 2 ], ends [ 2 ], enus ) );
	}
	// Cyrillic supplement
	ranges.push_back ( range ( CodePoint ( "Ԁ" ), CodePoint ( "ԯ" ), enus ) );
	// Armenian has multiple sections where no characters are defined.
	{
		CodePoint starts [ 3 ] = {
				CodePoint ( "Ա" ),
				CodePoint ( "ՙ" ),
				CodePoint ( "֍" ),
		};
		CodePoint ends [ 3 ] = {
				CodePoint ( "Ֆ" ),
				CodePoint ( "֊" ),
				CodePoint ( "֏" ),
		};
		for ( int i = 0; i < 3; i++ )
		{
			ranges.push_back ( range ( starts [ i ], ends [ i ], enus ) );
		}
	}
	// Hebrew has four sections
	{
		CodePoint starts [ 4 ] = {
				CodePoint ( "\u0591" ),
				CodePoint ( "׆" ),
				CodePoint ( "\u05C7" ),
				CodePoint ( "א" ),
		};
		CodePoint ends [ 4 ] = {
				CodePoint ( "\u05C5" ),
				CodePoint ( "׆" ),
				CodePoint ( "\u05C7" ),
				CodePoint ( "״" ),
		};
		for ( int i = 0; i < 4; i++ )
		{
			ranges.push_back ( range (
					starts [ i ], ends [ i ], ( i & 1 ? enus : mark ) ) );
		}
	}
	// Arabic has multiple sections. This is due to the frequent switching
	// between combining marks and non-combining marks in the code-page.
	{
		CodePoint starts [ 16 ] = {
				CodePoint ( "\u0600" ),
				CodePoint ( "؆" ),
				CodePoint ( "\u0610" ),
				CodePoint ( "؛" ),
				CodePoint ( "\u061E" ),
				CodePoint ( "\u061D" ),
				CodePoint ( "\u064C" ),
				CodePoint ( "٠" ),
				CodePoint ( "\u0670" ),
				CodePoint ( "ٱ" ),
				CodePoint ( "\u06D6" ),
				CodePoint ( "۞" ),
				CodePoint ( "\u06DF" ),
				CodePoint ( "۩" ),
				CodePoint ( "\u06EA" ),
				CodePoint ( "ۮ" ),
		};
	}
}

std::shared_ptr<io::console::manip::CodePointManager::CodePointRange>
		io::console::manip::CodePointManager::getRangeFor (
				CodePoint const &cp )
{
	for ( auto &range : ranges )
	{
		if ( cp >= range->start && cp <= range->end )
		{
			return range;
		}
	}
	throw std::runtime_error ( "Unknown codepoint: " + ( std::string ) cp );
}