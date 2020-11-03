#ifndef ANGULAGRAMVIEW_H
#define ANGULAGRAMVIEW_H

#include <QChartView>
#include <QChart>
#include <QLineSeries>

#include "include/topinoabstractview.h"
#include "include/topinodocument.h"

class AngulagramView : public TopinoAbstractView {
        Q_OBJECT

    public:
        AngulagramView(QWidget *parent, TopinoDocument &doc);
        ~AngulagramView();

        void modelHasChanged() override;
        bool isToolSupported(const TopinoAbstractView::tools& value) const override;

        /* Edit functions to call. */
        void cut() override;
        void copy() override;
        void paste() override;
        void erase() override;

        void selectAll() override;
        void selectNone() override;
        void selectNext() override;

        /* Which edit functions are available to the user in the image view? */
        bool isEditFunctionSupported(const TopinoAbstractView::editfunc& value) const override;

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        QGraphicsScene *chartScene = nullptr;
        QtCharts::QChart *chart = nullptr;

};

#endif // ANGULAGRAMVIEW_H
