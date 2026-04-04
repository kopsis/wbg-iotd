{
  description = "Wayland wallpaper tool with IOTD support.";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
    systems.url = "github:nix-systems/x86_64-linux";
    flake-utils = {
      url = "github:numtide/flake-utils";
      inputs.systems.follows = "systems";
    };
  };

  outputs =
    { self
    , nixpkgs
    , flake-utils
    , ...
    }:
    {
    # For more information about the C/C++ infrastructure in nixpkgs: https://nixos.wiki/wiki/C
    flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = nixpkgs.legacyPackages.${system};
      pname = "wbg-iotd"; #package name
      version = "1.3.0";
      src = ./.;
      buildInputs = with pkgs; [
        # add library dependencies here i.e.
        cjson
        curl
        libjpeg
        pixman
        tllist
        wayland
        wayland-protocols
      ];
      nativeBuildInputs = with pkgs; [
        # add build dependencies here
        meson ninja
        pkg-config
        wayland-scanner
      ];
    in
    rec {
      devShells.default = pkgs.mkShell {
        inherit buildInputs nativeBuildInputs;

        # You can use NIX_CFLAGS_COMPILE to set the default CFLAGS for the shell
        NIX_CFLAGS_COMPILE = "-O1 -DWBG_VERSION=\"${version}\"";
        # You can use NIX_LDFLAGS to set the default linker flags for the shell
        #NIX_LDFLAGS = "-L${lib.getLib zstd}/lib -lzstd";
      };

      # Pinned gcc: remain on gcc10 even after `nix flake update`
      #default = pkgs.mkShell.override { stdenv = pkgs.gcc10Stdenv; } {
      #  inherit buildInputs nativeBuildInputs;
      #};

      # Clang example:
      #default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
      #  inherit buildInputs nativeBuildInputs;
      #};

      packages.default = pkgs.stdenv.mkDerivation {
        inherit buildInputs nativeBuildInputs pname version src;
        NIX_CFLAGS_COMPILE = "-O1 -DWBG_VERSION=\"${version}\"";
        installPhase = ''
          install -m755 -D ./wbg-iotd $out/bin/wbg-iotd
        '';
      };

      overlay = overlays.default;
      overlays.default = final: _: {
        wbg-iotd = final.stdenv.mkDerivation {
          inherit buildInputs nativeBuildInputs pname version src;
          NIX_CFLAGS_COMPILE = "-O1 -DWBG_VERSION=\"${version}\"";
          installPhase = ''
            install -m755 -D ./wbg-iotd $out/bin/wbg-iotd
          '';
        };
      };

    });

    nixosModules.wbg-iotd = ./modules/wbg-iotd.nix;
    };
}
