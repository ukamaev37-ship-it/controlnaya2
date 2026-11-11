#!/usr/bin/env bash
set -euo pipefail
project_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
cmake -S "$project_dir" -B "$project_dir/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$project_dir/build" -j2
exec env -u GTK_PATH -u GTK_EXE_PREFIX -u GTK_IM_MODULE_FILE -u GIO_MODULE_DIR -u GTK_MODULES "$project_dir/build/control_work_02" "$@"
