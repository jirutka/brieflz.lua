# vim: set ts=4:

readonly REPO_NAME='jirutka/brieflz.lua'

readonly HOST_OS=$(uname -s)
readonly TEMP_DIR="$(pwd)/.tmp"
readonly VENV_DIR="$(pwd)/.venv"

case "$HOST_OS" in
	Darwin)
		alias sha256sum='shasum -a 256'
		alias md5sum='md5';;
	MINGW* | Windows)
		alias busted='busted.bat'
		alias luarocks='luarocks.bat';;
esac

einfo() {
	# bold cyan
	printf '\033[1;36m> %s\033[0m\n' "$@" >&2
}

ewarn() {
	# bold yellow
	printf '\033[1;33m> %s\033[0m\n' "$@" >&2
}

die() {
	# bold red
	printf '\033[1;31mERROR:\033[0m %s\n' "$1" >&2
	shift
	printf '  %s\n' "$@"
	exit 2
}

# Prints version number based on the last git tag with prefix "v". If HEAD is
# not tagged (i.e. this is not a release), then it prints last version with
# suffix "_git<n>g<abbrev>". If there's no tag with prefix "v" (i.e. there's no
# release yet), it prints "0.0.0_git<n>g<abbrev>".
git_based_version() {
	# First check that we are in a git repository.
	git rev-parse HEAD >/dev/null

	{ git describe --tags --match 'v*' 2>/dev/null || echo 'v0.0.0'; } \
		| cut -c 2- | sed -E 's/\-([0-9]+)\-g([0-9a-f]+)/_git\1g\2/'
}

# Returns 0 if this is a release.
# If running on Travis, then it's a release if building tag with prefix "v",
# otherwise if git HEAD is tagged with prefix "v".
is_release() {
	if [ -n "${TRAVIS:-}" ]; then
		[ -n "${TRAVIS_TAG:-}" ] && [ "${TRAVIS_TAG#v}" != "$TRAVIS_TAG" ]
	elif git rev-parse HEAD >/dev/null; then
		git describe --tags --exact-match --match 'v*' >/dev/null 2>&1
	else
		ewarn 'Could not detect release; not in a git repository or on Travis CI'
	fi
}

# Prints version of the specified Lua binary, or "lua" on PATH if $1 is empty.
# This is API version, i.e. it's the same for Lua and LuaJIT.
lua_version() {
	local lua_path="${1:-lua}"

	$lua_path -e 'print(_VERSION:sub(5))'
}

# Fetches the given URL and verifies SHA256 checksum.
wgets() (
	local url="$1"
	local sha256="$2"
	local dest="${3:-.}"

	mkdir -p "$dest" \
		&& cd "$dest" \
		&& rm -f "${url##*/}" \
		&& wget -T 10 "$url" \
		&& echo "$sha256  ${url##*/}" | sha256sum -c
)
