#include "include/topinotool.h"

/* Receives the unit prefix (e.g. nano, micro, milli, etc) for a double value and updates the
 * value to match the prefix. */
QString TopinoTools::getUnitPrefix(qreal &value) {
    /* Extreme case, the value is zero */
    if (value == 0.0) {
        return QString("");
    }

    /* These are the prefixes and the respective exponents */
    const char	*prefixes[]	= { "p", "n", "µ", "m", "", "k", "M", "G", "T" };
    double	exponents[] = { 1.0e-12,  1.0e-9, 1.0e-6, 1.0e-3, 1, 1.0e3, 1.0e6, 1.0e9, 1.0e12 };

    /* Divide the value by exponents until it is between 0.5 and 500. Return the respective
     * exponent */
    for ( int i = 0; i < 9; i++ ) {
        double newValue = value / exponents[i];

        if ( newValue < 500.0 ) {
            value = newValue;
            return QString(prefixes[i]);
        }
    }

    /* No prefix found (should only happen for really large or small numbers) */
    return QString("");
}

QString TopinoTools::getDesaturationModeName(TopinoTools::desaturationModes mode)
{
    /* Names for the desaturation modes */
    const char *desatNames[desaturationModes::desatCOUNT] = {
        "Lightness",
        "Luminance",
        "Average",
        "Maximum",
        "Red channel",
        "Green channel",
        "Blue channel"
    };

    /* Mode should be in the boundaries (could be an integer passed) */
    if ((mode < 0) || (mode >= desaturationModes::desatCOUNT)) {
        return "";
    }

    /* Return respective name */
    return desatNames[mode];
}
