with import <nixpkgs> {};
stdenv.mkDerivation {
  name = "nsh";
  buildInputs = [ readline ];
  nativeBuildInputs = [ python3 jq gdb gcc autoconf automake libtool valgrind bear afl ];
  # not suitable for debug builds
  hardeningDisable = [ "fortify" ];
}
