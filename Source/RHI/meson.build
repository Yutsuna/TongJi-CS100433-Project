#{
# Source/RHI/meson.build
# Source/RHI meson entry-point
#}

ModuleDefinition = {
    'Name' : 'RHI',
    'Type' : 'SharedLibrary',
    'OutputName' : 'Lumina',
    'SourceDir' : meson.current_source_dir(),
    'Private' : 'Private',
    'Public' : 'Public',
    'Tests' : 'Tests',
    'Depends' : ['Core','Maths'],
    'ExternalDeps' : [
        dependency('glfw3'),
        dependency('glm'),
        dependency('gl'),
    ],
    'DebugEnvironmentFile' : 'lsan.supp',
}
