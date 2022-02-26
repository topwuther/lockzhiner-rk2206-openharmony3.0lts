OUT_DIR="$1"

function main()
{
    echo "${OUT_DIR}"
    cp -f lwipopts.h ${OUT_DIR}
}

main "$@"
