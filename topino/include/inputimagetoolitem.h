#ifndef INPUTIMAGETOOLITEM_H
#define INPUTIMAGETOOLITEM_H

#include <QtMath>
#include <QCursor>
#include <QPainter>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsPixmapItem>

#include "include/topinographicsitem.h"

class InputImageToolItem : public TopinoGraphicsItem {
  public:
    InputImageToolItem(int newitemid, QGraphicsItem *parent = nullptr);
    ~InputImageToolItem();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;
    QPainterPath shape() const override;

    itemtype getItemType() const override;
    void updateScale() override;

    QPixmap getPixmap() const;
    void setPixmap(const QPixmap value);

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

    /* These methods create data representations of the item in various
     * formats (and allow converting it back, too!) */
    QString toString() const override;
    void fromString(const QString &value) override;

  private:
    /* Individual parts of the image item to keep track of what the user is interacting with
     * at the moment. */
    enum parts {
        none = 0,
        image = 1
    };
    parts partClicked = parts::none;

    /* Image data as pixmap */
    QPixmap pixmap;

    /* Rectangle for the whole tool including the border around the image */
    QRectF rect;

    void calculateRect();

    /* Tools for drawing */
    QBrush emptybox;
    QPen borderpen;
};

#endif // INPUTIMAGETOOLITEM_H
