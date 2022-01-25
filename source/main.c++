/**
 * @file main.c++
 * @author Joshua Buchanan (joshuarobertbuchanan@gmail.com)
 * @brief Entry point for our program
 * @version 1
 * @date 2022-01-22
 *
 * @copyright Copyright (C) 2022. Intellectual property of the author(s) listed above.
 *
 */

#include "info.h++"

 /**
  * @brief The entry point for our program
  *
  * @param argc the amount of arguments passed. Always >=1
  * @param argv the values of those arguments
  * @return 0 success
  * @return int if not 0, then unsuccessful.
  */
int main ( int const argc , char const *const *const argv )
{
    outputCommandline ( std::cout , argc , argv );
}