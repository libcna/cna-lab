#!/bin/sh
# Launch a copied Linux package without depending on the original build directory.
set -eu
package_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
export LD_LIBRARY_PATH="$package_dir:$package_dir/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
cd "$package_dir"
exec "$package_dir/cna_backrooms" "$@"
