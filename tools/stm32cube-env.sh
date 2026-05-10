#!/usr/bin/env bash

set -euo pipefail

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
    echo "Source this script instead of executing it: source tools/stm32cube-env.sh" >&2
    exit 1
fi

to_unix_path() {
    local value="$1"

    if [[ -z "$value" ]]; then
        printf '%s' "$value"
        return
    fi

    if command -v cygpath >/dev/null 2>&1; then
        cygpath -u "$value"
        return
    fi

    printf '%s' "$value" | sed -E 's#^([A-Za-z]):#/\L\1#; s#\\#/#g'
}

to_unix_path_list() {
    local list="$1"
    local old_ifs="$IFS"
    local path
    local result=()

    IFS=';'
    for path in $list; do
        [[ -n "$path" ]] || continue
        result+=("$(to_unix_path "$path")")
    done
    IFS="$old_ifs"

    local joined=""
    local item
    for item in "${result[@]}"; do
        if [[ -z "$joined" ]]; then
            joined="$item"
        else
            joined="${joined}:$item"
        fi
    done

    printf '%s' "$joined"
}

extract_extension_version() {
    local prefix="$1"
    local name="$2"

    if [[ "$name" =~ ^${prefix}([0-9]+(\.[0-9]+)+) ]]; then
        printf '%s' "${BASH_REMATCH[1]}"
        return 0
    fi

    printf '0.0.0'
}

find_best_extension_dir() {
    local prefix="$1"
    local user_profile="${USERPROFILE:-${HOME}}"
    local extensions_root
    local matches=()
    local extension
    local candidates=()
    local version
    local chosen_line

    extensions_root="$(to_unix_path "$user_profile")/.vscode/extensions"

    for extension in "$extensions_root"/${prefix}*; do
        [[ -d "$extension" ]] || continue
        matches+=("$extension")
    done

    if [[ "${#matches[@]}" -eq 0 ]]; then
        echo "No extension matched '${prefix}*' under '$extensions_root'." >&2
        return 1
    fi

    for extension in "${matches[@]}"; do
        version="$(extract_extension_version "$prefix" "$(basename "$extension")")"
        candidates+=("${version}"$'\t'"$extension")
    done

    chosen_line="$(printf '%s\n' "${candidates[@]}" | sort -t $'\t' -k1,1Vr -k2,2r | head -n 1)"
    if [[ "${#matches[@]}" -gt 1 ]]; then
        echo "Selecting extension '${chosen_line#*$'\t'}' from ${#matches[@]} candidates for '${prefix}*'." >&2
    fi

    printf '%s' "${chosen_line#*$'\t'}"
}

prepend_path_entry() {
    local entry="$1"

    [[ -n "$entry" ]] || return
    case ":${PATH}:" in
        *":${entry}:"*) ;;
        *) PATH="${entry}${PATH:+:${PATH}}" ;;
    esac
}

apply_resolve_output() {
    local output="$1"
    local line
    local trimmed
    local key
    local value

    while IFS= read -r line; do
        trimmed="${line#"${line%%[![:space:]]*}"}"
        if [[ "$trimmed" != *=\'*\' ]]; then
            continue
        fi

        key="${trimmed%%=*}"
        value="${trimmed#*=}"
        value="${value#\'}"
        value="${value%\'}"

        if [[ "$key" == "PATH" ]]; then
            local converted_path
            converted_path="$(to_unix_path_list "$value")"
            if [[ -n "$converted_path" ]]; then
                PATH="${converted_path}${PATH:+:${PATH}}"
            fi
            continue
        fi

        export "$key=$(to_unix_path "$value")"
    done <<< "$output"
}

get_resolved_command_dir() {
    local output="$1"
    local line
    local trimmed
    local command_path

    while IFS= read -r line; do
        trimmed="${line#"${line%%[![:space:]]*}"}"
        if [[ "$trimmed" == Command:\ \'*\' ]]; then
            command_path="${trimmed#Command: \'}"
            command_path="${command_path%\'}"
            to_unix_path "${command_path%/*}"
            return 0
        fi
    done <<< "$output"

    return 1
}

core_dir="$(find_best_extension_dir 'stmicroelectronics.stm32cube-ide-core-')"
build_dir="$(find_best_extension_dir 'stmicroelectronics.stm32cube-ide-build-cmake-')"

cube_exe="${core_dir}/resources/binaries/win32/x86_64/cube.exe"
cube_cmake_exe="${build_dir}/resources/cube-cmake/win32/x86_64/cube-cmake.exe"
cube_dir="${cube_exe%/*}"
cube_cmake_dir="${cube_cmake_exe%/*}"

if [[ ! -f "$cube_exe" ]]; then
    echo "Missing cube executable at '$cube_exe'." >&2
    return 1
fi

if [[ ! -f "$cube_cmake_exe" ]]; then
    echo "Missing cube-cmake executable at '$cube_cmake_exe'." >&2
    return 1
fi

export CUBE_EXE="$cube_exe"
export CUBE_CMAKE_EXE="$cube_cmake_exe"

cube_bundle_line="$("$cube_exe" --get-current-value cube_bundle_path | tr -d '\r')"
cmsis_pack_line="$("$cube_exe" --get-current-value cmsis_pack_root | tr -d '\r')"

export CUBE_BUNDLE_PATH="$(to_unix_path "${cube_bundle_line#*=}")"
export CMSIS_PACK_ROOT="$(to_unix_path "${cmsis_pack_line#*=}")"

resolved_output="$("$cube_exe" --resolve cmake | tr -d '\r')"
apply_resolve_output "$resolved_output"
clangd_resolved_output="$("$cube_exe" --resolve starm-clangd | tr -d '\r')"
apply_resolve_output "$clangd_resolved_output"
prepend_path_entry "$cube_dir"
prepend_path_entry "$cube_cmake_dir"
prepend_path_entry "$(get_resolved_command_dir "$clangd_resolved_output")"

export PATH

echo "STM32Cube environment loaded."
echo "  CUBE_EXE=$CUBE_EXE"
echo "  CUBE_CMAKE_EXE=$CUBE_CMAKE_EXE"
echo "  CUBE_BUNDLE_PATH=$CUBE_BUNDLE_PATH"
echo "  CMSIS_PACK_ROOT=$CMSIS_PACK_ROOT"
if [[ -n "${GCC_TOOLCHAIN_ROOT:-}" ]]; then
    echo "  GCC_TOOLCHAIN_ROOT=$GCC_TOOLCHAIN_ROOT"
fi
