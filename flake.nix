{
  description = "A POSIX shell you can read the source of";

  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system}; in
      rec {
        packages = flake-utils.lib.flattenTree {
          nsh = pkgs.stdenv.mkDerivation {
            pname = "nsh";
            version = "1.0";
            src = ./.;
            buildInputs = [ pkgs.readline ];
            nativeBuildInputs = [ pkgs.meson pkgs.ninja ];
          };
        };
        defaultPackage = packages.nsh;
        apps.hello = flake-utils.lib.mkApp { drv = packages.nzh; };
        defaultApp = apps.nsh;
        shell = pkgs.mkShell {
            inputsFrom = [ packages.nsh ];
            hardeningDisable = [ "fortify" ];
            packages = [ pkgs.doxygen pkgs.gdb pkgs.afl pkgs.valgrind ];
        };
      }
    );
}