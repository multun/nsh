with import <nixpkgs> {};
stdenv.mkDerivation {
  name = "nsh";
  buildInputs = [ readline ];
  nativeBuildInputs = [ doxygen python3 jq gdb gcc valgrind afl ];
  # not suitable for debug builds
  hardeningDisable = [ "fortify" ];
}
