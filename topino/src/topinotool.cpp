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

QString TopinoTools::getDesaturationModeName(TopinoTools::desaturationModes mode) {
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

qreal TopinoTools::imageMoment(const QImage& grayImage, int p, int q) {
    /* Check if the image is a gray image (8 bit) */
    if (grayImage.format() != QImage::Format_Grayscale8) {
        return 0.0;
    }

    /* Calculate the matrix value by iterating over all pixels */
    qreal value = 0;

    const uchar *pixels = grayImage.bits();
    int width = grayImage.width();
    int height = grayImage.height();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            value += (qreal)pixels[y * width + x] * qPow(x, p) * qPow(y, q);
        }
    }

    /* Return matrix M_pq */
    return value;
}

QPointF TopinoTools::imageCentroid(const QImage& image) {
    /* Create a copy of the image and convert it to a gray image (8bit) */
    QImage grayImage = image.convertToFormat(QImage::Format_Grayscale8);

    /* Calculate the M_00 (area), M_01 (sum for y), and M_10 (sum for x)
     * matrix elements, which are needed to calculate the centroid. If the
     * area is zero, return the center of the image as centroid. */
    qreal area = imageMoment(grayImage, 0, 0);
    qreal sumy = imageMoment(grayImage, 0, 1);
    qreal sumx = imageMoment(grayImage, 1, 0);

    if (area == 0.0) {
        return QPointF(image.width() / 2.0, image.height() / 2.0);
    }

    /* Centroid is simply the division of the x and y matrix elements over
     * the area element. */
    return QPointF(sumx / area, sumy / area);
}

QImage TopinoTools::imageSumAreaTable(const QImage& image) {
    /* Create a copy of the image and convert it to a gray image (8bit). Also,
     * create an empty image that will hold the table data. */
    QImage grayImage = image.convertToFormat(QImage::Format_Grayscale8);
    QImage tableImage = QImage(grayImage.size(), QImage::Format_Grayscale8);

    /* Iterate over all pixels and calculate the summed area */
    const uchar *pixels = grayImage.bits();
    uchar *table = tableImage.bits();
    int width = grayImage.width();
    int height = grayImage.height();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            /* Important: check if values are over 255 and cap them if necessary! */
            int left = (x > 0) ? pixels[y * width + x - 1] : 0;
            int top = (y > 0) ? pixels[(y-1) * width + x] : 0;
            int topleft = (x > 0) && (y > 0) ? pixels[(y - 1) * width + x - 1] : 0;

            int value = pixels[y * width + x] + left + top + topleft;

            table[y * width + x] = (value > 255) ? 255 : value;
        }
    }

    /* Return the sum area table in form of an image */
    return tableImage;
}

uchar TopinoTools::imageMaxColorValue(const QImage& grayImage) {
    /* Check if the image is a gray image (8 bit) */
    if (grayImage.format() != QImage::Format_Grayscale8) {
        return 255;
    }

    /* Find max value */
    const uchar *pixels = grayImage.bits();
    int width = grayImage.width();
    int height = grayImage.height();

    uchar value = pixels[0];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            value = qMax(pixels[y * width + x], value);
        }
    }

    return value;
}

uchar TopinoTools::imageMinColorValue(const QImage& grayImage) {
    /* Check if the image is a gray image (8 bit) */
    if (grayImage.format() != QImage::Format_Grayscale8) {
        return 0;
    }

    /* Find min value */
    const uchar *pixels = grayImage.bits();
    int width = grayImage.width();
    int height = grayImage.height();

    uchar value = pixels[0];
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            value = qMin(pixels[y * width + x], value);
        }
    }

    return value;

}

void TopinoTools::imagePointsGrayValue(const QImage& grayImage, QList<QPointF>& list, uchar value) {
    /* Check if the image is a gray image (8 bit) */
    if (grayImage.format() != QImage::Format_Grayscale8) {
        return;
    }

    /* Find all pixels with the provided value */
    const uchar *pixels = grayImage.bits();
    int width = grayImage.width();
    int height = grayImage.height();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (pixels[y * width + x] == value) {
                list.append(QPointF(x, y));
            }
        }
    }
}

QPointF TopinoTools::getMassCenter(const QList<QPointF>& list) {
    /* List should have at least one point */
    if (list.size() == 0 ) {
        return QPointF(0.0, 0.0);
    }

    /* Mass center (or centroid) is simply all x and y values over
     * the number of points */
    QPointF center;

    for(auto iter = list.begin(); iter != list.end(); ++iter) {
        center += (*iter);
    }
    center /= list.size();

    return center;
}
