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

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        QGraphicsScene *chartScene = nullptr;
        QtCharts::QChart *chart = nullptr;

};

#endif // ANGULAGRAMVIEW_H
