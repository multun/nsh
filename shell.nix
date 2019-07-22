with import <nixpkgs> {};
stdenv.mkDerivation {
  name = "dish";
  buildInputs = [ readline ];
  nativeBuildInputs = [ python3 jq gdb gcc autoconf automake libtool ];
  # not suitable for debug builds
  hardeningDisable = [ "fortify" ];
}
