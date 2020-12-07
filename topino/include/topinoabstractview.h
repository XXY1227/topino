#ifndef TOPINOABSTRACTVIEW_H
#define TOPINOABSTRACTVIEW_H

#include <include/iobserver.h>

#include <QClipboard>
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

    enum editfunc {
        editCut = 0,
        editCopy = 1,
        editPaste = 2,
        editErase = 3,
        editSelectAll = 4,
        editSelectNone = 5,
        editSelectNext = 6,
        editfuncCOUNT = 7
    };

    /* Tool functions to get and select the current tool used by the view. */
    TopinoAbstractView::tools getCurrentTool() const;
    void setCurrentTool(const TopinoAbstractView::tools& value);

    /* This function will determine which tools are available to the user by selecting a particular view. */
    virtual bool isToolSupported(const TopinoAbstractView::tools& value) const = 0;

    /* View is about to show */
    virtual void showView() = 0;

    /* Virtual edit functions to call */
    virtual void cut(QClipboard *clipboard) = 0;
    virtual void copy(QClipboard *clipboard) = 0;
    virtual void paste(QClipboard *clipboard) = 0;
    virtual void erase() = 0;

    virtual void selectAll() = 0;
    virtual void selectNone() = 0;
    virtual void selectNext() = 0;

    /* This function will determine which edit functions are available to the user by selecting a particular view */
    virtual bool isEditFunctionSupported(const TopinoAbstractView::editfunc& value) const = 0;

    /* Zoom functions for handling the zooming in a particular view. */
    double getZoomFactor() const ;
    void setZoomFactor(const double zoomTo);
    void zoomByFactor(const double factor);

    /* Resets the view */
    virtual void resetView() = 0;

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
