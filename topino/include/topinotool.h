#ifndef TOPINOTOOL_H
#define TOPINOTOOL_H

#include <QColor>
#include <QImage>
#include <QRgb>
#include <QString>
#include <QtMath>

namespace TopinoTools {

/* Tableau10 colors by Chris Gerrard; more information at:
 * http://tableaufriction.blogspot.com/2012/11/finally-you-can-use-tableau-data-colors.html */
const QColor colorsTableau10[10] = {
    QColor( 31, 119, 180),
    QColor(214,  39,  40),
    QColor( 44, 160,  44),
    QColor(148, 103, 189),
    QColor(255, 127,  14),
    QColor(140,  86,  75),
    QColor(227, 119, 194),
    QColor(127, 127, 127),
    QColor(188, 189,  34),
    QColor( 23, 190, 207)
};

/* Selection color pairs - a dark variant (for the border) and a light variant (for the
 * content). Sorted for RGB channels, i.e. the first color pair is red. */
const QColor colorsSelectionRGB[3][2] = {
    { QColor(235,  70,  55), QColor(250, 230, 230) },
    { QColor( 30, 160,  95), QColor(230, 250, 240) },
    { QColor( 25, 115, 230), QColor(230, 240, 250) }
};


/* Desaturation modes */
enum desaturationModes {
    desatLightness = 0,
    desatLuminance = 1,
    desatAverage = 2,
    desatMaximum = 3,
    desatRed = 4,
    desatGreen = 5,
    desatBlue = 6,
    desatCOUNT = 7
};

/* Names for the desaturation modes */
QString getDesaturationModeName(desaturationModes mode);

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

/* This function receives the unit prefixes and matching double values for a
 * given value. */
QString getUnitPrefix(qreal &value);

/* Function to calculate the moment (M_pq) of a gray image (8bit). See the following
 * Wikipedia article for more details: https://en.wikipedia.org/wiki/Image_moment. */
qreal imageMoment(const QImage &grayImage, int p, int q);

/* Function to calculate the centroid of an image. See the Wikipedia article above
 * for more details. */
QPointF imageCentroid(const QImage &image);

/* Function to create a summed area table from the top left to the bottom right of
 * an image. See https://en.wikipedia.org/wiki/Summed-area_table for details. */
QImage imageSumAreaTable(const QImage &image);

/* Function to find max color value of a gray image (8bit). Returns 255 if image
 * is not gray. */
uchar imageMaxColorValue(const QImage &grayImage);

/* Function to find min color value of a gray image (8bit). Returns 0 if image
 * is not gray. */
uchar imageMinColorValue(const QImage &grayImage);

/* Get all points in a gray image with the provided color value and added to list. */
void imagePointsGrayValue(const QImage &grayImage, QList<QPointF>& list, uchar value);

/* Calculates the mass center of a given list of points */
QPointF getMassCenter(const QList<QPointF>& list);

}

#endif // TOPINOTOOL_H
