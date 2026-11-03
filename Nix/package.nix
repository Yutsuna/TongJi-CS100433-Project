{
  pkgs,
  ...
}:
let
  gccStdenv = pkgs.gcc16Stdenv;
  deps = import ./deps.nix { inherit pkgs; };
  version = builtins.head (builtins.split "\n" (builtins.readFile ../VERSION.md));
in
gccStdenv.mkDerivation {
  pname = "TongJi-CS100433-Project";
  inherit version;
  src = pkgs.lib.cleanSource ../.;

  nativeBuildInputs = deps.nativeBuildInputs;
  buildInputs = deps.buildInputs ++ [ pkgs.gcc16.cc.lib ];

  buildPhase = ''
    runHook preBuild
    make -j$NIX_BUILD_CORES
    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall
    mkdir -p $out/bin
    mv Ex* $out/bin
    cp -r Shaders $out/bin/
    runHook postInstall
  '';

  meta = {
    description = "TongJi-CS100433-Project";
    platforms = pkgs.lib.platforms.all;
  };
}
