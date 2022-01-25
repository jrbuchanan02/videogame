/**
 * @file console.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Definition for Console
 * @version 1
 * @date 2022-01-22
 * 
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 * 
 */
#include "console.h++"

#include "syncbuf.h++"
#include "util/memory/rememberer.h++"

struct game::io::console::Console::impl_s {
    static util::memory::Rememberer < ssbuf > globalBuffers;

    std::shared_ptr < ssbuf > buffers [ 2 ]; // input and output

    
};