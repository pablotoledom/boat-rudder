#!/bin/bash
# Boat Rudder - system file manifest (develop_docs/plans/update-system-plan.md).
#
# The manifest lists every file a Boat Rudder version installs - the system's
# own files, the ones an update may replace - each with its SHA-256, in
# sha256sum(1) format with paths relative to the install directory, so
# `sha256sum -c` checks an install against it. Everything else in an install -
# html/content/, configs/, ssl/, data/, the uploaded favicons and fonts - is
# the site's own and never listed, so nothing that works from the manifest
# can touch it.
#
#   manifest.sh create [<source_root>]
#       Prints the manifest of a source tree. Run after compile_prod.sh: the
#       binary is <source_root>/bin/boat-rudder.
#
#   manifest.sh adopt <install_dir> [<source_root>]
#       Gives an install made before manifests existed its state directory:
#       writes <install_dir>/.boat-rudder/VERSION and manifest.sha256 from the
#       source of the version it runs, and reports installed files that differ
#       from that source (edited on the host), are missing, or aren't part of it
#       (added on the host, or left over from an older version). The installed
#       binary is listed with its own hash - builds aren't byte-reproducible,
#       so it can't be compared with a fresh one.
#
#   manifest.sh verify <install_dir>
#       Compares an install with its manifest: modified and missing files.
#       Exits 1 if there are any.
#
# <source_root> defaults to the checkout this script belongs to.
set -euo pipefail

STATE_DIR=".boat-rudder"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEFAULT_SOURCE="$(cd "$SCRIPT_DIR/.." && pwd)"

die() { echo "manifest.sh: $*" >&2; exit 2; }

usage() {
    sed -n '/^#   manifest.sh create/,/^# <source_root> defaults/p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//' >&2
    exit 2
}

# The system's files of <source_root>, one "<install path>\t<source path>" per
# line, sorted by install path. Directories are walked whole: a file the
# version ships under html/templates, html/themes or html/assets is a system
# file wherever it sits.
system_files() {
    local src="$1"
    local bad
    bad="$(cd "$src" && find html/templates html/themes html/assets -type f -name '*[[:cntrl:]]*' -print -quit)"
    [ -z "$bad" ] || die "control character in a file name under $src: refusing to list it"

    {
        printf 'boat-rudder\tbin/boat-rudder\n'
        local f
        for f in scripts/image-optimizer.sh scripts/manifest.sh scripts/boat-rudder-update \
                 html/favicon.ico html/favicon-nb.ico; do
            [ -f "$src/$f" ] && printf '%s\t%s\n' "$f" "$f"
        done
        (cd "$src" && find html/templates html/themes html/assets -type f -print) |
            while IFS= read -r f; do printf '%s\t%s\n' "$f" "$f"; done
    } | LC_ALL=C sort -t "$(printf '\t')" -k1,1
}

sha() { sha256sum -- "$1" | cut -d' ' -f1; }

cmd_create() {
    local src="${1:-$DEFAULT_SOURCE}"
    [ -f "$src/VERSION" ] || die "$src has no VERSION file - not a Boat Rudder source tree"
    [ -x "$src/bin/boat-rudder" ] || die "$src/bin/boat-rudder not found - build first (compile_prod.sh)"

    local install_path source_path
    while IFS=$'\t' read -r install_path source_path; do
        printf '%s  %s\n' "$(sha "$src/$source_path")" "$install_path"
    done < <(system_files "$src")
}

cmd_adopt() {
    local dest="${1:-}"
    local src="${2:-$DEFAULT_SOURCE}"
    [ -n "$dest" ] || usage
    [ -f "$src/VERSION" ] || die "$src has no VERSION file - not a Boat Rudder source tree"
    [ -x "$dest/boat-rudder" ] && [ -d "$dest/html" ] || die "$dest doesn't look like a Boat Rudder install"
    [ -w "$dest" ] || die "can't write to $dest (run with sudo)"
    if [ -f "$dest/$STATE_DIR/manifest.sha256" ]; then
        die "$dest already has a manifest - use 'verify', or remove $dest/$STATE_DIR to adopt again"
    fi

    # Globals, not locals: the EXIT trap runs after this function returns.
    tmp="$(mktemp)"; modified="$(mktemp)"; missing="$(mktemp)"; listed="$(mktemp)"
    trap 'rm -f "$tmp" "$modified" "$missing" "$listed"' EXIT

    local install_path source_path shipped
    while IFS=$'\t' read -r install_path source_path; do
        echo "$install_path" >> "$listed"
        if [ "$install_path" = "boat-rudder" ]; then
            printf '%s  %s\n' "$(sha "$dest/boat-rudder")" "$install_path" >> "$tmp"
            continue
        fi
        shipped="$(sha "$src/$source_path")"
        printf '%s  %s\n' "$shipped" "$install_path" >> "$tmp"
        if [ ! -f "$dest/$install_path" ]; then
            echo "$install_path" >> "$missing"
        elif [ "$(sha "$dest/$install_path")" != "$shipped" ]; then
            echo "$install_path" >> "$modified"
        fi
    done < <(system_files "$src")

    mkdir -p "$dest/$STATE_DIR"
    install -m 644 "$src/VERSION" "$dest/$STATE_DIR/VERSION"
    install -m 644 "$tmp" "$dest/$STATE_DIR/manifest.sha256"
    if [ -f "$src/scripts/minisign.pub" ]; then
        install -m 644 "$src/scripts/minisign.pub" "$dest/$STATE_DIR/minisign.pub"
    fi

    local extra
    extra="$(cd "$dest" && find html/templates html/themes html/assets -type f -print 2>/dev/null |
             LC_ALL=C sort | LC_ALL=C comm -23 - <(LC_ALL=C sort "$listed"))"

    echo "Adopted $dest as version $(head -n1 "$src/VERSION") ($(wc -l < "$tmp") system files)."
    echo "State written to $dest/$STATE_DIR/."
    report "Edited on this host (an update will replace them, keeping a copy in its backup)" "$(cat "$modified")"
    report "Missing on this host (an update will add them)" "$(cat "$missing")"
    report "Not part of this version (added on this host, or left over - an update never touches them)" "$extra"
}

cmd_verify() {
    local dest="${1:-}"
    [ -n "$dest" ] || usage
    [ -f "$dest/$STATE_DIR/manifest.sha256" ] || die "$dest has no manifest - run 'adopt' first"

    local out modified missing
    out="$(cd "$dest" && LC_ALL=C sha256sum -c "$STATE_DIR/manifest.sha256" 2>/dev/null || true)"
    modified="$(printf '%s\n' "$out" | sed -n 's/: FAILED$//p')"
    missing="$(printf '%s\n' "$out" | sed -n 's/: FAILED open or read$//p')"

    echo "Version $(head -n1 "$dest/$STATE_DIR/VERSION" 2>/dev/null || echo '?') - $(wc -l < "$dest/$STATE_DIR/manifest.sha256") system files."
    report "Modified" "$modified"
    report "Missing" "$missing"
    [ -z "$modified$missing" ] && echo "Every system file matches the manifest." && return 0
    return 1
}

# Prints a titled list, or nothing for an empty one.
report() {
    local title="$1" items="$2"
    [ -n "$items" ] || return 0
    echo ""
    echo "$title ($(printf '%s\n' "$items" | wc -l)):"
    printf '%s\n' "$items" | sed 's/^/  /'
}

case "${1:-}" in
    create) shift; cmd_create "$@" ;;
    adopt)  shift; cmd_adopt "$@" ;;
    verify) shift; cmd_verify "$@" ;;
    *)      usage ;;
esac
