{
  pkgs,
  inputs,
  ...
}:
let
  deps = import ./deps.nix { inherit pkgs; };
  inherit (deps) nativeBuildInputs buildInputs devInputs;
in
pkgs.mkShell {
  nativeBuildInputs = nativeBuildInputs ++ devInputs;
  inherit buildInputs;

  shellHook = ''
    export LD_LIBRARY_PATH="/usr/lib:/usr/lib/dri:/usr/lib/wsl/lib:$LD_LIBRARY_PATH"

    ${pkgs.lib.optionalString pkgs.stdenv.hostPlatform.isLinux ''
      export NIX_CFLAGS_LINK="-fuse-ld=mold"
    ''}
  '';
}
