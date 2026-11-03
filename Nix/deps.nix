{
  pkgs,
  ...
}:
{
  nativeBuildInputs =
    with pkgs;
    [
      gcc16
      meson
      ninja
      pkg-config
    ]
    ++ lib.optionals pkgs.stdenv.hostPlatform.isLinux [
      mold
    ];

  buildInputs = with pkgs; [
    glm
    glfw
    imgui
  ];

  devInputs = with pkgs; [
    glslang
    clang-tools
    gdb
    valgrind
    bear
    mesa
  ];
}
