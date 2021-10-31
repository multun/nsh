{
  description = "A POSIX shell you can read the source of";

  inputs.flake-utils.url = "github:numtide/flake-utils";
  inputs.pre-commit-hooks.url = "github:cachix/pre-commit-hooks.nix";

  inputs.flake-utils.inputs.nixpkgs.follows = "nixpkgs";
  inputs.pre-commit-hooks.inputs.nixpkgs.follows = "nixpkgs";

  outputs = { self, nixpkgs, flake-utils, pre-commit-hooks }:
    flake-utils.lib.eachDefaultSystem (system:
      let pkgs = nixpkgs.legacyPackages.${system}; in
      rec {
        checks = {
          pre-commit-check = pre-commit-hooks.lib.${system}.run {
            src = ./.;
            hooks = {
              nixpkgs-fmt.enable = true;
              clang-format = {
                enable = true;
                types_or = [ "c" ];
              };
            };
          };
        };
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
          inherit (self.checks.${system}.pre-commit-check) shellHook;
          # toolchain hardening injects unwanted compiler flags.
          # fortify injects -O2 along with -D_FORTIFY_SOURCE=2,
          # which breaks the debugging experience
          hardeningDisable = [ "all" ];
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
