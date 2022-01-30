/**
 * @file codepoint.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Information and everything on Code Points
 * @version 1
 * @date 2022-01-30
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <list>
#include <memory>
#include <string>

namespace io::console::manip
{
	struct CodePointAttributes
	{
		std::int64_t width =
				-1; // the width of the character in columns. -2 means 0.5
		char visible = -1; // whether this character is visible
		char offense =
				-1; // whether this character moves the cursor in a manner
		            // different from width-columns in the direction we read.

		static CodePointAttributes const
				blend ( CodePointAttributes const &over,
		                CodePointAttributes const &norm )
		{
			CodePointAttributes result;
			result.width   = over.width == -1 ? norm.width : over.width;
			result.visible = over.visible == -1 ? norm.visible : over.visible;
			result.offense = over.offense == -1 ? norm.offense : over.offense;
			return result;
		}
	};

	class CodePoint;

	class CodePointManager
	{
		friend class CodePoint;

		struct CodePointRange;

		std::list<std::shared_ptr<CodePointRange>> ranges;

		// constructs the manager
		CodePointManager ( );

		std::shared_ptr<CodePointRange> getRangeFor ( CodePoint const &cp );
	};

	std::unique_ptr<CodePointManager> manager = nullptr;

	class CodePoint
	{
		std::string         bytes;
		CodePointAttributes attributes;
		public:
		CodePoint ( std::string const &        bytes,
		            CodePointAttributes const &attributes = { } )
				: bytes ( bytes ), attributes ( attributes )
		{
			if ( !manager.get ( ) )
			{
				manager = std::unique_ptr<CodePointManager> (
						new CodePointManager ( ) );
			}
		}

		CodePointAttributes const getAttributes ( ) const noexcept;

		std::string const &getBytes ( ) const noexcept { return bytes; }

		bool const isControl ( ) const noexcept
		{
			static std::string xxx =
					"\u0080"; // "xxx" is the actual replacement in the unicode
			                  // documentation
			static std::string nbsp = "\u00A0"; // non-breaking space
			if ( bytes.front ( ) < ' ' )
			{
				return true;
			}
			if ( bytes.at ( 0 ) == xxx.at ( 0 ) )
			{
				if ( bytes.at ( 1 ) >= xxx.at ( 1 )
				     && bytes.at ( 1 ) <= nbsp.at ( 1 ) )
				{
					return true;
				}
			}
			return false;
		}

		explicit operator std::string const & ( ) const noexcept
		{
			return bytes;
		}

		friend std::ostream &operator<< ( std::ostream &   os,
		                                  CodePoint const &cp ) noexcept
		{
			return os << ( std::string ) cp;
		}

		std::partial_ordering const
				operator<=> ( CodePoint const &cp ) const noexcept
		{
			std::size_t effectiveSize   = bytes.size ( );
			std::size_t cpEffectiveSize = cp.bytes.size ( );

			auto checkControl = [ & ] ( CodePoint const &with,
			                            std::size_t &    size ) {
				if ( with.isControl ( ) )
				{
					if ( with.getBytes ( ).at ( 0 ) < ' ' ) // C0 control
					{
						size = 1;
					} else
					{
						size = 2;
					}
				}
			};
			checkControl ( *this, effectiveSize );
			checkControl ( cp, cpEffectiveSize );
			if ( effectiveSize != cpEffectiveSize )
			{
				return bytes.size ( ) <=> cp.bytes.size ( );
			} else
			{
				auto index = [ & ] ( int i ) { return bytes.size ( ) - i; };

				auto comparison = [ & ] ( int i ) {
					return bytes.at ( index ( i ) )
					       != cp.bytes.at ( index ( i ) );
				};

				auto ordering = [ & ] ( int i ) {
					return bytes.at ( index ( i ) )
					       <=> cp.bytes.at ( index ( i ) );
				};

				for ( int i = effectiveSize; i > 0; i-- )
				{
					if ( comparison ( i ) )
					{
						return ordering ( i );
					}
				}
				return std::partial_ordering::equivalent;
			}
		}
	};

	struct CodePointManager::CodePointRange
			: public std::enable_shared_from_this<CodePointRange>
	{
		public:
		CodePoint           start, end;
		CodePointAttributes attributes;

		CodePointRange ( CodePoint const &          s,
		                 CodePoint const &          e,
		                 CodePointAttributes const &a ) noexcept
				: start ( s ), end ( e ), attributes ( a )
		{ }
	};

	CodePointAttributes const CodePoint::getAttributes ( ) const noexcept
	{
		return CodePointAttributes::blend (
				attributes, manager->getRangeFor ( *this )->attributes );
	}
} // namespace io::console::manip