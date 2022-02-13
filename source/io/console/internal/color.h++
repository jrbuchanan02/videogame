/**
 * @file color.h++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Console colors.
 * @details The console colors here are strictly for palette manipulation. In
 * this sense, we should really think of the colors more similarly to voices in
 * a sound-synthesizer chip. In this metaphore, we have eight voices of output,
 * we want to have n-voices of input and some combination of mixing them
 * together to create eight voices of output that change over time in some
 * manner. However, our first iteration should limit itself to direct color.
 * @version 1
 * @date 2022-02-13
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed
 * above.
 *
 */
#pragma once

#include <defines/constants.h++>
#include <defines/macros.h++>
#include <defines/types.h++>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>

namespace io::console::internal
{

    class Color
    {
    private:
        Color const &binaryOperation (
                Color const                                            &that,
                defines::BinaryFunction< defines::UnboundColor const &,
                                         defines::UnboundColor > const &fn )
                const noexcept
        {
            Color result;
            for ( std::uint8_t i = 0; i < 3; i++ )
            {
                result.set ( i, fn ( this->get ( i ), that.get ( i ) ) );
            }
            return result;
        }
    protected:
        defines::UnboundColor base [ 3 ];

        virtual defines::UnboundColor const &
                get ( std::uint8_t const &index ) const
        {
            if ( index > 2 )
            {
                throw std::range_error ( "Range out of bounds for a color!" );
            } else
            {
                return base [ index ];
            }
        }
    public:
        Color ( ) noexcept = default;
        Color ( defines::UnboundColor const &r,
                defines::UnboundColor const &g,
                defines::UnboundColor const &b ) noexcept :
                base { r, g, b }
        { }
        Color ( Color const & ) noexcept = default;
        Color ( Color && ) noexcept      = default;
        virtual ~Color ( )               = default;
        Color &operator= ( Color const & ) noexcept = default;
        Color &operator= ( Color && ) noexcept = default;

        defines::UnboundColor const &
                operator[] ( std::uint8_t const &index ) const
        {
            return get ( index );
        }

        virtual bool references ( Color *const &color ) const noexcept
        {
            return false;
        }

        void set ( std::uint8_t const &index, defines::UnboundColor const &to )
        {
            if ( index > 2 )
            {
                throw std::range_error ( "Range out of bounds for a color!" );
            } else
            {
                base [ index ] = to;
            }
        }

        std::uint8_t const &getBound ( std::uint8_t const &index ) const
        {
            defines::UnboundColor temp = ( *this ) [ index ];
            if ( temp > 0xff )
            {
                return 0xff;
            }
            if ( temp < 0x00 )
            {
                return 0x00;
            }
            return defines::BoundColor ( temp );
        }

        Color const &operator+ ( Color const &that ) const noexcept
        {
            return binaryOperation ( that,
                                     std::plus< defines::UnboundColor > ( ) );
        }

        Color const &operator- ( Color const &that ) const noexcept
        {
            return binaryOperation ( that,
                                     std::minus< defines::UnboundColor > ( ) );
        }

        Color const &operator* ( Color const &that ) const noexcept
        {
            return binaryOperation (
                    that,
                    std::multiplies< defines::UnboundColor > ( ) );
        }

        Color const &operator/ ( Color const &that ) const noexcept
        {
            return binaryOperation (
                    that,
                    std::divides< defines::UnboundColor > ( ) );
        }

        Color const &operator+ ( defines::UnboundColor const &c ) const noexcept
        {
            return *this + Color ( c, c, c );
        }

        Color const &operator- ( defines::UnboundColor const &c ) const noexcept
        {
            return *this - Color ( c, c, c );
        }

        Color const &operator* ( defines::UnboundColor const &c ) const noexcept
        {
            return *this * Color ( c, c, c );
        }

        Color const &operator/ ( defines::UnboundColor const &c ) const noexcept
        {
            return *this / Color ( c, c, c );
        }
    };

    class ColorManager
    {
        // default colors.
        static inline Color *defaults [ 8 ] = {
                new Color ( 0x00, 0x00, 0x00 ),
                new Color ( 0x7F, 0x00, 0x00 ),
                new Color ( 0x00, 0x7F, 0x00 ),
                new Color ( 0x7F, 0x7F, 0x00 ),
                new Color ( 0x00, 0x00, 0x7F ),
                new Color ( 0x7F, 0x00, 0x7F ),
                new Color ( 0x00, 0x7F, 0x7F ),
                new Color ( 0x7F, 0x7F, 0x7F ),
        };
        std::map< std::size_t, Color * > mutable colors;

        /**
         * @brief Get a color used for drawing. The indices 0 - 7 of the colors
         * map are always the colors currently being drawin to the screen. This
         * function gets one of those colors (checking bounds) and adds the
         * default color to the colors map if necessary.
         *@throws runtime_error if index > 7
         * @param index the color to get.
         * @return Color* the pointer to the appropriate color.
         */
        Color *getDrawn ( std::uint8_t const &index ) const
        {
            if ( index > 7 )
            {
                RUNTIME_ERROR ( "Index out of bounds for drawing color: ",
                                index )
            }
            if ( colors.contains ( index ) )
            {
                return colors.at ( index );
            } else
            {
                colors.emplace ( ( std::size_t ) index, defaults [ index ] );
                return colors.at ( index );
            }
        }
    public:
        /**
         * @brief The default cosntructor does nothing specifically to make
         * it a standard-layout type.
         *
         */
        constexpr ColorManager ( ) noexcept { }

        std::string command ( ) const
        {
            std::stringstream stream;
            auto generateCommand = [ & ] ( std::size_t index ) -> std::string {
                std::stringstream result;
                result << "\u001b]" << defines::paletteChangePrefix << std::hex;
                result << defines::SentColor ( index );
                result << defines::paletteChangeSpecif;
                result << defines::SentColor (
                        getDrawn ( index )->getBound ( 0 ) );
                for ( std::size_t i = 1; i < 3; i++ )
                {
                    result << defines::paletteChangeDelimt;
                    result << defines::SentColor (
                            getDrawn ( index )->getBound ( i ) );
                }
                result << "\u001b\\";
                return result.str ( );
            };
            for ( std::size_t i = 0; i < 8; i++ )
            {
                stream << generateCommand ( i );
            }
            return stream.str ( );
        }

        /**
         * @brief Gets the specified color, adding a default one to the internal
         * list of colors if necessary.
         * @note Colors 0-7 are drawn directly to the screen, those are the
         * color outputs.
         * @param at the index of said color
         * @return Color *& the color (now) at that index. You are free to
         * modify this color, but be aware that the default constructed color
         * may be invalid.
         */
        Color *&getColor ( std::size_t at ) const noexcept
        {
            if ( !colors.contains ( at ) )
            {
                colors.emplace ( at, nullptr );
            }
            return colors.at ( at );
        }
        /**
         * @brief Removes all colors not used in the calculation of the eight
         * on-screen colors.
         *
         */
        void prune ( ) const noexcept
        {
            std::list< std::size_t > remove { };
            for ( auto &pair : colors )
            {
                if ( pair.first < 8 )
                {
                    continue;
                } else
                {
                    bool referenced = false;
                    for ( std::size_t i = 0; i < 8; i++ )
                    {
                        referenced |=
                                getDrawn ( i )->references ( pair.second );
                    }

                    if ( !referenced )
                    {
                        remove.push_back ( pair.first );
                    }
                }
            }
            for ( auto &toRemove : remove )
            {
                // paranoid safeguard
                if ( toRemove < 8 )
                    continue;
                delete colors.at ( toRemove );
                colors.erase ( toRemove );
            }
        }
    };

    class IndirectColor : public Color
    {
        std::shared_ptr< Color >                        reference;
        defines::BinaryFunction< Color const *, Color > transform;
    protected:
        virtual defines::UnboundColor const &
                get ( std::uint8_t const &index ) const override
        {
            if ( !reference )
            {
                RUNTIME_ERROR ( "No color for any indirection!" );
            } else
            {
                return transform ( reference.get ( ), this ) [ index ];
            }
        }
    public:
        static inline defines::BinaryFunction< Color const *, Color > convert (
                defines::BinaryFunction< Color, Color > const &function )
        {
            return [ = ] ( Color const *lhs, Color const *rhs ) -> Color {
                return function ( *lhs, *rhs );
            };
        }

        static inline defines::BinaryFunction< Color const *, Color >
                defaultTransformation =
                        [] ( Color const *lhs, Color const *rhs ) {
                            return *lhs;
                        };

        IndirectColor ( ) noexcept = default;
        virtual ~IndirectColor ( ) = default;
        IndirectColor ( defines::UnboundColor const &r,
                        defines::UnboundColor const &g,
                        defines::UnboundColor const &b ) noexcept :
                Color ( r, g, b )
        { }
        IndirectColor ( Color *const &ref,
                        defines::BinaryFunction< Color const *, Color > const
                                &fun                   = defaultTransformation,
                        defines::UnboundColor const &r = 0,
                        defines::UnboundColor const &g = 0,
                        defines::UnboundColor const &b = 0 ) noexcept :
                Color ( r, g, b ),
                reference { ref }, transform { fun }
        { }

        bool const setReference ( Color const *const &ref ) noexcept
        {
            if ( ref->references ( this ) )
            {
                return false;
            } else
            {
                reference = std::make_shared< Color > ( ref );
            }
        }

        virtual bool references ( Color *const &color ) const noexcept override
        {
            if ( color == reference.get ( ) )
            {
                return true;
            } else if ( reference )
            {
                return reference->references ( color );
            } else
            {
                return false;
            }
        }
    };
} // namespace io::console::internal