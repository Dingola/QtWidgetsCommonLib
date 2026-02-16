#pragma once

#include <QtPlugin>

#include "ILanguagePreferences.h"
#include "IMainWindowStatePersistence.h"
#include "IThemePreferences.h"

namespace QtWidgetsCommonLib
{

/**
 * @class IUiPreferences
 * @brief Aggregated interface for UI-related preferences.
 *
 * This interface combines language, theme, and window state persistence preferences.
 */
class IUiPreferences: public ILanguagePreferences,
                      public IMainWindowStatePersistence,
                      public IThemePreferences
{
    public:
        /**
         * @brief Virtual destructor for safe polymorphic use.
         */
        virtual ~IUiPreferences() = default;
};

}  // namespace QtWidgetsCommonLib

Q_DECLARE_INTERFACE(QtWidgetsCommonLib::IUiPreferences, "de.adrianhelbig.IUiPreferences")
