#ifndef TOPINOTOOL_H
#define TOPINOTOOL_H

#include <algorithm>
#include <QColor>
#include <QImage>
#include <QRgb>
#include <QString>
#include <QtMath>

/* Important: do not include LevenbergMarquardt directly, but by NLO header! */
#include <Eigen/Eigen>
#include <unsupported/Eigen/NonLinearOptimization>

namespace TopinoTools {

/* Tableau10 colors by Chris Gerrard; more information at:
 * http://tableaufriction.blogspot.com/2012/11/finally-you-can-use-tableau-data-colors.html */
const QColor colorsTableau10[10] = {
    QColor( 31, 119, 180),  /* 0: Blue */
    QColor(214,  39,  40),  /* 1: Red */
    QColor( 44, 160,  44),  /* 2: Green */
    QColor(148, 103, 189),  /* 3: Violet */
    QColor(255, 127,  14),  /* 4: Orange */
    QColor(140,  86,  75),  /* 5: Brown */
    QColor(227, 119, 194),  /* 6: Pink */
    QColor(127, 127, 127),  /* 7: Gray */
    QColor(188, 189,  34),  /* 8: Yellow */
    QColor( 23, 190, 207)   /* 9: Light blue */
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

/* Multiplicative epsilon for qreal calculations with very small numbers */
const qreal MULTIPLICATIVE_EPSILON = 1 + 1e-14;

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

/* Calculates points on 8 directions from the given central point on the given image depending
 * on the slope going from the central point to the outside. Works only on gray images. */
QList<QPointF> imageSlopePoints(const QImage &image, const QPointF &center);

/* Calculates the point of slope on a given array of pixels. Returns it as t-value,
 * i.e. 0.0 for index = 0 and 1.0 for max index. */
qreal pixelSlopePoint(const QList<uchar>& pixels);

/* Calculate the standard deviation of a list of int values (for the noise) */
qreal pixelNoise(const QList<int>& values);

/* Calculates the max distance between one point (center) to a list of other points */
qreal centerMaxDistance(const QPointF &center, const QList<QPointF> &points);

/* Calculates the smallest circle (returned in center, radius) for a given list of points. Algorithm based
 * on Nayukis version but adapted to Qt (https://www.nayuki.io/page/smallest-enclosing-circle). */
void calculateSmallestCircle(const QList<QPointF>& points, QPointF &center, qreal &radius);

/* Helper function that calculcates the smallest circle (returned in center, radius) based on two points
 * that are already IN the circle. */
void calculateSmallestCircleTwoPoints(const QList<QPointF>& points, int p, int q, QPointF &center, qreal &radius);

/* Helper function that creates a circum circle (returned in center and radius) around three points a, b, and c. This
 * is based on the algorithm given at Wikipedia (https://en.wikipedia.org/wiki/Circumscribed_circle). */
void calculateCircumCircle(const QPointF &a, const QPointF &b, const QPointF &c, QPointF &center, qreal &radius);

/* Checks if a point is in the circle given by radius and position */
bool isPointInCircle(const QPointF &point, const QPointF &center, const qreal radius);

/* Calculates the cross product (p1.x * p2.y - p2.x * p2.y) of two points */
qreal crossProduct(const QPointF &p, const QPointF &q);

/* Smoothes a data set of points by Gaussian Kernel smoothing. See the following Wiki page
 * for more details: https://en.wikipedia.org/wiki/Kernel_smoother */
void smoothByGaussianKernel(QVector<QPointF>& points, int size = 5, qreal sigma = 1.0);

/* Helper function: Gaussian kernel function */
qreal gaussianKernel(qreal value, qreal sigma);

/* Helper structures for finding and defining extrema */
enum slopeDirection {
    directionFlat = 0,
    directionAscending = 1,
    directionDescending = 2
};

enum extremaType {
    extremaMinimum = 0,
    extremaMaximum = 1
};

struct Extrema {
    int index = -1;
    QPointF pos;
    extremaType type = extremaMinimum;
};

/* Finds extrema in a set of points using the y-values. Data should be smoothed
 * before using this. */
void getExtrema(const QVector<QPointF> &points, QVector<Extrema>& extrema);

/* Filters extrema and removes the ones which are below the given threshold. */
void filterExtrema(QVector<Extrema>& extrema, qreal threshold);

/* Counts extrema of a certain type */
int countExtrema(const QVector<Extrema>& extrema, extremaType type);

/* Splits the extrema and just returns either maxima or minima */
QVector<Extrema> splitExtrema(const QVector<Extrema>& extrema, extremaType type);

/* Helper structure for finding and working with sections */
struct Section {
    int indexLeft = -1;
    int indexRight = -1;
    int indexMax = -1;

    bool isNull() {
        return (indexLeft == -1) || (indexRight == -1) || (indexMax == -1);
    }
};

/* Create sections from the given extrema, threshold, and data set. Two sections will be
 * the border sections, i.e. one from the first point on the left that is above the threshold
 * to the first minima and one from the last minima to the last point above the threshold
 * on the right side. */
QVector<Section> getSections(const QVector<QPointF> &points, const QVector<Extrema>& extrema, qreal threshold);

/* Helper structure for a Lorentzian: y = height * (width/2)^2 / ((width/2)^2 + (pos - x)^2) + offset*/
struct Lorentzian {
    qreal pos = 0.0;
    qreal height = 0.0;
    qreal width = 0.0;
    qreal offset = 0.0;
    qreal rsquare = 0.0;

    qreal f(qreal x) const {
        qreal denominator = qPow(width/2.0, 2.0) + qPow(pos - x, 2.0);

        if (denominator == 0.0) {
            return 0.0;
        }

        return height * qPow(width/2.0, 2.0) / denominator + offset;
    }
};

/* Functor for Eigen-Solver. Based on a tutorial of Sarvagya, which can be found at
 * https://medium.com/@sarvagya.vaish/levenberg-marquardt-optimization-part-2-5a71f7db27a0 */
struct LorentzianFunctor {
    /* Data values to fit to */
    QVector<QPointF> dataValues;

    /* This is what calculates a value at x and respective errors */
    int operator()(const Eigen::VectorXd &p, Eigen::VectorXd &fvec) const {
        /* p is a vector with n parameter values and fvec is a vector with m error values
         * for m data points. */
        Lorentzian parameters;
        parameters.pos      = p(0);
        parameters.height   = p(1);
        parameters.width    = p(2);
        parameters.offset   = p(3);

        /* For all values calculate the error between the data value and the predicted value. */
        for(int i = 0; i < values(); ++i) {
            fvec(i) = dataValues[i].y() - parameters.f(dataValues[i].x());
        }

        return 0;
    }

    // Compute the jacobian of the errors
    int df(const Eigen::VectorXd &p, Eigen::MatrixXd &fjac) const {
        /* p is a vector with n parameter values containing the current estimates while
         * fjac has dimensions m x n containing the jacobian matrix of errors. */

        /* Vary each parameter a little bit by a very small epsilon in plus and minus
         * direction and see what the difference is -> that is numerically good
         * approximation of the differential (no derivates needed). */
        double epsilon = 1e-5;

        for (int i = 0; i < p.size(); ++i) {
            /* Only vary the current parameter and just copy the rest from p */
            Eigen::VectorXd paraPlus(p);
            Eigen::VectorXd paraMinus(p);
            paraPlus(i) += epsilon;
            paraMinus(i) -= epsilon;

            /* Calculate the errors */
            Eigen::VectorXd fvecPlus(values());
            operator()(paraPlus, fvecPlus);
            Eigen::VectorXd fvecMinus(values());
            operator()(paraMinus, fvecMinus);

            /* Calculate the difference (and divide it by the full range, i.e. 2 * epsilon)
             * to get the slope = differential. */
            Eigen::VectorXd fvecDiff(values());
            fvecDiff = (fvecPlus - fvecMinus) / (2.0 * epsilon);

            /* Put it differential vector into the fjac matrix at the respective
             * position. */
            fjac.block(0, i, values(), 1) = fvecDiff;
        }

        return 0;
    }

    /* Number of data points/values */
    int values() const {
        return dataValues.size();
    }

    /* Number of parameters (also called "inputs"); here there are exactly four parameters. */
    int inputs() const {
        return 4;
    }
};

/* Fits Lorentzians into the data set and the sections provided. */
QVector<Lorentzian> calculateLorentzians(const QVector<QPointF> &points, const QVector<Section> &sections, qreal threshold);

/* Calculates one Lorentzian for one section provided */
Lorentzian calculateSingleLorentzian(const QVector<QPointF> &points, const Section &section, qreal threshold);

/* Calculates the R-square value for a given Lorentzian parameter set and real points */
qreal calculateLorentzianR2(const QVector<QPointF> &points, const Lorentzian &parameters);

/* Calculates the resolution for the given streams/peaks */
qreal calculateResolution(qreal pos1, qreal width1, qreal pos2, qreal width2);

}

#endif // TOPINOTOOL_H
