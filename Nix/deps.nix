{
  pkgs,
  ...
}:
{
  nativeBuildInputs = with pkgs; [
    gcc16
    ccache
    meson
    ninja
    pkg-config
  ] ++ lib.optionals pkgs.stdenv.hostPlatform.isLinux [
    mold
    mesa
  ];

  buildInputs = with pkgs; [
    glm
    glfw
    imgui
  ];

  devInputs = with pkgs; [
    valgrind
    gdb
    clang-tools
    jq
    nil
    nixd
    muon
  ];

}
