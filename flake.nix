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
        apps.nsh = flake-utils.lib.mkApp { drv = packages.nsh; };
        defaultApp = apps.nsh;
        devShell = pkgs.mkShell {
            inputsFrom = [ packages.nsh ];
            packages = [
              pkgs.python3
              pkgs.doxygen
              pkgs.gdb
              pkgs.afl
              pkgs.valgrind
            ];
        };
      }
    );
}
