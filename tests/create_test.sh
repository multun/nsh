#!/bin/sh

base_tests_path="$(dirname "$(realpath "$0")")"

create_tests() {
    for test_data in "$@"; do
        test_desc="${test_data#*:}"
        test_desc="${test_desc%%:*}"
        test_content="${test_data#*:}"
        test_content="${test_content#*:}"
        test_cat="${test_data%%/*}"
        test_name="${test_data%%:*}"
        test_name="${test_name#*/}"

        printf 'cat:\t%s\n' "${test_cat}"
        printf 'name:\t%s\n' "${test_name}"
        printf 'desc:\t%s\n' "${test_desc}"
        printf 'content:\t%s\n' "${test_content}"

        test_out="$(echo "${test_content}" | sh 2>/dev/null)"
        test_err="$(echo "${test_content}" | sh 2>&1 >/dev/null)"
        echo "${test_content}" | sh >/dev/null 2>/dev/null
        test_retcode=$?

        echo "test_out: $test_out"
        echo "test_err: $test_err"

        test_path="${base_tests_path}/itests/${test_cat}/${test_name}.json"
        echo "${test_path}"
        jq -n \
           --arg desc "${test_desc}" \
           --arg stdin "${test_content}" \
           --arg stdout "${test_out}" \
           --arg stderr "${test_err}" \
           --argjson retcode "${test_retcode}" \
           '{desc: $desc, stdin: $stdin, stdout: $stdout, stderr: $stderr, retcode: $retcode}' \
           | tee "${test_path}"

    done
}

if (($# > 0)); then
    create_tests "$@"
else
    while IFS='' read -r test_data; do
          create_tests "${test_data}"
    done
fi
