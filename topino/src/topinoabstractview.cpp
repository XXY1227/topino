#include "include/topinoabstractview.h"

TopinoAbstractView::TopinoAbstractView(QWidget* parent, TopinoDocument& doc) : QGraphicsView(parent), document(doc) {
}

TopinoAbstractView::~TopinoAbstractView() {
}

TopinoAbstractView::tools TopinoAbstractView::getCurrentTool() const {
    return currentTool;
}

void TopinoAbstractView::setCurrentTool(const TopinoAbstractView::tools& value) {
    /* Check if the tool is defined and supported by this view. */
    if ((value < tools::toolCount) && isToolSupported(value)) {
        currentTool = value;
    }
}

double TopinoAbstractView::getZoomFactor() const {
    /* Use determinant of the transformation matrix as the zoom factor (determinant = scaling of the _area_) */
    return transform().det();
}

void TopinoAbstractView::setZoomFactor(const double zoomTo) {
    /* Zoom factor should be between 3.125% and 6400%. Due to double innaccuracies the check is actually less
     * than 6500% (65.0) and greater than 3% (0.03). */
    if ((zoomTo < 0.03) || (zoomTo > 65.0))
        return;

    /* New zoom factor over current zoom factor is the factor we need to scale the view; since we are using
     * the area as the reference, the actual scale factor is the square root of the ratio! */
    double scaleFactor = sqrt(zoomTo / getZoomFactor());
    scale(scaleFactor, scaleFactor);

    /* Tell everyone that the view changed */
    emit viewHasChanged();
}

void TopinoAbstractView::zoomByFactor(const double factor) {
    /* Technically the scale function allows simply to forward the factor; however, it scales by sides while
     * factor is the scaling of the area; so, we just forward the factor to setZoomFactor, which also checks
     * for the right range. */
    setZoomFactor(getZoomFactor() * factor);
}
