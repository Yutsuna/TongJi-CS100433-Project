/// @file TestRunner.cpp
/// @brief Standard entry-point for Lumina module unit test suites.

#include <Tests/UnitTest.hpp>

#include <Logging/Logger.hpp>

Lumina::Int32 main ( const Lumina::Int32 argc, const Lumina::AnsiChar *argv[] )
{
    Lumina::FStringView SuiteFilter{};
    if ( argc > 1 )
    {
        SuiteFilter = argv[1];
    }

    struct FGuardedScope final
    {
        constexpr FGuardedScope () noexcept
        {
            Lumina::FLogger::GetInstance().Initialize();
        };
        ~FGuardedScope () noexcept
        {
            Lumina::FLogger::GetInstance().Shutdown();
        }
    } GuardedScope{};

    const Lumina::Tests::FUnitTestTestRunSummary Summary = Lumina::Tests::FUnitTestRegistry::GetInstance().Run( SuiteFilter );

    return Summary.IsSuccessful() ? 0 : 84;
}
