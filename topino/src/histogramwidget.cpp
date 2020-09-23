#include "include/histogramwidget.h"

HistogramWidget::HistogramWidget(QWidget *parent) : QWidget(parent) {
    /* Prepare the painting tools for drawing later */
    backgroundBrush = QBrush(QColor(42, 42, 42), Qt::SolidPattern);
    selectionBrush = QBrush(QColor(84, 84, 84), Qt::SolidPattern);
    barBrush = QBrush(QColor(125, 65, 65), Qt::SolidPattern);
    barBrushSelected = QBrush(QColor(255, 125, 125), Qt::SolidPattern);
    cursorSelection = QCursor(QPixmap(":/ui/cursors/hori_double_arrow.png"), 24, 24);

    /* Enable hover events */
    setMouseTracking(true);
}

HistogramWidget::~HistogramWidget() {

}

void HistogramWidget::mousePressEvent(QMouseEvent* event) {
    /* Clicked at one of the borders of the selection */
    if (event->buttons() & Qt::LeftButton) {
        if (inMinBorder(event->pos())) {
            partClicked = parts::minborder;
        } else if (inMaxBorder(event->pos())) {
            partClicked = parts::maxborder;
        }
    }

    /* Continue with processing */
    QWidget::mousePressEvent(event);
}

void HistogramWidget::mouseMoveEvent(QMouseEvent* event) {
    /* Save position and calculate the bar index under the mouse cursor */
    QPoint pos = event->pos();
    int barUnderMouse = (int)(pos.x() / (width() / (qreal)histogram.size()));

    /* Clamp the value */
    barUnderMouse = qMax(0, barUnderMouse);
    barUnderMouse = qMin(histogram.size()-1, barUnderMouse);

    /* Set mouse cursor */
    if (inMinBorder(pos) || inMaxBorder(pos)) {
        setCursor(QCursor(cursorSelection));
    } else {
        setCursor(QCursor(Qt::ArrowCursor));
    }

    /* Depending on which part clicked, the position of the points is updated */
    switch (partClicked) {
    case parts::minborder:
        if (barUnderMouse < maxSelValue) {
            minSelValue = barUnderMouse;

            /* Send notice that the data of this widget changed */
            emit valuesChanged(minSelValue, maxSelValue);
        }
        break;
    case parts::maxborder:
        if (barUnderMouse > minSelValue) {
            maxSelValue = barUnderMouse;

            /* Send notice that the data of this widget changed */
            emit valuesChanged(minSelValue, maxSelValue);
        }
        break;
    default:
        break;
    }

    /* Update the widget (drawing etc.) and continue with processing */
    update();
    QWidget::mouseMoveEvent(event);
}

void HistogramWidget::mouseReleaseEvent(QMouseEvent* event) {
    /* Release the part clicked */
    partClicked = parts::none;

    /* Process all release events */
    QWidget::mouseReleaseEvent(event);
}

void HistogramWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);

    /* Save width and height for later */
    int width = this->width();
    int height = this->height();

    /* Fill background */
    painter.fillRect(0, 0, width, height, backgroundBrush);

    /* Nothing to draw? Then leave! */
    if (histogram.isEmpty()) {
        return;
    }

    /* Calculate the bar width; between each bar and at the very front and back, there
     * is an additional space for the markers */
    qreal barWidth = width / (qreal)histogram.size();
    qreal barScale = height / (qreal)maxIntensityValue;

    /* Draw the selection rectangle (will be in the background of the histogram) */
    painter.fillRect(QRectF(barWidth * minSelValue, 0, barWidth * (maxSelValue - minSelValue + 1), height), selectionBrush);

    /* Draw each bar */
    painter.setBrush(barBrush);
    painter.setPen(Qt::NoPen);
    for (int i = 0; i < histogram.size(); ++i) {
        /* Set brush depending on inside or outside */
        if ((i >= minSelValue) && (i <= maxSelValue)) {
            painter.setBrush(barBrushSelected);
        } else {
            painter.setBrush(barBrush);
        }

        /* Draw bars */
        qreal barHeight = histogram[i] * barScale;
        //painter.fillRect(QRectF(barWidth * i, height - barHeight, barWidth, barHeight), Qt::SolidPattern);
        painter.drawRect(QRectF(barWidth * i, height - barHeight, barWidth, barHeight));
    }
}

bool HistogramWidget::inMinBorder(QPointF pt) const {
    qreal barWidth = width() / (qreal)histogram.size();
    return QRectF((barWidth * minSelValue) - 1, 0, barWidth + 2, height()).contains(pt);

}

bool HistogramWidget::inMaxBorder(QPointF pt) const {
    qreal barWidth = width() / (qreal)histogram.size();
    return QRectF((barWidth * maxSelValue) - 1, 0, barWidth + 2, height()).contains(pt);
}

int HistogramWidget::getMaxSelValue() const {
    return maxSelValue;
}

void HistogramWidget::setMaxSelValue(int value) {
    if (value > minSelValue) {
        maxSelValue = value;
        update();
    }
}

int HistogramWidget::getMinSelValue() const {
    return minSelValue;
}

void HistogramWidget::setMinSelValue(int value) {
    if (value < maxSelValue) {
        minSelValue = value;
        update();
    }
}

void HistogramWidget::mirrorMinMaxValue() {
    /* This mirrors the min and max values; useful for inverting the image and
     * wanting to invert the histogram selection window as well */
    int tempMinValue = minSelValue;
    minSelValue = histogram.size() - maxSelValue;
    maxSelValue = histogram.size() - tempMinValue;
    update();
}

QVector<int> HistogramWidget::getHistogram() const {
    return histogram;
}

void HistogramWidget::setHistogram(const QVector<int>& value) {
    /* Old histogram was empty? Then set min/max values to
     * the boundaries of the new value */
    if (histogram.isEmpty()) {
        minSelValue = 0;
        maxSelValue = value.size()-1;
    }

    /* Save new histogram */
    histogram = value;

    /* Find maximum value (so we save the iteration each drawing circle) */
    maxIntensityValue = 0;
    for (int i = 0; i < histogram.size(); ++i) {
        maxIntensityValue = qMax(maxIntensityValue, histogram[i]);
    }

    /* Reset min and max values if they are outside of the boundaries of
     * the new histogram */
    if (minSelValue >= histogram.size()) {
        minSelValue = 0;
    }

    if (maxSelValue >= histogram.size()) {
        maxSelValue = histogram.size()-1;
    }

    /* Redraw */
    update();
}
