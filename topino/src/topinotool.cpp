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

QList<QPointF> TopinoTools::imageSlopePoints(const QImage& image, const QPointF& center) {
    /* Check if the image is a gray image (8 bit) */
    if (image.format() != QImage::Format_Grayscale8) {
        return QList<QPointF>();
    }

    /* Image should be a square */
    if ((image.width() != image.height()) || (image.width() == 0)) {
        return QList<QPointF>();
    }

    /* Circle radius is half the image size */
    qreal radius = image.width() / 2.0;

    /* These are the 4 vectors and 8 directional points we want to check */
    QPointF vectors[4] = {QPointF(0.0, -1.0),     /* North/South */
                          QPointF(1.0, -1.0),     /* North east/South west */
                          QPointF(1.0, 0.0),      /* East/West */
                          QPointF(1.0, 1.0)
                         };     /* South east/North west */

    QPointF dirPts[8];
    for(int i = 0; i < 4; ++i) {
        dirPts[i*2 + 0] = center + radius * vectors[i];
        dirPts[i*2 + 1] = center - radius * vectors[i];
    }

    /* Array that will hold all the points we find */
    QList<QPointF> slopePoints;

    /* For each point, check the pixel data between center and this point */
    const uchar *pixels = image.bits();
    for(int i = 0; i < 8; ++i) {
        QLineF line(center, dirPts[i]);

        /* The t parameter (for the pointAt function of QLineF) goes from 0.0 (=p1) to
         * 1.0 (=p2). However, we need a (almost) pixel-wide step size. So, let's
         * calculate it! */
        qreal step = 1.0 / radius;

        qDebug("Line %d: step size: %.3f", i+1, step);

        QList<uchar> linePixels;
        for (qreal t = 0.0; t < 1.0; t += step) {
            QPoint pt = line.pointAt(t).toPoint();
            linePixels.append(pixels[pt.y() * image.height() + pt.x()]);
        }

        qDebug("Line %d: pixels: %d", i+1, linePixels.size());
        qreal position = pixelSlopePoint(linePixels);

        qDebug("Line %d: position: %.1f", i+1, position);
        QPointF newPt = line.pointAt(position);
        if ((position > 0.0) && image.rect().contains(newPt.toPoint()) && isPointInCircle(newPt, center, radius)) {
            slopePoints.append(newPt);
        }
    }

    return slopePoints;
}

qreal TopinoTools::pixelSlopePoint(const QList<uchar>& pixels) {
    if (pixels.size() == 0) {
        return 0.0;
    }

    /* First, let's generate an array with slope values. Add one
     * extra to make it even size with pixels array. */
    QList<int> slopes;

    for(int i = 0; i < (pixels.size() - 1); ++i) {
        qDebug("Pixels: %d %d", (int)pixels[i], (int)pixels[i+1]);
        slopes.append((int)pixels[i+1] - (int)pixels[i]);
    }
    slopes.append(0);

    /* Calculate the signal-to-noise ration (3 * S/N) for the
     * slopes to decide later which values to keep and which
     * not. */
    qreal noise = 3.0 * pixelNoise(slopes);

    /* Find the slope with the first slope that is above the
     * threshold from the END of the array! */
    int index = 0;
    while(slopes.size() > 0) {
        int slope = slopes.takeLast();

        qDebug("Checking slope %d and noise %.1f", qAbs(slope), noise);

        if (qAbs(slope) > noise)
            break;

        index++;
    }

    /* If index ran out then return simply 1.0 (point at the beginning) */
    if (index >= pixels.size())
        return 0.0;

    /* Index is now from the end, so do 1.0 - the fraction */
    return 1.0 - ((qreal)index / (qreal)pixels.size());
}

qreal TopinoTools::pixelNoise(const QList<int>& values) {
    /* There need to be at least two values in the list, otherwise we cannot
     * calculate the standard deviation */
    if (values.size() < 2) {
        return 0.0;
    }

    /* Calculate the sum and mean */
    qreal sum = 0.0;
    for (auto iter = values.begin(); iter != values.end(); ++iter) {
        sum += (*iter);
    }
    qreal mean = sum / (qreal)values.size();

    /* Calculate the denominator of the fraction */
    qreal dev_squared = 0.0;
    for (auto iter = values.begin(); iter != values.end(); ++iter) {
        dev_squared += qPow((*iter) - mean, 2.0);
    }
    /* Divide by N - 1 */
    dev_squared /= ((qreal)values.size() - 1.0);

    /* Return the standard deviation, i.e. the root */
    return qSqrt(dev_squared);
}

qreal TopinoTools::centerMaxDistance(const QPointF& center, const QList<QPointF>& points) {
    qreal maxdistance = 0.0;

    for(auto iter = points.begin(); iter != points.end(); ++iter) {
        qreal dist = qSqrt(qPow(center.x() - (*iter).x(), 2.0) + qPow(center.y() - (*iter).y(), 2.0));

        maxdistance = qMax(dist, maxdistance);
    }

    /* Return the max distance */
    return maxdistance;
}

void TopinoTools::calculateSmallestCircle(const QList<QPointF>& points, QPointF& center, qreal& radius) {
    /* Copy list and shuffle in random order */
    QList<QPointF> circlePoints = points;
    std::random_shuffle(circlePoints.begin(), circlePoints.end());

    /* Initialize center and radius */
    center = QPointF(0.0, 0.0);
    radius = -1.0;

    /* Progressively add points to circle or recompute circle */
    for (int p = 0; p < circlePoints.size(); ++p) {
        /* If the circle is either not initialized, yet, or the point is not in the circle, then
         * we need to add it. */
        if ((radius < 0.0) || !isPointInCircle(circlePoints[p], center, radius)) {
            /* Check all points to p+1 and adjust the circle based on these two points */
            for (int q = 0; q < (p+1); ++q) {
                if (!isPointInCircle(points[q], center, radius)) {
                    /* If no radius (i.e. circle) has been set yet, then simple calculcate
                     * the half distance between two points, use this as center and the
                     * distance to one of the points (is the same to both) as radius. */
                    if (radius == 0.0) {
                        center = QPointF((points[p].x() + points[q].x()) / 2.0, (points[p].y() + points[q].y()) / 2.0);
                        radius = qSqrt(qPow(points[p].x() - center.x(), 2.0) + qPow(points[p].y() - center.y(), 2.0));
                        /* Otherwise, add this point to the circle using p and q explicitly. */
                    } else {
                        calculateSmallestCircleTwoPoints(points, p, q, center, radius);
                    }
                }
            }
        }
    }
}

void TopinoTools::calculateSmallestCircleTwoPoints(const QList<QPointF>& points, int p, int q, QPointF& center, qreal& radius) {
    /* Indices should be in the list */
    if ((p < 0) || (q < 0) || (p >= points.size()) || (q >= points.size()))
        return;

    /* Create three circles in total. The first one is simply based on points p and q. */
    QPointF c_simple((points[p].x() + points[q].x()) / 2.0, (points[p].y() + points[q].y()) / 2.0);
    qreal r_simple = qSqrt(qPow(points[p].x() - c_simple.x(), 2.0) + qPow(points[p].y() - c_simple.y(), 2.0));

    QPointF c_left(0.0, 0.0), c_right(0.0, 0.0);
    qreal r_left = -1.0, r_right = -1.0;

    /* Check all points in the two-point circle */
    QPointF pq = points[q] - points[p];
    for (int r = 0; r < (p+1); ++r) {
        /* Point is already in the simple circle? Then ignore/continue! */
        if (isPointInCircle(points[r], c_simple, r_simple))
            continue;

        /* Form a circum circle and classify it to the left or right side/circle */
        qreal cross = crossProduct(pq, points[r] - points[p]);
        QPointF circum_center(0.0, 0.0);
        qreal circum_radius = -1.0;
        calculateCircumCircle(points[p], points[q], points[r], circum_center, circum_radius);

        if (circum_radius < 0.0)
            continue;

        if (cross > 0.0 && (r_left < 0.0 || crossProduct(pq, circum_center - points[p]) > crossProduct(pq, c_left - points[p]))) {
            c_left = circum_center;
            r_left = circum_radius;
        } else if (cross < 0.0 && (r_right < 0.0 || crossProduct(pq, circum_center - points[p]) < crossProduct(pq, c_right - points[p]))) {
            c_right = circum_center;
            r_right = circum_radius;
        }
    }

    /* Simply return the SMALLEST of both the left and right circle. If one of them is not valid, then
     * return the other. If both are invalid, return the simple circle. */
    qDebug("left-radius: %.1f right-radius: %.1f simple-radius: %.1f", r_left, r_right, r_simple);
    if ((r_left < 0.0) && (r_right < 0.0)) {
        center = c_simple;
        radius = r_simple;
    } else if (r_left < 0.0) {
        center = c_right;
        radius = r_right;
    } else if (r_right < 0.0) {
        center = c_left;
        radius = r_left;
    } else if (r_left <= r_right) {
        center = c_left;
        radius = r_left;
    } else {
        center = c_right;
        radius = r_right;
    }
}

void TopinoTools::calculateCircumCircle(const QPointF& a, const QPointF& b, const QPointF& c, QPointF& center, qreal& radius) {
    qDebug("calculateCircumCircle()");
    /* Simply based on the math given at https://en.wikipedia.org/wiki/Circumscribed_circle */
    qreal ox = (qMin(qMin(a.x(), b.x()), c.x()) + qMin(qMin(a.x(), b.x()), c.x())) / 2.0;
    qreal oy = (qMin(qMin(a.y(), b.y()), c.y()) + qMin(qMin(a.y(), b.y()), c.y())) / 2.0;
    qreal ax = a.x() - ox,  ay = a.y() - oy;
    qreal bx = b.x() - ox,  by = b.y() - oy;
    qreal cx = c.x() - ox,  cy = c.y() - oy;
    qreal d = (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by)) * 2.0;

    if (d == 0) {
        center = QPointF(0.0, 0.0);
        radius = -1.0;
        return;
    }

    qreal x = ((ax*ax + ay*ay) * (by - cy) + (bx*bx + by*by) * (cy - ay) + (cx*cx + cy*cy) * (ay - by)) / d;
    qreal y = ((ax*ax + ay*ay) * (cx - bx) + (bx*bx + by*by) * (ax - cx) + (cx*cx + cy*cy) * (bx - ax)) / d;

    /* This is the center point */
    center = QPointF(ox + x, oy + y);

    /* Distances to each point, take the maximum as radius */
    qreal dist_ca = qSqrt(qPow(a.x() - center.x(), 2.0) + qPow(a.y() - center.y(), 2.0));
    qreal dist_cb = qSqrt(qPow(b.x() - center.x(), 2.0) + qPow(b.y() - center.y(), 2.0));
    qreal dist_cc = qSqrt(qPow(c.x() - center.x(), 2.0) + qPow(c.y() - center.y(), 2.0));
    radius = qMax(qMax(dist_ca, dist_cb), dist_cc);
}

bool TopinoTools::isPointInCircle(const QPointF& point, const QPointF& center, const qreal radius) {
    qreal distance = qSqrt(qPow(point.x() - center.x(), 2.0) + qPow(point.y() - center.y(), 2.0));

    return distance <= radius * MULTIPLICATIVE_EPSILON;
}

qreal TopinoTools::crossProduct(const QPointF& p, const QPointF& q) {
    return p.x() * q.y() - p.y() * q.x();
}

