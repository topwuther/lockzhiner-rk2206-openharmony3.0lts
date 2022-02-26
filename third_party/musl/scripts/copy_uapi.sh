
while getopts "o:i:h" arg
do
    case "${arg}" in
        "o")
            OUT_DIR=${OPTARG}
            ;;
        "i")
            SOURCE_DIR=${OPTARG}
            ;;
        "h")
            echo "help"
            ;;
        ?)
            echo "unkonw argument"
            exit 1
            ;;
    esac
done

if [ ! -d "${OUT_DIR}" ];then
    mkdir -p ${OUT_DIR}
fi
cp -r ${SOURCE_DIR}/* ${OUT_DIR}
rm -rf ${OUT_DIR}/asm-arm


