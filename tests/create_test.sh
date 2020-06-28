#!/usr/bin/env bash

usage() {
    echo 'cat/name:desc:content_file'
}

out_tmp="$(mktemp)"
err_tmp="$(mktemp)"

clean_files () {
    rm -f -- "${out_tmp}"
    rm -f -- "${err_tmp}"
}

trap clean_files EXIT

base_tests_path="$(dirname "$(realpath "$0")")"

create_tests() {
    for test_data in "$@"; do
        test_desc="${test_data#*:}"
        test_desc="${test_desc%%:*}"
        test_content_file="${test_data#*:}"
        test_content_file="${test_content_file#*:}"
        test_cat="${test_data%%/*}"
        test_name="${test_data%%:*}"
        test_name="${test_name#*/}"

        printf 'cat:\t%s\n' "${test_cat}"
        printf 'name:\t%s\n' "${test_name}"
        printf 'desc:\t%s\n' "${test_desc}"
        printf 'content file:\t%s\n' "${test_content_file}"

        cat "${test_content_file}" | sh 1>"${out_tmp}" 2>/dev/null
        cat "${test_content_file}" | sh 2>"${err_tmp}" 1>/dev/null
        cat "${test_content_file}" | sh >/dev/null 2>/dev/null
        test_retcode=$?

        echo "test_out:"
        cat "${out_tmp}"

        echo "test_err:"
        cat "${err_tmp}"

        test_path="${base_tests_path}/itests/${test_cat}/${test_name}.json"
        mkdir -p -- "$(dirname "${test_path}")"
        echo "${test_path}"
        jq -n \
           --arg desc "${test_desc}" \
           --rawfile stdin "${test_content_file}" \
           --rawfile stdout "${out_tmp}" \
           --rawfile stderr "${err_tmp}" \
           --argjson retcode "${test_retcode}" \
           '{desc: $desc, stdin: $stdin, stdout: $stdout, stderr: $stderr, retcode: $retcode}' \
           | tee "${test_path}"

    done
}

if [ $# != 0 ]; then
    create_tests "$@"
else
    usage
    while IFS='' read -e -r test_data; do
          create_tests "${test_data}"
    done
fi
