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
    in rec
    {
      packages.x86_64-linux = rec
      {
        hpcstat = pkgs.pkgsStatic.callPackage ./.
          { inherit (pkgs.pkgsStatic.localPackages) zxorm zpp-bits nameof; inherit openssh; standalone = true; };
        default = hpcstat;
        openssh = (pkgs.pkgsStatic.openssh.override { withLdns = false; etcDir = null; })
          .overrideAttrs (prev: { doCheck = false; patches = prev.patches ++ [ ./openssh.patch ];});
      };
      devShell.x86_64-linux = pkgs.mkShell
      {
        inputsFrom = [ packages.x86_64-linux.hpcstat ];
        nativeBuildInputs = with pkgs; [ clang-tools_18 ];
        CMAKE_EXPORT_COMPILE_COMMANDS = "1";
      };
    };
}
