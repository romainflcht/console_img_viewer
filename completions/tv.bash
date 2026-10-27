# bash completion for tv(1)

_tv()
{
    local cur prev
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    # Values of the options taking a fixed list of words.
    case "$prev" in
        -c|--colors)
            COMPREPLY=( $(compgen -W "auto truecolor 256 16 8 mono" -- "$cur") )
            return ;;
        --dither)
            COMPREPLY=( $(compgen -W "auto fs ordered none" -- "$cur") )
            return ;;
        --fit)
            COMPREPLY=( $(compgen -W "both width height none" -- "$cur") )
            return ;;
        -s|--size|-w|--width|-H|--height|-n|--plays|-f|--fps|--frame)
            return ;;
    esac

    # --opt=value form.
    case "$cur" in
        --colors=*)
            COMPREPLY=( $(compgen -P "--colors=" -W "auto truecolor 256 16 8 mono" -- "${cur#*=}") )
            return ;;
        --dither=*)
            COMPREPLY=( $(compgen -P "--dither=" -W "auto fs ordered none" -- "${cur#*=}") )
            return ;;
        --fit=*)
            COMPREPLY=( $(compgen -P "--fit=" -W "both width height none" -- "${cur#*=}") )
            return ;;
        -*)
            COMPREPLY=( $(compgen -W "-s --size= -w --width= -H --height= -u --upscale --fit= -C --center
                -c --colors= --dither= -a --ascii --utf8 -l --loop -n --plays= -L --file-loop -f --fps=
                --frame= -q --quiet -t --title -i --info -h --help -V --version" -- "$cur") )
            [[ ${COMPREPLY[0]} == *= ]] && compopt -o nospace
            return ;;
    esac

    # Image files (and directories to walk into).
    compopt -o filenames
    COMPREPLY=( $(compgen -f -X '!*.@(jpg|jpeg|JPG|JPEG|png|PNG|gif|GIF|webp|WEBP|bmp|BMP|pbm|pgm|ppm|pnm)' -- "$cur")
                $(compgen -d -- "$cur") )
}

complete -F _tv tv
