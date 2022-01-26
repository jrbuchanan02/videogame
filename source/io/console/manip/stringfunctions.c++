/**
 * @file stringfunctions.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief The code for the string functions
 * @version 1
 * @date 2022-01-25
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 *
 */
#include <io/console/manip/stringfunctions.h++>

#include <functional>
#include <string>

enum class C0Codes
{
    nul = 0 ,
    soh ,
    stx ,
    etx ,
    eot ,
    enq ,
    ack ,
    bel ,
    bs ,
    ht ,
    lf ,
    vt ,
    ff ,
    cr ,
    so ,
    si ,
    dle ,
    dc1 ,
    dc2 ,
    dc3 ,
    dc4 ,
    nak ,
    syn ,
    etb ,
    can ,
    em ,
    sub ,
    esc ,
    fs ,
    gs ,
    rs ,
    us ,
    del = 0x7f ,
};
enum class C1Codes
{
    bph = 'B',
    nbh,
    ind,
    nel,
    ssa,
    esa,
    hts,
    htj,
    vts,
    pld,
    plu,
    ri,
    ss2,
    ss3,
    dcs,
    pu1,
    pu2,
    sts,
    cch,
    mw,
    spa,
    epa,
    sos,
    sgc,
    sci,
    csi,
    st,
    osc,
    pm,
    apc,
};

static_assert( ( char ) C0Codes::esc == '\x1b' , "Invalid C0 Codes!" );
static_assert( ( char ) C1Codes::csi == '[', "Invalid C1 Codes!");

#define C0Code(X)                                                               \
io::console::manip::ANSISequence const io::console::manip::X ( ) noexcept       \
{                                                                               \
    ANSISequence result = "";                                                   \
    result += (char)C0Codes::X;                                                 \
    return result;                                                              \
}

C0Code ( nul )
C0Code ( soh )
C0Code ( stx )
C0Code ( etx )
C0Code ( eot )
C0Code ( enq )
C0Code ( ack )
C0Code ( bel )
C0Code ( bs )
C0Code ( ht )
C0Code ( lf )
C0Code ( vt )
C0Code ( ff )
C0Code ( cr )
C0Code ( so )
C0Code ( si )
C0Code ( dle )
C0Code ( dc1 )
C0Code ( dc2 )
C0Code ( dc3 )
C0Code ( dc4 )
C0Code ( nak )
C0Code ( syn )
C0Code ( etb )
C0Code ( can )
C0Code ( em )
C0Code ( sub )
C0Code ( esc )
C0Code ( fs )
C0Code ( gs )
C0Code ( rs )
C0Code ( us )
C0Code ( del )

#undef C0Code
#define C1Code(X)                                                               \
io::console::manip::ANSISequence const io::console::manip::X ( ) noexcept       \
{                                                                               \
    ANSISequence result = esc();                                                \
    result += (char)C1Codes::X;                                                 \
    return result;                                                              \
}

C1Code ( bph )
C1Code ( nbh )
C1Code ( ind )
C1Code ( nel )
C1Code ( ssa )
C1Code ( esa )
C1Code ( hts )
C1Code ( htj )
C1Code ( vts )
C1Code ( pld )
C1Code ( plu )
C1Code ( ri )
C1Code ( ss2 )
C1Code ( ss3 )
C1Code ( dcs )
C1Code ( pu1 )
C1Code ( pu2 )
C1Code ( sts )
C1Code ( cch )
C1Code ( mw )
C1Code ( spa )
C1Code ( epa )
C1Code ( sos )
C1Code ( sgc )
C1Code ( sci )
C1Code ( csi )
C1Code ( st )
C1Code ( osc )
C1Code ( pm )
C1Code ( apc )