{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  name = "c-development-env";

  buildInputs = [
    pkgs.clang
    pkgs.gcc
    pkgs.cmake
    pkgs.gnumake
    pkgs.valgrind
    pkgs.gperf
    pkgs.docker
    pkgs.docker-compose
    pkgs.python3
    pkgs.fish
  ];

  shellHook = ''
    echo "Nix enviorment loaded."
    export fish_prompt='function fish_prompt; echo -n \"[nix-shell](pwd)> \"; end'
    exec fish
  '';
}