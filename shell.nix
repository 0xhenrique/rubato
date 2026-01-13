{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    gcc
    clang
    pkg-config
    gnumake
    binutils
  ];
}
