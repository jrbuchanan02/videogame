/**
 * @file stringfunctions.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief String functions -- text wrapping, text justification
 * @version 1
 * @date 2022-01-26
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <string>
#include <vector>

namespace io::console::manip
{
	/**
	 * @brief Splits the string into a vector of its code-points.
	 * @param std::string the string to split
	 * @throw Throws std::runtime_error if there is an invalid / unknown code
	 * point.
	 * @return std::vector < std::string >
	 */
	std::vector<std::string> splitByCodePoint ( std::string );

	/**
	 * @brief Removes any "offending" characters from the specified string and
	 * splits the string by code point.
	 * @note An "offending" character is a:
	 *  - Whitespace character other than the space key
	 *  - Control Character with a command other than SetGraphicsRendition or
	 *  the windows / linux pallette control commands.
	 * @param std::string the string to process
	 * @return std::vector < std::string >
	 */
	std::vector<std::string> processString ( std::string );

	/**
	 * @brief Re-builds a string after being split by code point.
	 * @note this function exists because some of the further
	 * processing in the string functions benefits from working
	 * with an entire string at once as opposed to each individual
	 * character in the string
	 * @param std::vector<std::string> the split string to rebuild
	 * @return std::string
	 */
	std::string recombine ( std::vector<std::string> );

	/**
	 * @brief Wraps the string to the given number of columns, returning the
	 * string split into each individual line.
	 * @note Internally calls processString and then recombine.
	 * @note This algorithm can be classified as "best effort". If it cannot
	 * split a sequence of text which occupies the entire line, that text will
	 * wrap, but the function will do its best to keep that one word wrapping to
	 * the lines which contain it.
	 * @param std::string the string to wrap
	 * @param std::uint32_t the amount of columns
	 * @return std::vector < std::string > the wrapped string.
	 */
	std::vector<std::string> wrap ( std::string, std::uint32_t );

	/**
	 * @brief Attempts to center the string on a line of the specified amount
	 * of columns. This function assumes that we have already wrapped the
	 * string.
	 * @note The string may appear slightly visually different as center
	 * attempts to make lines with one column-difference in length appear more
	 * symmetrical. Center has a specific order it will try before giving up:
	 * 1. Insert a space right after a space ending a word and before the start
	 * of the next word.
	 * 2. Attempt to stretch a hyphen (- to --), equal-sign (= to ==), or
	 * similar character.
	 * 3. Attempt to find a known fullwidth character to replace an existing one
	 * with
	 * 4. Attempt to find two known halfwidth characters to replace one with.
	 * If all four passes fail, center gives up and leaves the text with an
	 * odd length.
	 *
	 * @return std::string
	 */
	std::string center ( std::string, std::uint32_t );
} // namespace io::console::manip