{
  pkgs,
  ...
}:
let
  gccStdenv = pkgs.gcc16Stdenv;
  deps = import ./deps.nix { inherit pkgs; };
  version = builtins.head (builtins.split "\n" (builtins.readFile ../VERSION.md));
  inherit (deps) buildInputs nativeBuildInputs;
in
gccStdenv.mkDerivation {
  pname = "TongJi-CS100433-Project";
  src = pkgs.lib.cleanSource ../.;

  inherit version nativeBuildInputs;

  buildInputs = buildInputs ++ [ pkgs.gcc16.cc.lib ];

  mesonBuildType = "release";
  mesonBuildDir = "Build";

  meta = {
    description = "TongJi-CS100433-Project";
    platforms = pkgs.lib.platforms.all;
  };
}
