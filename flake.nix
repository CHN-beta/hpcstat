{
  inputs.nixos.url = "github:CHN-beta/nixos";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = inputs:
    let
      pkgs = (import inputs.nixpkgs
      {
        system = "x86_64-linux";
        config.allowUnfree = true;
        overlays = [(final: prev:
        {
          localPackages = import "${inputs.nixos}/local/pkgs"
            { pkgs = final; inherit (inputs.nixpkgs) lib; topInputs = inputs.nixos.inputs; };
        })];
      });
    in
    {
      packages.x86_64-linux = rec
      {
        hpcstat = pkgs.pkgsStatic.stdenv.mkDerivation
        {
          name = "hpcstat";
          src = ./.;
          buildInputs = with pkgs.pkgsStatic;
            [ boost fmt localPackages.zxorm nlohmann_json localPackages.zpp-bits range-v3 ];
          nativeBuildInputs = with pkgs; [ cmake pkg-config ];
          postInstall = "cp ${openssh}/bin/{ssh-add,ssh-keygen} $out/bin";
        };
        default = hpcstat;
        openssh = (pkgs.pkgsStatic.openssh.override { withLdns = false; etcDir = null; })
          .overrideAttrs (prev: { doCheck = false; patches = prev.patches ++ [ ./openssh.patch ];});
      };
      devShell.x86_64-linux = pkgs.mkShell
      {
        nativeBuildInputs = with pkgs; [ pkg-config cmake clang-tools_18 ];
        buildInputs = (with pkgs.pkgsStatic;
          [ fmt boost localPackages.zxorm nlohmann_json localPackages.zpp-bits range-v3 ]);
        # hardeningDisable = [ "all" ];
        # NIX_DEBUG = "1";
        CMAKE_EXPORT_COMPILE_COMMANDS = "1";
      };
    };
}
