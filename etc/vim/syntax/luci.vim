" Vim syntax file
" Language:     Luci
" Maintainer:   Joe Naegele <joseph.naegele@gmail.com>
" Last Change:  2013-09-16

" Quit when a (custom) syntax file was already loaded
if exists("b:current_syntax")
  finish
endif

syn keyword luciStatement       pass break continue return
"syn keyword luciStatement      global
syn keyword luciStatement       def nextgroup=luciFunction skipwhite
syn keyword luciConditional     if else
syn keyword luciRepeat          for while do
syn keyword luciOperator        in
"syn keyword luciOperator       and in is not or
" syn keyword luciException     except finally raise try
" syn keyword luciInclude       from import

syn keyword luciFunction        def

syn keyword luciTodo            FIXME NOTE NOTES TODO XXX contained
syn match   luciComment "#.*$" contains=luciTodo,@Spell
syn region  luciComment start="/\*" end="\*/" contains=luciTodo,@Spell

syn match   luciCurlyError "}"
syn region  luciBlock start="{" end="}" contains=ALLBUT,luciCurlyError fold

" Triple-quoted strings can contain doctests.
syn region  luciString
      \ start=+[uU]\=\z(["]\)+ end="\z1" skip="\\\\\|\\\z1"
      \ contains=luciEscape,@Spell
syn region  luciRawString
      \ start=+[uU]\=[rR]\z(["]\)+ end="\z1" skip="\\\\\|\\\z1"
      \ contains=@Spell

syn match   luciEscape  +\\[abfnrtv'"\\]+ contained
syn match   luciEscape  "\\\o\{1,3}" contained
syn match   luciEscape  "\\x\x\{2}" contained
syn match   luciEscape  "\%(\\u\x\{4}\|\\U\x\{8}\)" contained
" Python allows case-insensitive Unicode IDs: http://www.unicode.org/charts/
syn match   luciEscape  "\\N{\a\+\%(\s\a\+\)*}" contained
syn match   luciEscape  "\\$"

" It is very important to understand all details before changing the
" regular expressions below or their order.
" The word boundaries are *not* the floating-point number boundaries
" because of a possible leading or trailing decimal point.
" The expressions below ensure that all valid number literals are
" highlighted, and invalid number literals are not.  For example,
"
" - a decimal point in '4.' at the end of a line is highlighted,
" - a second dot in 1.0.0 is not highlighted,
" - 08 is not highlighted,
" - 08e0 or 08j are highlighted,
"
" and so on, as specified in the 'Python Language Reference'.
" http://docs.luci.org/reference/lexical_analysis.html#numeric-literals
" numbers (including longs and complex)
syn match   luciNumber        "\<0[oO]\=\o\+[Ll]\=\>"
syn match   luciNumber        "\<0[xX]\x\+[Ll]\=\>"
syn match   luciNumber        "\<0[bB][01]\+[Ll]\=\>"
syn match   luciNumber        "\<\%([1-9]\d*\|0\)[Ll]\=\>"
syn match   luciNumber        "\<\d\+[jJ]\>"
syn match   luciNumber        "\<\d\+[eE][+-]\=\d\+[jJ]\=\>"
syn match   luciNumber
      \ "\<\d\+\.\%([eE][+-]\=\d\+\)\=[jJ]\=\%(\W\|$\)\@="
syn match   luciNumber
      \ "\%(^\|\W\)\@<=\d*\.\d\+\%([eE][+-]\=\d\+\)\=[jJ]\=\>"

" Group the built-ins in the order in the 'Python Library Reference' for
" easier comparison.
" Python built-in functions are in alphabetical order.

" contants
syn keyword luciConstant    true false nil
syn keyword luciConstant    stdout stderr stdin

" built-in functions
syn keyword luciBuiltin     help
syn keyword luciBuiltin     dir
syn keyword luciBuiltin     exit
syn keyword luciBuiltin     print
syn keyword luciBuiltin     type
syn keyword luciBuiltin     assert
syn keyword luciBuiltin     str int float hex
syn keyword luciBuiltin     open close read write
syn keyword luciBuiltin     input
syn keyword luciBuiltin     readline readlines
syn keyword luciBuiltin     range
syn keyword luciBuiltin     len sum max min
syn keyword luciBuiltin     copy
syn keyword luciBuiltin     contains
"syn keyword luciBuiltin     abs
"syn keyword luciBuiltin     round
"syn keyword luciBuiltin     divmod
"syn keyword luciBuiltin     del
"syn keyword luciBuiltin     hash
"syn keyword luciBuiltin     chr ord
"syn keyword luciBuiltin     reversed sorted
"syn keyword luciBuiltin     zip

"syn keyword luciExceptions    Exception

" Sync at the beginning of class, function, or method definition.
syn sync match luciSync grouphere NONE "^\s*\%(def\)\s\+\h\w*\s*("

" The default highlight links.  Can be overridden later.
hi def link luciStatement       Statement
hi def link luciConditional     Conditional
hi def link luciRepeat          Repeat
hi def link luciOperator        Operator
hi def link luciException       Exception
hi def link luciInclude         Include
hi def link luciFunction        Function
hi def link luciComment         Comment
hi def link luciTodo            Todo
hi def link luciString          String
hi def link luciRawString       String
hi def link luciEscape          Special
hi def link luciNumber          Number
hi def link luciBuiltin         Define
hi def link luciConstant        Special
hi def link luciExceptions      Structure
hi def link luciCurlyError      Error

let b:current_syntax = "luci"

" vim:set sw=4 sts=4 et:
