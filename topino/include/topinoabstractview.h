#ifndef TOPINOABSTRACTVIEW_H
#define TOPINOABSTRACTVIEW_H

#include <include/iobserver.h>

#include <QtMath>
#include <QGraphicsView>

#include "include/topinodocument.h"

class TopinoAbstractView : public QGraphicsView, public IObserver {
    Q_OBJECT

  public:
    TopinoAbstractView(QWidget* parent, TopinoDocument &doc);
    ~TopinoAbstractView();

  public:
    enum tools {
        selection = 0,
        rulerLine = 1,
        inletCircle = 2,
        toolCount = 3
    };

    /* Tool functions to get and select the current tool used by the view. */
    TopinoAbstractView::tools getCurrentTool() const;
    void setCurrentTool(const TopinoAbstractView::tools& value);

    /* This function will determine which tools are available to the user by selecting a particular view. */
    virtual bool isToolSupported(const TopinoAbstractView::tools& value) const = 0;

    /* Zoom functions for handling the zooming in a particular view. */
    double getZoomFactor() const ;
    void setZoomFactor(const double zoomTo);
    void zoomByFactor(const double factor);

  signals:
    /* Called by the every function that changes somehow the view(port) of the scene, e.g. by zooming. */
    void viewHasChanged();

  protected:
    /* Reference to the document to get/set data */
    TopinoDocument &document;

    /* The tool currently selected and used */
    TopinoAbstractView::tools currentTool = TopinoAbstractView::tools::selection;

};

#endif // TOPINOABSTRACTVIEW_H
