{
  inputs = {
    nipkxgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
  };

  outputs = {nixpkgs, ...}: let
    pkgs = import nixpkgs {
      system = "x86_64-linux";
    };
  in {
    devShells.x86_64-linux.default = pkgs.mkShell {
      buildInputs = with pkgs; [gsl];
      packages = with pkgs; [gdb];
    };
  };
}
