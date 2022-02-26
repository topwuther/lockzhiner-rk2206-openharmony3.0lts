SOURCE_FILE="$1"
DEST_FILE="$2"

function main()
{
    echo "cp -f ${SOURCE_FILE} ${DEST_FILE}"
    cp -f ${SOURCE_FILE} ${DEST_FILE}
}

main "$@"
