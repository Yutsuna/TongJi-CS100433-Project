/**
 * @file LoggingCategory.cpp
 * @brief Implementation of the FLogCategory struct, which represents a logging category.
 */

#include <Logging/LoggingCategory.hpp>

/**
 * Public
 */

Lumina::FLogCategory::FLogCategory ( const AnsiChar *const InCategoryName, const ELogVerbosity::Type InDefaultVerbosity ) noexcept
    : CategoryName( InCategoryName ), DefaultVerbosity( InDefaultVerbosity )
{
    /** Empty */
}

Lumina::FLogCategory::FLogCategory ( const FStringView InCategoryName, const ELogVerbosity::Type InDefaultVerbosity ) noexcept
    : CategoryName( InCategoryName ), DefaultVerbosity( InDefaultVerbosity )
{
    /** Empty */
}
