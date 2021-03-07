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
    //std::random_shuffle(circlePoints.begin(), circlePoints.end());

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


void TopinoTools::smoothByGaussianKernel(QVector<QPointF> &points, int size, qreal sigma) {
    /* First, we need to prepare the kernel weight array. */
    QVector<qreal> kernel;

    int midIndex = size / 2;
    for (int i = 0; i < size; ++i) {
        kernel.push_back(gaussianKernel(i - midIndex, sigma));
    }

    /* We process through each data point and generate a new value based on the
     * neighbouring data points and a Gaussian kernel. We start not directly at
     * the beginning of the array, but at the "midindex" (and end just before
     * the length - midindex) so that we have enough points to the left and
     * right to smooth. Ideally, the points to the left and right should be
     * background anyway. */
    QVector<QPointF> smoothendPoints;
    qreal sum = 0.0;
    for(int i = midIndex; i < (points.length() - midIndex); ++i) {
        sum = 0.0;

        /* Now smoothen the points around this middle point with the kernel
         * weights. */
        for(int k = 0; k < size; ++k) {
            sum += points[i + k - midIndex].y() * kernel[k];
        }

        /* Save the new value. */
        smoothendPoints.append(QPointF(points[i].x(), sum));
    }

    /* Return the smoothened points in the same array. */
    points = smoothendPoints;
}

qreal TopinoTools::gaussianKernel(qreal value, qreal sigma) {
    return 1.0 / sqrt(2.0 * M_PI * sigma * sigma) * qExp(- (value * value) / (2.0 * sigma * sigma));
}

void TopinoTools::getExtrema(const QVector<QPointF>& points, QVector<Extrema>& extrema) {
    /* Clear list to be sure that there is no remaining data in it */
    extrema.clear();

    /* Make sure there are at least 3 points in points; otherwise this does not make much
     * sense there. */
    if (points.length() < 3) {
        return;
    }

    /* The idea is here to just go through all points and see if there is a switch from ascending to
     * descending or vice versa. */
    TopinoTools::Extrema currentExtrema;
    TopinoTools::slopeDirection lastDirection = directionAscending;

    currentExtrema.index = 0;
    currentExtrema.pos = points[0];

    for(int i = 1; i < points.length(); ++i) {
        /* Delta is always between this point and the last one */
        qreal delta = points[i].y() - points[i-1].y();

        /* Value changed? Then update the index/value of the current extrema */
        if (delta != 0.0) {
            currentExtrema.index = i;
            currentExtrema.pos = points[i];
        }

        /* Asceding now, but last direction was descending -> Minima found! */
        if ((delta > 0) && (lastDirection == directionDescending)) {
            /* Calculate the average index if we are sitting on a flat point (to get the middle) */
            currentExtrema.index = (currentExtrema.index + i) / 2;
            currentExtrema.pos = points[i];
            currentExtrema.type = extremaMinimum;

            /* Save extrema */
            extrema.push_back(currentExtrema);
        }
        /* Descending now, but last direction was ascending -> Maxima found! */
        else if ((delta < 0) && (lastDirection == directionAscending)) {
            /* Calculate the average index if we are sitting on a flat point (to get the middle) */
            currentExtrema.index = (currentExtrema.index + i) / 2;
            currentExtrema.pos = points[i];
            currentExtrema.type = extremaMaximum;

            /* Save extrema */
            extrema.push_back(currentExtrema);
        }

        /* Set last direction */
        /* Ascending */
        if (delta > 0) {
            lastDirection = directionAscending;
            /* Descending */
        } else if (delta < 0) {
            lastDirection = directionDescending;
        }
    }
}

void TopinoTools::filterExtrema(QVector<TopinoTools::Extrema>& extrema, qreal threshold) {
    /* Create a new list with the filtered extrema */
    QVector<TopinoTools::Extrema> filteredExtrema;

    for(auto it = extrema.begin(); it != extrema.end(); ++it) {
        if ((*it).pos.y() > threshold) {
            filteredExtrema.push_back(*it);
        }
    }

    /* Copy filtered list to parameter */
    extrema = filteredExtrema;
}

int TopinoTools::countExtrema(const QVector<TopinoTools::Extrema>& extrema, TopinoTools::extremaType type) {
    int count = 0;

    for(auto it = extrema.begin(); it != extrema.end(); ++it) {
        if (it->type == type) {
            count++;
        }
    }

    return count;
}

QVector<TopinoTools::Extrema> TopinoTools::splitExtrema(const QVector<TopinoTools::Extrema>& extrema, TopinoTools::extremaType type) {
    QVector<TopinoTools::Extrema> splitted;

    for(auto it = extrema.begin(); it != extrema.end(); ++it) {
        if (it->type == type) {
            splitted.append(*it);
        }
    }

    return splitted;
}

QVector<TopinoTools::Section> TopinoTools::getSections(const QVector<QPointF>& points,
        const QVector<TopinoTools::Extrema> &extrema, qreal threshold) {
    QVector<TopinoTools::Section> sections;

    /* Split the extrema into minima and maxima */
    QVector<TopinoTools::Extrema> minima = splitExtrema(extrema, TopinoTools::extremaMinimum);
    QVector<TopinoTools::Extrema> maxima = splitExtrema(extrema, TopinoTools::extremaMaximum);

    if (minima.length() != (maxima.length() - 1)) {
        qDebug("Sections: number of minima (%d) and maxima (%d) wrong.", minima.length(), maxima.length());
        return sections;
    }

    /* Add the first and last pseudo minima to the list */
    for(int i = 0; i < points.length(); ++i) {
        if (points[i].y() > threshold) {
            TopinoTools::Extrema first;
            first.index = i;
            first.pos = points[i];
            first.type = TopinoTools::extremaMinimum;
            minima.insert(0, first);
            break;
        }
    }

    for(int i = 0; i < points.length(); ++i) {
        if (points[points.length() - i - 1].y() > threshold) {
            TopinoTools::Extrema last;
            last.index = points.length() - i - 1;
            last.pos = points[points.length() - i - 1];
            last.type = TopinoTools::extremaMinimum;
            minima.append(last);
            break;
        }
    }

    /* Now, we have n minima and n-1 maxima in the list. We simply go from minima i to minima i+1
     * and create the respective section i with maxima i. */
    for (int i = 0; i < (minima.length()-1); ++i) {
        TopinoTools::Section currentSection;

        currentSection.indexLeft = minima[i].index;
        currentSection.indexRight = minima[i+1].index;
        currentSection.indexMax = maxima[i].index;

        sections.append(currentSection);
    }

    return sections;
}


QVector<TopinoTools::Lorentzian> TopinoTools::calculateLorentzians(const QVector<QPointF>& points,
        const QVector<TopinoTools::Section>& sections, qreal threshold) {
    /* Create a vector for returning the fit Lorentzians */
    QVector<TopinoTools::Lorentzian> data;

    /* Fit each section */
    for(auto it = sections.begin(); it != sections.end(); ++it) {
        data.append(calculateSingleLorentzian(points, *it, threshold));
    }

    /* Return all the Lorentzians! */
    return data;
}

TopinoTools::Lorentzian TopinoTools::calculateSingleLorentzian(const QVector<QPointF>& points,
        const TopinoTools::Section& section, qreal threshold) {
    /* Prepare data and fill with a good guess of parameters */
    TopinoTools::Lorentzian data;

    QPointF max = points[section.indexMax];
    QPointF left = points[section.indexLeft];
    QPointF right = points[section.indexRight];

    data.height = max.y() - threshold;
    data.offset = threshold;
    data.pos = max.x();
    data.width = (right.x() - left.x()) / 2.0;

    /* Transfer the parameters into an vector. The order is pos (index=0),
     * height (index=1), width (index=2), and offset (index=3). */
    Eigen::VectorXd p(4);
    p(0) = data.pos;
    p(1) = data.height;
    p(2) = data.width;
    p(3) = data.offset;

    /* Create a functor for the solving algorithm */
    QVector<QPointF> dataPoints = points.mid(section.indexLeft, section.indexRight - section.indexLeft);
    LorentzianFunctor functor;
    functor.dataValues = dataPoints;

    Eigen::LevenbergMarquardt<LorentzianFunctor> lm(functor);
    int status = lm.minimize(p);

    /* Copy the data */
    qDebug("Minimization return with %d", status);
    data.pos    = p(0);
    data.height = p(1);
    data.width  = p(2);
    data.offset = p(3);

    /* Additionally, calculate the R2 value to get a guess of the fitting quality (also used for linearity). */
    data.rsquare = calculateLorentzianR2(dataPoints, data);

    return data;
}


qreal TopinoTools::calculateLorentzianR2(const QVector<QPointF>& points, const TopinoTools::Lorentzian& parameters) {
    /* No points given? */
    if (points.length() == 0) {
        return 0.0;
    }

    /* Mean value of the points y-values */
    qreal mean = std::accumulate(points.begin(), points.end(), 0.0,
    [](const qreal &a, const QPointF& b) {
        return a + b.y();
    } ) / points.length();

    qreal ss_res = 0.0;
    qreal ss_tot = 0.0;
    for(auto it = points.begin(); it != points.end(); ++it) {
        ss_res += qPow(it->y() - parameters.f(it->x()), 2.0);
        ss_tot += qPow(it->y() - mean, 2.0);
    }

    /* If ss_tot == 0 that means all points y-values are zero */
    if (ss_tot == 0.0) {
        return 0.0;
    }

    /* Calculate the R-square value */
    return 1.0 - (ss_res / ss_tot);
}

qreal TopinoTools::calculateResolution(qreal pos1, qreal width1, qreal pos2, qreal width2) {
    /* Uses the definition with 1/2 and FWHM. See the Angulagram paper at
     * https://pubs.acs.org/doi/10.1021/acs.analchem.8b02186 (Equation 7) */
    return 2.0 * qAbs(pos2 - pos1) / (width1 + width2);
}
