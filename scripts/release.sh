#!/bin/bash
# Boat Rudder - publish a signed source release
# (develop_docs/plans/update-system-plan.md, phase 2). Run by the maintainer,
# on the maintainer's machine - never on a server.
#
#   scripts/release.sh <version> [--key <minisign secret key>] [--publish]
#
# 1. checks the work tree is clean, VERSION says <version>, and the tag
#    v<version> doesn't exist yet;
# 2. builds dist/boat-rudder-<version>-src.tar.gz with `git archive` from
#    HEAD - versioned files only, under boat-rudder-<version>/ - gzip -n, so the
#    same commit always gives the same bytes;
# 3. signs it with minisign (asks for the key's password) and verifies the
#    signature against scripts/minisign.pub, the key installs trust;
# 4. tags HEAD as v<version> (annotated) - only once the signature checks out;
# 5. with --publish: pushes the tag and creates the GitHub release with both
#    files and the version's CHANGELOG.md section (needs the `gh` CLI, logged
#    in). Without it, prints what to upload by hand.
#
# GitHub's automatic "Source code" archives can't be used instead: they are
# generated on demand and their bytes aren't guaranteed stable, so a
# signature over them could stop matching.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$ROOT_DIR"

REPO="jpablotoledom/boat-rudder"
KEY="${HOME}/.minisign/boat-rudder.key"
PUBLISH=0
VERSION_ARG=""

die() { echo "release.sh: $*" >&2; exit 1; }

while [ $# -gt 0 ]; do
    case "$1" in
        --key) KEY="${2:-}"; shift 2 ;;
        --publish) PUBLISH=1; shift ;;
        -*) die "unknown option $1" ;;
        *) VERSION_ARG="${1#v}"; shift ;;
    esac
done
[ -n "$VERSION_ARG" ] || die "usage: scripts/release.sh <version> [--key <secret key>] [--publish]"

V="$VERSION_ARG"
TAG="v$V"
PKG="dist/boat-rudder-$V-src.tar.gz"

# ---- 1. Checks ---------------------------------------------------------------
command -v minisign > /dev/null || die "minisign not found (apt install minisign)"
if [ "$PUBLISH" = "1" ]; then
    command -v gh > /dev/null || die "--publish needs gh (GitHub CLI) - install it, or run without --publish and upload by hand"
fi
[ -f "$KEY" ] || die "secret key $KEY not found (--key <path>)"
[ -f scripts/minisign.pub ] || die "scripts/minisign.pub missing - the public key every install trusts"
[ "$(head -n1 VERSION)" = "$V" ] || die "VERSION says $(head -n1 VERSION), not $V - update and commit it first"
[ -z "$(git status --porcelain)" ] || die "the work tree has uncommitted changes - commit or stash them first"
if git rev-parse -q --verify "refs/tags/$TAG" > /dev/null; then die "tag $TAG already exists"; fi
grep -q "^## $TAG\$" CHANGELOG.md || echo "Warning: CHANGELOG.md has no '## $TAG' section - the release will have no notes."

COMMIT="$(git rev-parse HEAD)"

# ---- 2. Package --------------------------------------------------------------
mkdir -p dist
git archive --format=tar --prefix="boat-rudder-$V/" "$COMMIT" | gzip -n -9 > "$PKG"
echo "Package: $PKG ($(du -h "$PKG" | cut -f1)) from ${COMMIT:0:7}"

# ---- 3. Sign and verify ------------------------------------------------------
minisign -S -s "$KEY" -m "$PKG" -t "boat-rudder $V"
minisign -Vm "$PKG" -p scripts/minisign.pub > /dev/null || die "the signature doesn't verify against scripts/minisign.pub - wrong key? (nothing tagged)"
echo "Signed and verified: $PKG.minisig"

# ---- 4. Tag - only now, so a failed signature leaves nothing behind ----------
git tag -a "$TAG" -m "Boat Rudder $V" "$COMMIT"
echo "Tagged ${COMMIT:0:7} as $TAG."

# ---- 5. Publish (refs/tags/: a branch may have the same name) ----------------
NOTES="dist/boat-rudder-$V-notes.md"
awk -v tag="## $TAG" '$0 == tag { on = 1; next } on && /^## v/ { exit } on' CHANGELOG.md > "$NOTES"

if [ "$PUBLISH" = "1" ]; then
    git push origin "refs/tags/$TAG"
    gh release create "$TAG" "$PKG" "$PKG.minisig" -R "$REPO" --title "Boat Rudder $V" --notes-file "$NOTES"
    echo "Published: https://github.com/$REPO/releases/tag/$TAG"
else
    cat <<EOF

Not published (pass --publish to do it with the gh CLI). By hand:
  1. git push origin refs/tags/$TAG
  2. https://github.com/$REPO/releases/new?tag=$TAG
     title "Boat Rudder $V", notes from $NOTES, and attach BOTH files:
       $PKG
       $PKG.minisig
Servers find a release only when it has both files.
EOF
fi
