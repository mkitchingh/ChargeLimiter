#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
test_dir=$(mktemp -d)
trap 'rm -rf "$test_dir"' EXIT
"${CXX:-c++}" -std=c++11 -Wall -Wextra -Werror tests/charge_policy_test.cpp -o "$test_dir/policy"
"$test_dir/policy"
node --check www/js/app.js
node --test tests/app.test.cjs
python3 -m json.tool www/lang.json > /dev/null
