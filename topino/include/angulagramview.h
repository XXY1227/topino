#ifndef ANGULAGRAMVIEW_H
#define ANGULAGRAMVIEW_H

#include <QAreaSeries>
#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>

#include "include/topinoabstractview.h"
#include "include/topinodocument.h"

class AngulagramView : public TopinoAbstractView {
        Q_OBJECT

    public:
        AngulagramView(QWidget *parent, TopinoDocument &doc);
        ~AngulagramView();

        void modelHasChanged() override;
        bool isToolSupported(const TopinoAbstractView::tools& value) const override;

        /* Show this view */
        void showView();

        /* Edit functions to call. */
        void cut(QClipboard *clipboard) override;
        void copy(QClipboard *clipboard) override;
        void paste(QClipboard *clipboard) override;
        void erase() override;

        void selectAll() override;
        void selectNone() override;
        void selectNext() override;

        /* Which edit functions are available to the user in the image view? */
        bool isEditFunctionSupported(const TopinoAbstractView::editfunc& value) const override;

        /* Getter/Setting functions */
        qreal getScalingFactor() const;
        void setScalingFactor(const qreal& value);

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        /* Scene and chart items */
        QGraphicsScene *chartScene = nullptr;
        QtCharts::QChart *chart = nullptr;

        /* This is the scaling factor for the data on the y-axis. All data in the document/data
         * object is scaled by this factor before it is added to the chart. */
        qreal scalingFactor;

        /* Re-creates the axes of the chart */
        void createAxes();

        /* Create the diffent series for the chart */
        void createRawDataSeries();

};

#endif // ANGULAGRAMVIEW_H
