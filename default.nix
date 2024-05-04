{
  stdenv, cmake, pkg-config, standalone ? false, makeWrapper,
  boost, fmt, zxorm, nlohmann_json, zpp-bits, range-v3, nameof, openssh
}: stdenv.mkDerivation
{
  name = "hpcstat";
  src = ./.;
  buildInputs =
    [ boost fmt zxorm nlohmann_json zpp-bits range-v3 nameof ];
  nativeBuildInputs = [ cmake pkg-config makeWrapper ];
  postInstall =
    if standalone then "cp ${openssh}/bin/{ssh-add,ssh-keygen} $out/bin"
    else
    ''
      wrapProgram $out/bin/hpcstat --set HPCSTAT_SHAREDIR $out/share/hpcstat \
        --set HPCSTAT_DATADIR /var/lib/hpcstat
    '';
}