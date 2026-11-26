#{
# Source/Meson/Private/meson.build
# Builds each module's target from its Private/ sources.
#
# Reads  : ModuleDefinitions, PublicIncludes, GlobalModules, ALL_WARNINGS,
#          DebugEnvironmentFile (optional module key)
# Writes : GlobalModules, GlobalTestModules
#
# Include scopes:
#   - PublicIncludeDirectories  -> exported through GlobalModules, visible to
#                                  every module that depends on this one.
#   - PrivateIncludeDirectories -> only reachable through ModuleInternalDependency,
#                                  used by this module's own targets and tests.
#}
DebugEnvironmentFiles = []

foreach ModuleDefinition : ModuleDefinitions
    if ModuleDefinition.has_key('DebugEnvironmentFile')
        DebugEnvironmentFiles += [
            SourceRoot / ModuleDefinition.get('SourceDirRel') / ModuleDefinition.get('DebugEnvironmentFile')
        ]
    endif
endforeach

DebugEnvironmentSources = []

if DebugEnvironmentFiles.length() > 0 and 'address' in get_option('b_sanitize').split(',')
    DebugEnvironmentConfiguration = configuration_data()
    DebugEnvironmentConfiguration.set('SUPPRESSIONS', ','.join(DebugEnvironmentFiles))
    DebugEnvironmentSources += [configure_file(
        input : meson.project_source_root() / 'Meson' / 'LSanEnvironment.cpp.in',
        output : 'LSanEnvironment.cpp',
        configuration : DebugEnvironmentConfiguration,
    )]
endif

foreach ModuleDefinition : ModuleDefinitions
    ModuleName = ModuleDefinition.get('Name')
    ModuleType = ModuleDefinition.get('Type', 'StaticLibrary')
    ModuleSourceDirRel = ModuleDefinition.get('SourceDirRel')
    ModuleDependencies = ModuleDefinition.get('Depends', [])

    ModulePath = SourceRoot / ModuleSourceDirRel
    PrivatePath = ModulePath / ModuleDefinition.get('Private', 'Private')

    PublicIncludeDirectories = PublicIncludes.get(ModuleName, [])
    PrivateIncludeDirectories = []

    PrivateDirectory = ModuleDefinition.get('Private', 'Private')
    RelPrivatePath = '../../@0@/@1@'.format(ModuleSourceDirRel, PrivateDirectory)

    if FileSystem.is_dir(RelPrivatePath)
        PrivateIncludeDirectories += include_directories(RelPrivatePath)
    endif

    # Shared libraries generate their export header in the build directory.
    # It is part of the public interface, but the module's own targets need it
    # as well, so add it before building the internal dependency.
    if ModuleType == 'SharedLibrary'
        ExportConfiguration = configuration_data()
        ExportConfiguration.set('MODULE', ModuleName.to_upper())

        configure_file(
            input : meson.project_source_root() / 'Meson' / 'ExportTemplate.hpp.inl',
            output : '@0@Export.hpp'.format(ModuleName),
            configuration : ExportConfiguration,
        )

        PublicIncludeDirectories += include_directories('.')
    endif

    PrivateSources = []

    if FileSystem.is_dir(PrivatePath)
        FindResult = run_command(
            'find', PrivatePath,
            '-type', 'f', '(', '-name', '*.cpp', '-o', '-name', '*.c', ')',
            check : false,
        )
        foreach FilePath : FindResult.stdout().strip().split('\n')
            if FilePath != ''
                PrivateSources += files(FilePath)
            endif
        endforeach
    endif

    ResolvedDependencies = []

    foreach DependencyName : ModuleDependencies
        if not GlobalModules.has_key(DependencyName)
            error(
                'Module @0@ depends on @1@ which is not built yet or not found.'
                .format(ModuleName, DependencyName)
            )
        endif
        ResolvedDependencies += GlobalModules.get(DependencyName)
    endforeach

    ResolvedDependencies += ModuleDefinition.get('ExternalDeps', [])

    # Internal, linkable dependency for the module itself (targets and tests).
    # It carries Private/ on purpose; GlobalModules never receives it, so
    # dependent modules only ever see PublicIncludeDirectories.
    ModuleInternalDependency = declare_dependency(
        include_directories : PublicIncludeDirectories + PrivateIncludeDirectories,
        dependencies : ResolvedDependencies,
    )

    CompileArguments = ALL_WARNINGS

    if ModuleType == 'Executable'
        MainFilePath = PrivatePath / 'Main.cpp'
        MainSources = []
        LibrarySources = PrivateSources

        MainFile = files(MainFilePath)[0]
        foreach SourcePath : PrivateSources
            if SourcePath == MainFile
                MainSources += SourcePath
                LibrarySources = []
                foreach OtherPath : PrivateSources
                    if OtherPath != SourcePath
                        LibrarySources += OtherPath
                    endif
                endforeach
            endif
        endforeach

        if MainSources.length() == 0
            MainSources = PrivateSources
            LibrarySources = []
        endif

        MainSources += DebugEnvironmentSources

        if LibrarySources.length() > 0
            ModuleLibrary = static_library(
                ModuleName + 'Lib', LibrarySources,
                dependencies : ModuleInternalDependency,
                cpp_args : CompileArguments,
                c_args : CompileArguments,
                install : false,
            )

            ModuleTarget = executable(
                ModuleName, MainSources,
                dependencies : ModuleInternalDependency,
                link_with : ModuleLibrary,
                cpp_args : CompileArguments,
                c_args : CompileArguments,
                install : true,
            )
            ModuleDependency = declare_dependency(
                include_directories : PublicIncludeDirectories,
                link_with : ModuleLibrary,
                dependencies : ResolvedDependencies,
            )
        else
            ModuleTarget = executable(
                ModuleName, MainSources,
                dependencies : ModuleInternalDependency,
                cpp_args : CompileArguments,
                c_args : CompileArguments,
                install : true,
            )
            ModuleDependency = declare_dependency(
                include_directories : PublicIncludeDirectories,
                dependencies : ResolvedDependencies,
            )
        endif

    elif ModuleType == 'SharedLibrary'
        BuildDefines = ['-D' + ModuleName.to_upper() + '_BUILD_SHARED']

        ModuleImplementation = static_library(
            ModuleName + 'Implementation', PrivateSources,
            dependencies : ModuleInternalDependency,
            cpp_args : CompileArguments + BuildDefines,
            c_args : CompileArguments + BuildDefines,
            pic : true,
            install : false,
        )
        ModuleTarget = shared_library(
            ModuleName,
            link_whole : ModuleImplementation,
            dependencies : ResolvedDependencies,
            install : true,
        )
        ModuleDependency = declare_dependency(
            include_directories : PublicIncludeDirectories,
            link_with : ModuleTarget,
            dependencies : ResolvedDependencies,
        )
        ModuleTestDependency = declare_dependency(
            dependencies : ModuleInternalDependency,
            link_whole : ModuleImplementation,
        )

    elif ModuleType == 'StaticLibrary'
        ModuleTarget = static_library(
            ModuleName, PrivateSources,
            dependencies : ModuleInternalDependency,
            cpp_args : CompileArguments,
            c_args : CompileArguments,
            install : true,
        )
        ModuleDependency = declare_dependency(
            include_directories : PublicIncludeDirectories,
            link_with : ModuleTarget,
            dependencies : ResolvedDependencies,
        )

    else
        error('Unknown module type: @0@'.format(ModuleType))
    endif

    if ModuleType != 'SharedLibrary'
        ModuleTestDependency = declare_dependency(
            dependencies : [ModuleInternalDependency, ModuleDependency],
        )
    endif

    GlobalModules += { ModuleName : ModuleDependency }
    GlobalTestModules += { ModuleName : ModuleTestDependency }
endforeach
