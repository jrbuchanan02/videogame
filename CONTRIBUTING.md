# Contributing to Videogame.
We welcome your contributions and ideas to videogame. Just remember:

1. Make sure your code is readable.
2. Follow the formatting guidelines. Whitespacing and stuff is documented at the repository root. Running `make format` will format all the code appropriately.
 However, in addition to those formatting rules, follow these as well:
 - function names, variable names are `camelCase`
 - namespace names are `snake_case` and the namespace matches the directory that a file is in, so a file in ./source/foo/bar would have the namespace `foo::bar`
 - all type names are `CamelCase` except for when a class uses pImpl, in that case, the implementation is always named `impl_s`.
 - If possible, write any function, algorithm, etc. only *once*. Use macros, templates, function calls, polymorphic-types, and any other tricks you have to 
  make the source code more consise while maximizing readabliity.
 - Whenever there is operating system specific code, combine it as much as possible, but ensure that any operating system's specific code will result in the same behavior. That
  is, there should never be an "os specific" feature.
 - If it exists in one of the external dependencies or in the standard library, don't implement yourself.


For contributing to the text data, follow these rules:
1. Don't touch the unicode character database. 
2. All text needs to be in a .json file. The following fields are **required**:
  - a key "language" which contains a unique identifier for the language, for example, "en-US" for US english.
  - a key "transliterations", an array of strings containing the values "NOT", "YES", and / or "ALT". These values
  may repeat or be in any order, but all three have to exist.
    + the values here correspond to different transliterations of text. NOT means no transliteration and the text in 
    another alphabet is copied verbatim. YES means to follow general translation rules, for example, the japanese characters
    ます would always be transliterated "masu" even though sometimes it's pronounced "mas". ALT means to follow the 
    exact pronounciation of the language transliterated into.
  - A key "text" which contains an array of objects that **must be the same length as the array in "transliterations"**.
   Each object contains keys corresponding to the text-id's videogame uses to grab the string (so, the key "title" is the
   title that Videogame uses) and the value is a string (or array of strings) corresponding to the value of the text. 
    + since this line must be 80 columns or less, use an array of strings to have the strings span across multiple lines, 
      beware that Videogame will concatinate the strings without inserting anything between them, so the field:
      ```json
      "long-text": [
        "hello,",
        "world!"
      ],
      ```
      is read as "hello,world!" and not "hello, world!"
    + The items in "text" correspond to the same indices in "transliterations", so, if the first item in transliterations is NOT, then
    the first item in text is considered to be NOT-transliterated. 
