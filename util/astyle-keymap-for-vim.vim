" Intro:
"   More human-friendly astyle key mapping.
"   It searches 2 places for .astylerc:
"       ./.astylerc     # Thus you can keep a .astylerc for each of your project.
"       ~/.astylerc     # You may have a global .astylerc config.
"
"   If no .astylerc found, It uses predefined default settings.
"
" Install:
"   cp <this-script> ~/.vim/ftplugin/c/astyle-format.vim
" This makes it apply key mapping for only *.c files
"
" Usage:
"   Open a `.c` file, type <Leader>a
"
" By default a <Leader> key is backslash `\`. Thus type `\a`
" You can change vim map leader key in .vimrc with:
"   let mapleader = ","
"
" Or you could want to change key mapping at the bottom.

fun! s:format_astyle() "{{{

    if ! executable('astyle')
        echom 'astyle not found'
        return
    endif

    let msg = ''

    " load .astylerc from cwd
    let rcfn = ".astylerc"
    if filereadable(rcfn)
        let msg = ' from ./astylerc'
        let lines = readfile(rcfn)
    elseif filereadable($HOME . '/.astylerc')
        let msg = ' from ~/astylerc'
        " let astyle to find a rc file
        let lines = []
    else
        let msg = ' from default setting'
        let lines = [
              \ "--mode=c",
              \ "--style=attach",
              \ "--unpad-paren",
              \ "--pad-oper",
              \ "--pad-paren-in",
              \ "--pad-header",
              \ "--convert-tabs",
              \ "--indent=spaces=4",
              \ "--add-brackets",
              \ "--align-pointer=name",
              \ ]
    endif

    call filter(lines, 'v:val !~ "\\v^#"')
    call filter(lines, 'v:val !~ "\\v^ *$"')

    " support long options without leading '--'
    let ls = []
    for line in lines
        if line !~ '\v^-'
            let line = '--' . line
        endif
        let ls += [line]
    endfor

    let arg = join(ls, " ")

    call view#win#SaveCursorPosition()
    exe 'silent %!astyle 2>/dev/null' arg
    call view#win#RestoreCursorPosition()

    echom 'astyle codes with setting' . msg
endfunction "}}}

nnoremap <leader>a :call <SID>format_astyle()<CR>
