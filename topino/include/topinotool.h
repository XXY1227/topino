#ifndef TOPINOTOOL_H
#define TOPINOTOOL_H

#include <QRgb>

namespace TopinoTools {

/* The following two functions calculate the lightness value, typically
 * used for desaturation of images. Lightness = 0.5 × (max(R,G,B) + min(R,G,B)) */
inline Q_DECL_CONSTEXPR int qLightness(int r, int g, int b) {
    return (qMax(qMax(r, g), b) + qMin(qMin(r, g), b)) / 2;
}

inline Q_DECL_CONSTEXPR int qLightness(QRgb rgb) {
    return qLightness(qRed(rgb), qGreen(rgb), qBlue(rgb));
}

/* The following two functions calculate the luminance value, typically
 * used for desaturation of images. Luminance = 0.21 × R + 0.72 × G + 0.07 × B.
 * There are different weights for this model out there. We are using here
 * the once from GIMP (https://docs.gimp.org/2.8/en/gimp-tool-desaturate.html). */
inline Q_DECL_CONSTEXPR int qLuminance(int r, int g, int b) {
    return (r * 21 + g * 72 + b * 7) / 100;
}

inline Q_DECL_CONSTEXPR int qLuminance(QRgb rgb) {
    return qLuminance(qRed(rgb), qGreen(rgb), qBlue(rgb));
}

/* The following two functions calculate the average value of red, green, and
 * blue value. Is used for desaturation of images. */
inline Q_DECL_CONSTEXPR int qAverage(int r, int g, int b) {
    return (r + g + b) / 3;
}

inline Q_DECL_CONSTEXPR int qAverage(QRgb rgb) {
    return qAverage(qRed(rgb), qGreen(rgb), qBlue(rgb));
}

/* The following two functions calculate the maximum value of red, green, and
 * blue value. Is used for desaturation of images. */
inline Q_DECL_CONSTEXPR int qMaximum(int r, int g, int b) {
    return qMax(qMax(r, g), b);
}

inline Q_DECL_CONSTEXPR int qMaximum(QRgb rgb) {
    return qMaximum(qRed(rgb), qGreen(rgb), qBlue(rgb));
}

}

#endif // TOPINOTOOL_H
